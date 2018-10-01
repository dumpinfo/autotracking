// viewbutton.h : main header file for the VIEWBUTTON application
//

#if !defined(AFX_VIEWBUTTON_H__DFF8A623_D6C0_4251_93EF_4FEC01443298__INCLUDED_)
#define AFX_VIEWBUTTON_H__DFF8A623_D6C0_4251_93EF_4FEC01443298__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonApp:
// See viewbutton.cpp for the implementation of this class
//

class CViewbuttonApp : public CWinApp
{
public:
	CViewbuttonApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewbuttonApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CViewbuttonApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWBUTTON_H__DFF8A623_D6C0_4251_93EF_4FEC01443298__INCLUDED_)
