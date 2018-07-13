#include "StdAfx.h"
#include "SSnapshotCtrl.h"

SSnapshotCtrl::SSnapshotCtrl(void)
:m_pImgMask(NULL)
,m_pBrushMask(NULL)
{
	m_bSelected = false;
	m_bSelectOperate = false;
	m_bDrawOperate = false;

	m_hCurSelect = LoadCursor(NULL, IDC_SIZEALL);
	m_hCurTopRight = LoadCursor(NULL, IDC_SIZENESW);
	m_hCurBottomRight = LoadCursor(NULL, IDC_SIZENWSE);
	m_hCurTop = LoadCursor(NULL, IDC_SIZENS);
	m_hCurBottom = LoadCursor(NULL, IDC_SIZENS);
	m_hCurLeft = LoadCursor(NULL, IDC_SIZEWE);
	m_hCurRight = LoadCursor(NULL, IDC_SIZEWE);
	m_hCurTopLeft = LoadCursor(NULL, IDC_SIZENWSE);
	m_hCurBottomLeft = LoadCursor(NULL, IDC_SIZENESW);
	m_hCurHand = LoadCursor(NULL, IDC_HAND);
	m_hCurArrow = LoadCursor(NULL, IDC_ARROW);

	m_crBorder = RGBA(22, 119, 189, 255);
	m_eDraging = EcPosType::Null;

	m_evtSet.addEvent(EVENTID(EventCapturing));
	m_evtSet.addEvent(EVENTID(EventRectMoving));
	m_evtSet.addEvent(EVENTID(EventRectCaptured));
	m_evtSet.addEvent(EVENTID(EventRectDbClk));

	m_crPen = RGBA(0,0,0,255);
	m_nPenSize = 1;

	if (!m_pImgMask)
		m_pImgMask = new Image(L"SC_MASK.png");
	if (!m_pBrushMask)
		m_pBrushMask = new Image(L"mask.png");
}

SSnapshotCtrl::~SSnapshotCtrl(void)
{
	//if (m_pBitmap)
	//	m_pBitmap->DeleteObject();

	if (m_pImgMask)
	{
		delete m_pImgMask;
		m_pImgMask = NULL;
	}

	if (m_pBrushMask)
	{
		delete m_pBrushMask;
		m_pBrushMask = NULL;
	}

	for (int i = 0; i < m_vecBitmap.size(); i++)
	{
		m_vecBitmap[i]->DeleteObject();
	}
}

void SSnapshotCtrl::OnPaint(IRenderTarget *pRT)
{
	HDC hDC = pRT->GetDC();
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(hDC);
	dcCompatible.SelectBitmap(*m_pBitmap);

	SOUI::CRect rect(0,0, m_nScreenX, m_nScreenY);
	BitBlt(hDC, 0,0, rect.Width(), rect.Height(), dcCompatible, 0,0, SRCCOPY);

	Graphics graph(hDC);
	RectF rcDrawRect;
	rcDrawRect.X = 0;
	rcDrawRect.Y = 0;
	rcDrawRect.Width = (REAL)rect.Width();
	rcDrawRect.Height = (REAL)rect.Height();
	if (m_rcCapture.IsRectEmpty())
	{
		graph.DrawImage(m_pImgMask, rcDrawRect, 0,0, 2, 2, UnitPixel);
	}
	else
	{
		RectF rcLeft, rcTop, rcRight, rcBottom;
		CalcGrayAreaRect(m_rcCapture, rcLeft, rcTop, rcRight, rcBottom);
		SOUI::CRect rcMask(0,0,10,10);
		if (!rcLeft.IsEmptyArea())
			graph.DrawImage(m_pImgMask, rcLeft, 0,0, 2, 2, UnitPixel);
		if (!rcTop.IsEmptyArea())
			graph.DrawImage(m_pImgMask, rcTop, 0,0, 2, 2, UnitPixel);
		if (!rcRight.IsEmptyArea())
			graph.DrawImage(m_pImgMask, rcRight, 0,0, 2, 2, UnitPixel);
		if (!rcBottom.IsEmptyArea())
			graph.DrawImage(m_pImgMask, rcBottom, 0,0, 2, 2, UnitPixel);
	}

	if (!m_rcCapture.IsRectEmpty())
	{
		SOUI::CRect rcLine(m_rcCapture);
		rcLine.InflateRect(1, 1);
		RectF rcBorder;
		rcBorder.X = rcLine.left;
		rcBorder.Y = rcLine.top;
		rcBorder.Width = rcLine.Width();
		rcBorder.Height = rcLine.Height();

		Gdiplus::Color color(GetRValue(m_crBorder), GetGValue(m_crBorder), GetBValue(m_crBorder));
		Pen hPen(color);
		hPen.SetWidth(1);
		graph.DrawRectangle(&hPen, rcBorder);
	}

	for (int i = 0; i < 8; ++i)
	{
		RectF rcDot;
		rcDot.X = m_rcPos[i].left;
		rcDot.Y = m_rcPos[i].top;
		rcDot.Width = m_rcPos[i].Width();
		rcDot.Height = m_rcPos[i].Height();

		Gdiplus::Color color(GetRValue(m_crBorder), GetGValue(m_crBorder), GetBValue(m_crBorder));
		SolidBrush hBrush(color);
		hBrush.SetColor(color);
		graph.FillRectangle(&hBrush, rcDot);
	}
	
	if (m_bDrawOperate)
	{
		switch (m_nOperateType)
		{
		case 1:  //rectangle
			{
				if (!m_rcRectangle.IsRectEmpty())
					DrawRectangle(pRT, m_rcRectangle);
			}
			break;
		case 2:	//ellipse
			{
				if (!m_rcEllipse.IsRectEmpty())
					DrawEllipse(pRT, m_rcEllipse);
			}
			break;

		case 3:	//arrow
			DrawArrow(pRT, m_pt);
			break;

		case 4:	//doodle
			DrawDoodle(pRT, m_vecDoodlePoints);
			break;

		case 5:	//mask
			DrawMask(pRT, m_vecMaskPoints);
			break;

		default:
			break;
		}
	}


	pRT->ReleaseDC(hDC);
}

