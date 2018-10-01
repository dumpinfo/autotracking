// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__501697B9_F75E_49A2_8CF1_6AC18015DADF__INCLUDED_)
#define AFX_MAINFRM_H__501697B9_F75E_49A2_8CF1_6AC18015DADF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "XPReBar.h"	// Added by ClassView
#include "XPSliderCtrl.h"	// Added by ClassView

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;


	CXPReBar m_wndReBar;
	CXPSliderCtrl m_wndSlider;
	CImageList m_imglistToolBarHighLighted;
	CImageList m_imglistToolBarDisabled;
	CImageList m_imglistToolBar;

// Generated message map functions
protected:
	void OnUpdatePlayerState(CCmdUI *pCmdUI);
	LRESULT OnUpdateCursorPosition(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdateFPS(WPARAM wParam, LPARAM lParam);
	LRESULT OnUpdatePositions(WPARAM wParam, LPARAM lParam);
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnOpen();
	afx_msg void OnAdjust();
	afx_msg void OnCapture();
	afx_msg void OnPause();
	afx_msg void OnPlay();
	afx_msg void OnStep();
	afx_msg void OnStop();
	afx_msg void OnPurge();
	afx_msg void OnUpdateOpen(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePurge(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStep(CCmdUI* pCmdUI);
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__501697B9_F75E_49A2_8CF1_6AC18015DADF__INCLUDED_)
