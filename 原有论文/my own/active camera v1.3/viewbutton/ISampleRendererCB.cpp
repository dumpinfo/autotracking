// ISampleRendererCB.cpp: implementation of the CISampleRendererCB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Viewbutton.h"
#include "ISampleRendererCB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CISampleRendererCB::CISampleRendererCB(CISampleGrabberCB *pISampleGrabberCB)
{
	// Initialize member variables
	m_pIDirectDraw = NULL;
	m_pIPrimarySurface = m_pIOffScreenSurface = NULL;
	m_pFrameBuffer = NULL;
	m_pBmpInfo = NULL;
	m_lVideoStep = m_lVideoHeight = m_lVideoWidth = 0L;
	m_wBPP = 0u;
	m_hDisplayWnd = NULL;
	m_pISampleGrabberCB = pISampleGrabberCB;
	m_lLetGrabberPaint = 1L;

	// Create critical section
	::InitializeCriticalSection(&m_CriticalSection);
}

CISampleRendererCB::~CISampleRendererCB()
{
	// Release resources first
	Destroy();

	// Delete critical section
	::DeleteCriticalSection(&m_CriticalSection);
}

ULONG CISampleRendererCB::AddRef()
{
	return 1u;
}

ULONG CISampleRendererCB::Release()
{
	return 1u;
}

HRESULT CISampleRendererCB::QueryInterface(REFIID iid, void **ppvObject)
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

HRESULT CISampleRendererCB::SampleCB(double SampleTime, IMediaSample *pSample)
{
	// Frame processing routine
	BYTE *pBuffer = NULL;
	long lBufferLen = 0L;
	pSample->GetPointer(&pBuffer);
	lBufferLen = pSample->GetSize();
	FrameCallBack(pBuffer, lBufferLen);

	return S_OK;
}

HRESULT CISampleRendererCB::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	// Frame processing routine
	FrameCallBack(pBuffer, BufferLen);

	return S_OK;
}

void CISampleRendererCB::FrameCallBack(BYTE *pBuffer, long lBufferLen)
{
	long lFrameBufferLen = m_lVideoStep * m_lVideoHeight;
	if(pBuffer && lBufferLen >= lFrameBufferLen)
	{
		// Copy data to frame buffer
		BOOL bRepaint = FALSE;
		::EnterCriticalSection(&m_CriticalSection);
		if(bRepaint = (BOOL)m_pFrameBuffer)
		{
			memcpy(m_pFrameBuffer, pBuffer, lFrameBufferLen);
		}
		::LeaveCriticalSection(&m_CriticalSection);

		// Clear tag and let display window repaint
		if(bRepaint && (!m_pISampleGrabberCB || 
			!m_pISampleGrabberCB->ShouldUseCustomPaint()) && ::IsWindow(m_hDisplayWnd))
		{
			::InterlockedExchange(&m_lLetGrabberPaint, 0L);
			::InvalidateRect(m_hDisplayWnd, NULL, FALSE);
		}
	}
}

