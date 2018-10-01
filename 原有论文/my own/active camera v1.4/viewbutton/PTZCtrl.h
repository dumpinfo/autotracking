// PTZCtrl.h: interface for the CPTZCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PTZCTRL_H__9D01DE90_9E99_4581_87E4_A4EF5485178B__INCLUDED_)
#define AFX_PTZCTRL_H__9D01DE90_9E99_4581_87E4_A4EF5485178B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxmt.h>

enum{
		PTZ_STOP,
		PTZ_LEFT,
		PTZ_RIGHT,
		PTZ_UP,
		PTZ_DOWN,
		PTZ_ZOOMIN,
		PTZ_ZOOMOUT,
		PTZ_PRESET,
		PTZ_SET,
		PTZ_READ,
		PTZ_MEDIA
};


class CPTZCtrl  
{
public:
	bool ObtainData( int * iL, int * iT, int * iP, int * iZ);
	bool SetLastAction(int Action);
	bool ObtainAction(int * iAction);
	bool ObtainLastAction( int * iAction);

	
	bool PTZZoomOut();
	bool PTZZoomIn();
	bool ReleasePTZ();
	bool PTZDown();
	bool PTZUp();
	bool PTZRight();
	bool PTZLeft();
	bool PTZStop();
	bool SetPTZState();
	bool ReadPTZState();
	bool Initialize(CView * pView);
	CPTZCtrl();
	virtual ~CPTZCtrl();
	CView * m_pView;
protected:
//	HANDLE m_hEvent;
//	CEvent * m_Event;
	HWND m_hWnd;
	int m_iLabel;
	bool DataReadFromPTZ();
	bool DataReadFromFile();
	int m_iZoom;
	int m_iPan;
	int m_iTilt;
	HANDLE m_hThread;
	int m_Action;
	int m_iLastState;

	CRITICAL_SECTION m_CriticalSection;
	VISCACamera_t * pCamera;
	VISCAInterface_t * pVISCAinter;



	static DWORD WINAPI PTZCtrlThread(LPVOID lpParameter);
};

#endif // !defined(AFX_PTZCTRL_H__9D01DE90_9E99_4581_87E4_A4EF5485178B__INCLUDED_)
