// viewbuttonView.h : interface of the CViewbuttonView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIEWBUTTONVIEW_H__D1D59B4F_FCC5_4EFE_80A5_FCE5EAF25497__INCLUDED_)
#define AFX_VIEWBUTTONVIEW_H__D1D59B4F_FCC5_4EFE_80A5_FCE5EAF25497__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BtnST.h"
#include "MYButton.h"
#include "savePTZ.h"
#include "VideoDirectShow.h"	// Added by ClassView
#include "MYStatic.h"
#include "EnumDeviceDlg.h"
#include "PTZCtrl.h"	// Added by ClassView
#include "Interface.h"


class CViewbuttonView : public CView
{
protected: // create from serialization only
	CViewbuttonView();
	DECLARE_DYNCREATE(CViewbuttonView)

// Attributes
public:
	CViewbuttonDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewbuttonView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	VISCACamera_t * pCamera;
	VISCAInterface_t * pVISCAinter;
	void OnUpdatePurge(CCmdUI *pCmdUI);
	void OnUpdatePause(CCmdUI* pCmdUI);
	CVideoDirectShow m_cVideo;
	void OnOpen();
	LRESULT OnVSCommand(WPARAM wParam, LPARAM lParam);
	int m_pointnum;
	FILE * m_pFILE;
	bool m_bBegin;
	CRgn m_cRgn;
	CPoint m_spoint;
	virtual ~CViewbuttonView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	POINT	gaClickPoint[4];
	bool m_bInitialSet;
	bool m_bInitialBegin;
	CButtonST* m_pbtnPTZup;
	CButtonST* m_pbtnPTZright;
	CButtonST* m_pbtnPTZdown;
	CButtonST* m_pbtnPTZleft;
	CButtonST* m_pbtnPTZZoomOut;
	CButtonST* m_pbtnPTZZoomIn;
	CMYButton m_pbtnSingle;
	CMYButton m_pbtnPTZAuto;
	CBrush m_brushBackground;
	CRect m_rtDisplay;
	LPWSTR m_szwFileName;
// Generated message map functions
protected:
	int giClickCount;
	CRECT m_RectInit;
	unsigned char * m_pResult;
	unsigned char * m_pInFrame;
	int m_Height;
	int m_Width;
	void InitBitmap( int nWidth, int nHeight);
	CInterface * m_pInterface;
	bool m_bRunning;

	LRESULT OnPTZMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnAlgorithmMessage(WPARAM wParam, LPARAM lParam);

	CPTZCtrl m_cPTZControl;
	bool m_bVideoFile;
	bool m_bCapture;
	void PTZcomIntialize();
	void OnUpdateStep(CCmdUI *pCmdUI);
	void OnUpdatePlay(CCmdUI* pCmdUI);
	void OnUpdateStop(CCmdUI* pCmdUI);
	void OnUpdatePlayerState(CCmdUI* pCmdUI);
	LRESULT OnVSUpdateCommandUI(WPARAM wParam, LPARAM lParam);
	LRESULT OnMediaEvent(WPARAM wParam, LPARAM lParam);
	LRESULT OnButtonMessage(WPARAM wParam, LPARAM lParam);
	void OnPurge();
	void OnStep();
	void ToggleAlgorithmActivation();
	void ToggleRuleEditor();
	void OnStop();
	void OnPause();
	void OnPlay();
	void OnSetting();
	void OnAdjust();
	void OnCapture();
	void Display();
	CMYStatic m_StaticDisplay;
	BOOL m_bPTZAuto;
	BOOL m_bSingleShow;
	BOOL m_bBuffering;
	CListBox m_ClistBox;
	CClientDC * m_pDC;
	//{{AFX_MSG(CViewbuttonView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg   void   OnCheckSingle();
	afx_msg   void   OnCheckAuto();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSaveptzstate();
	afx_msg void OnReadptzstate();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnUpdateReadptzstate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSaveptzstate(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BITMAPINFO m_Bmi;
	BOOL m_bShowVideo;
};

#ifndef _DEBUG  // debug version in viewbuttonView.cpp
inline CViewbuttonDoc* CViewbuttonView::GetDocument()
   { return (CViewbuttonDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWBUTTONVIEW_H__D1D59B4F_FCC5_4EFE_80A5_FCE5EAF25497__INCLUDED_)
