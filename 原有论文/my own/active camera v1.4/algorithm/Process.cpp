// Process.cpp: implementation of the CProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcess::CProcess()
{
	m_pOutFrame = NULL;


	m_bThreadEnd = false;
	m_pWorkThread = NULL;

	m_pInImage = NULL;

	m_nIndex = 0;
	gImgInitMaskHalf = NULL;
	gBbins = 10;
	gGbins = 10;
	gRbins = 10;
	::InitializeCriticalSection(&m_CriticalSection);
}

CProcess::~CProcess()
{

	::EnterCriticalSection(&m_CriticalSection);
	m_bThreadEnd = true;		
	::LeaveCriticalSection(&m_CriticalSection);	

	WaitForSingleObject(m_pWorkThread->m_hThread, INFINITE);
//	feat_TrackCleanUp();
	::PostMessage(m_hWnd,ALGORITHM_MESSAGE,0,0);


	if(m_pOutFrame)
	{
		delete[] m_pOutFrame;
		m_pOutFrame = NULL;
	}

	if (m_pInImage)
	{
		cvReleaseImage(&m_pInImage);
		m_pInImage = NULL;
	}
	::DeleteCriticalSection(&m_CriticalSection);
}

void CProcess::RecieveFrames(unsigned char *pBuf, LONG lWidth, LONG lHeight)
{
	::EnterCriticalSection(&m_CriticalSection);
	if(m_pInImage && (lWidth*lHeight)<=(m_WidthStep*m_Height))
	{
		memcpy(m_pInImage->imageData, pBuf, lWidth*lHeight*3);
		m_WidthStep = lWidth;
		m_HeightStep = lHeight;
		m_nIndex++;
	}
	::LeaveCriticalSection(&m_CriticalSection);
}

void CProcess::OutputFrames(unsigned char *pSrc, unsigned char *pDst)
{
	::EnterCriticalSection(&m_CriticalSection);
	if(m_pOutFrame)
	{
		memcpy(pDst, m_pOutFrame, m_WidthStep*m_HeightStep*3);
	}
	::LeaveCriticalSection(&m_CriticalSection);	
}



void CProcess::Free()
{

	::EnterCriticalSection(&m_CriticalSection);
	m_bThreadEnd = true;		
	::LeaveCriticalSection(&m_CriticalSection);	

	WaitForSingleObject(m_pWorkThread->m_hThread, INFINITE);
//	feat_TrackCleanUp();
	::PostMessage(m_hWnd,ALGORITHM_MESSAGE,0,0);
	if(m_pOutFrame)
	{
		delete[] m_pOutFrame;
		m_pOutFrame = NULL;
	}
	if (m_pInImage)
	{
		cvReleaseImage(&m_pInImage);;
		m_pInImage = NULL;
	}
}