HRESULT CISampleRendererCB::Initialize(VIDEOINFOHEADER *pVideoInfoHdr, HWND hDisplayWnd)
{
	// Release resources first
	Destroy();

	// Validate parameters
	if(!pVideoInfoHdr || pVideoInfoHdr->bmiHeader.biWidth <= 0L || 
		pVideoInfoHdr->bmiHeader.biHeight <= 0L|| 
		(pVideoInfoHdr->bmiHeader.biBitCount != 16u && 
		pVideoInfoHdr->bmiHeader.biBitCount != 24u && 
		pVideoInfoHdr->bmiHeader.biBitCount != 32u || 
		pVideoInfoHdr->bmiHeader.biCompression != BI_RGB) && 
		(pVideoInfoHdr->bmiHeader.biBitCount != 16u || 
		pVideoInfoHdr->bmiHeader.biCompression != BI_BITFIELDS) || 
		!::IsWindow(hDisplayWnd))
	{
		return E_FAIL;
	}

	// Create and initialize DirectDraw object
	if(FAILED(::DirectDrawCreate(NULL, &m_pIDirectDraw, NULL)))
	{
		Destroy();
		return E_FAIL;
	}
	if(FAILED(m_pIDirectDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL)))
	{
		Destroy();
		return E_FAIL;
	}

	// Create primary surface
	DDSURFACEDESC ddSurfaceDesc;
	memset(&ddSurfaceDesc, 0, sizeof(DDSURFACEDESC));
	ddSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	ddSurfaceDesc.dwFlags = DDSD_CAPS;
	ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if(FAILED(m_pIDirectDraw->CreateSurface(&ddSurfaceDesc, &m_pIPrimarySurface, NULL)))
	{
		Destroy();
		return E_FAIL;
	}

	// Create off-screen surface
	memset(&ddSurfaceDesc, 0, sizeof(DDSURFACEDESC));
	ddSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddSurfaceDesc.dwWidth = (DWORD)pVideoInfoHdr->bmiHeader.biWidth;
	ddSurfaceDesc.dwHeight = (DWORD)pVideoInfoHdr->bmiHeader.biHeight;
	if(FAILED(m_pIDirectDraw->CreateSurface(&ddSurfaceDesc, &m_pIOffScreenSurface, NULL)))
	{
		Destroy();
		return E_FAIL;
	}

	// Create, initialize and install clipper
	IDirectDrawClipper *pIClipper = NULL;
	if(FAILED(m_pIDirectDraw->CreateClipper(0u, &pIClipper, NULL)))
	{
		Destroy();
		return E_FAIL;
	}
	if(FAILED(pIClipper->SetHWnd(0u, hDisplayWnd)) || 
		FAILED(m_pIPrimarySurface->SetClipper(pIClipper)))
	{
		pIClipper->Release();
		Destroy();
		return E_FAIL;
	}
	pIClipper->Release();

	// Record frame parameters
	m_wBPP = pVideoInfoHdr->bmiHeader.biBitCount;
	m_lVideoWidth = pVideoInfoHdr->bmiHeader.biWidth;
	m_lVideoHeight = pVideoInfoHdr->bmiHeader.biHeight;
	m_lVideoStep = (m_wBPP / 8L * m_lVideoWidth + 3L) / 4L * 4L;

	// Allocate and initialize allocate buffer
	::EnterCriticalSection(&m_CriticalSection);
	m_pFrameBuffer = new BYTE[m_lVideoStep * m_lVideoHeight];
	memset(m_pFrameBuffer, 0, m_lVideoStep * m_lVideoHeight);
	::LeaveCriticalSection(&m_CriticalSection);

	// Allocate and initialize bitmap info
	if(pVideoInfoHdr->bmiHeader.biCompression == BI_RGB)
	{
		m_pBmpInfo = (BITMAPINFO *)new BYTE[sizeof(BITMAPINFOHEADER)];
		memset(m_pBmpInfo, 0, sizeof(BITMAPINFOHEADER));
	}
	else
	{
		m_pBmpInfo = (BITMAPINFO *)new BYTE[sizeof(BITMAPINFOHEADER) + 
			3 * sizeof(RGBQUAD)];
		memset(m_pBmpInfo, 0, sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD));
		((DWORD *)m_pBmpInfo->bmiColors)[0] = 0xF800;
		((DWORD *)m_pBmpInfo->bmiColors)[1] = 0x07E0;
		((DWORD *)m_pBmpInfo->bmiColors)[2] = 0x001F;
	}
	m_pBmpInfo->bmiHeader = pVideoInfoHdr->bmiHeader;

	// Record display window handle
	m_hDisplayWnd = hDisplayWnd;

	// Let it repaint
	::InvalidateRect(m_hDisplayWnd, NULL, FALSE);

	return S_OK;
}

