#if !defined(AFX_MYSTATIC_H__94BA3C64_7A4A_416F_BD84_93ADE77FF2FE__INCLUDED_)
#define AFX_MYSTATIC_H__94BA3C64_7A4A_416F_BD84_93ADE77FF2FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MYStatic.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMYStatic window

class CMYStatic : public CStatic
{
// Construction
public:
	CMYStatic();
	CBrush m_brushBackground;
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMYStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMYStatic();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMYStatic)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSTATIC_H__94BA3C64_7A4A_416F_BD84_93ADE77FF2FE__INCLUDED_)
