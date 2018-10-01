// PTZCtrl.cpp: implementation of the CPTZCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "viewbutton.h"
#include "PTZCtrl.h"
#include "savePTZ.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPTZCtrl::CPTZCtrl()
{
	::InitializeCriticalSection(&m_CriticalSection);
	m_hThread = 0;

	pCamera = NULL;
	pVISCAinter = NULL;
}

CPTZCtrl::~CPTZCtrl()
{
	//terminate the running thread
	TerminateThread(m_hThread,0);

	if (pCamera) delete pCamera;
	if (pVISCAinter) delete pVISCAinter;

	::DeleteCriticalSection(&m_CriticalSection);
}

bool CPTZCtrl::Initialize(CView *pView)
{
	//terminate the running thread
	TerminateThread(m_hThread,0);
	
	m_hWnd = pView->GetSafeHwnd();
	m_pView = pView;
	m_iLastState = PTZ_STOP;
	m_Action = PTZ_STOP;

	if (pCamera) delete pCamera;
	if (pVISCAinter) delete pVISCAinter;

	pCamera = new VISCACamera_t;
	pVISCAinter = new VISCAInterface_t;

//初始化串口	
    int camera_num;
    if (VISCA_open_serial(pVISCAinter, "COM1:")!=VISCA_SUCCESS)
    {
		if(VISCA_open_serial(pVISCAinter, "COM2:")!=VISCA_SUCCESS)
			return false;
    }
    pVISCAinter->broadcast=0;
    VISCA_set_address(pVISCAinter, &camera_num);
    pCamera->address=1;
    VISCA_clear(pVISCAinter, pCamera);
    VISCA_get_camera_info(pVISCAinter, pCamera);

//开辟PTZ控制线程
	DWORD dwThreadID;
	m_hThread = CreateThread(NULL, 0, CPTZCtrl::PTZCtrlThread, (void*)this, 0, &dwThreadID );
	if(m_hThread == 0)
		return false;

	return true;
}

bool CPTZCtrl::ReadPTZState()
{
	CsavePTZ savedlg(NULL);
	int nResult = savedlg.DoModal();
	
	if(nResult == IDOK)
	{
		::EnterCriticalSection(&m_CriticalSection);
		m_Action = PTZ_READ;
		m_iLabel = savedlg.m_iNum;
		::LeaveCriticalSection(&m_CriticalSection);


		return true;		
	}
	else
	{
		return false;
	}


}

bool CPTZCtrl::SetPTZState()
{
	CsavePTZ savedlg(NULL);
	int nResult = savedlg.DoModal();
	
	if(nResult == IDOK)
	{
		::EnterCriticalSection(&m_CriticalSection);
		m_Action = PTZ_SET;
		m_iLabel = savedlg.m_iNum;
		::LeaveCriticalSection(&m_CriticalSection);


		return true;		
	}
	else
	{
		return false;
	}
}

bool CPTZCtrl::PTZStop()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_Action = PTZ_STOP;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

bool CPTZCtrl::PTZLeft()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_Action = PTZ_LEFT;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

bool CPTZCtrl::PTZRight()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_Action = PTZ_RIGHT;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

bool CPTZCtrl::PTZUp()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_Action = PTZ_UP;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

bool CPTZCtrl::PTZDown()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_Action = PTZ_DOWN;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

bool CPTZCtrl::ReleasePTZ()
{
	TerminateThread(m_hThread,0);

	if(pVISCAinter)
	{
		VISCA_set_zoom_stop(pVISCAinter,pCamera);
		VISCA_set_pantilt_stop(pVISCAinter,pCamera, 6,6);
	}
	if(pVISCAinter)
	{
		VISCA_close_serial(pVISCAinter);
		if(pVISCAinter)
		{
			delete pVISCAinter;
			pVISCAinter = NULL;
		}
		if(pCamera)
		{
			delete pCamera;
			pCamera = NULL;
		}
	}
	return true;
}

bool CPTZCtrl::PTZZoomIn()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_Action = PTZ_ZOOMIN;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

