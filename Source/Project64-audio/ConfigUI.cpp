/****************************************************************************
*                                                                           *
* Project64-audio - A Nintendo 64 audio plugin.                             *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2017 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#include "ConfigUI.h"
#pragma warning(push)
#pragma warning(disable : 4091) // warning C4091: 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared
#pragma warning(disable : 4302) // warning C4302: 'type cast': truncation from 'LPCTSTR' to 'WORD'
#pragma warning(disable : 4458) // warning C4458: declaration of 'dwCommonButtons' hides class member
#pragma warning(disable : 4838) // warning C4838: conversion from 'int' to 'UINT' requires a narrowing conversion
#pragma warning(disable : 4996) // warning C4996: 'GetVersionExA': was declared deprecated
#pragma warning(disable : 4302) // warning C4302: 'type cast': truncation from 'LPCTSTR' to 'WORD'
#define _ATL_DISABLE_NOTHROW_NEW
#include <atlbase.h>
#include <wtl/atlapp.h>
#include <wtl/atldlgs.h>
#include <wtl/atlctrls.h>
#include <wtl/atlcrack.h>
#pragma warning(pop)
#include <Settings/Settings.h>
#include "trace.h"
#include "AudioSettings.h"
#include "SettingsID.h"
#include "resource.h"

void SetComboBoxIndex(CComboBox & cmb, uint32_t data)
{
    cmb.SetCurSel(0);
    for (DWORD i = 0, n = cmb.GetCount(); i < n; i++)
    {
        if (cmb.GetItemData(i) == data)
        {
            cmb.SetCurSel(i);
            break;
        }
    }
}

class CGeneralSettings :
    public CPropertyPageImpl<CGeneralSettings>
{
public:
    enum { IDD = IDD_CONFIG };

    BEGIN_MSG_MAP(CDebugSettings)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER_EX(IDC_MUTE, ItemChanged)
        NOTIFY_HANDLER_EX(IDC_VOLUME, NM_RELEASEDCAPTURE, ItemChangedNotify);
    CHAIN_MSG_MAP(CPropertyPageImpl<CGeneralSettings>)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        m_btnMute.Attach(GetDlgItem(IDC_MUTE));
        m_btnMute.SetCheck(!g_settings->AudioEnabled() ? BST_CHECKED : BST_UNCHECKED);

        m_Volume.Attach(GetDlgItem(IDC_VOLUME));
        m_Volume.SetPos(100 - g_settings->GetVolume());
        m_Volume.SetTicFreq(20);
        m_Volume.SetRangeMin(0);
        m_Volume.SetRangeMax(100);
        return TRUE;
    }

    bool OnApply()
    {
        g_settings->SetAudioEnabled(m_btnMute.GetCheck() != BST_CHECKED);
        g_settings->SetVolume(100 - m_Volume.GetPos());
        FlushSettings();
        return true;
    }

private:
    LRESULT	ItemChangedNotify(NMHDR* /*pNMHDR*/)
    {
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
        return 0;
    }

    void ItemChanged(UINT /*Code*/, int /*id*/, HWND /*ctl*/)
    {
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
    }

    CTrackBarCtrl m_Volume;
    CButton m_btnMute;
};