UINT CProcess::WorkThread(LPVOID lpParameter)
{
	CProcess *pThis = (CProcess *) lpParameter;
	bool bEnd = false;
	long nLastIndex = 0;
	long curIndex = 0;
	int centx = pThis->m_Width/2;
	int centy = pThis->m_Height/2;
	bool bKltFlag = false;
	IplImage * m_pInTemp;
	m_pInTemp = cvCreateImage(cvSize(pThis->m_WidthStep, pThis->m_Height), 8, 3);
	double score;
	while(!bEnd)
	{
		::EnterCriticalSection(&(pThis->m_CriticalSection));
		bEnd = pThis->m_bThreadEnd;
		curIndex = pThis->m_nIndex;
		::LeaveCriticalSection(&(pThis->m_CriticalSection));	
		if(nLastIndex!=curIndex)
		{
			nLastIndex = curIndex;

			::EnterCriticalSection(&(pThis->m_CriticalSection));
			cvCopy(pThis->m_pInImage, m_pInTemp);
			::LeaveCriticalSection(&(pThis->m_CriticalSection));
			
			cvFlip(m_pInTemp);
			CRECT Rect;
			::EnterCriticalSection(&(pThis->m_CriticalSection));
			Rect = pThis->m_RectState;
			::LeaveCriticalSection(&(pThis->m_CriticalSection));

//			his_TrackNextFrame(m_pInTemp, Rect, &(pThis->gTkResult));
			pThis->m_Tracker.TrackNextFrame((unsigned char *)m_pInTemp->imageData, Rect, &(pThis->gTkResult));	
			::EnterCriticalSection(&(pThis->m_CriticalSection));
			pThis->m_RectState = pThis->gTkResult.targetBox;
//			utl_RectCheckBound(&(pThis->m_RectState), centx*2, centy*2);
			Rect = pThis->m_RectState;
			score = pThis->gTkResult.score;
			::LeaveCriticalSection(&(pThis->m_CriticalSection));
			
//			if (score<=0.1)
//			{
//				::SendMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,0,0);
//				continue;
//			}
			
			int tempx = (Rect.left+Rect.right)/2 - centx;
			int tempy = (Rect.bottom+Rect.top)/2 - centy;
			int wlimit = (Rect.right-Rect.left)/5;
			int hlimit = (Rect.bottom-Rect.top)/5;
			
			if (fabs((double)tempx)<wlimit && fabs((double)tempy)<hlimit)
			{
				::SendMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,0,0);
			}
			else if ( fabs((double)tempx) < fabs((double)tempy) )
			{
				if (tempy>0)
				{
					::SendMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,4,0);//0:stop; 1:left; 2:up; 3:right; 4:down
				}
				else
				{
					::SendMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,2,0);
				}
			}
			else
			{
				if (tempx>0)
				{
					::SendMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,3,0);
				}
				else
				{
					::SendMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,1,0);
				}
			}
		}
		Sleep(1);
//		bEnd = true;
	}
	::PostMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,0,0);
	return 0;
}



bool CProcess::Start(int nWidth, int nHeight, HWND hWnd, CRECT InitRect, unsigned char * pInFrame)
{
	m_hWnd = hWnd;
	m_Width = nWidth;
	m_Height = nHeight;
	gTkResult.FGImage = NULL;
	gTkResult.FGMask = NULL;
	gTkResult.ObjMask = NULL;
	
	gZoomAccum = 1;
	if((m_Width%4)==0)
	{
		m_WidthStep = m_Width;
	}
	else
	{
		m_WidthStep = (m_Width/4+1)*4;
	}
	m_HeightStep = nHeight;

	m_bThreadEnd = false;

	if(m_pOutFrame)
	{
		delete[] m_pOutFrame;
		m_pOutFrame = NULL;
	}

	if (m_pInImage)
	{
		cvReleaseImage(&m_pInImage);
		m_pInImage = NULL;
	}

//	feat_TrackCleanUp();
	m_pInImage  = cvCreateImage(cvSize(m_WidthStep, m_Height), 8, 3);//new unsigned char[m_WidthStep*m_Height*3];
	gImgInitMaskHalf = cvCreateImage(cvSize(m_WidthStep, m_Height), 8, 1);

	memcpy(m_pInImage->imageData, pInFrame, m_WidthStep*m_Height*3);

//	his_TrackInit_Bins(m_pInImage, gImgInitMaskHalf, InitRect, gRbins, gGbins, gBbins);
	m_Tracker.Initial((unsigned char *)m_pInImage->imageData, NULL, InitRect, m_pInImage->width, m_pInImage->height);
	m_RectState = InitRect;

//	gTkResult.FGImage = his_GetBackProjImage();
	for (int i=0;i<6;i++)
	{		
		gAffineMatrix[i] = 0;
	}
	gBlobWidth = gBlobWidthInit = InitRect.right-InitRect.left;
	gBlobHeight = gBlobHeightInit = InitRect.bottom-InitRect.top;
//	klt_TrackInit(m_pInImage);
	
	m_pWorkThread = AfxBeginThread(CProcess::WorkThread, (void *)this);
	m_pWorkThread->m_bAutoDelete = false;
//	m_pWorkThread->SetThreadPriority(THREAD_PRIORITY_HIGHEST);
	return true;
}

bool CProcess::GetState(CRECT *pRect)
{
	::EnterCriticalSection(&(m_CriticalSection));
	(*pRect) = m_RectState;
	::LeaveCriticalSection(&(m_CriticalSection));
	return true;
}