bool CPTZCtrl::PTZZoomOut()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_Action = PTZ_ZOOMOUT;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

DWORD WINAPI CPTZCtrl::PTZCtrlThread(LPVOID lpParameter)
{
	CPTZCtrl* pThis = (CPTZCtrl*) lpParameter;
	while(true)
	{
		int nAction;
		int nLastAction;
		pThis->ObtainAction(&nAction);
		pThis->ObtainLastAction(&nLastAction);
		if(nAction!=nLastAction)
		{
			if(nAction == PTZ_LEFT)
			{
				if(pThis->pVISCAinter)
				{
					VISCA_set_pantilt_left(pThis->pVISCAinter, pThis->pCamera, 6,6);
//					pThis->m_pView->SetTimer(2,200,NULL);
				}
			}
			else if(nAction == PTZ_STOP)
			{
				if(pThis->pVISCAinter)
				{
					VISCA_set_zoom_stop(pThis->pVISCAinter,pThis->pCamera);
					VISCA_set_pantilt_stop(pThis->pVISCAinter,pThis->pCamera, 6,6);
				}
			}
			else if(nAction == PTZ_RIGHT)
			{
				if(pThis->pVISCAinter)
				{
					VISCA_set_pantilt_right(pThis->pVISCAinter, pThis->pCamera,  6,6);
//					pThis->m_pView->SetTimer(2,200,NULL);
				}			
			}
			else if(nAction == PTZ_UP)
			{
				if(pThis->pVISCAinter)
				{
					VISCA_set_pantilt_up(pThis->pVISCAinter, pThis->pCamera, 6,6);
//					pThis->m_pView->SetTimer(2,200,NULL);
				}			
			}
			else if(nAction == PTZ_DOWN)
			{
				if(pThis->pVISCAinter)
				{
					VISCA_set_pantilt_down(pThis->pVISCAinter, pThis->pCamera,  6,6);
//					pThis->m_pView->SetTimer(2,200,NULL);
				}			
			}
			else if(nAction == PTZ_ZOOMIN)
			{
				if(pThis->pVISCAinter)
				{
					VISCA_set_zoom_wide_speed(pThis->pVISCAinter, pThis->pCamera,6);		
//					pThis->m_pView->SetTimer(3,200,NULL);				
				}			
			}
			else if(nAction == PTZ_ZOOMOUT)
			{
				if(pThis->pVISCAinter)
				{
					VISCA_set_zoom_tele_speed(pThis->pVISCAinter, pThis->pCamera,6);					
//					pThis->m_pView->SetTimer(3,200,NULL);	
				}			
			}
			else if(nAction == PTZ_READ)
			{
				if(pThis->pVISCAinter)
				{
					pThis->DataReadFromPTZ();
				}				
			}
			else if(nAction == PTZ_SET)
			{
				if(pThis->pVISCAinter)
				{
					pThis->DataReadFromFile();
				}			
			}
			else if(nAction == PTZ_PRESET)
			{
				if(pThis->pVISCAinter)
				{
					
				}			
			}
			pThis->SetLastAction(nAction);
		}
		Sleep(2);
	}
	return 0;
}