BOOL SSnapshotCtrl::OnEraseBkgnd(SOUI::IRenderTarget * pRT)
{
// 	CDC* pDC = CDC::FromHandle(pRT->GetDC());
// 	BITMAP	bmp;
// 	m_pBitmap->GetBitmap(&bmp);
// 	CDC dcCompatible;
// 	dcCompatible.CreateCompatibleDC(pDC);
// 	dcCompatible.SelectObject(m_pBitmap);
// 
// 	SOUI::CRect rect(0,0, m_nScreenX, m_nScreenY);
// 	pDC->BitBlt(0,0, rect.Width(), rect.Height(), &dcCompatible ,0,0, SRCCOPY);
// 
// 	pRT->ReleaseDC(pDC->GetSafeHdc());

	return TRUE;
}

void SSnapshotCtrl::OnLButtonDown(UINT nFlags, SOUI::CPoint point)
{
	SetCapture();
	m_ptDown = point;
	
	if (m_rcCapture.IsRectEmpty())
		m_bSelected = true;

	if (!m_bSelectOperate)
	{
		m_eDraging = HitPos(point);
		switch(m_eDraging)
		{
		case EcPosType::TopLeft:
		case EcPosType::BottomRight:
			::SetCursor(m_hCurTopLeft);
			break;
		case EcPosType::TopCenter:
		case EcPosType::BottomCenter:
			::SetCursor(m_hCurTop);
			break;
		case EcPosType::TopRight:
		case EcPosType::BottomLeft:
			::SetCursor(m_hCurTopRight);
			break;
		case EcPosType::LeftCenter:
		case EcPosType::RightCenter:
			::SetCursor(m_hCurLeft);
			break;
		case EcPosType::SelectRect:
			::SetCursor(m_hCurSelect);
			m_ptDown = point - m_rcCapture.TopLeft();
			break;
		case EcPosType::Null:
		default:
			break;
		}
	}
	else
	{
		if (PtInRect(&m_rcCapture, point))
			m_bDrawOperate = true;
	}
}

void SSnapshotCtrl::OnLButtonUp(UINT nFlags, SOUI::CPoint point)
{
	m_bSelected = false;
	m_eDraging = EcPosType::Null;
	CalcPos();
	Invalidate();
	ReleaseCapture();

	if (m_bDrawOperate)
	{
		m_pBitmap = new CBitmap(CopyCurBitmap(0,0, m_nScreenX, m_nScreenY));
		if (m_pBitmap)
			m_vecBitmap.push_back(m_pBitmap);
	}

	m_bDrawOperate = false;
	m_rcEllipse.SetRectEmpty();
	m_rcRectangle.SetRectEmpty();
	m_pt.SetPoint(m_ptDown.x, m_ptDown.y);
	m_vecDoodlePoints.clear();
	m_vecMaskPoints.clear();

	EventRectCaptured evt(this);
	evt.pt = point;
	FireEvent(evt);

	Invalidate();
}

