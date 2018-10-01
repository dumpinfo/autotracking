// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "viewbutton.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(IDM_OPEN, OnOpen)
	ON_COMMAND(IDM_ADJUST, OnAdjust)
	ON_COMMAND(IDM_CAPTURE, OnCapture)
	ON_COMMAND(IDM_PAUSE, OnPause)
	ON_COMMAND(IDM_PLAY, OnPlay)
	ON_COMMAND(IDM_STEP, OnStep)
	ON_COMMAND(IDM_STOP, OnStop)
	ON_COMMAND(IDM_PURGE, OnPurge)
	ON_UPDATE_COMMAND_UI(IDM_OPEN, OnUpdateOpen)
	ON_UPDATE_COMMAND_UI(IDM_PLAY, OnUpdatePlay)
	ON_UPDATE_COMMAND_UI(IDM_PAUSE, OnUpdatePause)
	ON_UPDATE_COMMAND_UI(IDM_PURGE, OnUpdatePurge)
	ON_UPDATE_COMMAND_UI(IDM_STOP, OnUpdateStop)
	ON_UPDATE_COMMAND_UI(IDM_STEP, OnUpdateStep)
	//}}AFX_MSG_MAP
	ON_MESSAGE(VS_UPDATEFPS, OnUpdateFPS)
	ON_MESSAGE(VS_UPDATEPOSITIONS, OnUpdatePositions)
	ON_MESSAGE(VS_UPDATECURSORPOS, OnUpdateCursorPosition)
	ON_UPDATE_COMMAND_UI(IDS_PLAYERSTATE, OnUpdatePlayerState)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	CBitmap bmpImageList;
	m_imglistToolBar.Create(48, 48, ILC_COLORDDB | ILC_MASK, 0, 1);
	m_imglistToolBarDisabled.Create(48, 48, ILC_COLORDDB | ILC_MASK, 0, 1);
	m_imglistToolBarHighLighted.Create(48, 48, ILC_COLORDDB | ILC_MASK, 0, 1);
	bmpImageList.LoadBitmap(IDB_TOOLBAR);
	m_imglistToolBar.Add(&bmpImageList, RGB(255, 255, 255));
	bmpImageList.DeleteObject();
	bmpImageList.LoadBitmap(IDB_TOOLBAR_DISABLED);
	m_imglistToolBarDisabled.Add(&bmpImageList, RGB(255, 255, 255));
	bmpImageList.DeleteObject();
	bmpImageList.LoadBitmap(IDB_TOOLBAR_HIGHLIGHTED);
	m_imglistToolBarHighLighted.Add(&bmpImageList, RGB(255, 255, 255));
	bmpImageList.DeleteObject();	
}

