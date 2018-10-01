// ISampleGrabberCB.h: interface for the CISampleGrabberCB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISAMPLEGRABBERCB_H__C32EBF21_4F50_4C10_BB6B_556D312A9A7D__INCLUDED_)
#define AFX_ISAMPLEGRABBERCB_H__C32EBF21_4F50_4C10_BB6B_556D312A9A7D__INCLUDED_

#include "ISampleRendererCB.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CISampleRendererCB;

class CISampleGrabberCB : public ISampleGrabberCB  
{
public:
	void ObtainFrame(unsigned char * pDst);
	HRESULT PaintReprocessedFrame(CClientDC * pDC,CRect rtDisplay);
	void SetParameterFileName(LPCTSTR szParameterFileName);
	void SetImageDirectory(LPCTSTR szImageDirectory);
	HRESULT SaveCurrentFrame(LPCTSTR szImageFileName, BOOL bBmp = TRUE);
	HRESULT SaveCurrentFrame(CString &szImageFileName, BOOL bBmp = TRUE);
	BOOL ShouldUseCustomPaint();
	HRESULT PaintReprocessedFrame(HDC hDC, RECT &rtDisplay);
	void InsertBlackFrame();
	void PickUpAlarmInfo(CListCtrl *pListCtrl);
	BOOL QueryAlarmState();
	void SetAlgorithmWorkingState(BOOL bActivated);
	BOOL IsAlgorithmActivated();
	void AdjustAlgorithmParameters(CWnd *pParent);
	void ReloadRule(int nType, int nRegionID);
	void SetRuleFileName(LPCTSTR szRuleFileName);
	void SetSceneID(int nSceneID);
	void Destroy();
	HRESULT Initialize(VIDEOINFOHEADER *pVideoInfoHdr);
	virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample);
	virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
	virtual ULONG STDMETHODCALLTYPE Release(void);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	CISampleGrabberCB(CISampleRendererCB *pISampleRendererCB);
	virtual ~CISampleGrabberCB();

protected:
	int m_nSceneID;
	CString m_szImageDirectory;
	CISampleRendererCB * m_pISampleRendererCB;
	void ReprocessCurrentFrame();
	BOOL m_bReprocessed;
	BOOL m_bBlackFrame;
	BOOL m_bActivated;
	BYTE *m_pBmpFileBuffer;
	BYTE *m_pFrameBuffer;
	BYTE *m_pReprocessedFrameBuffer;
	long m_lVideoStep;
	long m_lVideoHeight;
	long m_lVideoWidth;
	CRITICAL_SECTION m_CriticalSection;
	void FrameCallBack(BYTE *pBuffer, long lBufferLen);
};

#endif // !defined(AFX_ISAMPLEGRABBERCB_H__C32EBF21_4F50_4C10_BB6B_556D312A9A7D__INCLUDED_)