void SSnapshotCtrl::OnLButtonDblClk(UINT nFlags, SOUI::CPoint point)
{
	SaveCapBmpToClipboard();

	for (int i = 0; i < m_vecBitmap.size(); i++)
	{
		m_vecBitmap[i]->DeleteObject();
	}
	m_vecBitmap.clear();
	EventRectDbClk evt(this);
	FireEvent(evt);
}

void SSnapshotCtrl::OnMouseMove(UINT nFlags, SOUI::CPoint point)
{
	if (!m_bSelectOperate)
	{
		if (m_bSelected)
		{
			if(!m_rcCapture.IsRectEmpty())
				m_rcCapture.SetRectEmpty();

			if (m_ptDown == point)
				return;

			m_rcCapture.SetRect(m_ptDown, point);
			m_rcCapture.NormalizeRect();

			CalcPos();
			Invalidate();

			EventCapturing evt(this);
			evt.pt = point;
			FireEvent(evt);
		}

		if (EcPosType::Null == m_eDraging)
		{
			ShowCursor(HitPos(point));
			return;
		}

		SOUI::CRect rcWnd = GetWindowRect();
		switch(m_eDraging)
		{
		case EcPosType::Null:
			break;
		case EcPosType::TopLeft:
			{
				if (point.x > m_rcCapture.right && point.y > m_rcCapture.bottom)
				{
					m_eDraging = EcPosType::BottomRight;

					m_rcCapture.left = m_rcCapture.right;
					m_rcCapture.right = point.x;
					m_rcCapture.top = m_rcCapture.bottom;
					m_rcCapture.bottom = point.y;
				}
				else if (point.x > m_rcCapture.right)
				{
					m_eDraging = EcPosType::TopRight;
					m_rcCapture.left = m_rcCapture.right;
					m_rcCapture.right = point.x;
				}
				else if (point.y > m_rcCapture.bottom)
				{
					m_eDraging = EcPosType::BottomLeft;
					m_rcCapture.top = m_rcCapture.bottom;
					m_rcCapture.bottom = point.y;
				}
				else
				{
					m_rcCapture.left = (point.x > rcWnd.left) ? point.x : rcWnd.left;
					m_rcCapture.top = (point.y > rcWnd.top) ? point.y : rcWnd.top;
				}
			}
			break;
		case EcPosType::TopCenter:
			{
				if (point.y > m_rcCapture.bottom)
				{
					m_eDraging = EcPosType::BottomCenter;
					m_rcCapture.top = m_rcCapture.bottom;
					m_rcCapture.bottom = point.y;
				}
				else
				{
					m_rcCapture.top = (point.y > rcWnd.top) ? point.y : rcWnd.top;
				}
			}
			break;
		case EcPosType::TopRight:
			{
				if (point.x < m_rcCapture.left && point.y > m_rcCapture.bottom)
				{
					m_eDraging = EcPosType::BottomLeft;

					m_rcCapture.right = m_rcCapture.left;
					m_rcCapture.left = point.x;

					m_rcCapture.top = m_rcCapture.bottom;
					m_rcCapture.bottom = point.y;
				}
				else if (point.x < m_rcCapture.left)
				{
					m_eDraging = EcPosType::TopLeft;

					m_rcCapture.right = m_rcCapture.left;
					m_rcCapture.left = point.x;
				}
				else if (point.y > m_rcCapture.bottom)
				{
					m_eDraging = EcPosType::BottomRight;

					m_rcCapture.top = m_rcCapture.bottom;
					m_rcCapture.bottom = point.y;
				}
				else
				{
					m_rcCapture.top = (point.y > rcWnd.top) ? point.y : rcWnd.top;
					m_rcCapture.right = (point.x < rcWnd.right) ? point.x : rcWnd.right;
				}
			}
			break;
		case EcPosType::RightCenter:
			{
				if (point.x < m_rcCapture.left)
				{// 如果 过线了 就 换成 反的
					m_eDraging = EcPosType::LeftCenter;
					m_rcCapture.right = m_rcCapture.left;
					m_rcCapture.left = point.x;
				}
				else
				{
					m_rcCapture.right = (point.x < rcWnd.right) ? point.x : rcWnd.right;
				}
			}
			break;
		case EcPosType::BottomRight:
			{
				if (point.x < m_rcCapture.left && point.y < m_rcCapture.top)
				{
					m_eDraging = EcPosType::TopLeft;

					m_rcCapture.right = m_rcCapture.left;
					m_rcCapture.left = point.x;

					m_rcCapture.bottom = m_rcCapture.top;
					m_rcCapture.top = point.y;
				}
				else if (point.x < m_rcCapture.left)
				{
					m_eDraging = EcPosType::BottomLeft;

					m_rcCapture.right = m_rcCapture.left;
					m_rcCapture.left = point.x;
				}
				else if (point.y < m_rcCapture.top)
				{
					m_eDraging = EcPosType::TopRight;

					m_rcCapture.bottom = m_rcCapture.top;
					m_rcCapture.top = point.y;
				}
				else
				{
					m_rcCapture.bottom = (point.y < rcWnd.bottom) ? point.y : rcWnd.bottom;
					m_rcCapture.right = (point.x < rcWnd.right) ? point.x : rcWnd.right;
				}
			}
			break;	
		case EcPosType::BottomCenter:
			{
				if (point.y < m_rcCapture.top)
				{
					m_eDraging = EcPosType::TopCenter;
					m_rcCapture.bottom = m_rcCapture.top;
					m_rcCapture.top = point.y;
				}
				else
				{
					m_rcCapture.bottom = (point.y < rcWnd.bottom) ? point.y : rcWnd.bottom;
				}
			}
			break;		
		case EcPosType::BottomLeft:
			{
				if (point.x > m_rcCapture.right && point.y < m_rcCapture.top)
				{
					m_eDraging = EcPosType::TopRight;

					m_rcCapture.bottom = m_rcCapture.top;
					m_rcCapture.top = point.y;

					m_rcCapture.left = m_rcCapture.right;
					m_rcCapture.right = point.x;
				}
				else if (point.x > m_rcCapture.right)
				{
					m_eDraging = EcPosType::BottomRight;
					m_rcCapture.left = m_rcCapture.right;
					m_rcCapture.right = point.x;
				}
				else if (point.y < m_rcCapture.top)
				{
					m_eDraging = EcPosType::TopLeft;
					m_rcCapture.bottom = m_rcCapture.top;
					m_rcCapture.top = point.y;
				}
				else
				{
					m_rcCapture.bottom = (point.y < rcWnd.bottom) ? point.y : rcWnd.bottom;
					m_rcCapture.left = (point.x > rcWnd.left) ? point.x : rcWnd.left;
				}
			}
			break;
		case EcPosType::LeftCenter:
			{
				if (point.x > m_rcCapture.right)
				{
					m_eDraging = EcPosType::RightCenter;
					m_rcCapture.left = m_rcCapture.right;
					m_rcCapture.right = point.x;
				}
				else
				{
					m_rcCapture.left = (point.x > rcWnd.left) ? point.x : rcWnd.left;
				}
			}			
			break;		
		case EcPosType::SelectRect:
			{			
				SOUI::CPoint ptLT = point - m_ptDown;			// 相对 鼠标点击 时  的 偏移量  也就是 移动 的 值 
				if (ptLT.x < rcWnd.left)
					ptLT.x = rcWnd.left;
				else if (ptLT.x > rcWnd.right - m_rcCapture.Width())
					ptLT.x = rcWnd.right - m_rcCapture.Width();
				if (ptLT.y < rcWnd.top)
					ptLT.y = rcWnd.top;
				else if (ptLT.y > rcWnd.bottom - m_rcCapture.Height())
					ptLT.y = rcWnd.bottom - m_rcCapture.Height();
				m_rcCapture.MoveToXY(ptLT);
			}
			break;
		default:
			break;
		}

		ShowCursor(m_eDraging);
		CalcPos();
		Invalidate();
		if (EcPosType::SelectRect == m_eDraging)
		{
			EventRectMoving evt(this);
			evt.pt = point;
			FireEvent(evt);
		}
		else
		{
			EventCapturing evt(this);
			evt.pt = point;
			FireEvent(evt);
		}
	}
	else
	{
		if (m_bDrawOperate)
		{
			SOUI::CPoint pt = point;
			if (pt.x <= m_rcCapture.left)
				pt.x = m_rcCapture.left;
			if (pt.x >= m_rcCapture.right)
				pt.x = m_rcCapture.right;
			if (point.y <= m_rcCapture.top)
				pt.y = m_rcCapture.top;
			if (point.y >= m_rcCapture.bottom)
				pt.y = m_rcCapture.bottom;

			switch (m_nOperateType)
			{
			case 1:
				{
					m_rcRectangle.SetRect(m_ptDown, pt);
					m_rcRectangle.NormalizeRect();
				}
				break;

			case 2:
				{
					m_rcEllipse.SetRect(m_ptDown, pt);
					m_rcEllipse.NormalizeRect();
				}
				break;
			case 3:
				m_pt = pt;
				break;
			case 4:		//doodle
				m_vecDoodlePoints.push_back(pt);
				break;
			case 5:		//mask
				m_vecMaskPoints.push_back(pt);
				break;
			default:
				break;
			}

			Invalidate();
		}
	}
}