CMainFrame::~CMainFrame()
{
	m_imglistToolBar.DeleteImageList();
	m_imglistToolBarDisabled.DeleteImageList();
	m_imglistToolBarHighLighted.DeleteImageList();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	

	// Create status bar
	UINT Indicators[] = {ID_SEPARATOR, IDS_CURSORPOSITION, IDS_PLAYERPOSITION, 
		IDS_PLAYERSTATE, IDS_FPS};
	if(!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(Indicators, 5))
	{
		return -1;
	}

	// Initialize status bar
	UINT nID = 0u;
	UINT nStyle = 0u;
	int cxWidth = 0;
	m_wndStatusBar.GetPaneInfo(0, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(0, nID, nStyle, 0);
	m_wndStatusBar.GetPaneInfo(1, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(1, ID_SEPARATOR, nStyle, cxWidth);
	m_wndStatusBar.SetPaneText(1, "");
	m_wndStatusBar.GetPaneInfo(2, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(2, ID_SEPARATOR, nStyle, cxWidth);
	m_wndStatusBar.SetPaneText(2, "");
	m_wndStatusBar.SetPaneText(3, "");
	m_wndStatusBar.GetPaneInfo(4, nID, nStyle, cxWidth);
	m_wndStatusBar.SetPaneInfo(4, ID_SEPARATOR, nStyle, cxWidth);
	m_wndStatusBar.SetPaneText(4, "");
	

	// Create tool bar
	if(!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT, 
		CBRS_ALIGN_TOP | CBRS_FLYBY | CBRS_TOOLTIPS | WS_CHILD | WS_VISIBLE))
	{
		return -1;
	}

	// Initialize tool bar
	TBBUTTON Buttons[11];
	memset(Buttons, 0, sizeof(TBBUTTON) * 11);
	Buttons[0].fsState = TBSTATE_ENABLED;
	Buttons[0].fsStyle = TBSTYLE_BUTTON;
	Buttons[0].iBitmap = 0;
	Buttons[0].idCommand = IDM_OPEN;
	Buttons[1].fsState = TBSTATE_ENABLED;
	Buttons[1].fsStyle = TBSTYLE_BUTTON;
	Buttons[1].iBitmap = 1;
	Buttons[1].idCommand = IDM_CAPTURE;
	Buttons[2].fsState = TBSTATE_ENABLED;
	Buttons[2].fsStyle = TBSTYLE_BUTTON;
	Buttons[2].iBitmap = 2;
	Buttons[2].idCommand = IDM_PURGE;
	Buttons[3].fsStyle = TBSTYLE_SEP;
	Buttons[4].fsState = TBSTATE_ENABLED;
	Buttons[4].fsStyle = TBSTYLE_BUTTON;
	Buttons[4].iBitmap = 3;
	Buttons[4].idCommand = IDM_ADJUST;
	Buttons[5].fsState = TBSTATE_ENABLED;
	Buttons[5].fsStyle = TBSTYLE_DROPDOWN;
	Buttons[5].iBitmap = 4;
	Buttons[5].idCommand = IDM_VARIABLE;
	Buttons[6].fsStyle = TBSTYLE_SEP;
	Buttons[7].fsState = TBSTATE_ENABLED;
	Buttons[7].fsStyle = TBSTYLE_BUTTON;
	Buttons[7].iBitmap = 5;
	Buttons[7].idCommand = IDM_PLAY;
	Buttons[8].fsState = TBSTATE_ENABLED;
	Buttons[8].fsStyle = TBSTYLE_BUTTON;
	Buttons[8].iBitmap = 6;
	Buttons[8].idCommand = IDM_PAUSE;
	Buttons[9].fsState = TBSTATE_ENABLED;
	Buttons[9].fsStyle = TBSTYLE_BUTTON;
	Buttons[9].iBitmap = 7;
	Buttons[9].idCommand = IDM_STOP;
	Buttons[10].fsState = TBSTATE_ENABLED;
	Buttons[10].fsStyle = TBSTYLE_BUTTON;
	Buttons[10].iBitmap = 8;
	Buttons[10].idCommand = IDM_STEP;
	m_wndToolBar.GetToolBarCtrl().SetImageList(&m_imglistToolBar);
	m_wndToolBar.GetToolBarCtrl().SetDisabledImageList(&m_imglistToolBarDisabled);
	m_wndToolBar.GetToolBarCtrl().SetHotImageList(&m_imglistToolBarHighLighted);
	m_wndToolBar.GetToolBarCtrl().SetBitmapSize(CSize(48, 48));
	m_wndToolBar.GetToolBarCtrl().SetButtonSize(CSize(55, 55));
	m_wndToolBar.GetToolBarCtrl().AddButtons(11, Buttons);
	m_wndToolBar.GetToolBarCtrl().AutoSize();


	// Create rebar
	if(!m_wndReBar.Create(this))
	{
		return -1;
	}

	// Create slider control
	if(!m_wndSlider.Create(TBS_HORZ | TBS_AUTOTICKS | TBS_BOTTOM | 
		WS_CHILD | WS_VISIBLE | WS_DISABLED, CRect(0, 0, 200, 30), 
		&m_wndReBar, IDC_SLIDER))
	{
		return -1;
	}

	// Add bars into rebar
	if(!m_wndReBar.AddBar(&m_wndToolBar) || !m_wndReBar.AddBar(&m_wndSlider))
	{
		return -1;
	}

	// Maximize and initialize slider bar
	m_wndReBar.GetReBarCtrl().MaximizeBand(1u);
	m_wndSlider.SetRange(0, 1000);
	m_wndSlider.SetTicFreq(50);
	m_wndSlider.SetPageSize(50);
	m_wndSlider.SetLineSize(1);


	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	 cs.style   =   cs.style   &   ~WS_MAXIMIZEBOX;  
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if(GetStyle() & WS_MINIMIZE)
	{
		ShowWindow(SW_SHOWNOACTIVATE);
	}
	GetSafeOwner()->SetForegroundWindow();

	// Ask user
	if(GetSafeOwner()->MessageBox("您确定要退出本程序吗？", NULL, 
		MB_ICONQUESTION | MB_YESNO) == IDYES)
	{
		CFrameWnd::OnClose();
	}
}

void CMainFrame::OnOpen() 
{
	// Just let children know
	SendMessageToDescendants(VS_COMMAND, (WPARAM)IDM_OPEN);
}

void CMainFrame::OnAdjust() 
{
	// TODO: Add your command handler code here
	
}

void CMainFrame::OnCapture() 
{
	// TODO: Add your command handler code here
	SendMessageToDescendants(VS_COMMAND, (WPARAM)IDM_CAPTURE);
}

void CMainFrame::OnPause() 
{
	// TODO: Add your command handler code here
	SendMessageToDescendants(VS_COMMAND, (WPARAM)IDM_PAUSE);
}

void CMainFrame::OnPlay() 
{
	// TODO: Add your command handler code here
	SendMessageToDescendants(VS_COMMAND, (WPARAM)IDM_PLAY);
}

void CMainFrame::OnStep() 
{
	// TODO: Add your command handler code here
	SendMessageToDescendants(VS_COMMAND, (WPARAM)IDM_STEP);
}

void CMainFrame::OnStop() 
{
	// TODO: Add your command handler code here
	SendMessageToDescendants(VS_COMMAND, (WPARAM)IDM_STOP);
}

void CMainFrame::OnPurge() 
{
	// TODO: Add your command handler code here
	SendMessageToDescendants(VS_COMMAND, (WPARAM)IDM_PURGE);
}

LRESULT CMainFrame::OnUpdatePositions(WPARAM wParam, LPARAM lParam)
{
	LONG lCurrent = (LONG)wParam;
	LONG lStop = (LONG)lParam;

	// Update slider state
	if(lStop <= 0L)
	{
		lStop = -lStop;
		if(m_wndSlider.IsWindowEnabled())
		{
			m_wndSlider.EnableWindow(FALSE);
		}
	}
	else
	{
		if(!m_wndSlider.IsWindowEnabled())
		{
			m_wndSlider.EnableWindow();
		}
	}

	// Update slider positions
	if(!m_wndSlider.IsWindowEnabled() || ::GetCapture() != m_wndSlider.GetSafeHwnd())
	{
		int dwSliderPos = lCurrent >= 0 ? 
			(lStop ? (int)(lCurrent * 1000.0 / lStop + 0.5) : 0) : 0;
		if(dwSliderPos != m_wndSlider.GetPos())
		{
			m_wndSlider.SetPos(dwSliderPos);
		}
	}

	// Update status bar
	CString szPositions("");
	if(lCurrent >= 0L)
	{
		lCurrent = (lCurrent + 500) / 1000;
		lStop = (lStop + 500) / 1000;
		szPositions.Format("%02d:%02d:%02d/%02d:%02d:%02d", 
			lCurrent / 3600, (lCurrent % 3600) / 60, lCurrent % 60, 
			lStop / 3600, (lStop % 3600) / 60, lStop % 60);
	}
	if(szPositions != m_wndStatusBar.GetPaneText(2))
	{
		m_wndStatusBar.SetPaneText(2, szPositions);
	}

	return 0L;
}

LRESULT CMainFrame::OnUpdateFPS(WPARAM wParam, LPARAM lParam)
{
	// Update FPS
	CString szFPS("");
	int nAveFrameRate = (int)wParam;
	if(nAveFrameRate >= 0)
	{
		double FPS = 99.9;
		if(nAveFrameRate < 10000)
		{
			FPS = nAveFrameRate / 100.0;
		}
		szFPS.Format("%.1f FPS", FPS);
	}
	if(szFPS != m_wndStatusBar.GetPaneText(4))
	{
		m_wndStatusBar.SetPaneText(4, szFPS);
	}

	return 0L;
}

LRESULT CMainFrame::OnUpdateCursorPosition(WPARAM wParam, LPARAM lParam)
{
	// Update cursor position
	CString szPosition("");
	int dwCursorX = (int)wParam;
	int dwCursorY = (int)lParam;
	if(dwCursorX >= 0 || dwCursorY >= 0)
	{
		szPosition.Format("(%03d, %03d)", dwCursorX, dwCursorY);
	}
	if(szPosition != m_wndStatusBar.GetPaneText(1))
	{
		m_wndStatusBar.SetPaneText(1, szPosition);
	}

	return 0L;
}

void CMainFrame::OnUpdatePlayerState(CCmdUI *pCmdUI)
{
	SendMessageToDescendants(VS_UPDATECOMMANDUI, (WPARAM)IDS_PLAYERSTATE, (LPARAM)pCmdUI);
}

void CMainFrame::OnUpdateOpen(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CMainFrame::OnUpdatePlay(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	SendMessageToDescendants(VS_UPDATECOMMANDUI, (WPARAM)IDM_PLAY, (LPARAM)pCmdUI);
}

void CMainFrame::OnUpdatePause(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	SendMessageToDescendants(VS_UPDATECOMMANDUI, (WPARAM)IDM_PAUSE, (LPARAM)pCmdUI);
}

void CMainFrame::OnUpdatePurge(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	SendMessageToDescendants(VS_UPDATECOMMANDUI, (WPARAM)IDM_PURGE, (LPARAM)pCmdUI);
}

void CMainFrame::OnUpdateStop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	SendMessageToDescendants(VS_UPDATECOMMANDUI, (WPARAM)IDM_STOP, (LPARAM)pCmdUI);
}

void CMainFrame::OnUpdateStep(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	SendMessageToDescendants(VS_UPDATECOMMANDUI, (WPARAM)IDM_STEP, (LPARAM)pCmdUI);
}

