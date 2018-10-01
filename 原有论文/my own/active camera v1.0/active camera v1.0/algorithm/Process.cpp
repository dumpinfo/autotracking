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
	m_pInFrame = NULL;
	m_pOutFrame = NULL;
	m_pInTemp = NULL;
	m_pOutTemp = NULL;

	m_bThreadEnd = false;
	m_pWorkThread = NULL;

	m_pGauss = NULL;

	m_nIndex = 0;
	::InitializeCriticalSection(&m_CriticalSection);
}

CProcess::~CProcess()
{
	if(m_pInFrame)
	{
		delete[] m_pInFrame;
		m_pInFrame = NULL;
	}
	if(m_pOutFrame)
	{
		delete[] m_pOutFrame;
		m_pOutFrame = NULL;
	}
	if(m_pInTemp)
	{
		delete[] m_pInTemp;
		m_pInTemp = NULL;
	}
	if(m_pOutTemp)
	{
		delete[] m_pOutTemp;
		m_pOutTemp = NULL;
	}
	if(m_pGauss)
	{
		delete[] m_pGauss;
		m_pGauss = NULL;
	}
	::DeleteCriticalSection(&m_CriticalSection);
}

void CProcess::RecieveFrames(unsigned char *pBuf, LONG lWidth, LONG lHeight)
{
	::EnterCriticalSection(&m_CriticalSection);
	if(m_pInFrame && (lWidth*lHeight)<=(m_WidthStep*m_Height))
	{
		memcpy(m_pInFrame, pBuf, lWidth*lHeight*3);
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


BOOL CProcess::Start(int nWidth, int nHeight)
{
	m_Width = nWidth;
	m_Height = nHeight;
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

	if(m_pInFrame)
	{
		delete[] m_pInFrame;
		m_pInFrame = NULL;
	}
	if(m_pOutFrame)
	{
		delete[] m_pOutFrame;
		m_pOutFrame = NULL;
	}
	if(m_pInTemp)
	{
		delete[] m_pInTemp;
		m_pInTemp = NULL;
	}
	if(m_pOutTemp)
	{
		delete[] m_pOutTemp;
		m_pOutTemp = NULL;
	}
	if(m_pGauss)
	{
		delete[] m_pGauss;
		m_pGauss = NULL;
	}

	m_pInFrame  = new unsigned char[m_WidthStep*m_Height*3];
	m_pOutFrame = new unsigned char[m_WidthStep*m_Height*3];
	m_pInTemp   = new unsigned char[m_WidthStep*m_Height*3];
	m_pOutTemp  = new unsigned char[m_WidthStep*m_Height*3];
	m_nIndex = 0;


	if (m_pGauss == NULL)
	{
		m_pGauss = new BK[m_WidthStep*m_Height*K];
		m_pGauss->label = 0;
	}


	m_pWorkThread = AfxBeginThread(CProcess::WorkThread, (void *)this);
	m_pWorkThread->m_bAutoDelete = true;
//	m_pWorkThread->SetThreadPriority(THREAD_PRIORITY_HIGHEST);
	return true;
}

void CProcess::Free()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_bThreadEnd = true;		
	::LeaveCriticalSection(&m_CriticalSection);	

	WaitForSingleObject(m_pWorkThread->m_hThread, INFINITE);
//	TerminateThread(m_pWorkThread->m_hThread,0);
//	CloseHandle(m_pWorkThread->m_hThread);

	if(m_pInFrame)
	{
		delete[] m_pInFrame;
		m_pInFrame = NULL;
	}
	if(m_pOutFrame)
	{
		delete[] m_pOutFrame;
		m_pOutFrame = NULL;
	}
	if(m_pInTemp)
	{
		delete[] m_pInTemp;
		m_pInTemp = NULL;
	}
	if(m_pOutTemp)
	{
		delete[] m_pOutTemp;
		m_pOutTemp = NULL;
	}
	if(m_pGauss)
	{
		delete[] m_pGauss;
		m_pGauss = NULL;
	}

}

UINT CProcess::WorkThread(LPVOID lpParameter)
{
	CProcess *pThis = (CProcess *) lpParameter;
	bool bEnd = false;
	long nLastIndex = 0;
	long curIndex = 0;
	while(!bEnd)
	{
		::EnterCriticalSection(&(pThis->m_CriticalSection));
		bEnd = pThis->m_bThreadEnd;
		curIndex = pThis->m_nIndex;
		::LeaveCriticalSection(&(pThis->m_CriticalSection));	
		if(nLastIndex!=curIndex)
		{
			nLastIndex = curIndex;
			pThis->ObjectDetection();
			::PostMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,-1,0);
		}
		Sleep(1);
	}
	::PostMessage(pThis->m_hWnd,ALGORITHM_MESSAGE,1,0);
	return 0;
}

