// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include "CWindowEnumer.h"
	
CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_bLayoutInited = FALSE;
	m_Captured = false;
}

CMainDlg::~CMainDlg()
{
	
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);
	return 0;
}

BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	m_bLayoutInited = TRUE;
	ShowWindow(SW_HIDE);

	m_iHotKeyId = GlobalAddAtom(_T("LaoYangJieTuHotKey")) - 0xC000;
	if (!RegisterHotKey(m_hWnd, m_iHotKeyId, MOD_ALT| MOD_CONTROL, 'Y'))
	{
		SMessageBox(m_hWnd, _T("注册热键失败"), _T("Snapshot"), MB_OK);
		m_iHotKeyId = 0xFFFF;
	}
	OnStart();
	return 0;
}
void CMainDlg::OnDestroy()
{
	if(m_iHotKeyId!= 0xFFFF)
		UnregisterHotKey(m_hWnd, m_iHotKeyId);
	SHostWnd::OnDestroy();
}
//TODO:消息映射
void CMainDlg::OnClose()
{
	DestroyWindow();
}

void CMainDlg::OnStart()
{
	if (!m_Captured)
	{
		m_Captured = true;
		CSnapshotDlg snapshotDlg;
		CWindowEnumer::EnumAllTopWindow();
		snapshotDlg.DoModal();
		m_Captured = false;
	}
}

void CMainDlg::OnHotKey(int nHotKeyID, UINT uModifiers, UINT uVirtKey)
{
	if (nHotKeyID != m_iHotKeyId)
	{
		SetMsgHandled(FALSE);
		return;
	}
	OnStart();
}

LRESULT CMainDlg::StartCap(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	OnStart();
	return S_OK;
}
