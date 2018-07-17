#include "stdafx.h"
#include "CEdit9527.h"
#include "SSnapshotCtrl.h"

namespace SOUI
{
	CEdit9527::CEdit9527() :m_bDraging(FALSE), m_iHei(0),m_iWid(0)
	{
	}

	CEdit9527::~CEdit9527()
	{
	}
	void CEdit9527::PaintToDC(HDC hdc)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		RECTL rcL = { rcClient.left,rcClient.top,rcClient.right,rcClient.bottom };
		int nOldMode = ::SetGraphicsMode(hdc, GM_COMPATIBLE);
		m_pTxtHost->GetTextService()->TxDraw(
			DVASPECT_CONTENT,          // Draw Aspect
			/*-1*/0,                        // Lindex
			NULL,                    // Info for drawing optimazation
			NULL,                    // target device information
			hdc,            // Draw device HDC
			NULL,                        // Target device HDC
			&rcL,            // Bounding client rectangle
			NULL,             // Clipping rectangle for metafiles
			&rcClient,        // Update rectangle
			NULL,                        // Call back function
			NULL,                    // Call back parameter
			TXTVIEW_ACTIVE);
		::SetGraphicsMode(hdc, nOldMode);
	}

	LRESULT CEdit9527::OnCreate(LPVOID)
	{
		LRESULT bRet = __super::OnCreate(NULL);
		if (bRet == 0)
		{
			SSendMessage(EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE);//ENM_CHANGE
			GetEventSet()->subscribeEvent(EVT_RE_NOTIFY, Subscriber(&CEdit9527::OnEditNotify, this));
			
			GetEventSet()->subscribeEvent(EVT_KILLFOCUS, Subscriber(&CEdit9527::OnKillFocus, this));

		}
		return bRet;
	}

	bool CEdit9527::OnKillFocus(EventArgs * e)
	{
		GetEventSet()->setMutedState(true);
		__super::OnKillFocus(m_swnd);
		if (GetWindowText().IsEmpty())
		{
#ifndef _DEBUG
			GetParent()->DestroyChild(this);
#endif // !_DEBUG
		}
		GetEventSet()->setMutedState(false);
		return true;
	}

	bool CEdit9527::OnEditNotify(EventArgs * e)
	{
		EventRENotify *pEvtNotify = sobj_cast<EventRENotify>(e);
		switch (pEvtNotify->iNotify)
		{
		case EN_CHANGE:;//ENM_REQUESTRESIZE时将不会更新滚动条信息
			break;
		case EN_REQUESTRESIZE:
			REQRESIZE * prqs = (REQRESIZE *)(pEvtNotify->pv);
			m_iHei = prqs->rc.bottom - prqs->rc.top;
			m_iWid = prqs->rc.right - prqs->rc.left;
			UpdataSize2();
			break;
		}
		return true;
	}

	BOOL CEdit9527::OnSetCursor(const CPoint & pt)
	{
		if (!__super::OnSetCursor(pt))
		{
			SetCursor(GETRESPROVIDER->LoadCursor(IDC_SIZEALL));
		}
		return TRUE;
	}

	void CEdit9527::OnNcLButtonDown(UINT nHitTest, CPoint point)
	{
		__super::OnNcLButtonDown(nHitTest, point);
		SetCapture();
		m_bDraging = TRUE;
		m_ptClick = point;
		BringWindowToTop();
	}

	void CEdit9527::OnNcMouseMove(UINT nHitTest, CPoint point)
	{
		if (m_bDraging)
		{
			CRect rcWnd = GetWindowRect();
			CPoint pt = rcWnd.TopLeft() + (point - m_ptClick);
			((SSnapshotCtrl*)GetParent())->GetEtMovePos(pt, rcWnd.Width(),rcWnd.Height());
			rcWnd.MoveToXY(pt);
			Move(rcWnd);
			m_ptClick = point;
			GetParent()->Invalidate();
		}
	}

	void CEdit9527::UpdataSize2()
	{
		CRect rcWnd = GetWindowRect();
		//增大
		if (rcWnd.Width() < m_iWid + 10)
		{
			int max_wid = ((SSnapshotCtrl*)GetParent())->GetEtMaxWid(rcWnd);
			int iMax;
			SStringT width;
			iMax = max(m_iWid + 10, 60);
			iMax = min(max_wid, iMax);
			width.Format(L"%d", iMax);
			SetAttribute(L"width", width);
			if (iMax == max_wid)
				SSendMessage(EM_SETTARGETDEVICE, 0, 0);
		}
		else
		{
			SStringT width;
			int iMax;
			iMax = max(m_iWid + 10, 60);
			width.Format(L"%d", iMax);
			SetAttribute(L"width", width);
			SSendMessage(EM_SETTARGETDEVICE, 0,1);
		}

		if (rcWnd.Height() < m_iHei + 10)
		{
			int max_wid = ((SSnapshotCtrl*)GetParent())->GetEtMaxHei(rcWnd);
			int iMax;
			SStringT height;
			iMax = max(m_iHei + 10, 30);
			iMax = min(max_wid, iMax);
			height.Format(L"%d", iMax);
			SetAttribute(L"height", height);			
		}
		else
		{
			SStringT height;
			int iMax;			
			iMax = max(m_iHei + 10, 30);
			height.Format(L"%d", iMax);
			SetAttribute(L"height", height);
		}
	}

	void CEdit9527::UpdataSize()
	{		
		if (HasScrollBar(FALSE))//if(false)
		{
			int iMin, iMax;
			GetScrollRange(FALSE, &iMin, &iMax);
			SStringT width;
			iMax = max(iMax + 16, 60);
			int max_wid = ((SSnapshotCtrl*)GetParent())->GetEtMaxWid(GetWindowRect());
			iMax = min(max_wid, iMax);
			width.Format(L"%d", iMax);
			SetAttribute(L"width", width);
			if (iMax == max_wid)
				SSendMessage(EM_SETTARGETDEVICE, 0, 0);
		}
		else
		{
			SStringT width;
			width.Format(L"%d", m_iWid+20);
			//SetAttribute(L"width", width);
		}
		if (HasScrollBar(TRUE))
		{

			int iMin, iMax;
			GetScrollRange(TRUE, &iMin, &iMax);
			SStringT height;
			iMax = max(iMax + 16, 30);
			int max_hei = ((SSnapshotCtrl*)GetParent())->GetEtMaxHei(GetWindowRect());
			iMax = min(max_hei, iMax);
			height.Format(L"%d", iMax);			
			SetAttribute(L"height", height);
		}
		else
		{
			SStringT height;
			height.Format(L"%d", m_iHei+10);
			//SetAttribute(L"height", height);
		}
	}

	void CEdit9527::OnNcLButtonUp(UINT nHitTest, CPoint point)
	{
		m_bDraging = FALSE;
		ReleaseCapture();
		CRect parentRc = GetParent()->GetClientRect();
		CRect rcWnd = GetWindowRect();
		CPoint relpos = rcWnd.TopLeft() - parentRc.TopLeft();
		SStringT pos;
		pos.Format(L"%d,%d", relpos.x, relpos.y);
		SetAttribute(L"pos", pos);
		SSendMessage(EM_SETTARGETDEVICE, 0, 1);
		Move(NULL);
		UpdataSize2();
	}

	void CEdit9527::OnNcPaint(IRenderTarget * pRT)
	{
		if (!IsVisible(TRUE)) return;
		if (!m_style.GetMargin().IsRectNull() && IsFocused())
		{
			CAutoRefPtr<IPen> pen, oldpen;
			pRT->CreatePen(PS_DASHDOT, RGBA(0, 0, 0, 255), 1, &pen);
			pRT->SelectObject(pen, (IRenderObj**)&oldpen);
			CRect rcWindow = GetWindowRect();
			//CRect rcClient = GetClientRect();
			rcWindow.InflateRect(0, 0, 1, 1);
			pRT->DrawRectangle(rcWindow);
			pRT->SelectObject(oldpen, NULL);
		}
	}

	BOOL CEdit9527::OnEraseBkgnd(IRenderTarget * pRT)
	{
		return TRUE;
	}

	void CEdit9527::OnMouseHover(WPARAM wParam, CPoint ptPos)
	{
		SetFocus();
	}

	void CEdit9527::OnMouseLeave()
	{
		if (!m_bDraging)
			KillFocus();
	}
}