void SSnapshotCtrl::CalcGrayAreaRect(SOUI::CRect rcArea, RectF& rcLeft, RectF& rcTop, RectF& rcRight, RectF& rcBottom)
{
	if (rcArea.IsRectEmpty())
		return;

	SOUI::CRect rect(0, 0, m_nScreenX, m_nScreenY);

	//左边
	rcLeft.X = 0;
	rcLeft.Y = 0;
	rcLeft.Width = (REAL)rcArea.left;
	rcLeft.Height = (REAL)rect.Height();

	//右边
	rcRight.X = (REAL)rcArea.right;
	rcRight.Y = 0;
	rcRight.Width = (REAL)rect.right - rcArea.right;
	rcRight.Height = (REAL)rect.Height();

	//上边
	rcTop.Y = 0;
	rcTop.X = (REAL)rcArea.left;
	rcTop.Width = (REAL)rcArea.Width();
	rcTop.Height = (REAL)rcArea.top;

	//下边
	rcBottom.X = (REAL)rcArea.left;
	rcBottom.Y = (REAL)rcArea.bottom;
	rcBottom.Width = (REAL)rcArea.Width();
	rcBottom.Height = (REAL)rect.bottom - rcArea.bottom;
}

void SSnapshotCtrl::CalcPos()
{
	SOUI::CRect rcLine(m_rcCapture);
	rcLine.InflateRect(1, 1);
	CAutoRefPtr<IPen> curPen, oldPen;

	SOUI::CPoint center = rcLine.CenterPoint();

	// 上左 方块
	m_rcPos[(int)EcPosType::TopLeft].SetRect(rcLine.left - 1, rcLine.top - 1, rcLine.left + 3, rcLine.top + 3);

	// 上中 方块
	m_rcPos[(int)EcPosType::TopCenter].SetRect(center.x - 2, rcLine.top - 1, center.x + 2, rcLine.top + 3);

	// 上右 方块
	m_rcPos[(int)EcPosType::TopRight].SetRect(rcLine.right - 3, rcLine.top - 1, rcLine.right + 1, rcLine.top + 3);

	// 右中 方块
	m_rcPos[(int)EcPosType::RightCenter].SetRect(rcLine.right - 3, center.y - 2, rcLine.right + 1, center.y + 2);

	// 下右 方块
	m_rcPos[(int)EcPosType::BottomRight].SetRect(rcLine.right - 3, rcLine.bottom - 3, rcLine.right + 1, rcLine.bottom + 1);

	// 下中 方块
	m_rcPos[(int)EcPosType::BottomCenter].SetRect(center.x - 2, rcLine.bottom - 3, center.x + 2, rcLine.bottom + 1);

	// 下左 方块
	m_rcPos[(int)EcPosType::BottomLeft].SetRect(rcLine.left - 1, rcLine.bottom - 3, rcLine.left + 3, rcLine.bottom + 1);

	// 左中 方块
	m_rcPos[(int)EcPosType::LeftCenter].SetRect(rcLine.left - 1, center.y - 2, rcLine.left + 3, center.y + 2);
}

