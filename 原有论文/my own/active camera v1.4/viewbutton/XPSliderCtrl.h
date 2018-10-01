#if !defined(AFX_XPSLIDERCTRL_H__1839CF5B_7E47_4AC0_A653_C741497DAF63__INCLUDED_)
#define AFX_XPSLIDERCTRL_H__1839CF5B_7E47_4AC0_A653_C741497DAF63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// XPSliderCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CXPSliderCtrl window

class CXPSliderCtrl : public CSliderCtrl
{
// Construction
public:
	CXPSliderCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXPSliderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CXPSliderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CXPSliderCtrl)
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XPSLIDERCTRL_H__1839CF5B_7E47_4AC0_A653_C741497DAF63__INCLUDED_)
