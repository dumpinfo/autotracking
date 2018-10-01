// viewbuttonView.cpp : implementation of the CViewbuttonView class
//

#include "stdafx.h"
#include "viewbutton.h"

#include "viewbuttonDoc.h"
#include "viewbuttonView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "algorithm")

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonView

IMPLEMENT_DYNCREATE(CViewbuttonView, CView)

BEGIN_MESSAGE_MAP(CViewbuttonView, CView)
	//{{AFX_MSG_MAP(CViewbuttonView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_CHECK_SIGNLE,OnCheckSingle)
	ON_BN_CLICKED(IDC_CHECK_AUTO,OnCheckAuto)
	ON_WM_SIZE()
	ON_COMMAND(IDM_SAVEPTZSTATE, OnSaveptzstate)
	ON_COMMAND(IDM_READPTZSTATE, OnReadptzstate)
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_UPDATE_COMMAND_UI(IDM_READPTZSTATE, OnUpdateReadptzstate)
	ON_UPDATE_COMMAND_UI(IDM_SAVEPTZSTATE, OnUpdateSaveptzstate)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_MESSAGE(VS_COMMAND, OnVSCommand)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_MESSAGE(VS_MEDIAEVENT, OnMediaEvent)
	ON_MESSAGE(VS_UPDATECOMMANDUI, OnVSUpdateCommandUI)
	ON_MESSAGE(PTZ_MESSAGE, OnPTZMessage)
	ON_MESSAGE(BUTTON_MESSAGE, OnButtonMessage)
	ON_MESSAGE(ALGORITHM_MESSAGE, OnAlgorithmMessage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonView construction/destruction

CViewbuttonView::CViewbuttonView()
{
	// TODO: add construction code here
	// Load background brush
	CBitmap bmpBackground;
	BOOL bl = bmpBackground.LoadBitmap(IDB_BITMAP1);
	bl = m_brushBackground.CreatePatternBrush(&bmpBackground);
	bmpBackground.DeleteObject();
	m_pDC = NULL;
	m_bShowVideo = false;
	m_bBegin = false;
	m_pbtnPTZup = NULL;
	m_pbtnPTZdown = NULL;
	m_pbtnPTZleft = NULL;
	m_pbtnPTZright = NULL;
	m_pbtnPTZZoomOut = NULL;
	m_pbtnPTZZoomIn = NULL;
	m_bSingleShow = false;
	m_bPTZAuto    = false;
	m_szwFileName = NULL;
	m_rtDisplay.right = 320;
	m_rtDisplay.left = 0;
	m_rtDisplay.top = 0;
	m_rtDisplay.bottom = 240;

	m_bBuffering = false;

	pCamera = NULL;
	pVISCAinter = NULL;

	m_bCapture = false;
	m_bVideoFile = false;
	m_bRunning = false;

	m_pInterface = new CInterface();

	m_pResult = NULL;
	m_pInFrame  = NULL;
}

CViewbuttonView::~CViewbuttonView()
{

	if (m_pbtnPTZup) delete m_pbtnPTZup;
	if (m_pbtnPTZdown) delete m_pbtnPTZdown;
	if (m_pbtnPTZleft) delete m_pbtnPTZleft;
	if (m_pbtnPTZright) delete m_pbtnPTZright;
	if (m_pbtnPTZZoomOut) delete m_pbtnPTZZoomOut;
	if (m_pbtnPTZZoomIn) delete m_pbtnPTZZoomIn;
	if (m_pResult) delete m_pResult;
	if (m_pInFrame) delete m_pInFrame;

	if(pVISCAinter)
		VISCA_close_serial(pVISCAinter);

	if (m_pDC) delete m_pDC;
	if (pCamera) delete pCamera;
	if (pVISCAinter) delete pVISCAinter;
}

BOOL CViewbuttonView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	 cs.style =  cs.style | WS_CLIPCHILDREN;
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonView drawing

void CViewbuttonView::OnDraw(CDC* pDC)
{
	// Draw current frame
	CRect rtDisplay = m_rtDisplay;
	int width       = rtDisplay.Width();
	int height      = rtDisplay.Height();

	CRect recttemp;
	m_ClistBox.GetClientRect(&recttemp);

	CRect rtClient;
	GetClientRect(rtClient);
	

	double ratio       = m_rtDisplay.Height()/(123.+m_rtDisplay.Height());
	rtDisplay.left     = (rtClient.Width()-recttemp.Width())/2-m_rtDisplay.Width()/2;
	rtDisplay.right    = (rtClient.Width()-recttemp.Width())/2-m_rtDisplay.Width()/2+m_rtDisplay.Width();
	rtDisplay.top      = ratio*rtClient.Height()/2 - m_rtDisplay.Height()/2;
	rtDisplay.bottom   = ratio*rtClient.Height()/2 - m_rtDisplay.Height()/2 + m_rtDisplay.Height();

	pDC->LPtoDP(rtDisplay);

//	m_cVideo.PaintVideoFrame(rtDisplay);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonView printing

BOOL CViewbuttonView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CViewbuttonView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CViewbuttonView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CViewbuttonView diagnostics

#ifdef _DEBUG
void CViewbuttonView::AssertValid() const
{
	CView::AssertValid();
}

void CViewbuttonView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CViewbuttonDoc* CViewbuttonView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CViewbuttonDoc)));
	return (CViewbuttonDoc*)m_pDocument;
}
#endif //_DEBUG