void CISampleRendererCB::Destroy()
{
	HWND hDisplayWnd = m_hDisplayWnd;

	// Release resources and reset variables
	if(m_pIOffScreenSurface)
	{
		m_pIOffScreenSurface->Release();
		m_pIOffScreenSurface = NULL;
	}
	if(m_pIPrimarySurface)
	{
		m_pIPrimarySurface->Release();
		m_pIPrimarySurface = NULL;
	}
	if(m_pIDirectDraw)
	{
		m_pIDirectDraw->Release();
		m_pIDirectDraw = NULL;
	}
	::EnterCriticalSection(&m_CriticalSection);
	if(m_pFrameBuffer)
	{
		delete []m_pFrameBuffer;
		m_pFrameBuffer = NULL;
	}
	::LeaveCriticalSection(&m_CriticalSection);
	if(m_pBmpInfo)
	{
		delete [](BYTE *)m_pBmpInfo;
		m_pBmpInfo = NULL;
	}
	::InterlockedExchange(&m_lVideoWidth, 0L);
	::InterlockedExchange(&m_lVideoHeight, 0L);
	::InterlockedExchange(&m_lVideoStep, 0L);
	m_wBPP = 0u;
	m_hDisplayWnd = NULL;

	// Let display repaint later
	if(::IsWindow(hDisplayWnd))
	{
		::InvalidateRect(hDisplayWnd, NULL, TRUE);
	}
}

HRESULT CISampleRendererCB::PaintVideoFrame(RECT &rtDisplay)
{
	if(m_pISampleGrabberCB && m_lLetGrabberPaint)
	{
		HRESULT hr = E_FAIL;
		if(::IsWindow(m_hDisplayWnd))
		{
			HDC hDC = ::GetDC(m_hDisplayWnd);
			hr = m_pISampleGrabberCB->PaintReprocessedFrame(hDC, rtDisplay);
			::ReleaseDC(m_hDisplayWnd, hDC);
		}
		return hr;
	}
	else
	{
		return PaintDirectDraw(rtDisplay) || PaintGDI(rtDisplay) ? S_OK : E_FAIL;
	}
}

BOOL CISampleRendererCB::PaintDirectDraw(RECT rtDisplay)
{
	// Paint current frame using DirectDraw
	if(m_pIPrimarySurface && m_pIOffScreenSurface && ::IsWindow(m_hDisplayWnd))
	{
		// Try to restore losted surfaces
		if(FAILED(m_pIPrimarySurface->IsLost()))
		{
			if(FAILED(m_pIPrimarySurface->Restore()))
			{
				return FALSE;
			}
		}
		if(FAILED(m_pIOffScreenSurface->IsLost()))
		{
			if(FAILED(m_pIOffScreenSurface->Restore()))
			{
				return FALSE;
			}
		}

		// Lock off-screen surface
		DDSURFACEDESC ddSurfaceDesc;
		memset(&ddSurfaceDesc, 0, sizeof(DDSURFACEDESC));
		ddSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
		if(FAILED(m_pIOffScreenSurface->Lock(NULL, &ddSurfaceDesc, 
			DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL)))
		{
			return FALSE;
		}

		// Validate surface property
		long lVideoHeight = m_lVideoHeight;
		long lVideoWidthBytes = m_wBPP / 8L * m_lVideoWidth;
		long lVideoStep = m_lVideoStep;
		if(!ddSurfaceDesc.lpSurface || ddSurfaceDesc.lPitch < lVideoWidthBytes || 
			ddSurfaceDesc.dwHeight < (DWORD)lVideoHeight)
		{
			m_pIOffScreenSurface->Unlock(ddSurfaceDesc.lpSurface);
			return FALSE;
		}

		// Copy data
		::EnterCriticalSection(&m_CriticalSection);
		if(m_pFrameBuffer)
		{
			BYTE *pSurfaceBuffer = (BYTE *)ddSurfaceDesc.lpSurface;
			BYTE *pFrameBuffer = m_pFrameBuffer + lVideoStep * (lVideoHeight - 1);
			for(long i=0; i<lVideoHeight; i++)
			{
				memcpy(pSurfaceBuffer, pFrameBuffer, lVideoWidthBytes);
				pSurfaceBuffer += ddSurfaceDesc.lPitch;
				pFrameBuffer -= lVideoStep;
			}
		}
		::LeaveCriticalSection(&m_CriticalSection);

		// Unlock surface
		m_pIOffScreenSurface->Unlock(ddSurfaceDesc.lpSurface);

		// Blt to primary surface
		::ClientToScreen(m_hDisplayWnd, (POINT *)&rtDisplay);
		::ClientToScreen(m_hDisplayWnd, (POINT *)&rtDisplay + 1);
		return SUCCEEDED(m_pIPrimarySurface->Blt(&rtDisplay, m_pIOffScreenSurface, 
			NULL, DDBLT_WAIT, NULL));
	}
	else
	{
		return FALSE;
	}
}

