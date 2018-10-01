#if !defined(AFX_MYBUTTON_H__4CF1E477_DFF4_41A5_8D5B_B9FC398398DE__INCLUDED_)
#define AFX_MYBUTTON_H__4CF1E477_DFF4_41A5_8D5B_B9FC398398DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MYButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMYButton window

class CMYButton : public CButton
{
// Construction
public:
	CMYButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMYButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMYButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMYButton)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	CBrush m_brushBackground;
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYBUTTON_H__4CF1E477_DFF4_41A5_8D5B_B9FC398398DE__INCLUDED_)