int CViewbuttonView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_pbtnPTZup = new CButtonST;
	HRGN r= ::CreateEllipticRgn(0,0,41,41);
	
	m_pbtnPTZup->Create(_T(""), 
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 
						CRect(0, 0, 41, 41), this, IDC_PTZUP);
	// Set the same font of the application
	m_pbtnPTZup->SetFont(GetFont());
	m_pbtnPTZup->DrawTransparent(true);
		
//	HRGN r= ::CreateEllipticRgn(0,0,41,41);
	m_pbtnPTZup->SetWindowRgn(r,true);
	m_pbtnPTZup->SetBitmaps(IDB_PTZup_BUTTONDOWN,0,IDB_PTZup);
	m_pbtnPTZup->SetFlat();
	m_pbtnPTZup->SetAlign(ST_ALIGN_OVERLAP);
	m_pbtnPTZup->SizeToContent();
	m_pbtnPTZup->SetWindowRgn(r,true);	
	CRgn * rup = new CRgn;
	rup->CreateEllipticRgn( 0,0,41,41 );
    m_pbtnPTZup->SetRgn1(rup);
    m_pbtnPTZup->DrawBorder(FALSE);

	m_pbtnPTZdown = new CButtonST;
	m_pbtnPTZdown->Create(_T(""), 
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP , 
						CRect(40, 40, 70, 70), this, IDC_PTZDOWN);
	// Set the same font of the application
	m_pbtnPTZdown->SetFont(GetFont());
	r= CreateEllipticRgn(0,0,40,40);
	m_pbtnPTZdown->SetWindowRgn(r,true);
	m_pbtnPTZdown->DrawTransparent(true);
	m_pbtnPTZdown->SetBitmaps(IDB_PTZdown_BUTTONDOWN,0,IDB_PTZdown);
	m_pbtnPTZdown->SetFlat();
	m_pbtnPTZdown->SetAlign(ST_ALIGN_OVERLAP);
	m_pbtnPTZdown->SizeToContent();
	CRgn * rdown = new CRgn;
	rdown->CreateEllipticRgn( 0,0,41,41 );
    m_pbtnPTZdown->SetRgn1(rdown);
    m_pbtnPTZdown->DrawBorder(FALSE);

	m_pbtnPTZleft = new CButtonST;
	m_pbtnPTZleft->Create(_T(""), 
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 
						CRect(80, 80, 110, 110), this, IDC_PTZLEFT);
	// Set the same font of the application
	m_pbtnPTZleft->SetFont(GetFont());
	m_pbtnPTZleft->DrawTransparent(true);
	m_pbtnPTZleft->SetBitmaps(IDB_PTZleft_BUTTONDOWN,0,IDB_PTZleft);
	m_pbtnPTZleft->SetFlat();
	m_pbtnPTZleft->SetAlign(ST_ALIGN_OVERLAP);
	m_pbtnPTZleft->SizeToContent();
	CRgn * rleft = new CRgn;
	rleft->CreateEllipticRgn( 0,0,41,41 );
    m_pbtnPTZleft->SetRgn1(rleft);
	m_pbtnPTZleft->DrawBorder(FALSE);

	m_pbtnPTZright = new CButtonST;
	m_pbtnPTZright->Create(_T(""), 
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 
						CRect(120, 120, 150, 150), this, IDC_PTZRIGHT);
	// Set the same font of the application
	m_pbtnPTZright->SetFont(GetFont());	
	m_pbtnPTZright->DrawTransparent(true);
	m_pbtnPTZright->SetBitmaps(IDB_PTZright_BUTTONDOWN,0,IDB_PTZright);
	m_pbtnPTZright->SetFlat();
	m_pbtnPTZright->SetAlign(ST_ALIGN_OVERLAP);
	m_pbtnPTZright->SizeToContent();
	CRgn * rright = new CRgn;
	rright->CreateEllipticRgn( 0,0,41,41 );
    m_pbtnPTZright->SetRgn1(rright);
	m_pbtnPTZright->DrawBorder(FALSE);

	m_pbtnPTZZoomOut = new CButtonST;
	m_pbtnPTZZoomOut->Create(_T(""), 
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 
						CRect(220, 220, 32, 32), this, IDC_ZOOMOUT);
	// Set the same font of the application
	m_pbtnPTZZoomOut->SetFont(GetFont());	
	m_pbtnPTZZoomOut->DrawTransparent(true);
	m_pbtnPTZZoomOut->SetBitmaps(IDB_ZOOMOUT,0,IDB_ZOOMOUT1);
	m_pbtnPTZZoomOut->SetFlat();
	m_pbtnPTZZoomOut->SetAlign(ST_ALIGN_OVERLAP);
	m_pbtnPTZZoomOut->SizeToContent();

	m_pbtnPTZZoomIn = new CButtonST;
	m_pbtnPTZZoomIn->Create(_T(""), 
						WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, 
						CRect(320, 320, 32, 32), this, IDC_ZOOMIN);
	// Set the same font of the application
	m_pbtnPTZZoomIn->SetFont(GetFont());	
	m_pbtnPTZZoomIn->DrawTransparent(true);
	m_pbtnPTZZoomIn->SetBitmaps(IDB_ZOOMIN,0,IDB_ZOOMIN1);
	m_pbtnPTZZoomIn->SetFlat();
	m_pbtnPTZZoomIn->SetAlign(ST_ALIGN_OVERLAP);
	m_pbtnPTZZoomIn->SizeToContent();