BOOL CISampleRendererCB::PaintGDI(RECT &rtDisplay)
{
	// Paint current frame using GDI
	if(m_pBmpInfo && ::IsWindow(m_hDisplayWnd))
	{
		BOOL bRet = FALSE;
		HDC hDC = ::GetDC(m_hDisplayWnd);
		::SetStretchBltMode(hDC, COLORONCOLOR);
		::EnterCriticalSection(&m_CriticalSection);
		if(m_pFrameBuffer)
		{
			bRet = ::StretchDIBits(hDC, rtDisplay.left, rtDisplay.top, 
				rtDisplay.right - rtDisplay.left, rtDisplay.bottom - rtDisplay.top, 
				0, 0, m_lVideoWidth, m_lVideoHeight, m_pFrameBuffer, m_pBmpInfo, 
				DIB_RGB_COLORS, SRCCOPY) != GDI_ERROR;
		}
		::LeaveCriticalSection(&m_CriticalSection);
		::ReleaseDC(m_hDisplayWnd, hDC);
		return bRet;
	}
	else
	{
		return FALSE;
	}
}

void CISampleRendererCB::NotifyGrabberPaint(long lLetGrabberPaint /* = 1L */)
{
	// Set tag and let display window repaint
	::InterlockedExchange(&m_lLetGrabberPaint, lLetGrabberPaint);
	if(::IsWindow(m_hDisplayWnd))
	{
		::InvalidateRect(m_hDisplayWnd, NULL, FALSE);
	}
}

HRESULT CISampleRendererCB::CopyVideoFrame()
{
	HRESULT hr = E_FAIL;
	if(::IsWindow(m_hDisplayWnd))
	{
		// Empty clipboard
		::OpenClipboard(m_hDisplayWnd);
		::EmptyClipboard();

		// Copy data
		if(m_lVideoWidth > 0 && m_lVideoHeight > 0)
		{
			HDC hDC = ::GetDC(m_hDisplayWnd);
			HDC hMemDC = ::CreateCompatibleDC(hDC);
			HBITMAP hFrameBmp = ::CreateCompatibleBitmap(hDC, 
				(int)m_lVideoWidth, (int)m_lVideoHeight);
			HBITMAP hOldBmp = (HBITMAP)::SelectObject(hMemDC, hFrameBmp);
			if(m_pISampleGrabberCB && m_lLetGrabberPaint)
			{
				RECT rtDisplay = {0, 0, m_lVideoWidth, m_lVideoHeight};
				hr = m_pISampleGrabberCB->PaintReprocessedFrame(hMemDC, rtDisplay);
			}
			else if(m_pBmpInfo)
			{
				::SetStretchBltMode(hMemDC, COLORONCOLOR);
				::EnterCriticalSection(&m_CriticalSection);
				if(m_pFrameBuffer)
				{
					hr = ::StretchDIBits(hMemDC, 0, 0, m_lVideoWidth, m_lVideoHeight, 
						0, 0, m_lVideoWidth, m_lVideoHeight, m_pFrameBuffer, m_pBmpInfo, 
						DIB_RGB_COLORS, SRCCOPY) != GDI_ERROR ? S_OK : E_FAIL;
				}
				::LeaveCriticalSection(&m_CriticalSection);
			}
			::SelectObject(hMemDC, hOldBmp);
			::SetClipboardData(CF_BITMAP, hFrameBmp);
			::DeleteDC(hMemDC);
			::ReleaseDC(m_hDisplayWnd, hDC);
		}

		// Close clipboard
		::CloseClipboard();
	}

	return hr;
}