SSnapshotCtrl::EcPosType SSnapshotCtrl::HitPos(SOUI::CPoint& pt)
{
	for (int i = 0; i < 8; ++i)
	{
		if (m_rcPos[i].PtInRect(pt))
			return EcPosType(i);
	}

	if (m_rcCapture.PtInRect(pt))
		return EcPosType::SelectRect;

	return EcPosType::Null;
}

void SSnapshotCtrl::ShowCursor(EcPosType ePos)
{
	switch (ePos)
	{
	case EcPosType::TopLeft:
	case EcPosType::BottomRight:
		::SetCursor(m_hCurTopLeft);
		break;
	case EcPosType::TopCenter:
	case EcPosType::BottomCenter:
		::SetCursor(m_hCurTop);
		break;
	case EcPosType::TopRight:
	case EcPosType::BottomLeft:
		::SetCursor(m_hCurTopRight);
		break;
	case EcPosType::LeftCenter:
	case EcPosType::RightCenter:
		::SetCursor(m_hCurLeft);
		break;
	case EcPosType::SelectRect:
		::SetCursor(m_hCurSelect);
		break;
	case EcPosType::Null:
	default:
		break;
	}
}

void SSnapshotCtrl::SetBmpResource(CBitmap* pBmp)
{
	m_pBitmap = pBmp;

	m_vecBitmap.push_back(pBmp);
	Invalidate();
}

