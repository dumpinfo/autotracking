// ISampleGrabberCB.cpp: implementation of the CISampleGrabberCB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Viewbutton.h"
#include "ISampleGrabberCB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CISampleGrabberCB::CISampleGrabberCB(CISampleRendererCB *pISampleRendererCB)
{
	// Initialize data
	m_nSceneID = 0;
	m_bActivated = TRUE;
	m_bReprocessed = m_bBlackFrame = FALSE;
	m_lVideoWidth = m_lVideoHeight = m_lVideoStep = 0L;
	m_pReprocessedFrameBuffer = m_pFrameBuffer = m_pBmpFileBuffer = NULL;
	m_pISampleRendererCB = pISampleRendererCB;
	m_szImageDirectory.Format("");

	// Create critical section
	::InitializeCriticalSection(&m_CriticalSection);
}

CISampleGrabberCB::~CISampleGrabberCB()
{
	// Release resources first
	Destroy();

	// Delete critical section
	::DeleteCriticalSection(&m_CriticalSection);
}

ULONG CISampleGrabberCB::AddRef()
{
	return 1u;
}

ULONG CISampleGrabberCB::Release()
{
	return 1u;
}

HRESULT CISampleGrabberCB::QueryInterface(REFIID iid, void **ppvObject)
{
	if(ppvObject)
	{
		if(iid == IID_IUnknown || iid == IID_ISampleGrabberCB)
		{
			*ppvObject = this;
			return S_OK;
		}
		else
		{
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}
	}
	else
	{
		return E_POINTER;
	}
}

HRESULT CISampleGrabberCB::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	// Frame processing routine
	FrameCallBack(pBuffer, BufferLen);

	return S_OK;
}

HRESULT CISampleGrabberCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
	// Frame processing routine
	BYTE *pBuffer = NULL;
	long lBufferLen = 0L;
	pSample->GetPointer(&pBuffer);
	lBufferLen = pSample->GetSize();
	FrameCallBack(pBuffer, lBufferLen);

	return S_OK;
}