//	m_pbtnSingle = new CButton;
//	m_pbtnSingle->Create(_T("单屏显示"), WS_CHILD|WS_VISIBLE| WS_GROUP| WS_TABSTOP, 
//		CRect(100,250,100,130), this, IDC_CHECK_SIGNLE);

//	m_pbtnSingle->SetFont(GetFont());	
//	m_pbtnSingle->DrawTransparent(true);
//	m_pbtnSingle->SetBitmaps(IDB_ZOOMIN,0,IDB_ZOOMIN1);
//	m_pbtnSingle->SetFlat();
//	m_pbtnSingle->SetAlign(ST_ALIGN_OVERLAP);
//	m_pbtnSingle->SizeToContent();

	m_pDC = new CClientDC(this);


//	HRGN rrgn;
//	rrgn=::CreateRectRgn(0,0,0,0);
//	CombineRgn(rrgn,m_cRgn.operator HRGN(),0,RGN_COPY);
//	CRect rtClient;
//	GetClientRect(rtClient);


	m_bBegin = true;

	FILE * pFT;
	pFT = fopen("res\\Transformer.txt","r");
	int pointnum;
	fscanf(pFT,"%d ", &pointnum);
	int offsetX, offsetY;
	fscanf(pFT,"%d %d ",&offsetX,&offsetY);
	int x,y;
	fscanf(pFT,"%d %d ",&x,&y);
	SetCapture();
	m_pDC->BeginPath();
	m_pDC->MoveTo(CPoint(x,y));
	for(int i = 1; i<pointnum; i++)
	{
		fscanf(pFT,"%d %d ",&x,&y);
		m_pDC->LineTo(CPoint(x,y));
//		m_pDC->MoveTo(CPoint(x,y));
	}
	m_pDC->EndPath();
	m_cRgn.CreateFromPath(m_pDC);

	m_cRgn.OffsetRgn(offsetX, offsetY);

	ReleaseCapture();
	fclose(pFT);


	m_ClistBox.Create(WS_CHILD | WS_VISIBLE | LBS_WANTKEYBOARDINPUT,CRect(300, 0, 350, 150),this,IDC_LIST);
//	m_pbtnSingle = new CButtonST;



	m_pbtnSingle.Create(_T("单屏显示"), WS_CHILD|WS_VISIBLE| WS_GROUP|BS_CHECKBOX, 
		CRect(100,250,250,130), this, IDC_CHECK_SIGNLE);
	m_pbtnSingle.SetFont(GetFont());
	m_pbtnSingle.SetCheck(0);

	m_pbtnPTZAuto.Create(_T("自动转动"), WS_CHILD|WS_VISIBLE| WS_GROUP|BS_CHECKBOX, 
		CRect(200,250,250,130), this, IDC_CHECK_AUTO);
	m_pbtnPTZAuto.SetFont(GetFont());	
	m_pbtnPTZAuto.SetCheck(0);

//	m_StaticDisplay.Create(_T(""),WS_CHILD|WS_VISIBLE| WS_GROUP,CRect(0,0,50,130),this,IDC_DISP);

	return 0;
}

BOOL CViewbuttonView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	CRect rtClient;
	CPoint ptBrushOrg(0, 0);
	OnPrepareDC(pDC);
	GetClientRect(rtClient);
	pDC->DPtoLP(rtClient);
	pDC->LPtoDP(&ptBrushOrg);
	pDC->SetBrushOrg(ptBrushOrg);
	m_brushBackground.UnrealizeObject();
//	pDC->ExcludeClipRect(m_rtDisplay);

	pDC->FillRect(rtClient, &m_brushBackground);
//	pDC->SelectClipRgn(NULL, RGN_COPY);	
	if(!m_bShowVideo)
	{
		CBitmap bmp;
	   if (bmp.LoadBitmap(IDB_TRANSFORMER))
	   {
		  // Get the size of the bitmap
		  BITMAP bmpInfo;
		  bmp.GetBitmap(&bmpInfo);

		  // Create an in-memory DC compatible with the
		  // display DC we're using to paint
		  CDC dcMemory;
		  dcMemory.CreateCompatibleDC(pDC);

		  // Select the bitmap into the in-memory DC
		  CBitmap* pOldBitmap = dcMemory.SelectObject(&bmp);

		  // Find a centerpoint for the bitmap in the client area
		  CRect rect;
		  GetClientRect(&rect);
		  CRect recttemp;
		  m_ClistBox.GetClientRect(&recttemp);
		  int nX = rect.left + (rect.Width() - bmpInfo.bmWidth - recttemp.Width()) / 2;
		  int nY = rect.top + ((m_rtDisplay.Height()/(123.+m_rtDisplay.Height())*rect.Height()) - bmpInfo.bmHeight) / 2;

		  // Copy the bits from the in-memory DC into the on-
		  // screen DC to actually do the painting. Use the centerpoint
		  // we computed for the target offset.
		  if(m_bBegin)
		  {



			  int x = (rect.Width() - recttemp.Width())/2;
			  int y = (m_rtDisplay.Height()/(123.+m_rtDisplay.Height())*rect.Height())/2;
			  int nOffsetResult = m_cRgn.OffsetRgn(x,y);
			  ASSERT( nOffsetResult != ERROR || nOffsetResult != NULLREGION );

			  pDC->SelectClipRgn(&m_cRgn,RGN_AND);
			  m_cRgn.OffsetRgn(-x,-y);
			  
		  }
		  pDC->BitBlt(nX, nY, bmpInfo.bmWidth, bmpInfo.bmHeight, &dcMemory, 
			 0, 0, SRCCOPY);
//			m_bBegin = true;
		  dcMemory.SelectObject(pOldBitmap);
	   }
		
	}
	return	TRUE;
}

