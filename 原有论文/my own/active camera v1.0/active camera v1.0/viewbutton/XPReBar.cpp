// XPReBar.cpp: implementation of the CXPReBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "viewbutton.h"
#include "XPReBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CXPReBar
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CXPReBar, CReBar)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

CXPReBar::CXPReBar()
{
	// Load background brush
	CBitmap bmpBackground;
	bmpBackground.LoadBitmap(IDB_CONTROLBACK);
	m_brushBackground.CreatePatternBrush(&bmpBackground);
	bmpBackground.DeleteObject();
}

CXPReBar::~CXPReBar()
{
	// Destroy background brush
	m_brushBackground.DeleteObject();
}

BOOL CXPReBar::OnEraseBkgnd(CDC *pDC)
{
	// Erase background by our custom method
	CRect rtClient;
	GetClientRect(rtClient);
	CPoint brushOrg(0, 0);
	pDC->LPtoDP(&brushOrg);
	pDC->SetBrushOrg(brushOrg);
	pDC->SetBkMode(TRANSPARENT);
	m_brushBackground.UnrealizeObject();
	pDC->FillRect(rtClient, &m_brushBackground);

	// Draw separator
	if(GetReBarCtrl().GetStyle() | RBS_BANDBORDERS)
	{
		int dwLastBottom = 0;
		int dwLastLeft = 0;
		UINT dwDrawedRow = 0;
		CPen penGray, penWhite, *pOldPen;
		penGray.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_GRAYTEXT));
		penWhite.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DHILIGHT));
		for(UINT i=0; i<GetReBarCtrl().GetBandCount(); i++)
		{
			// Skip hidden band
			REBARBANDINFO BandInfo;
			memset(&BandInfo, 0, sizeof(REBARBANDINFO));
			BandInfo.cbSize = sizeof(REBARBANDINFO);
			BandInfo.fMask = RBBIM_STYLE;
			GetReBarCtrl().GetBandInfo(i, &BandInfo);
			if(BandInfo.fStyle & RBBS_HIDDEN)
			{
				continue;
			}

			// Draw separators
			CRect rtBand;
			GetReBarCtrl().GetRect(i, rtBand);
			if(rtBand.bottom != dwLastBottom)
			{
				// Draw horizontal separator
				if(dwDrawedRow < GetReBarCtrl().GetRowCount() - 1)
				{
					pOldPen = (CPen *)pDC->SelectObject(&penGray);
					pDC->MoveTo(0, rtBand.bottom);
					pDC->LineTo(rtClient.Width(), rtBand.bottom);
					pDC->SelectObject(&penWhite);
					pDC->MoveTo(0, rtBand.bottom + 1);
					pDC->LineTo(rtClient.Width(), rtBand.bottom + 1);
					pDC->SelectObject(pOldPen);

					// Update context
					dwDrawedRow++;
				}

				// Update context
				dwLastLeft = 0;
				dwLastBottom = rtBand.bottom;
			}
			else if(rtBand.left != dwLastLeft)
			{
				// Draw vertical separator
				pOldPen = (CPen *)pDC->SelectObject(&penGray);
				pDC->MoveTo(rtBand.left - 2, rtBand.top);
				pDC->LineTo(rtBand.left - 2, rtBand.bottom);
				pDC->SelectObject(&penWhite);
				pDC->MoveTo(rtBand.left - 1, rtBand.top);
				pDC->LineTo(rtBand.left - 1, rtBand.bottom + 
					(dwDrawedRow < GetReBarCtrl().GetRowCount()));
				pDC->SelectObject(pOldPen);

				// Update context
				dwLastLeft = rtBand.left;
			}
		}
		penWhite.DeleteObject();
		penGray.DeleteObject();
	}

	return TRUE;
}

HBRUSH CXPReBar::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	// Return our custom brush
	CPoint ptBrushOrg(0, 0);
	MapWindowPoints(pWnd, &ptBrushOrg, 1u);
	pDC->SetBrushOrg(ptBrushOrg);
	pDC->SetBkMode(TRANSPARENT);
	m_brushBackground.UnrealizeObject();

	return (HBRUSH)m_brushBackground.GetSafeHandle();
}

void CXPReBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
	// Let main frame process this
	if(pScrollBar)
	{
		GetParentFrame()->SendMessage(WM_HSCROLL, MAKEWPARAM(nSBCode, nPos), 
			(LPARAM)pScrollBar->GetSafeHwnd());
	}
	else
	{
		GetParentFrame()->SendMessage(WM_HSCROLL, MAKEWPARAM(nSBCode, nPos));
	}
}