bool CPTZCtrl::ObtainLastAction(int *iAction)
{
	::EnterCriticalSection(&m_CriticalSection);
	*iAction = m_iLastState;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

bool CPTZCtrl::ObtainAction(int *iAction)
{
	::EnterCriticalSection(&m_CriticalSection);
	*iAction = m_Action;
	::LeaveCriticalSection(&m_CriticalSection);	
	return true;
}

bool CPTZCtrl::SetLastAction(int Action)
{
	::EnterCriticalSection(&m_CriticalSection);
	m_iLastState = Action;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}

bool CPTZCtrl::DataReadFromFile()
{
	::EnterCriticalSection(&m_CriticalSection);
	m_Action = m_iLastState = PTZ_MEDIA;
	::LeaveCriticalSection(&m_CriticalSection);
	int iNum = 0;
	::EnterCriticalSection(&m_CriticalSection);
	iNum = m_iLabel;
	::LeaveCriticalSection(&m_CriticalSection);	
	CString strTemp;
	strTemp.Format("ZOOM%d",iNum);
	int iZOOM = GetPrivateProfileInt(
					"STATE",
					strTemp,
					-1,
					".\\ptzstate.ini"
					);


	strTemp.Format("TILT%d",iNum);

	int iTILT = GetPrivateProfileInt(
					"STATE",
					strTemp,
					-1,
					".\\ptzstate.ini"
					);

	strTemp.Format("PAN%d",iNum);

	int iPAN = GetPrivateProfileInt(
					"STATE",
					strTemp,
					-1,
					".\\ptzstate.ini"
					);

	if(iZOOM == -1 || iPAN == -1 || iTILT == -1)
	{
		::SendMessage(m_hWnd, PTZ_MESSAGE, (WPARAM) -1, (LPARAM)-1);
		return false;
	}
	else
	{
		if(iZOOM>=0)
			VISCA_set_zoom_value(pVISCAinter, pCamera, iZOOM);

		VISCA_set_pantilt_absolute_position(pVISCAinter, pCamera, 
								5, 5, iPAN, iTILT);

		::EnterCriticalSection(&m_CriticalSection);
		m_iPan = iPAN;
		m_iTilt = iTILT;
		m_iZoom = iZOOM;
		::LeaveCriticalSection(&m_CriticalSection);
		::SendMessage(m_hWnd, PTZ_MESSAGE, (WPARAM) -1, 0);

		return true;
	}
	return false;

}

bool CPTZCtrl::DataReadFromPTZ()
{

	::EnterCriticalSection(&m_CriticalSection);
	m_Action = m_iLastState = PTZ_MEDIA;
	::LeaveCriticalSection(&m_CriticalSection);

	unsigned short iZOOM; 
	int iPAN, iTILT;

	unsigned int hr1 = VISCA_get_zoom_value(pVISCAinter, pCamera,&iZOOM);

	while(hr1)
	{
		Sleep(200);
		hr1 = VISCA_get_zoom_value(pVISCAinter, pCamera,&iZOOM);
	}
	
	unsigned int hr = VISCA_get_pantilt_position(pVISCAinter, pCamera,
		&iPAN, &iTILT);

	while(hr)
	{
		Sleep(200);
		hr = VISCA_get_pantilt_position(pVISCAinter, pCamera,
			&iPAN, &iTILT);
	}

	int iNum = 0;
	::EnterCriticalSection(&m_CriticalSection);
	iNum = m_iLabel;
	::LeaveCriticalSection(&m_CriticalSection);	

	CString strTemp;
	strTemp.Format("ZOOM%d",iNum);
	CString data;
	data.Format("%d",iZOOM);
	WritePrivateProfileString(
		"STATE",
		strTemp,
		data,
		".\\ptzstate.ini"
		);
	strTemp.Format("TILT%d",iNum);
	data.Format("%d",iTILT);
	WritePrivateProfileString(
		"STATE",
		strTemp,
		data,
		".\\ptzstate.ini"
		);

	strTemp.Format("PAN%d",iNum);
	data.Format("%d",iPAN);
	WritePrivateProfileString(
		"STATE",
		strTemp,
		data,
		".\\ptzstate.ini"
		);

	::EnterCriticalSection(&m_CriticalSection);
	m_iPan = iPAN;
	m_iTilt = iTILT;
	m_iZoom = iZOOM;
	::LeaveCriticalSection(&m_CriticalSection);
	
	::SendMessage(m_hWnd, PTZ_MESSAGE, (WPARAM) 1, 0);


	return true;
}

bool CPTZCtrl::ObtainData(int *iL, int *iT, int *iP, int *iZ)
{
	::EnterCriticalSection(&m_CriticalSection);
	*iP = m_iPan;
	*iT = m_iTilt;
	*iZ = m_iZoom;
	*iL = m_iLabel;
	::LeaveCriticalSection(&m_CriticalSection);
	return true;
}