void CViewbuttonView::OnCheckSingle()
{
	m_bSingleShow = !m_pbtnSingle.GetCheck();
	m_pbtnSingle.SetCheck(m_bSingleShow);
	

}

void CViewbuttonView::OnCheckAuto()
{
	m_bPTZAuto = !m_pbtnPTZAuto.GetCheck();
	m_pbtnPTZAuto.SetCheck(m_bPTZAuto);
}

void CViewbuttonView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	CRect rtClient;
	GetClientRect(rtClient);	
	// TODO: Add your message handler code here
	m_ClistBox.MoveWindow(rtClient.Width()*0.75,0,rtClient.Width()*0.25,rtClient.Height()+5);
	m_pbtnSingle.MoveWindow(200,200,90,20);
	m_pbtnPTZAuto.MoveWindow(200,300,90,20);
	CRect recttemp;
	m_ClistBox.GetClientRect(&recttemp);
	double ratio = m_rtDisplay.Height()/(123.+m_rtDisplay.Height());
	int relativex = (rtClient.Width()-recttemp.Width())/2;
	int relativey = ratio*rtClient.Height();
	m_pbtnPTZdown->MoveWindow(relativex-20,relativey+(rtClient.Height()-relativey)/2.0+20,41,41);
	m_pbtnPTZup->MoveWindow(relativex-20,relativey+(rtClient.Height()-relativey)/2.0-20-41,41,41);
	m_pbtnPTZleft->MoveWindow(relativex-20-41,relativey+(rtClient.Height()-relativey)/2-20,41,41);
	m_pbtnPTZright->MoveWindow(relativex+20,relativey+(rtClient.Height()-relativey)/2.0-20,41,41);

	m_pbtnPTZZoomIn->MoveWindow((relativex-61)/2-7,
		relativey+(rtClient.Height()-relativey)/2-32.4-(rtClient.Height()-relativey)/6,32,32);
	m_pbtnPTZZoomOut->MoveWindow((relativex-61)/2-7,
		relativey+(rtClient.Height()-relativey)/2.0+(rtClient.Height()-relativey)/6.0-13.5,32,32);

	m_pbtnPTZAuto.MoveWindow(relativex*2-(relativex-61)/2-45, 
		relativey+(rtClient.Height()-relativey)/2-10-(rtClient.Height()-relativey)/6,90,20);
	m_pbtnSingle.MoveWindow(relativex*2-(relativex-61)/2-45, 
		relativey+(rtClient.Height()-relativey)/2+(rtClient.Height()-relativey)/6,90,20);
	
//	m_StaticDisplay.MoveWindow(0,0,m_rtDisplay.Width(),m_rtDisplay.Height());

}

void CViewbuttonView::OnSaveptzstate() 
{
	// TODO: Add your command handler code here
	m_cPTZControl.ReadPTZState();
}

void CViewbuttonView::OnReadptzstate() 
{
	// TODO: Add your command handler code here
	m_cPTZControl.SetPTZState();
}

LRESULT CViewbuttonView::OnVSCommand(WPARAM wParam, LPARAM lParam)
{
	// Process messages
	switch(wParam)
	{
		case IDM_OPEN: OnOpen(); break;
		case IDM_CAPTURE: OnCapture(); break;
		case IDM_ADJUST: OnAdjust(); break;
		case IDM_SETTING: OnSetting(); break;
		case IDM_PLAY: OnPlay(); break;
		case IDM_PAUSE: OnPause(); break;
		case IDM_STOP: OnStop(); break;
		case IDM_RULE: ToggleRuleEditor(); break;
		case IDM_TOGGLEALGO: ToggleAlgorithmActivation(); break;
		case IDM_STEP: OnStep(); break;
		case IDM_PURGE: OnPurge(); break;
	}

	return 0L;
}

