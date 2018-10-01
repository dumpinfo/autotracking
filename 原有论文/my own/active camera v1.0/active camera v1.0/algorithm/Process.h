// Process.h: interface for the CProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESS_H__630FC9E9_35E2_4F7C_B542_066DD51469FC__INCLUDED_)
#define AFX_PROCESS_H__630FC9E9_35E2_4F7C_B542_066DD51469FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define ALGORITHM_MESSAGE                 1021

#include "BK.h"

class CProcess  
{
public:
	bool Start(int nWidth, int nHeight, HWND hWnd);
	void Free();
	BOOL Start(int nWidth, int nHeight);
	void OutputFrames(unsigned char *pSrc, unsigned char *pDst);
	void RecieveFrames(unsigned char *pBuf, LONG lWidth, LONG lHeight);
	CProcess();
	virtual ~CProcess();

protected:
	void ObjectDetection();
	BK * m_pGauss;
	HWND m_hWnd;
	CWinThread * m_pWorkThread;
	BOOL m_bThreadEnd;
	int m_HeightStep;
	int m_WidthStep;
	int m_Height;
	int m_Width;
	unsigned char * m_pOutFrame;
	unsigned char * m_pInFrame;
	unsigned char * m_pOutTemp;
	unsigned char * m_pInTemp;
	CRITICAL_SECTION m_CriticalSection;
	int m_nIndex;
private:
	static UINT WorkThread(LPVOID lpParameter);
};

#endif // !defined(AFX_PROCESS_H__630FC9E9_35E2_4F7C_B542_066DD51469FC__INCLUDED_)
