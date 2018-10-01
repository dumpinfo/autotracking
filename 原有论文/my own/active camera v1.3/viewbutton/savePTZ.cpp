// savePTZ.cpp : implementation file
//

#include "stdafx.h"
#include "viewbutton.h"
#include "savePTZ.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CsavePTZ dialog


CsavePTZ::CsavePTZ(CWnd* pParent /*=NULL*/)
	: CDialog(CsavePTZ::IDD, pParent)
{
	//{{AFX_DATA_INIT(CsavePTZ)
	m_iNum = 0;
	//}}AFX_DATA_INIT

	CBitmap bmpBackground;
	bmpBackground.LoadBitmap(IDB_CONTROLBACK);
	m_brushBackground.CreatePatternBrush(&bmpBackground);
	bmpBackground.DeleteObject();
}


void CsavePTZ::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CsavePTZ)
	DDX_Text(pDX, IDC_SAVESTATE, m_iNum);
	DDV_MinMaxInt(pDX, m_iNum, 0, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CsavePTZ, CDialog)
	//{{AFX_MSG_MAP(CsavePTZ)
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CsavePTZ message handlers

HBRUSH CsavePTZ::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	CPoint ptBrushOrg(0, 0);
//	MapWindowPoints(pWnd, &ptBrushOrg, 1u);
	pDC->SetBrushOrg(ptBrushOrg);
	pDC->SetBkMode(TRANSPARENT);
	m_brushBackground.UnrealizeObject();

	return (HBRUSH)m_brushBackground.GetSafeHandle();
}

int CsavePTZ::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rtClient;
	GetClientRect(rtClient);

	m_btnOK.Create(_T("确定"), WS_CHILD|WS_VISIBLE| WS_GROUP| BS_OWNERDRAW, 
		CRect(rtClient.Width()-115,15,rtClient.Width()-40,35), this, IDOK);
	m_btnOK.SetFont(GetFont());
	m_btnCANCEL.Create(_T("取消"), WS_CHILD|WS_VISIBLE| WS_GROUP| BS_OWNERDRAW, 
		CRect(rtClient.Width()-115,60,rtClient.Width()-40,80), this, IDCANCEL);
	m_btnCANCEL.SetFont(GetFont());

	// TODO: Add your specialized creation code here
	
	return 0;
}