void CViewbuttonView::OnOpen()
{

	KillTimer(1);
	CFileDialog dlgFile(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, 
		"媒体文件(所有类型)|*.asf;*.wm;*.wmv;*.avi;*.mpeg;*.mpg|\
		Windows Media 文件(asf) (*.asf;*.wm;*.wmv)|*.asf;*.wm;*.wmv|\
		Windows 视频文件(avi) (*.avi;*.wmv)|*.avi;*.wmv|\
		电影文件(mpeg) (*.mpeg;*mpg)|*.mpeg;*mpg|\
		所有文件(*.*)|*.*||", GetSafeOwner());
	if(dlgFile.DoModal() != IDOK)
	{
		return;
	}
	m_bVideoFile = false;
	int dwBufferLength;
	if(m_szwFileName)
	{
		delete m_szwFileName;
	}
	m_szwFileName = new WCHAR[dwBufferLength = 
		::MultiByteToWideChar(CP_ACP, 0u, dlgFile.GetFileName(), -1, NULL, 0)];
	memset(m_szwFileName, 0, sizeof(WCHAR) * dwBufferLength);
	::MultiByteToWideChar(CP_ACP, 0u, dlgFile.GetFileName(), -1, 
		m_szwFileName, dwBufferLength);
	CClientDC dc(this);
	int nBPP = dc.GetDeviceCaps(BITSPIXEL);
	if(FAILED(m_cVideo.OpenFile(m_szwFileName, GetSafeHwnd(),nBPP,&m_rtDisplay)))
	{
		if(pVISCAinter)
		{
			VISCA_close_serial(pVISCAinter);
			if(pVISCAinter)
			{
				delete pVISCAinter;
				pVISCAinter = NULL;
			}
			if(pCamera)
			{
				delete pCamera;
				pCamera = NULL;
			}
		}

		m_cVideo.ReleaseFilterGraph();
		m_bBuffering = false;
		// Show error message
		if(AfxGetMainWnd()->GetStyle() & WS_MINIMIZE)
		{
			AfxGetMainWnd()->ShowWindow(SW_SHOWNOACTIVATE);
		}
		GetSafeOwner()->SetForegroundWindow();
		GetSafeOwner()->MessageBox("打开视频源失败！", NULL, MB_ICONERROR | MB_OK);	
	}
	else
	{
		// Modify window title
		CString szTitle("");
		szTitle.LoadString(IDR_MAINFRAME);
		szTitle += " - ";
		szTitle += dlgFile.GetFileName();
		GetParentFrame()->SetWindowText(szTitle);


		// Force to update cursor display
		CRect rtClient;
		CPoint ptCursor;
		GetClientRect(rtClient);
		ClientToScreen(rtClient);
		::GetCursorPos(&ptCursor);
		if(WindowFromPoint(ptCursor) == this && rtClient.PtInRect(ptCursor))
		{
			SendMessage(WM_SETCURSOR, (WPARAM)GetSafeHwnd(), 
				MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
		}

		// Force to update toolbar state
		AfxGetApp()->OnIdle(0L);
		SetTimer(1,50,NULL);
		m_bCapture = false;
		m_bVideoFile = true;
		m_Width = m_rtDisplay.Width();
		m_Height = m_rtDisplay.Height();


		InitBitmap(m_Width, m_Height);
		m_pInterface->Stop();
		m_pInterface->Start(m_Width, m_Height, GetSafeHwnd());
	}
	
}

void CViewbuttonView::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent==1)
	{
		LONG lCurrent = 0L, lStop = 0L;
		LONGLONG llCurrent = (LONGLONG)0, llStop = (LONGLONG)0;	
		if(m_cVideo.GetPosition(&lCurrent, &lStop))
		{
			GetParentFrame()->SendMessage(VS_UPDATEPOSITIONS, 
				(WPARAM)lCurrent, (LPARAM)(lStop));		
		}
		else
		{
			GetParentFrame()->SendMessage(VS_UPDATEPOSITIONS, (WPARAM)-1, (LPARAM)-1);
		}

		int iFPS;
		if(m_cVideo.GetFPS(&iFPS))
		{
			GetParentFrame()->SendMessage(VS_UPDATEFPS, (WPARAM)iFPS);
		}
		else
		{
			GetParentFrame()->SendMessage(VS_UPDATEFPS, (WPARAM)-1);
		}


		// Monitor current cursor position
		CPoint ptCursor;
		CClientDC dc(this);
		CRect rtClient, rtDisplay;

		
		OnPrepareDC(&dc);
		::GetCursorPos(&ptCursor);
		GetClientRect(rtClient);
		ClientToScreen(rtClient);

		double ratio       = m_rtDisplay.Height()/(123.+m_rtDisplay.Height());
		CRect recttemp;
		m_ClistBox.GetClientRect(&recttemp);
		rtDisplay.left     = (rtClient.Width()-recttemp.Width())/2-m_rtDisplay.Width()/2;
		rtDisplay.right    = (rtClient.Width()-recttemp.Width())/2-m_rtDisplay.Width()/2+m_rtDisplay.Width();
		rtDisplay.top      = ratio*rtClient.Height()/2 - m_rtDisplay.Height()/2;
		rtDisplay.bottom   = ratio*rtClient.Height()/2 - m_rtDisplay.Height()/2 + m_rtDisplay.Height();
		


		dc.LPtoDP(rtDisplay);
		ClientToScreen(rtDisplay);
		if(WindowFromPoint(ptCursor) == this && rtClient.PtInRect(ptCursor) && 
			rtDisplay.PtInRect(ptCursor))
		{
			GetParentFrame()->SendMessage(VS_UPDATECURSORPOS, 
				(WPARAM)(ptCursor.x - rtDisplay.left), 
				(LPARAM)(ptCursor.y - rtDisplay.top));
		}
		else
		{
			GetParentFrame()->SendMessage(VS_UPDATECURSORPOS, 
				(WPARAM)-1, (LPARAM)-1);
		}
		
		if(m_pInFrame)
		{
			m_cVideo.ObtainFrame(m_pInFrame);
			m_pInterface->RecieveFrames(m_pInFrame,m_Width, m_Height);
		}
	}
	else if(nIDEvent==2)
	{
		KillTimer(2);
		m_cPTZControl.PTZStop();
	}
	else if(nIDEvent==3)
	{
		KillTimer(3);
		m_cPTZControl.PTZStop();
	}
	CView::OnTimer(nIDEvent);
}