void SSnapshotCtrl::SetScreenSize(int nScreenX, int nScreenY)
{
	m_nScreenX = nScreenX;
	m_nScreenY = nScreenY;
}

void SSnapshotCtrl::SetOperateType(int nOperateType /* = -1 */)
{
	m_bSelectOperate = true;

	/*	yangjinpeng 2018-07-13
	*	1、rect
	*	2、ellipse
	*	3、arrow
	*	4、doodle
	*	5、mask
	*	6、word
	*/
	m_nOperateType = nOperateType;
}

void SSnapshotCtrl::SetPenColor(const COLORREF& color)
{
	m_crPen = color;
}

void SSnapshotCtrl::SetPenSize(int nPenSize /* = 1 */)
{
	m_nPenSize = nPenSize;
}

void SSnapshotCtrl::RevokeOperate()
{
	if (m_vecBitmap.size() > 1)
	{
		m_vecBitmap.back()->DeleteObject();
		m_vecBitmap.pop_back();
		m_pBitmap = m_vecBitmap.back();
		Invalidate();
	}
}

HBITMAP SSnapshotCtrl::CopyCurBitmap(int nx, int ny, int nWidth, int nHeight)
{
	SOUI::CRect rcClient = GetClientRect();
	CAutoRefPtr<IRenderTarget> pMemRT;
	GETRENDERFACTORY->CreateRenderTarget(&pMemRT, rcClient.Width(), rcClient.Height());

	HDC hDC = pMemRT->GetDC();
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(hDC);
	dcCompatible.SelectBitmap(*m_pBitmap);
	SOUI::CRect rect(0,0, m_nScreenX, m_nScreenY);
	BitBlt(hDC, 0,0, rect.Width(), rect.Height(), dcCompatible, 0,0, SRCCOPY);

	switch (m_nOperateType)
	{
	case 1:
		{
			if (!m_rcRectangle.IsRectEmpty())
				DrawRectangle(pMemRT, m_rcRectangle);
		}
		break;
	case 2:
		{
			if (!m_rcEllipse.IsRectEmpty())
				DrawEllipse(pMemRT, m_rcEllipse);
		}
		break;
	case 3:
		DrawArrow(pMemRT, m_pt);
		break;
	case 4:
		DrawDoodle(pMemRT, m_vecDoodlePoints);
		break;
	case 5:
		DrawMask(pMemRT, m_vecMaskPoints);
		break;
	default:
		break;
	}

	HDC hMemDC; 
	HBITMAP   hBitmap, hOldBitmap; 
	hMemDC = CreateCompatibleDC(hDC);

	hBitmap = CreateCompatibleBitmap(hDC, nWidth, nHeight);
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hDC, 0, 0, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

	return hBitmap;
}

void SSnapshotCtrl::DrawRectangle(IRenderTarget *pRT, const SOUI::CRect& rcRectangle)
{
	SOUI::CRect rt(0,0, m_nScreenX, m_nScreenY);
	HDC hDC = pRT->GetDC();
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(hDC);
	dcCompatible.SelectBitmap(*m_pBitmap);

	BitBlt(hDC, 0,0, rt.Width(), rt.Height(), dcCompatible, 0,0, SRCCOPY);

	Graphics graphics(hDC);
	Gdiplus::Color color(GetRValue(m_crPen), GetGValue(m_crPen), GetBValue(m_crPen));
	Pen hPen(color);
	hPen.SetWidth(m_nPenSize);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);  

	RectF rcRect;
	rcRect.X = rcRectangle.left;
	rcRect.Y = rcRectangle.top;
	rcRect.Width = rcRectangle.Width();
	rcRect.Height = rcRectangle.Height();

	graphics.DrawRectangle(&hPen, rcRect.X, rcRect.Y, rcRect.Width, rcRect.Height);
	CDCHandle desDC = hDC;
	desDC.BitBlt(rt.left,rt.top, rt.Width(), rt.Height(), hDC, 0,0, SRCCOPY|CAPTUREBLT);
	pRT->ReleaseDC(hDC);
}

