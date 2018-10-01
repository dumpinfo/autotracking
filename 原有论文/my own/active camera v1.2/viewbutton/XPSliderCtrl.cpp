// XPSliderCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "AbnormalityDetector.h"
#include "XPSliderCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXPSliderCtrl

CXPSliderCtrl::CXPSliderCtrl()
{
}

CXPSliderCtrl::~CXPSliderCtrl()
{
}


BEGIN_MESSAGE_MAP(CXPSliderCtrl, CSliderCtrl)
	//{{AFX_MSG_MAP(CXPSliderCtrl)
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXPSliderCtrl message handlers

void CXPSliderCtrl::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CSliderCtrl::OnWindowPosChanged(lpwndpos);
	
	// Simulate a WM_SIZE message, this will force slider to correctly repaint itself
	if(lpwndpos->flags & SWP_NOSIZE)
	{
		CRect rtClient;
		GetClientRect(rtClient);
		SendMessage(WM_SIZE, 0, MAKELPARAM(rtClient.Width(), rtClient.Height()));
	}
}