void CViewbuttonView::Display()
{
/*	CRect rtDisplay = m_rtDisplay;
	int width       = rtDisplay.Width();
	int height      = rtDisplay.Height();

	CRect recttemp;
	m_ClistBox.GetClientRect(&recttemp);

	CRect rtClient;
	GetClientRect(rtClient);
	

	double ratio       = m_rtDisplay.Height()/(123.+m_rtDisplay.Height());
	rtDisplay.left     = (rtClient.Width()-recttemp.Width())/2-m_rtDisplay.Width()/2;
	rtDisplay.right    = (rtClient.Width()-recttemp.Width())/2-m_rtDisplay.Width()/2+m_rtDisplay.Width();
	rtDisplay.top      = ratio*rtClient.Height()/2 - m_rtDisplay.Height()/2;
	rtDisplay.bottom   = ratio*rtClient.Height()/2 - m_rtDisplay.Height()/2 + m_rtDisplay.Height();
	m_cVideo.PaintVideoFrame(rtDisplay);
*/
}

void CViewbuttonView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rtDisplay = m_rtDisplay;
	int width       = rtDisplay.Width();
	int height      = rtDisplay.Height();

	CRect recttemp;
	m_ClistBox.GetClientRect(&recttemp);

	CRect rtClient;
	GetClientRect(rtClient);
	

	double ratio       = m_rtDisplay.Height()/(123.+m_rtDisplay.Height());
	rtDisplay.left     = (rtClient.Width()-recttemp.Width())/2-m_rtDisplay.Width();
	rtDisplay.right    = (rtClient.Width()-recttemp.Width())/2-m_rtDisplay.Width()+m_rtDisplay.Width();
	rtDisplay.top      = ratio*rtClient.Height()/2 - m_rtDisplay.Height()/2;
	rtDisplay.bottom   = ratio*rtClient.Height()/2 - m_rtDisplay.Height()/2 + m_rtDisplay.Height();
	m_cVideo.PaintVideoFrame(rtDisplay);	

	rtDisplay.left     = rtDisplay.left + m_rtDisplay.Width();
	rtDisplay.right    = rtDisplay.right + m_rtDisplay.Width();

	if(m_pResult)
	{
		m_pInterface->OutputFrames(NULL, m_pResult);
		StretchDIBits(dc.m_hDC, rtDisplay.left, rtDisplay.top,
			m_Width, m_Height, 0,0,
			m_Width, m_Height, m_pResult, &m_Bmi, DIB_RGB_COLORS, SRCCOPY);
	}


	// TODO: Add your message handler code here
	
	// Do not call CView::OnPaint() for painting messages
}