void CISampleGrabberCB::FrameCallBack(BYTE *pBuffer, long lBufferLen)
{
	// Proctect
	::EnterCriticalSection(&m_CriticalSection);

	// Just copy data
	if(pBuffer && m_pFrameBuffer && lBufferLen >= m_lVideoStep * m_lVideoHeight)
	{
		memcpy(m_pFrameBuffer, pBuffer, m_lVideoStep * m_lVideoHeight);
	}

	// hhj: Add your frame processing code here
	if(m_bActivated)
	{
	}

	// Clear tags
	m_bReprocessed = m_bBlackFrame = FALSE;

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

HRESULT CISampleGrabberCB::Initialize(VIDEOINFOHEADER *pVideoInfoHdr)
{
	// Release resources
	Destroy();

	// Check parameter
	if(!pVideoInfoHdr || pVideoInfoHdr->bmiHeader.biWidth <= 0L || 
		pVideoInfoHdr->bmiHeader.biHeight <= 0L || 
		pVideoInfoHdr->bmiHeader.biCompression != BI_RGB || 
		pVideoInfoHdr->bmiHeader.biBitCount != 24u)
	{
		return E_FAIL;
	}

	// Protect data
	::EnterCriticalSection(&m_CriticalSection);

	// hhj: Add your initialization code here

	// Allocate resources and initialize them
	m_lVideoWidth = pVideoInfoHdr->bmiHeader.biWidth;
	m_lVideoHeight = pVideoInfoHdr->bmiHeader.biHeight;
	m_lVideoStep = (m_lVideoWidth * 3L + 3L) / 4L * 4L;
	m_pBmpFileBuffer = new BYTE[m_lVideoStep * m_lVideoHeight + 
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)];
	m_pFrameBuffer = m_pBmpFileBuffer + 
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	m_pReprocessedFrameBuffer = new BYTE[m_lVideoStep * m_lVideoHeight];
	memset(m_pBmpFileBuffer, 0, m_lVideoStep * m_lVideoHeight + 
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	memset(m_pReprocessedFrameBuffer, 0, m_lVideoStep * m_lVideoHeight);
	m_bReprocessed = FALSE;
	m_bBlackFrame = TRUE;

	// Initialize bitmap file structure
	BITMAPFILEHEADER &bmpFileHdr = *(BITMAPFILEHEADER *)m_pBmpFileBuffer;
	BITMAPINFOHEADER &bmpInfoHdr = *(BITMAPINFOHEADER *)(m_pBmpFileBuffer + 
		sizeof(BITMAPFILEHEADER));
	bmpFileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpFileHdr.bfSize = m_lVideoStep * m_lVideoHeight + 
		sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpFileHdr.bfType = 0x4D42;
	bmpInfoHdr.biBitCount = 24u;
	bmpInfoHdr.biCompression = BI_RGB;
	bmpInfoHdr.biHeight = m_lVideoHeight;
	bmpInfoHdr.biPlanes = 1u;
	bmpInfoHdr.biSize = sizeof(BITMAPINFOHEADER);
	bmpInfoHdr.biWidth = m_lVideoWidth;

	// Out
	::LeaveCriticalSection(&m_CriticalSection);

	return S_OK;
}

void CISampleGrabberCB::Destroy()
{
	// Protect data
	::EnterCriticalSection(&m_CriticalSection);

	// Notify renderer
	if(m_pISampleRendererCB)
	{
		m_pISampleRendererCB->NotifyGrabberPaint(0L);
	}

	// Release resources
	if(m_pBmpFileBuffer)
	{
		delete []m_pBmpFileBuffer;
	}
	if(m_pReprocessedFrameBuffer)
	{
		delete []m_pReprocessedFrameBuffer;
	}
	m_bReprocessed = m_bBlackFrame = FALSE;
	m_lVideoWidth = m_lVideoHeight = m_lVideoStep = 0L;
	m_pReprocessedFrameBuffer = m_pFrameBuffer = m_pBmpFileBuffer = NULL;

	// hhj: Add your destruction code here

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

void CISampleGrabberCB::SetSceneID(int nSceneID)
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Save scene ID
	m_nSceneID = nSceneID;

	// hhj: Add your transfer code here

	// Manipulate UI
	ReprocessCurrentFrame();

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

void CISampleGrabberCB::SetRuleFileName(LPCTSTR szRuleFileName)
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// hhj: Add your transfer code here

	// Manipulate UI
	ReprocessCurrentFrame();

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

void CISampleGrabberCB::ReloadRule(int nType, int nRegionID)
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// hhj: Add your transfer code here

	// Manipulate UI
	ReprocessCurrentFrame();

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

void CISampleGrabberCB::AdjustAlgorithmParameters(CWnd *pParent)
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// hhj: Get paremeter here

	// Out
	::LeaveCriticalSection(&m_CriticalSection);

	// hhj: Initilize dialog and DoModal

	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// hhj: Set paremeter here

	// Manipulate UI
	ReprocessCurrentFrame();

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

BOOL CISampleGrabberCB::IsAlgorithmActivated()
{
	BOOL bRet;

	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Retieve data
	bRet = m_bActivated;

	// Out
	::LeaveCriticalSection(&m_CriticalSection);

	return bRet;
}

void CISampleGrabberCB::SetAlgorithmWorkingState(BOOL bActivated)
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Save new algorithm working state
	BOOL bFormerActivated = m_bActivated;
	m_bActivated = bActivated;

	// hhj: Let algorithm module empty former context data
	if(m_bActivated && !bFormerActivated)
	{
	}

	// Manipulate UI
	ReprocessCurrentFrame();

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

BOOL CISampleGrabberCB::QueryAlarmState()
{
	BOOL bRet = FALSE;
	
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// hhj: Ask algorithm module about current state

	// Out
	::LeaveCriticalSection(&m_CriticalSection);

	return bRet;
}

void CISampleGrabberCB::PickUpAlarmInfo(CListCtrl *pListCtrl)
{
	if(pListCtrl)
	{
		// Protect
		::EnterCriticalSection(&m_CriticalSection);

		// hhj: Ask algorithm module about new alarm information records

		// Out
		::LeaveCriticalSection(&m_CriticalSection);
	}
}

void CISampleGrabberCB::InsertBlackFrame()
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Set frame buffer to black and post notification
	if(m_pFrameBuffer && m_pReprocessedFrameBuffer && !m_bBlackFrame)
	{
		// Set frame data to black plane
		memset(m_pFrameBuffer, 0, m_lVideoStep * m_lVideoHeight);
		memset(m_pReprocessedFrameBuffer, 0, m_lVideoStep * m_lVideoHeight);

		// Modify tages
		m_bBlackFrame = TRUE;
		m_bReprocessed = FALSE;

		// Notify renderer
		if(m_pISampleRendererCB)
		{
			m_pISampleRendererCB->NotifyGrabberPaint();
		}
	}

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

void CISampleGrabberCB::ReprocessCurrentFrame()
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Reprocess saved current frame
	if(m_pFrameBuffer && m_pReprocessedFrameBuffer && !m_bBlackFrame)
	{
		// Copy data
		memcpy(m_pReprocessedFrameBuffer, m_pFrameBuffer, m_lVideoStep * m_lVideoHeight);

		// hhj: Add your frame processing code here
		if(m_bActivated)
		{
		}

		// Set tag
		m_bReprocessed = TRUE;

		// Notify renderer
		if(m_pISampleRendererCB)
		{
			m_pISampleRendererCB->NotifyGrabberPaint();
		}
	}

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

HRESULT CISampleGrabberCB::PaintReprocessedFrame(HDC hDC, RECT &rtDisplay)
{
	HRESULT hr = E_FAIL;

	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Paint current processed frame
	if(hDC && m_pBmpFileBuffer && m_pReprocessedFrameBuffer)
	{
		::SetStretchBltMode(hDC, COLORONCOLOR);
//		CClientDC dc(GetDlgItem(IDC_DISP));


		CDC dDC;
		dDC.Attach(hDC);
		CRgn crgn;
		crgn.CreateRectRgn(rtDisplay.left, rtDisplay.top,rtDisplay.right,rtDisplay.bottom);
	
		dDC.SelectClipRgn(&crgn,RGN_AND);

		hr = StretchDIBits(hDC,rtDisplay.right, rtDisplay.bottom, 
			rtDisplay.right - rtDisplay.left, rtDisplay.bottom - rtDisplay.top, 
			0, 0, m_lVideoWidth, m_lVideoHeight, m_pReprocessedFrameBuffer, 
			(BITMAPINFO *)(m_pBmpFileBuffer + sizeof(BITMAPFILEHEADER)), 
			DIB_RGB_COLORS, SRCCOPY) != GDI_ERROR ? S_OK : E_FAIL;
	}

	// Out
	::LeaveCriticalSection(&m_CriticalSection);

	return hr;
}

BOOL CISampleGrabberCB::ShouldUseCustomPaint()
{
	BOOL bShouldUseCustomPaint = FALSE;

	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	bShouldUseCustomPaint = m_bReprocessed || m_bBlackFrame;

	// Out
	::LeaveCriticalSection(&m_CriticalSection);

	return bShouldUseCustomPaint;
}

HRESULT CISampleGrabberCB::SaveCurrentFrame(CString &szImageFileName, BOOL bBmp /* = TRUE */)
{
	HRESULT hr = E_FAIL;

	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Format file name
	if(!szImageFileName.GetLength())
	{
		SYSTEMTIME timeCurrent;
		memset(&timeCurrent, 0, sizeof(SYSTEMTIME));
		::GetLocalTime(&timeCurrent);
		szImageFileName.Format("%s第%d路 %04d年%02d月%02d日 %02d时%02d分%02d.%03d秒", 
			m_szImageDirectory, m_nSceneID, timeCurrent.wYear, timeCurrent.wMonth, 
			timeCurrent.wDay, timeCurrent.wHour, timeCurrent.wMinute, 
			timeCurrent.wSecond, timeCurrent.wMilliseconds);
		if(bBmp)
		{
			szImageFileName += ".bmp";
		}
		else
		{
			szImageFileName += ".jpg";
		}
	}

	// Call overrided version
	hr = SaveCurrentFrame((LPCTSTR)szImageFileName, bBmp);

	// Out
	;::LeaveCriticalSection(&m_CriticalSection);

	// Call overrided version
	return hr;
}

HRESULT CISampleGrabberCB::SaveCurrentFrame(LPCTSTR szImageFileName, BOOL bBmp /* = TRUE */)
{
	HRESULT hr = E_FAIL;
	
	// Protect
	::EnterCriticalSection(&m_CriticalSection);
	
	// Save current frame
	if(szImageFileName && m_pBmpFileBuffer)
	{
		if(bBmp)
		{
			HANDLE hFile = ::CreateFile(szImageFileName, GENERIC_WRITE, 0, 
				NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hFile != INVALID_HANDLE_VALUE)
			{
				DWORD dwWritten = 0u;
				DWORD dwFileSize = (DWORD)(m_lVideoStep * m_lVideoHeight + 
					sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
				::WriteFile(hFile, m_pBmpFileBuffer, dwFileSize, &dwWritten, NULL);
				hr = dwWritten == dwFileSize ? S_OK : E_FAIL;
				::CloseHandle(hFile);
			}
		}
		else
		{
			CxImage xImage;
			hr = xImage.Decode(m_pBmpFileBuffer, m_lVideoStep * m_lVideoHeight + 
				sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), 
				CXIMAGE_FORMAT_BMP) && 
				xImage.Save(szImageFileName, CXIMAGE_FORMAT_JPG) && 
				xImage.Destroy() ? S_OK : E_FAIL;
		}
	}
	
	// Out
	::LeaveCriticalSection(&m_CriticalSection);
	
	return hr;
}

void CISampleGrabberCB::SetImageDirectory(LPCTSTR szImageDirectory)
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Set image directory
	if(szImageDirectory)
	{
		m_szImageDirectory = szImageDirectory;
	}
	else
	{
		m_szImageDirectory.Format("");
	}

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

void CISampleGrabberCB::SetParameterFileName(LPCTSTR szParameterFileName)
{
	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// hhj: Add your transfer code here

	// Manipulate UI
	ReprocessCurrentFrame();

	// Out
	::LeaveCriticalSection(&m_CriticalSection);
}

HRESULT CISampleGrabberCB::PaintReprocessedFrame(CClientDC *pDC, CRect rtDisplay)
{
	HRESULT hr = E_FAIL;

	// Protect
	::EnterCriticalSection(&m_CriticalSection);

	// Paint current processed frame
	if(pDC && m_pBmpFileBuffer && m_pReprocessedFrameBuffer)
	{
//		::SetStretchBltMode(hDC, COLORONCOLOR);
//		CClientDC dc(GetDlgItem(IDC_DISP));


//		CDC dDC;
//		dDC.Attach(hDC);
//		CRgn crgn;
//		crgn.CreateRectRgn(rtDisplay.left, rtDisplay.top,rtDisplay.right,rtDisplay.bottom);
	
//		dDC.SelectClipRgn(&crgn,RGN_AND);

		hr = StretchDIBits(pDC->m_hDC,0, 0, 
			rtDisplay.right - rtDisplay.left, rtDisplay.bottom - rtDisplay.top, 
			0, 0, m_lVideoWidth, m_lVideoHeight, m_pReprocessedFrameBuffer, 
			(BITMAPINFO *)(m_pBmpFileBuffer + sizeof(BITMAPFILEHEADER)), 
			DIB_RGB_COLORS, SRCCOPY) != GDI_ERROR ? S_OK : E_FAIL;
	}

	// Out
	::LeaveCriticalSection(&m_CriticalSection);

	return hr;
}

void CISampleGrabberCB::ObtainFrame(unsigned char *pDst)
{
	::EnterCriticalSection(&m_CriticalSection);

	memcpy(pDst, m_pFrameBuffer, m_lVideoStep * m_lVideoHeight);
	::LeaveCriticalSection(&m_CriticalSection);
}
