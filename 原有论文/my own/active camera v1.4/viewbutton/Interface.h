// Interface.h: interface for the CInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACE_H__A0AA6148_4BF3_4234_83AC_139428210476__INCLUDED_)
#define AFX_INTERFACE_H__A0AA6148_4BF3_4234_83AC_139428210476__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "Process.h"
#define ALGORITHM_MESSAGE                 1021
#include "../algorithm/BasicDefinitions.h"

class AFX_EXT_CLASS CInterface  
{
public:
	bool GetState(CRECT * pRect);
	void Stop();
	BOOL Start(int nWidth, int nHeight);
	bool Start(int nWidth, int nHeight, HWND hWnd);
	bool Start(int nWidth, int nHeight, HWND hWnd, CRECT InitRect, unsigned char * m_pInFrame);
	void OutputFrames( unsigned char * pSrc, unsigned char * pDst);
	void RecieveFrames( unsigned char * pBuf, LONG lWidth, LONG  lHeight);
	int GetAction();
	CInterface();
	virtual ~CInterface();

protected:
//	CProcess * m_pProcess;
};

#endif // !defined(AFX_INTERFACE_H__A0AA6148_4BF3_4234_83AC_139428210476__INCLUDED_)