void CViewbuttonView::OnCapture()
{
	// Show device enumeration dialog

	CEnumDeviceDlg dlg(GetSafeOwner());
	if(dlg.DoModal() != IDOK || !dlg.m_pISelMoniker)
	{
		return;
	}
	m_bCapture = false;
	CClientDC dc(this);
	int nBPP = dc.GetDeviceCaps(BITSPIXEL);



	if(m_cVideo.CaptureVideo(dlg.m_pISelMoniker,GetSafeHwnd(),nBPP,&m_rtDisplay))
	{
		// Modify window title
		CString szTitle("");
		szTitle.LoadString(IDR_MAINFRAME);
		szTitle += " - 实时采集";
		GetParentFrame()->SetWindowText(szTitle);

		if(pVISCAinter)
		{
			VISCA_close_serial(pVISCAinter);
			if(pVISCAinter)
			{
				delete pVISCAinter;
				pVISCAinter = NULL;
			}
			if(pCamera)
			{
				delete pCamera;
				pCamera = NULL;
			}
		}
		// Force to update cursor display
		CRect rtClient;
		CPoint ptCursor;
		GetClientRect(rtClient);
		ClientToScreen(rtClient);
		::GetCursorPos(&ptCursor);
		if(::WindowFromPoint(ptCursor) == GetSafeHwnd() && rtClient.PtInRect(ptCursor))
		{
			SendMessage(WM_SETCURSOR, (WPARAM)GetSafeHwnd(), 
				MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
		}

		// Force to update toolbar state
		AfxGetApp()->OnIdle(0L);
		SetTimer(1,50,NULL);

//		PTZcomIntialize();
		m_cPTZControl.ReleasePTZ();
		m_cPTZControl.Initialize(this);
		m_bCapture = true;
		m_bVideoFile =  false;
		m_Width = m_rtDisplay.Width();
		m_Height = m_rtDisplay.Height();
		InitBitmap(m_Width, m_Height);
		m_pInterface->Stop();
		m_pInterface->Start(m_Width, m_Height, GetSafeHwnd());
	}
	else
	{
		// Show error message
		if(AfxGetMainWnd()->GetStyle() & WS_MINIMIZE)
		{
			AfxGetMainWnd()->ShowWindow(SW_SHOWNOACTIVATE);
		}
		GetSafeOwner()->SetForegroundWindow();
		GetSafeOwner()->MessageBox("打开视频源失败！", NULL, MB_ICONERROR | MB_OK);	
	}
	if(dlg.m_pISelMoniker)
	{
		dlg.m_pISelMoniker->Release();
	}
}

void CViewbuttonView::OnAdjust()
{

}

void CViewbuttonView::OnSetting()
{

}

void CViewbuttonView::OnPlay()
{

	m_cVideo.Play();

	// Force to update toolbar state
	AfxGetApp()->OnIdle(0L);
}

void CViewbuttonView::OnPause()
{

	m_cVideo.Pause();

	// Force to update toolbar state
	AfxGetApp()->OnIdle(0L);
}

void CViewbuttonView::OnStop()
{

	m_cVideo.Stop();

	// Force to update toolbar state
	AfxGetApp()->OnIdle(0L);
}

void CViewbuttonView::ToggleRuleEditor()
{

}

void CViewbuttonView::ToggleAlgorithmActivation()
{

}

void CViewbuttonView::OnStep()
{
	m_cVideo.Step();
}

void CViewbuttonView::OnPurge()
{
	m_cVideo.ReleaseFilterGraph();
	m_bBuffering = false;
	KillTimer(1);
	GetParentFrame()->SendMessage(VS_UPDATEPOSITIONS, (WPARAM)-1, (LPARAM)-1);
	GetParentFrame()->SendMessage(VS_UPDATEFPS, (WPARAM)-1);
	GetParentFrame()->SendMessage(VS_UPDATECURSORPOS, 
				(WPARAM)-1, (LPARAM)-1);


	m_cPTZControl.ReleasePTZ();
	if(m_pResult)
	{
		delete m_pResult;
		m_pResult =	NULL;
	}
	if(m_pInFrame)
	{
		delete m_pInFrame;
		m_pInFrame =	NULL;
	}
}

LRESULT CViewbuttonView::OnMediaEvent(WPARAM wParam, LPARAM lParam)
{
	BOOL bError = FALSE;
	BOOL bComplete = FALSE;
	BOOL bStepComplete = FALSE;
	BOOL bFormerBuffering = m_bBuffering;
	long lEventCode = 0L, lParam1 = 0L, lParam2 = 0L;
	while(m_cVideo.GetMediaEvent(&lEventCode, &lParam1, &lParam2))
	{
		if(lEventCode == EC_BUFFERING_DATA)
		{
			m_bBuffering = (BOOL)lParam1;
		}
		else if(lEventCode == EC_STEP_COMPLETE)
		{
			bStepComplete = TRUE;
		}
		else if(lEventCode == EC_COMPLETE || lEventCode == EC_USERABORT)
		{
			bComplete = TRUE;
		}
		else if(lEventCode == EC_ERRORABORT || lEventCode == EC_DEVICE_LOST)
		{
			bError = TRUE;
		}
		lEventCode = lParam1 = lParam2 = 0L;
	}
	if(bError)
	{
		// Stop playing and show error message
		m_cVideo.ReleaseFilterGraph();
		m_bBuffering = false;
		if(AfxGetMainWnd()->GetStyle() & WS_MINIMIZE)
		{
			AfxGetMainWnd()->ShowWindow(SW_SHOWNOACTIVATE);
		}
		GetSafeOwner()->SetForegroundWindow();
		GetSafeOwner()->MessageBox("播放或采集视频时发生错误！", NULL, 
			MB_ICONERROR | MB_OK);
	}
	else if(bComplete)
	{
		// Stop video and try to rewind it
		m_cVideo.Stop();
	}
	else if(bStepComplete)
	{
		// Pause video
		m_cVideo.Pause();
	}
	else if(m_bBuffering != bFormerBuffering)
	{
		// Force to update toolbar state
		AfxGetApp()->OnIdle(0L);
	}

	return 0L;
}


LRESULT CViewbuttonView::OnVSUpdateCommandUI(WPARAM wParam, LPARAM lParam)
{
	// Update command UI
	CCmdUI *pCmdUI = (CCmdUI *)lParam;
	if(pCmdUI)
	{
		switch(wParam)
		{
//		case IDM_ADJUST: OnUpdateAdjust(pCmdUI); break;
		case IDM_PLAY: OnUpdatePlay(pCmdUI); break;
		case IDM_PAUSE: OnUpdatePause(pCmdUI); break;
		case IDM_STOP: OnUpdateStop(pCmdUI); break;
//		case IDM_RULE: OnUpdateRule(pCmdUI); break;
		case IDS_PLAYERSTATE: OnUpdatePlayerState(pCmdUI); break;
//		case IDM_TOGGLEALGO: OnUpdateToggleAlgo(pCmdUI); break;
		case IDM_STEP: OnUpdateStep(pCmdUI); break;
		case IDM_PURGE: OnUpdatePurge(pCmdUI); break;
		}
	}

	return 0L;
}

void CViewbuttonView::OnUpdatePlayerState(CCmdUI *pCmdUI)
{
	if(m_bBuffering)
	{
		pCmdUI->SetText("正在缓冲");
	}
	else
	{
		HRESULT hr = E_FAIL;
		OAFilterState State = 0;
		if(m_cVideo.GetState(&State))
		{

			switch(State)
			{
				case State_Stopped: pCmdUI->SetText("已经停止"); hr = S_OK; break;
				case State_Paused: pCmdUI->SetText("已经暂停"); hr = S_OK; break;
				case State_Running: pCmdUI->SetText("正在播放"); hr = S_OK; break;
			}
		}
		else
		{
			pCmdUI->SetText("");
		}
	}
}

void CViewbuttonView::OnUpdatePause(CCmdUI *pCmdUI)
{
	BOOL bCanPause = FALSE;
	OAFilterState State = 0;
	if(m_cVideo.GetState(&State))
	{
		bCanPause = State == State_Running;
	}
	pCmdUI->Enable(bCanPause&m_bVideoFile);
}

void CViewbuttonView::OnUpdateStop(CCmdUI *pCmdUI)
{
	BOOL bCanStopped = FALSE;
	OAFilterState State = 0;
	if(m_cVideo.GetState(&State))
	{
		bCanStopped = State != State_Stopped;
	}
	pCmdUI->Enable(bCanStopped&m_bVideoFile);
}

void CViewbuttonView::OnUpdatePurge(CCmdUI *pCmdUI)
{
	OAFilterState State = 0;
	pCmdUI->Enable(m_cVideo.GetState(&State));
}

void CViewbuttonView::OnUpdatePlay(CCmdUI *pCmdUI)
{
	BOOL bCanPlay = FALSE;
	OAFilterState State = 0;
	if(m_cVideo.GetState(&State))
	{
		bCanPlay = State != State_Running;
	}
	pCmdUI->Enable(bCanPlay&m_bVideoFile);
}

void CViewbuttonView::OnUpdateStep(CCmdUI *pCmdUI)
{
	BOOL bCanStep = FALSE;
	if(m_cVideo.CanStep())
	{
		bCanStep = true;
	}
	pCmdUI->Enable(bCanStep&m_bVideoFile);
}

void CViewbuttonView::PTZcomIntialize()
{
	pCamera = new VISCACamera_t;
	pVISCAinter = new VISCAInterface_t;

    int camera_num;

    if (VISCA_open_serial(pVISCAinter, "COM1:")!=VISCA_SUCCESS)
	{
       ;
    }

    pVISCAinter->broadcast=0;
    VISCA_set_address(pVISCAinter, &camera_num);
    pCamera->address=1;
    VISCA_clear(pVISCAinter, pCamera);
 
    VISCA_get_camera_info(pVISCAinter, pCamera);

}

void CViewbuttonView::OnUpdateReadptzstate(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bCapture);
}

