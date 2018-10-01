// EnumDeviceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "viewbutton.h"
#include "EnumDeviceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnumDeviceDlg dialog


CEnumDeviceDlg::CEnumDeviceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEnumDeviceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnumDeviceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT


	// Load background brush
	CBitmap bmpBackground;
	bmpBackground.LoadBitmap(IDB_BITMAP1);
	m_brushBackground.CreatePatternBrush(&bmpBackground);
	bmpBackground.DeleteObject();
}


void CEnumDeviceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnumDeviceDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_COMBODEVICE, m_comboDevice);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnumDeviceDlg, CDialog)
	//{{AFX_MSG_MAP(CEnumDeviceDlg)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_COMBODEVICE, OnSelchangeCombodevice)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnumDeviceDlg message handlers


CEnumDeviceDlg::~CEnumDeviceDlg()
{
	// Destroy background brush
	m_brushBackground.DeleteObject();
}

HBRUSH CEnumDeviceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if(nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX)
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	else
	{
		// Return our custom brush
		CPoint ptBrushOrg(0, 0);
		MapWindowPoints(pWnd, &ptBrushOrg, 1u);
		pDC->SetBrushOrg(ptBrushOrg);
		pDC->SetBkMode(TRANSPARENT);
		m_brushBackground.UnrealizeObject();

		return (HBRUSH)m_brushBackground.GetSafeHandle();
	}
}

BOOL CEnumDeviceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	// Initialize dialog data
	m_pISelMoniker = NULL;
	
	// Enum video capture devices
	ICreateDevEnum *pICreateDevEnum = NULL;
	if(SUCCEEDED(::CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (LPVOID *)&pICreateDevEnum)))
	{
		IEnumMoniker *pIEnumMoniker = NULL;
		if(pICreateDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory, &pIEnumMoniker, 0u) == S_OK)
		{
			IMoniker *pIMoniker = NULL;
			while(pIEnumMoniker->Next(1u, &pIMoniker, NULL) == S_OK)
			{
				// Get it's name and add it into combo
				m_comboDevice.SetItemData(m_comboDevice.AddString(
					GenerateDeviceName(pIMoniker)), (DWORD)pIMoniker);
				pIMoniker = NULL;
			}
			pIEnumMoniker->Release();
		}
		pICreateDevEnum->Release();
	}
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEnumDeviceDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	// Release all monikers
	for(int i=m_comboDevice.GetCount()-1; i>=0; i--)
	{
		IMoniker *pIMoniker = (IMoniker *)m_comboDevice.GetItemData(i);
		if(pIMoniker && pIMoniker != m_pISelMoniker)
		{
			pIMoniker->Release();
		}
	}	
}

void CEnumDeviceDlg::OnOK() 
{
	// TODO: Add extra validation here
	int dwCurSel = m_comboDevice.GetCurSel();
	if(dwCurSel >= 0 && (m_pISelMoniker = 
		(IMoniker *)m_comboDevice.GetItemData(dwCurSel)))
	{
		CDialog::OnOK();
	}
}

CString CEnumDeviceDlg::GenerateDeviceName(IMoniker *pIMoniker)
{
	CString szRet("");
	if(pIMoniker)
	{
		// Get friendly name
		CString szFriendlyName("");
		IPropertyBag *pIPropertyBag = NULL;
		if(SUCCEEDED(pIMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, 
			(void **)&pIPropertyBag)))
		{
			VARIANT varProperty;
			memset(&varProperty, 0, sizeof(VARIANT));
			::VariantInit(&varProperty);
			if(SUCCEEDED(pIPropertyBag->Read(L"FriendlyName", &varProperty, NULL)))
			{
				int nBufferLength = ::WideCharToMultiByte(CP_ACP, 0u, 
					varProperty.bstrVal, -1, NULL, 0, NULL, NULL);
				::WideCharToMultiByte(CP_ACP, 0u, varProperty.bstrVal, -1, 
					szFriendlyName.GetBuffer(nBufferLength), nBufferLength, NULL, NULL);
				szFriendlyName.ReleaseBuffer();
				::VariantClear(&varProperty);
			}
			pIPropertyBag->Release();
		}

		// Get display name
		CString szDisplayName("");
		LPOLESTR wszDisplayName = NULL;
		if(SUCCEEDED(pIMoniker->GetDisplayName(NULL, NULL, &wszDisplayName)))
		{
			int nBufferLength = ::WideCharToMultiByte(CP_ACP, 0u, wszDisplayName, -1, 
				NULL, 0, NULL, NULL);
			::WideCharToMultiByte(CP_ACP, 0u, wszDisplayName, -1, 
				szDisplayName.GetBuffer(nBufferLength), nBufferLength, NULL, NULL);
			szDisplayName.ReleaseBuffer();

			// Free memory
			IMalloc *pIMalloc = NULL;
			if(SUCCEEDED(::CoGetMalloc(1u, &pIMalloc)))
			{
				pIMalloc->Free(wszDisplayName);
				pIMalloc->Release();
			}
		}

		// Generate device name
		int nDeviceId = 1;
		BOOL bNoMacth = TRUE;
		DWORD dwBufferLength = 100u;
		CString szKeyName("");
		CString szRetDisplayName("");
		while(1)
		{
			// Get "nDeviceId"th(/st/rd) recorded display name
			szKeyName.Format("Device_Instance_%d", nDeviceId);
			while(1)
			{
				DWORD dwRet = ::GetPrivateProfileString(szFriendlyName, szKeyName, "", 
					szRetDisplayName.GetBuffer((int)dwBufferLength), dwBufferLength, 
					m_szIniPath);
				szRetDisplayName.ReleaseBuffer();
				if(dwRet + 1u < dwBufferLength)
				{
					bNoMacth = !dwRet;
					break;
				}
				else
				{
					dwBufferLength *= 2;
				}
			}
			if(bNoMacth)
			{
				// No macth found, add new instance data
				szDisplayName.Insert(0, '\"');
				szDisplayName.Insert(szDisplayName.GetLength(), '\"');
				::WritePrivateProfileString(szFriendlyName, szKeyName, 
					szDisplayName, m_szIniPath);
				break;
			}
			else if(szDisplayName == szRetDisplayName)
			{
				// A former registered device has been found
				break;
			}
			else
			{
				nDeviceId++;
			}
		}
		szRet.Format("%s #%d", szFriendlyName, nDeviceId);
	}
	return szRet;
}

void CEnumDeviceDlg::OnSelchangeCombodevice() 
{
	// TODO: Add your control notification handler code here
	// Set OK button state
	if(m_comboDevice.GetCurSel() < 0)
	{
		if(m_btnOK.IsWindowEnabled())
		{
			m_btnOK.EnableWindow(FALSE);
		}
	}
	else
	{
		if(!m_btnOK.IsWindowEnabled())
		{
			m_btnOK.EnableWindow();
		}
	}	
}
