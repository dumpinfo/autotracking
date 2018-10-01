#if !defined(AFX_ENUMDEVICEDLG_H__DFA4CDFF_50B7_4B27_B3CB_D9A4AD6FC7ED__INCLUDED_)
#define AFX_ENUMDEVICEDLG_H__DFA4CDFF_50B7_4B27_B3CB_D9A4AD6FC7ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EnumDeviceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEnumDeviceDlg dialog

class CEnumDeviceDlg : public CDialog
{
// Construction
public:
	CEnumDeviceDlg(CWnd* pParent = NULL);   // standard constructor

	CString m_szIniPath;
	~CEnumDeviceDlg();
	CBrush m_brushBackground;
	IMoniker * m_pISelMoniker;
// Dialog Data
	//{{AFX_DATA(CEnumDeviceDlg)
	enum { IDD = IDD_ENUMDEVICE };
	CButton	m_btnOK;
	CComboBox	m_comboDevice;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEnumDeviceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString GenerateDeviceName(IMoniker *pIMoniker);

	// Generated message map functions
	//{{AFX_MSG(CEnumDeviceDlg)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnOK();
	afx_msg void OnSelchangeCombodevice();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENUMDEVICEDLG_H__DFA4CDFF_50B7_4B27_B3CB_D9A4AD6FC7ED__INCLUDED_)