void CViewbuttonView::OnUpdateSaveptzstate(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_bCapture);
}

LRESULT CViewbuttonView::OnPTZMessage(WPARAM wParam, LPARAM lParam)
{
	int i = (int)wParam;
	CString str;
	if(i==1)
	{
		str.Format("SAVE DATA:");	
		m_ClistBox.InsertString( -1, str );
	}
	else
	{
		str.Format("READ DATA:");	
		m_ClistBox.InsertString( -1, str );
		if(lParam == -1)
		{
			m_ClistBox.InsertString(-1,"error!!!!!");
			return false;
		}
		
	}
	int iL, iT, iP, iZ;
	m_cPTZControl.ObtainData(&iL,&iT,&iP,&iZ);
	
	str.Format(("ID:%d, ZOOM:%d, "), iL, iZ);

	m_ClistBox.InsertString( -1, str );

	str.Format(("TILT:%d, PAN:%d."), iT, iP);
	m_ClistBox.InsertString( -1, str );
	return true;
}

LRESULT CViewbuttonView::OnAlgorithmMessage(WPARAM wParam, LPARAM lParam)
{
	int i = (int)wParam;

	if(i==-1)
	{
		m_ClistBox.InsertString( 0, "thread run" );
	}
	else
	{
		m_ClistBox.InsertString( -1, "thread end" );
	}
	return true;
}

LRESULT CViewbuttonView::OnButtonMessage(WPARAM wParam, LPARAM lParam)
{
	if((HIWORD(wParam)) == BUTTON_INACTIVE)
	{
		m_cPTZControl.PTZStop();
	}
	else
	{
		if((LOWORD(wParam)) == IDC_PTZUP)
		{
			m_cPTZControl.PTZUp();
//			m_pInterface->Start(1,1, GetSafeHwnd());
		}
		else if((LOWORD(wParam)) == IDC_PTZDOWN)
		{
			m_cPTZControl.PTZDown();
//			m_pInterface->Stop();
		}
		else if((LOWORD(wParam)) == IDC_PTZLEFT)
		{
			m_cPTZControl.PTZLeft();
		}
		else if((LOWORD(wParam)) == IDC_PTZRIGHT)
		{
			m_cPTZControl.PTZRight();
		}
		else if((LOWORD(wParam)) == IDC_ZOOMOUT)
		{
			m_cPTZControl.PTZZoomOut();
		}
		else if((LOWORD(wParam)) == IDC_ZOOMIN)
		{
			m_cPTZControl.PTZZoomIn();
		}
		m_bRunning  = true;
	}
	return true;
}

void CViewbuttonView::InitBitmap(int nWidth, int nHeight)
{
	m_Bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_Bmi.bmiHeader.biWidth = nWidth;
	m_Bmi.bmiHeader.biHeight = nHeight;
	m_Bmi.bmiHeader.biPlanes = 1;
	m_Bmi.bmiHeader.biBitCount = 24;
	m_Bmi.bmiHeader.biCompression = BI_RGB;
	m_Bmi.bmiHeader.biSizeImage = 0;
	m_Bmi.bmiHeader.biXPelsPerMeter = 0;//(GetDeviceCaps(hdc, HORZRES) * 1000) / GetDeviceCaps(hdc, HORZSIZE);
	m_Bmi.bmiHeader.biYPelsPerMeter = 0;//(GetDeviceCaps(hdc, VERTRES) * 1000) / GetDeviceCaps(hdc, VERTSIZE);
	m_Bmi.bmiHeader.biClrUsed = 0;
	m_Bmi.bmiHeader.biClrImportant = 0;

	if (m_pResult) 
	{
		delete m_pResult;
		m_pResult = NULL;
	}

	if (m_pInFrame) 
	{
		delete m_pInFrame;
		m_pInFrame = NULL;
	}


	if((nWidth%4)==0)
	{
		m_pResult = new unsigned char[nWidth*nHeight*3];
		m_pInFrame = new unsigned char[nWidth*nHeight*3];
	}
	else
	{
		m_pResult = new unsigned char[(nWidth/4+1)*4*nHeight*3];
		m_pInFrame = new unsigned char[(nWidth/4+1)*4*nHeight*3];
	}
}
