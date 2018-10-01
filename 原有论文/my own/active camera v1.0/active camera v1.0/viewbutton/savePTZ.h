#if !defined(AFX_SAVEPTZ_H__2E1AF778_5ED7_411F_A502_86BCB266DA4C__INCLUDED_)
#define AFX_SAVEPTZ_H__2E1AF778_5ED7_411F_A502_86BCB266DA4C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// savePTZ.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CsavePTZ dialog
#include "MYButton.h"
class CsavePTZ : public CDialog
{
// Construction
public:
	CsavePTZ(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CsavePTZ)
	enum { IDD = IDD_SAVEPTZ };
	int		m_iNum;
	//}}AFX_DATA

	CBrush m_brushBackground;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CsavePTZ)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CMYButton m_btnCANCEL;
	CMYButton m_btnOK;

	// Generated message map functions
	//{{AFX_MSG(CsavePTZ)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEPTZ_H__2E1AF778_5ED7_411F_A502_86BCB266DA4C__INCLUDED_)
