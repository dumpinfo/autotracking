// VideoDirectShow.h: interface for the CVideoDirectShow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VIDEODIRECTSHOW_H__327E9BB3_53FE_402C_8F37_54A66E5942F1__INCLUDED_)
#define AFX_VIDEODIRECTSHOW_H__327E9BB3_53FE_402C_8F37_54A66E5942F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ISampleRendererCB.h"	// Added by ClassView
#include "ISampleGrabberCB.h"	// Added by ClassView

class CVideoDirectShow  
{
public:
	void ObtainFrame(unsigned char * pDst);
	bool CaptureVideo(IMoniker * pMoniker, HWND hWnd, int BitsPixel, CRect * pRect);
	bool CanStep();
	bool GetState(OAFilterState * pOAFstate);
	bool GetMediaEvent(long * plEventCode, long * plParam1, long * plParam2);
	bool IsPlaying();
	bool GetFPS(int * pIfps);
	bool GetPosition(LONG * pLCurrent, LONG * pLStop);
	void Stop();
	void Pause();
	void Play();
	void Step();
	void PaintVideoFrame(CRect display);
	BOOL OpenFile(LPWSTR pFilename, HWND hWnd, int BitsPixel, CRect *);
	CVideoDirectShow();
	virtual ~CVideoDirectShow();
	void ReleaseFilterGraph();
protected:
	HWND m_hWnd;
	void RemoveAllFilters();
	HRESULT ConnectFilters(IBaseFilter *pIUpFilter, IBaseFilter *pIDownFilter);
	
	IGraphBuilder * m_pIGraphBuilder;
	CISampleRendererCB m_ISampleRendererCB;
	CISampleGrabberCB m_ISampleGrabberCB;
	BOOL m_bCanUseGraphBuilder;
	BOOL m_bBuffering;
	BOOL m_bCanAdjust;
	BOOL m_bCanSeek;
};

#endif // !defined(AFX_VIDEODIRECTSHOW_H__327E9BB3_53FE_402C_8F37_54A66E5942F1__INCLUDED_)
