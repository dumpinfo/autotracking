// ISampleRendererCB.h: interface for the CISampleRendererCB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISAMPLERENDERERCB_H__FBAE22BB_99E4_4695_B6C5_5904BEF8579C__INCLUDED_)
#define AFX_ISAMPLERENDERERCB_H__FBAE22BB_99E4_4695_B6C5_5904BEF8579C__INCLUDED_

#include "ISampleGrabberCB.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CISampleGrabberCB;

class CISampleRendererCB : public ISampleGrabberCB  
{
public:
	HRESULT CopyVideoFrame();
	void  NotifyGrabberPaint(long lLetGrabberPaint = 1L);
	HRESULT PaintVideoFrame(RECT &rtDisplay);
	void Destroy();
	HRESULT Initialize(VIDEOINFOHEADER *pVideoInfoHdr, HWND hDisplayWnd);
	virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);
	virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample);
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
	virtual ULONG STDMETHODCALLTYPE Release(void);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	CISampleRendererCB(CISampleGrabberCB *pISampleGrabberCB);
	virtual ~CISampleRendererCB();

protected:
	long m_lLetGrabberPaint;
	CISampleGrabberCB * m_pISampleGrabberCB;
	BOOL PaintGDI(RECT &rtDisplay);
	BOOL PaintDirectDraw(RECT rtDisplay);
	CRITICAL_SECTION m_CriticalSection;
	HWND m_hDisplayWnd;
	WORD m_wBPP;
	long m_lVideoStep;
	long m_lVideoHeight;
	long m_lVideoWidth;
	BITMAPINFO * m_pBmpInfo;
	BYTE * m_pFrameBuffer;
	IDirectDrawSurface * m_pIOffScreenSurface;
	IDirectDrawSurface * m_pIPrimarySurface;
	IDirectDraw * m_pIDirectDraw;
	void FrameCallBack(BYTE *pBuffer, long lBufferLen);
};

#endif // !defined(AFX_ISAMPLERENDERERCB_H__FBAE22BB_99E4_4695_B6C5_5904BEF8579C__INCLUDED_)