void SSnapshotCtrl::DrawEllipse(IRenderTarget* pRT, const SOUI::CRect& rcEllipse)
{
	SOUI::CRect rt(0,0, m_nScreenX, m_nScreenY);
	HDC hDC = pRT->GetDC();
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(hDC);
	dcCompatible.SelectBitmap(*m_pBitmap);

	BitBlt(hDC, 0,0, rt.Width(), rt.Height(), dcCompatible, 0,0, SRCCOPY);

	Graphics graphics(hDC);
	Gdiplus::Color color(GetRValue(m_crPen), GetGValue(m_crPen), GetBValue(m_crPen));
	Pen hPen(color);
	hPen.SetWidth(m_nPenSize);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);  

	RectF rcEll;
	rcEll.X = rcEllipse.left;
	rcEll.Y = rcEllipse.top;
	rcEll.Width = rcEllipse.Width();
	rcEll.Height = rcEllipse.Height();

	graphics.DrawEllipse(&hPen, rcEll.X, rcEll.Y, rcEll.Width, rcEll.Height);
	CDCHandle hDesDC=hDC;
	hDesDC.BitBlt(rt.left,rt.top, rt.Width(), rt.Height(), hDC, 0,0, SRCCOPY|CAPTUREBLT);
	pRT->ReleaseDC(hDC);
}

void SSnapshotCtrl::DrawArrow(IRenderTarget* pRT, const SOUI::CPoint& point)
{
	if (m_ptDown == point)
		return;

	SOUI::CRect rt(0,0, m_nScreenX, m_nScreenY);
	HDC hDC = pRT->GetDC();
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(hDC);
	dcCompatible.SelectBitmap(*m_pBitmap);

	BitBlt(hDC, 0,0, rt.Width(), rt.Height(), dcCompatible, 0,0, SRCCOPY);

	LineArrow(hDC, m_ptDown, point, 30, 15, 40, m_crPen, m_nPenSize);
	CDCHandle hDesDC = hDC;
	hDesDC.BitBlt(rt.left,rt.top, rt.Width(), rt.Height(),hDC, 0,0, SRCCOPY|CAPTUREBLT);
	pRT->ReleaseDC(hDC);
}

void SSnapshotCtrl::LineArrow(HDC pDC, SOUI::CPoint p1, SOUI::CPoint p2, double theta, double alpha, int length, COLORREF clr, int size)
{
	theta = 3.1415926*theta/180;//转换为弧度
	alpha = 3.1415926*alpha/180;
	double Px,Py,P1x,P1y,P2x,P2y;
	double Pax,Pay,Pbx,Pby;
	//以P2为原点得到向量P2P1（P）
	Px=p1.x-p2.x;
	Py=p1.y-p2.y;
	//向量P旋转theta角得到向量P1
	P1x=Px*cos(theta)-Py*sin(theta);
	P1y=Px*sin(theta)+Py*cos(theta);
	//向量P旋转alpha角得到向量Pa
	Pax=Px*cos(alpha)-Py*sin(alpha);
	Pay=Px*sin(alpha)+Py*cos(alpha);
	//向量P旋转-theta角得到向量P2
	P2x=Px*cos(-theta)-Py*sin(-theta);
	P2y=Px*sin(-theta)+Py*cos(-theta);
	//向量P旋转-alpha角得到向量Pb
	Pbx=Px*cos(-alpha)-Py*sin(-alpha);
	Pby=Px*sin(-alpha)+Py*cos(-alpha);
	//伸缩向量至制定长度
	int len;
	if (size==1)
	{
		length = (int)(length/2);
		len	   = (int)(length-1.5);
	}
	else if (size == 2)
	{
		length = length*2/3+2;
		len = length - 2;
	}
	else
	{
		len = length -3;
	}

	double x1,x2;
	x1=sqrt(P1x*P1x+P1y*P1y);
	P1x=P1x*length/x1;
	P1y=P1y*length/x1;
	x2=sqrt(P2x*P2x+P2y*P2y);
	P2x=P2x*length/x2;
	P2y=P2y*length/x2;

	double xa,xb;
	xa=sqrt(Pax*Pax+Pay*Pay);
	Pax=Pax*(len)/x1;
	Pay=Pay*(len)/x1;
	xb=sqrt(Pbx*Pbx+Pby*Pby);
	Pbx=Pbx*(len)/x2;
	Pby=Pby*(len)/x2;
	//平移变量到直线的末端
	P1x=P1x+p2.x;
	P1y=P1y+p2.y;
	P2x=P2x+p2.x;
	P2y=P2y+p2.y;

	Pax=Pax+p2.x;
	Pay=Pay+p2.y;
	Pbx=Pbx+p2.x;
	Pby=Pby+p2.y;
	
	Gdiplus::Graphics graphics(pDC); 
	SolidBrush GdiBrush(Gdiplus::Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);  
	Point   pts[6];
	pts[0].X = (INT)p1.x;
	pts[0].Y = (INT)p1.y;
	pts[1].X = (INT)Pax;
	pts[1].Y = (INT)Pay;
	pts[2].X = (INT)P1x;
	pts[2].Y = (INT)P1y;
	pts[3].X = (INT)p2.x;
	pts[3].Y = (INT)p2.y;
	pts[4].X = (INT)P2x;
	pts[4].Y = (INT)P2y;
	pts[5].X = (INT)Pbx;
	pts[5].Y = (INT)Pby;
	graphics.FillPolygon(&GdiBrush, pts, 6);
}

