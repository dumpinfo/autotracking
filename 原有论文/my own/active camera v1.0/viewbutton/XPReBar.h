// XPReBar.h: interface for the CXPReBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XPREBAR_H__091EEEDF_7078_400D_98EA_EA3DB5D17419__INCLUDED_)
#define AFX_XPREBAR_H__091EEEDF_7078_400D_98EA_EA3DB5D17419__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CXPReBar : public CReBar  
{
public:
	CXPReBar();
	virtual ~CXPReBar();

protected:
	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CBrush m_brushBackground;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP();
};

#endif // !defined(AFX_XPREBAR_H__091EEEDF_7078_400D_98EA_EA3DB5D17419__INCLUDED_)
