// Process.h: interface for the CProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESS_H__630FC9E9_35E2_4F7C_B542_066DD51469FC__INCLUDED_)
#define AFX_PROCESS_H__630FC9E9_35E2_4F7C_B542_066DD51469FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "stdio.h"
#include "math.h"
#include "string.h"
#include "highgui.h"

//#include "Utility.h"
//#include "KltWrap.h"
//#include "Histogram.h"
//#include "MeanShiftWrap.h"
//#include "TemplateMatch.h"
//#include "Msfeature.h"
#include "FeatMS.h"


#define ALGORITHM_MESSAGE                 1021
class CProcess  
{
public:
	bool GetState(CRECT *pRect);
	bool Start(int nWidth, int nHeight, HWND hWnd, CRECT InitRect, unsigned char * pInFrame);
	void Free();
	void OutputFrames(unsigned char *pSrc, unsigned char *pDst);
	void RecieveFrames(unsigned char *pBuf, LONG lWidth, LONG lHeight);
	CProcess();
	virtual ~CProcess();

protected:
	CFeatMS m_Tracker;
	CRECT m_RectState;
	HWND m_hWnd;
	CWinThread * m_pWorkThread;
	BOOL m_bThreadEnd;
	int m_HeightStep;
	int m_WidthStep;
	int m_Height;
	int m_Width;
	unsigned char * m_pOutFrame;
	IplImage * m_pInImage;
	CRITICAL_SECTION m_CriticalSection;
	int m_nIndex;
	IplImage *gImgInitMaskHalf;
	int gRbins;
	int	gGbins;
	int	gBbins;
	TkResult gTkResult;
	float	gAffineMatrix[6];
	float	gZoomAccum; //accumulative scale factor
	float	zoom;
	int	gBlobWidthInit;
	int gBlobHeightInit; //initial dimension of tracking blob
	int	gBlobWidth;
	int gBlobHeight; //blob dimension of previous frame
private:
	static UINT WorkThread(LPVOID lpParameter);
};

#endif // !defined(AFX_PROCESS_H__630FC9E9_35E2_4F7C_B542_066DD51469FC__INCLUDED_)