void SSnapshotCtrl::DrawDoodle(IRenderTarget* pRT, const std::vector<SOUI::CPoint> vecPoints)
{
	SOUI::CRect rt(0,0, m_nScreenX, m_nScreenY);
	HDC hDC = pRT->GetDC();
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(hDC);
	dcCompatible.SelectBitmap(*m_pBitmap);

	BitBlt(hDC, 0,0, rt.Width(), rt.Height(), dcCompatible, 0,0, SRCCOPY);

	Graphics graphics(hDC);
	Gdiplus::Color color(GetRValue(m_crPen), GetGValue(m_crPen), GetBValue(m_crPen));
	Pen hPen(color);
	hPen.SetWidth(m_nPenSize);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);  

	Gdiplus::GraphicsPath doodlePath;
	for (int i = 0; i < vecPoints.size(); i++)
	{
		if (vecPoints.size() > i+1)
			doodlePath.AddLine(vecPoints[i].x, vecPoints[i].y, vecPoints[i + 1].x, vecPoints[i + 1].y);
	}

	graphics.DrawPath(&hPen, &doodlePath);
	CDCHandle hDesDC = hDC;
	hDesDC.BitBlt(rt.left,rt.top, rt.Width(), rt.Height(), hDC, 0,0, SRCCOPY|CAPTUREBLT);
	pRT->ReleaseDC(hDC);
}

void SSnapshotCtrl::DrawMask(IRenderTarget* pRT, const std::vector<SOUI::CPoint> vecPoints)
{
	int nMaskSize = 0;
	if (1 == m_nPenSize)
		nMaskSize = 15;
	else if (2 == m_nPenSize)
		nMaskSize = 25;
	else
		nMaskSize = 35;

	SOUI::CRect rt(0,0, m_nScreenX, m_nScreenY);
	HDC hDC = pRT->GetDC();
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(hDC);
	dcCompatible.SelectBitmap(*m_pBitmap);

	BitBlt(hDC, 0,0, rt.Width(), rt.Height(), dcCompatible, 0,0, SRCCOPY);

	Graphics graphics(hDC);
	Gdiplus::TextureBrush tBrush(m_pBrushMask);
	Pen hPen(&tBrush, nMaskSize);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);  

	Gdiplus::GraphicsPath maskPath;
	for (int i = 0; i < vecPoints.size(); i++)
	{
		if (vecPoints.size() > i+1)
		{
			if (vecPoints[i+1].x + nMaskSize >= m_rcCapture.left && 
				vecPoints[i+1].x + nMaskSize <= m_rcCapture.right &&
				vecPoints[i+1].y + nMaskSize >= m_rcCapture.top &&
				vecPoints[i+1].y + nMaskSize <= m_rcCapture.bottom)
			{
				maskPath.AddLine(vecPoints[i].x, vecPoints[i].y, vecPoints[i+1].x, vecPoints[i+1].y);
			}
		}
	}

	graphics.DrawPath(&hPen, &maskPath);
	CDCHandle hDesDC = hDC;
	hDesDC.BitBlt(rt.left, rt.top, rt.Width(), rt.Height(), hDC, 0, 0, SRCCOPY | CAPTUREBLT);
	pRT->ReleaseDC(hDC);
}

void SSnapshotCtrl::SaveCapBmpToClipboard()
{
	SOUI::CRect rcClient = GetClientRect();	
	CBitmap* pBitmap = m_vecBitmap.back();

	CDC deskDc = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	CDC tempDc;
	tempDc.CreateCompatibleDC(deskDc);
	CDC dcMemory;
	dcMemory.CreateCompatibleDC(deskDc);
	CBitmapHandle membitmap;
	membitmap.CreateCompatibleBitmap(deskDc, m_rcCapture.Width(), m_rcCapture.Height());
	tempDc.SelectBitmap(pBitmap->m_hBitmap);
	CBitmapHandle oldbitmap = dcMemory.SelectBitmap(membitmap);
	dcMemory.BitBlt(0, 0, m_rcCapture.Width(), m_rcCapture.Height(), tempDc, m_rcCapture.left, m_rcCapture.top, SRCCOPY);
	dcMemory.SelectBitmap(oldbitmap);
	
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, membitmap);
		CloseClipboard();
	}
}