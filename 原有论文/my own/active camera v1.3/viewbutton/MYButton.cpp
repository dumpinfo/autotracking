// MYButton.cpp : implementation file
//

#include "stdafx.h"
#include "viewbutton.h"
#include "MYButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMYButton

CMYButton::CMYButton()
{
	CBitmap bmpBackground;
	BOOL bl = bmpBackground.LoadBitmap(IDB_BITMAP1);
	bl = m_brushBackground.CreatePatternBrush(&bmpBackground);
	bmpBackground.DeleteObject();
}

CMYButton::~CMYButton()
{
}


BEGIN_MESSAGE_MAP(CMYButton, CButton)
	//{{AFX_MSG_MAP(CMYButton)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMYButton message handlers

BOOL CMYButton::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rtClient;
	GetClientRect(rtClient);
	CPoint brushOrg(0, 0);
	pDC->LPtoDP(&brushOrg);
	pDC->SetBrushOrg(brushOrg);
	m_brushBackground.UnrealizeObject();
	pDC->FillRect(rtClient, &m_brushBackground);
	return true;//CButton::OnEraseBkgnd(pDC);//
}

void CMYButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	CDC*	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	pDC->SetBkMode(TRANSPARENT);

	CRect rtClient;
	CPoint ptBrushOrg(0, 0);
//	OnPrepareDC(pDC);
	GetClientRect(rtClient);
	pDC->DPtoLP(rtClient);
	pDC->LPtoDP(&ptBrushOrg);
	pDC->SetBrushOrg(ptBrushOrg);
	m_brushBackground.UnrealizeObject();
	pDC->FillRect(rtClient, &m_brushBackground);
	CString strText;
	GetWindowText(strText);

  // Draw the button text using the text color red.
  COLORREF crOldColor = ::SetTextColor(lpDrawItemStruct->hDC, RGB(0,0,0));
  ::DrawText(lpDrawItemStruct->hDC, strText, strText.GetLength(), 
    &lpDrawItemStruct->rcItem, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
  ::SetTextColor(lpDrawItemStruct->hDC, crOldColor);
}

HBRUSH CMYButton::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	// TODO: Change any attributes of the DC here
	CPoint ptBrushOrg(0, 0);
//	MapWindowPoints(pWnd, &ptBrushOrg, 1u);
	pDC->SetBrushOrg(ptBrushOrg);
	pDC->SetBkMode(TRANSPARENT);
	m_brushBackground.UnrealizeObject();

	return (HBRUSH)m_brushBackground.GetSafeHandle();	
	// TODO: Return a non-NULL brush if the parent's handler should not be called
}