bool CProcess::Start(int nWidth, int nHeight, HWND hWnd)
{
	m_hWnd = hWnd;
	m_Width = nWidth;
	m_Height = nHeight;
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

	if(m_pInFrame)
	{
		delete[] m_pInFrame;
		m_pInFrame = NULL;
	}
	if(m_pOutFrame)
	{
		delete[] m_pOutFrame;
		m_pOutFrame = NULL;
	}
	if(m_pInTemp)
	{
		delete[] m_pInTemp;
		m_pInTemp = NULL;
	}
	if(m_pOutTemp)
	{
		delete[] m_pOutTemp;
		m_pOutTemp = NULL;
	}
	if(m_pGauss)
	{
		delete[] m_pGauss;
		m_pGauss = NULL;
	}

	if (m_pGauss == NULL)
	{
		m_pGauss = new BK[m_WidthStep*m_Height*K];
		m_pGauss->label = 0;
	}


	m_pInFrame  = new unsigned char[m_WidthStep*m_Height*3];
	m_pOutFrame = new unsigned char[m_WidthStep*m_Height*3];
	m_pInTemp   = new unsigned char[m_WidthStep*m_Height*3];
	m_pOutTemp  = new unsigned char[m_WidthStep*m_Height*3];


	m_pWorkThread = AfxBeginThread(CProcess::WorkThread, (void *)this);
	m_pWorkThread->m_bAutoDelete = false;
//	m_pWorkThread->SetThreadPriority(THREAD_PRIORITY_HIGHEST);
	return true;
}

void CProcess::ObjectDetection()
{
	::EnterCriticalSection(&m_CriticalSection);
	memcpy(m_pInTemp, m_pInFrame, m_WidthStep*m_HeightStep*3);
	::LeaveCriticalSection(&m_CriticalSection);
	

	int m = m_WidthStep;
	int n = m_HeightStep;


	memset(m_pOutTemp,0,m*n*sizeof(unsigned char)*3);



	if (m_pGauss->label == 0)
	{
		//initialize background model parameters
		BK *ptmp;	

		for(register int y=0; y<n; y++)
		{
			for(register int x=0; x<m; x++)
			{
				ptmp=m_pGauss+(y*m+x)*K;// Set pointer ptmp to first gaussian of the pixel in the pGauss array in heap
				for (register int z=0; z<K; z++)
				{
					ptmp->weight = 1.0/K;
					ptmp->mean[0]=m_pInTemp[(y*m+x)*3];
					ptmp->mean[1]=m_pInTemp[(y*m+x)*3+1];
					ptmp->mean[2]=m_pInTemp[(y*m+x)*3+2];
					ptmp->std = STD;
					ptmp++;
				}
			}		
		}
		m_pGauss->label = 1;
	}


//Ç°¾°¼ì²â
	int rgb[3];
	BOOL m_shadowsample = false;
	// loop through each pixel in RGB image
	for ( int y=0; y<n; y++)
	{ 
		for ( int x=0; x<m; x++)
		{
			// Get RGB value of each pixel
			rgb[0] = m_pInTemp[(y*m+x)*3];//get blue
			rgb[1] = m_pInTemp[(y*m+x)*3+1];//green
			rgb[2] = m_pInTemp[(y*m+x)*3+2];//red

			// Set pointer ptr to first gaussian of the pixel in the pGauss array in heap
			BK * ptr = m_pGauss+(y*m+x)*K;
			BK * pk = ptr;
			int m_BtempR = ptr->mean[0];
			int m_BtempG = ptr->mean[1];
			int m_BtempB = ptr->mean[2];
			m_shadowsample = false;
//			CSGMM * pStr = pSH+(y*m+x)*K1;
//			CSGMM * pSk = pStr;
					
			int match=0;
			int match1 = 0;
			bool borf=0;
			// loop through each gaussian of pixel to determine if there is a matc
			for (register int z=0; z<K; z++)
			{
				// match is defined as |X - u| < 2s for each RGB channel
				double num = FACTOR*pk->std;
				if ( fabs(rgb[0] - pk->mean[0]) < num && fabs(rgb[1] - pk->mean[1]) < num && fabs(rgb[2] - pk->mean[2]) < num )
				{
					// determine if pixel is background or foreground
					borf=ptr->bgd(pk);
					if ( !borf )
					{
						m_pOutTemp[(y*m+x)*3] = 255;
						m_pOutTemp[(y*m+x)*3+1] = 255;
						m_pOutTemp[(y*m+x)*3+2] = 255;
					}
					// Adjust weights
					ptr->adjWeights(pk);
					// Adjust mean and standard deviation
					pk->adjMeanStd(rgb);
					match=1;
					break;
				} 
				pk++;
			}


			

			// if no match, replace least probable gaussian and set mask pixel to background
			if (match == 0)
			{
				m_pOutTemp[(y*m+x)*3] = 255;
				m_pOutTemp[(y*m+x)*3+1] = 255;
				m_pOutTemp[(y*m+x)*3+2] = 255;
				ptr->least(rgb);
			}
			ptr->sortGauss();
		
		}
	}


	::EnterCriticalSection(&m_CriticalSection);
	memcpy(m_pOutFrame, m_pOutTemp, m_WidthStep*m_HeightStep*3);
	::LeaveCriticalSection(&m_CriticalSection);	
}