class CLogSettings :
    public CPropertyPageImpl<CLogSettings>
{
public:
    enum { IDD = IDD_LOG_SETTINGS };

    BEGIN_MSG_MAP(CDebugSettings)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER_EX(IDC_CMB_TRACE_THREAD, CBN_SELCHANGE, ItemChanged)
        COMMAND_HANDLER_EX(IDC_CMB_TRACE_PATH, CBN_SELCHANGE, ItemChanged)
        COMMAND_HANDLER_EX(IDC_CMB_TRACE_INIT_SHUTDOWN, CBN_SELCHANGE, ItemChanged)
        COMMAND_HANDLER_EX(IDC_CMB_TRACE_INTERFACE, CBN_SELCHANGE, ItemChanged)
        CHAIN_MSG_MAP(CPropertyPageImpl<CLogSettings>)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        m_cmbTraceThread.Attach(GetDlgItem(IDC_CMB_TRACE_THREAD));
        m_cmbTracePath.Attach(GetDlgItem(IDC_CMB_TRACE_PATH));
        m_cmbTraceInitShutdown.Attach(GetDlgItem(IDC_CMB_TRACE_INIT_SHUTDOWN));
        m_cmbTraceInterface.Attach(GetDlgItem(IDC_CMB_TRACE_INTERFACE));
        m_cmbTraceDriver.Attach(GetDlgItem(IDC_CMB_TRACE_AUDIO_DRIVER));

        struct {
            CComboBox & cmb;
            uint16_t SettingId;
        } TraceCMB[] =
        {
            { m_cmbTraceThread, Set_Logging_Thread },
            { m_cmbTracePath, Set_Logging_Path },
            { m_cmbTraceInitShutdown, Set_Logging_InitShutdown },
            { m_cmbTraceInterface, Set_Logging_Interface },
            { m_cmbTraceDriver, Set_Logging_Driver },
        };

        for (size_t i = 0, n = sizeof(TraceCMB) / sizeof(TraceCMB[0]); i < n; i++)
        {
            TraceCMB[i].cmb.SetItemData(TraceCMB[i].cmb.AddString("Error"), TraceError);
            TraceCMB[i].cmb.SetItemData(TraceCMB[i].cmb.AddString("Warning"), TraceWarning);
            TraceCMB[i].cmb.SetItemData(TraceCMB[i].cmb.AddString("Notice"), TraceNotice);
            TraceCMB[i].cmb.SetItemData(TraceCMB[i].cmb.AddString("Info"), TraceInfo);
            TraceCMB[i].cmb.SetItemData(TraceCMB[i].cmb.AddString("Debug"), TraceDebug);
            TraceCMB[i].cmb.SetItemData(TraceCMB[i].cmb.AddString("Verbose"), TraceVerbose);
            SetComboBoxIndex(TraceCMB[i].cmb, (uint32_t)GetSetting(TraceCMB[i].SettingId));
        }
        return TRUE;
    }

    bool OnApply()
    {
        struct {
            CComboBox & cmb;
            uint16_t SettingId;
        } TraceCMB[] =
        {
            { m_cmbTraceThread, Set_Logging_Thread },
            { m_cmbTracePath, Set_Logging_Path },
            { m_cmbTraceInitShutdown, Set_Logging_InitShutdown },
            { m_cmbTraceInterface, Set_Logging_Interface },
            { m_cmbTraceDriver, Set_Logging_Driver },
        };
        for (size_t i = 0, n = sizeof(TraceCMB) / sizeof(TraceCMB[0]); i < n; i++)
        {
            SetSetting(TraceCMB[i].SettingId, TraceCMB[i].cmb.GetItemData(TraceCMB[i].cmb.GetCurSel()));
        }
        FlushSettings();
        return true;
    }

private:
    void ItemChanged(UINT /*Code*/, int /*id*/, HWND /*ctl*/)
    {
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
    }

    CComboBox m_cmbTraceThread;
    CComboBox m_cmbTracePath;
    CComboBox m_cmbTraceInitShutdown;
    CComboBox m_cmbTraceInterface;
    CComboBox m_cmbTraceDriver;
};

class CAudioUI : public CPropertySheetImpl < CAudioUI >
{
public:
    CAudioUI();
    ~CAudioUI();

    BEGIN_MSG_MAP(CAudioUI)
        CHAIN_MSG_MAP(CPropertySheetImpl<CAudioUI>)
    END_MSG_MAP()

private:
    CGeneralSettings * m_pgGeneralSettings;
    CLogSettings * m_pgLogSettings;
};

CAudioUI::CAudioUI() :
    m_pgGeneralSettings(new CGeneralSettings),
    m_pgLogSettings(new CLogSettings)
{
    AddPage(&m_pgGeneralSettings->m_psp);
    if (g_settings->debugger_enabled())
    {
        AddPage(&m_pgLogSettings->m_psp);
    }
}

CAudioUI::~CAudioUI()
{
    delete m_pgLogSettings;
    delete m_pgGeneralSettings;
}

void ConfigAudio(void * hParent)
{
    CAudioUI().DoModal((HWND)hParent);
}