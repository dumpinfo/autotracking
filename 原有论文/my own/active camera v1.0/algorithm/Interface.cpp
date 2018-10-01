// Interface.cpp: implementation of the CInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Interface.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInterface::CInterface()
{
	m_pProcess = NULL;
}

CInterface::~CInterface()
{
	if(m_pProcess) 
	{
		m_pProcess->Free();
		delete m_pProcess;
		m_pProcess = NULL;
	}
}

int CInterface::GetAction()
{
	return 0;
}

void CInterface::RecieveFrames(unsigned char *pBuf, LONG lWidth, LONG lHeight)
{
	if(m_pProcess)
	{
		m_pProcess->RecieveFrames(pBuf, lWidth, lHeight);
	}
}

void CInterface::OutputFrames(unsigned char *pSrc, unsigned char *pDst)
{
	if(m_pProcess)
	{
		m_pProcess->OutputFrames(pSrc, pDst);
	}	
}

BOOL CInterface::Start(int nWidth, int nHeight)
{
	if(m_pProcess)
		return false;

	m_pProcess = new CProcess();
	return m_pProcess->Start(nWidth, nHeight);
}

void CInterface::Stop()
{
	if(m_pProcess) 
	{
		m_pProcess->Free();
		delete m_pProcess;
		m_pProcess = NULL;
	}
}

bool CInterface::Start(int nWidth, int nHeight, HWND hWnd)
{
	if(m_pProcess)
		return false;

	m_pProcess = new CProcess();
	return m_pProcess->Start(nWidth, nHeight, hWnd);
}
