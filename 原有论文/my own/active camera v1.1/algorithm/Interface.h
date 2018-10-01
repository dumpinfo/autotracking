// Interface.h: interface for the CInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACE_H__A0AA6148_4BF3_4234_83AC_139428210476__INCLUDED_)
#define AFX_INTERFACE_H__A0AA6148_4BF3_4234_83AC_139428210476__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "Process.h"


class AFX_EXT_CLASS CInterface  
{
public:
	bool GetState(RECT * pRect);
	bool Start(int nWidth, int nHeight, HWND hWnd);
	bool Start(int nWidth, int nHeight, HWND hWnd, RECT InitRect,unsigned char * pInFrame);
	void Stop();
	BOOL Start(int nWidth, int nHeight);
	void OutputFrames( unsigned char * pSrc, unsigned char * pDst);
	void RecieveFrames( unsigned char * pBuf, LONG lWidth, LONG  lHeight);
	int GetAction();
	CInterface();
	virtual ~CInterface();

protected:
	CProcess * m_pProcess;
};

#endif // !defined(AFX_INTERFACE_H__A0AA6148_4BF3_4234_83AC_139428210476__INCLUDED_)
