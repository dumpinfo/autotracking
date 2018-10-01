// VideoDirectShow.cpp: implementation of the CVideoDirectShow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "viewbutton.h"
#include "VideoDirectShow.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVideoDirectShow::CVideoDirectShow():
m_ISampleGrabberCB(&m_ISampleRendererCB), m_ISampleRendererCB(&m_ISampleGrabberCB)
{
	// Initialize parameters
	m_pIGraphBuilder = NULL;
	m_bCanUseGraphBuilder = TRUE;
	m_bBuffering = m_bCanSeek = m_bCanAdjust = FALSE;
}

CVideoDirectShow::~CVideoDirectShow()
{
	ReleaseFilterGraph();
}

BOOL CVideoDirectShow::OpenFile(LPWSTR pFilename, HWND hWnd, int BitsPixel, CRect * pRect)
{
	// Release filter graph firstly
	ReleaseFilterGraph();
	m_hWnd = hWnd;

	// Necessary parameters
	HRESULT hr = S_OK;
	AM_MEDIA_TYPE amMediaType;
	IBaseFilter *pISourceFilter = NULL;
	IBaseFilter *pISampleGrabber = NULL;
	IBaseFilter *pISampleRenderer = NULL;
	IBaseFilter *pIVideoRenderer = NULL;
	ISampleGrabber *pISampleGrabber2 = NULL;
	ISampleGrabber *pISampleRenderer2 = NULL;
	IVideoWindow *pIVideoWindow = NULL;
	IMediaEventEx *pIMediaEventEx = NULL;
	IMediaControl *pIMediaControl = NULL;

	// Create GraphBuilder object
	if(SUCCEEDED(hr))
	{
		hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, 
			CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (LPVOID *)&m_pIGraphBuilder);
	}

	// Add source filter
	if(SUCCEEDED(hr))
	{
		hr = m_pIGraphBuilder->AddSourceFilter(
			pFilename, L"Source Filter", &pISourceFilter);
	}	

	// Get source filter
	if(SUCCEEDED(hr))
	{
		m_bCanUseGraphBuilder = TRUE;
		hr = m_pIGraphBuilder->FindFilterByName(L"Source Filter", &pISourceFilter);
	}

	// Create sample grabber, sample renderer and video renderer
	if(SUCCEEDED(hr))
	{
		SUCCEEDED(hr = ::CoCreateInstance(CLSID_SampleGrabber, NULL, 
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pISampleGrabber)) && 
		SUCCEEDED(hr = ::CoCreateInstance(CLSID_SampleGrabber, NULL, 
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pISampleRenderer)) && 
		SUCCEEDED(hr = ::CoCreateInstance(CLSID_VideoRenderer, NULL, 
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pIVideoRenderer));
	}

	// Add them into filter graph
	if(SUCCEEDED(hr))
	{
		SUCCEEDED(hr = m_pIGraphBuilder->AddFilter(
			pISampleGrabber, L"Sample Grabber")) && 
		SUCCEEDED(hr = m_pIGraphBuilder->AddFilter(
			pISampleRenderer, L"Sample Renderer")) && 
		SUCCEEDED(hr = m_pIGraphBuilder->AddFilter(
			pIVideoRenderer, L"Video Renderer"));
	}

	// Initialize sample grabber
	if(SUCCEEDED(hr) && SUCCEEDED(hr = pISampleGrabber->QueryInterface(
		IID_ISampleGrabber, (void **)&pISampleGrabber2)))
	{
		memset(&amMediaType, 0, sizeof(AM_MEDIA_TYPE));
		amMediaType.majortype = MEDIATYPE_Video;
		amMediaType.subtype = MEDIASUBTYPE_RGB24;
		amMediaType.formattype = GUID_NULL;
		SUCCEEDED(hr = pISampleGrabber2->SetMediaType(&amMediaType)) && 
		SUCCEEDED(hr = pISampleGrabber2->SetCallback(&m_ISampleGrabberCB, 1L));
	}

	// Initialize sample renderer
	if(SUCCEEDED(hr) && SUCCEEDED(hr = pISampleRenderer->QueryInterface(
		IID_ISampleGrabber, (void **)&pISampleRenderer2)))
	{
		if(BitsPixel == 16 || BitsPixel == 24 || BitsPixel == 32)
		{
			memset(&amMediaType, 0, sizeof(AM_MEDIA_TYPE));
			amMediaType.majortype = MEDIATYPE_Video;
			amMediaType.subtype = (BitsPixel == 16 ? MEDIASUBTYPE_RGB565 : 
				(BitsPixel == 24 ? MEDIASUBTYPE_RGB24 : MEDIASUBTYPE_RGB32));
			amMediaType.formattype = GUID_NULL;
			SUCCEEDED(hr = pISampleRenderer2->SetMediaType(&amMediaType)) && 
			SUCCEEDED(hr = pISampleRenderer2->SetCallback(&m_ISampleRendererCB, 1L));
		}
		else
		{
			hr = E_FAIL;
		}
	}

	// Connect filters
	if(SUCCEEDED(hr))
	{
		SUCCEEDED(hr = ConnectFilters(pISourceFilter, pISampleGrabber)) && 
		SUCCEEDED(hr = ConnectFilters(pISampleGrabber, pISampleRenderer)) && 
		SUCCEEDED(hr = ConnectFilters(pISampleRenderer, pIVideoRenderer));
	}

	// Get connected media type of sample grabber and initialize m_ISampleGrabberCB
	if(SUCCEEDED(hr))
	{
		memset(&amMediaType, 0, sizeof(AM_MEDIA_TYPE));
		if(SUCCEEDED(hr = pISampleGrabber2->GetConnectedMediaType(&amMediaType)))
		{
			if(amMediaType.formattype == FORMAT_VideoInfo && 
				amMediaType.cbFormat >= sizeof(VIDEOINFOHEADER) && 
				amMediaType.pbFormat)
			{
				// Initialize m_ISampleGrabberCB here
				hr = m_ISampleGrabberCB.Initialize(
					(VIDEOINFOHEADER *)amMediaType.pbFormat);
			}
			else
			{
				hr = E_FAIL;
			}
			if(amMediaType.pbFormat)
			{
				::CoTaskMemFree(amMediaType.pbFormat);
			}
		}
	}

	// Get connected media type of sample renderer and initialize m_ISampleRendererCB
	if(SUCCEEDED(hr))
	{
		memset(&amMediaType, 0, sizeof(AM_MEDIA_TYPE));
		if(SUCCEEDED(hr = pISampleRenderer2->GetConnectedMediaType(&amMediaType)))
		{
			if(amMediaType.formattype == FORMAT_VideoInfo && 
				amMediaType.cbFormat >= sizeof(VIDEOINFOHEADER) && 
				amMediaType.pbFormat)
			{
				// Initialize m_ISampleRendererCB here
				VIDEOINFOHEADER *pVideoInfoHdr = (VIDEOINFOHEADER *)amMediaType.pbFormat;
				if(SUCCEEDED(hr = m_ISampleRendererCB.Initialize(
					pVideoInfoHdr, hWnd)))
				{
					// Calculate clipping rect
					pRect->right = pVideoInfoHdr->bmiHeader.biWidth;
					pRect->left = 0;
					pRect->bottom = pVideoInfoHdr->bmiHeader.biHeight;
					pRect->top = 0;
				}
			}
			else
			{
				hr = E_FAIL;
			}
			if(amMediaType.pbFormat)
			{
				::CoTaskMemFree(amMediaType.pbFormat);
			}
		}
	}

	// Initialize video window
	if(SUCCEEDED(hr) && SUCCEEDED(hr = m_pIGraphBuilder->QueryInterface(
		IID_IVideoWindow, (void **)&pIVideoWindow)))
	{
		hr = pIVideoWindow->put_AutoShow(OAFALSE);
	}

	// Install event notification
	if(SUCCEEDED(hr) && SUCCEEDED(hr = m_pIGraphBuilder->QueryInterface(
		IID_IMediaEventEx, (void **)&pIMediaEventEx)))
	{
		hr = pIMediaEventEx->SetNotifyWindow((OAHWND)hWnd, VS_MEDIAEVENT, 0L);
	}

	// Get run-time tags
	if(SUCCEEDED(hr))
	{
		// Does the source filter provides property pages?
		ISpecifyPropertyPages *pISpecifyPropertyPages = NULL;
		if(m_bCanAdjust = SUCCEEDED(pISourceFilter->QueryInterface(
			IID_ISpecifyPropertyPages, (void **)&pISpecifyPropertyPages)))
		{
			pISpecifyPropertyPages->Release();
		}

		// Can the filter graph be seeked?
		IMediaSeeking *pIMediaSeeking = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(
			IID_IMediaSeeking, (void **)&pIMediaSeeking)))
		{
			DWORD dwCaps = AM_SEEKING_CanSeekAbsolute;
			m_bCanSeek = pIMediaSeeking->CheckCapabilities(&dwCaps) == S_OK;
			pIMediaSeeking->Release();
		}
	}

	// Run the graph
	if(SUCCEEDED(hr) && SUCCEEDED(hr = m_pIGraphBuilder->QueryInterface(
		IID_IMediaControl, (void **)&pIMediaControl)))
	{
		hr = pIMediaControl->Run();
	}

	// Clean up
	if(pIMediaControl)
	{
		pIMediaControl->Release();
	}
	if(pIMediaEventEx)
	{
		pIMediaEventEx->Release();
	}
	if(pIVideoWindow)
	{
		pIVideoWindow->Release();
	}
	if(pISampleRenderer2)
	{
		pISampleRenderer2->Release();
	}
	if(pISampleGrabber2)
	{
		pISampleGrabber2->Release();
	}
	if(pIVideoRenderer)
	{
		pIVideoRenderer->Release();
	}
	if(pISampleRenderer)
	{
		pISampleRenderer->Release();
	}
	if(pISampleGrabber)
	{
		pISampleGrabber->Release();
	}
	if(pISourceFilter)
	{
		pISourceFilter->Release();
	}
	return hr;
}

void CVideoDirectShow::ReleaseFilterGraph()
{
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		// Stop it first
		IMediaControl *pIMediaControl = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(IID_IMediaControl, 
			(void **)&pIMediaControl)))
		{
			pIMediaControl->Stop();
			pIMediaControl->Release();
		}

		// Remove all filters
		RemoveAllFilters();

		// Relase GraphBuilder object finally
		m_pIGraphBuilder->Release();
		m_pIGraphBuilder = NULL;
	}

	// Release other resources
	m_ISampleGrabberCB.Destroy();
	m_ISampleRendererCB.Destroy();

	// Reset other parameters
	m_bBuffering = m_bCanSeek = m_bCanAdjust = FALSE;


}

HRESULT CVideoDirectShow::ConnectFilters(IBaseFilter *pIUpFilter, IBaseFilter *pIDownFilter)
{
	// Validation
	if(!pIUpFilter || !pIDownFilter)
	{
		return E_POINTER;
	}
	if(!m_pIGraphBuilder || !m_bCanUseGraphBuilder)
	{
		return E_FAIL;
	}

	// Enum output pins of up stream filter
	HRESULT hr = E_FAIL;
	IEnumPins *pIEnumUpPins = NULL;
	if(SUCCEEDED(pIUpFilter->EnumPins(&pIEnumUpPins)))
	{
		IPin *pIOutputPin = NULL;
		while(pIEnumUpPins->Next(1u, &pIOutputPin, NULL) == S_OK)
		{
			// Check it's direction
			PIN_DIRECTION OutputPinDir = PINDIR_INPUT;
			pIOutputPin->QueryDirection(&OutputPinDir);
			if(OutputPinDir != PINDIR_OUTPUT)
			{
				pIOutputPin->Release();
				pIOutputPin = NULL;
				continue;
			}

			// Check it's connection
			IPin *pIConnectedInputPin = NULL;
			if(SUCCEEDED(pIOutputPin->ConnectedTo(&pIConnectedInputPin)))
			{
				pIConnectedInputPin->Release();
				pIOutputPin->Release();
				pIOutputPin = NULL;
				continue;
			}

			// Enum input pins and connection
			IEnumPins *pIEnumDownPins = NULL;
			if(SUCCEEDED(pIDownFilter->EnumPins(&pIEnumDownPins)))
			{
				IPin *pIInputPin = NULL;
				while(pIEnumDownPins->Next(1u, &pIInputPin, NULL) == S_OK)
				{
					// Check it's direction
					PIN_DIRECTION InputPinDir = PINDIR_OUTPUT;
					pIInputPin->QueryDirection(&InputPinDir);
					if(InputPinDir != PINDIR_INPUT)
					{
						pIInputPin->Release();
						pIInputPin = NULL;
						continue;
					}

					// Check it's connection
					IPin *pIConnectedOutputPin = NULL;
					if(SUCCEEDED(pIInputPin->ConnectedTo(&pIConnectedOutputPin)))
					{
						pIConnectedOutputPin->Release();
						pIInputPin->Release();
						pIInputPin = NULL;
						continue;
					}

					// Try to connect them
					hr = m_pIGraphBuilder->Connect(pIOutputPin, pIInputPin);

					// Release interface
					pIInputPin->Release();
					pIInputPin = NULL;

					// Determine state
					if(SUCCEEDED(hr))
					{
						break;
					}
				}
				pIEnumDownPins->Release();
			}

			// Release interface
			pIOutputPin->Release();
			pIOutputPin = NULL;

			// Determine state
			if(SUCCEEDED(hr))
			{
				break;
			}
		}
		pIEnumUpPins->Release();
	}

	return hr;
}

void CVideoDirectShow::RemoveAllFilters()
{
	// Should be invoked only after the graph have been stopped
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IEnumFilters *pIEnumFilters = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->EnumFilters(&pIEnumFilters)))
		{
			IBaseFilter *pIFilter = NULL;
			while(pIEnumFilters->Next(1u, &pIFilter, NULL) == S_OK)
			{
				m_pIGraphBuilder->RemoveFilter(pIFilter);
				pIFilter->Release();
				pIFilter = NULL;
				pIEnumFilters->Reset();
			}
			pIEnumFilters->Release();
		}
	}
}

void CVideoDirectShow::PaintVideoFrame(CRect display)
{
	m_ISampleRendererCB.PaintVideoFrame(display);
}

void CVideoDirectShow::Step()
{
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IVideoFrameStep *pIVideoFrameStep = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(IID_IVideoFrameStep, 
			(void **)&pIVideoFrameStep)))
		{
			pIVideoFrameStep->Step(1u, NULL);
			pIVideoFrameStep->Release();
		}
	}	
}

void CVideoDirectShow::Play()
{
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IMediaControl *pIMediaControl = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(
			IID_IMediaControl, (void **)&pIMediaControl)))
		{
			pIMediaControl->Run();
			pIMediaControl->Release();

		}
	}
}

void CVideoDirectShow::Pause()
{
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IMediaControl *pIMediaControl = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(
			IID_IMediaControl, (void **)&pIMediaControl)))
		{
			pIMediaControl->Pause();
			pIMediaControl->Release();

		}
	}
}

void CVideoDirectShow::Stop()
{
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IMediaControl *pIMediaControl = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(
			IID_IMediaControl, (void **)&pIMediaControl)))
		{
			if(m_bCanSeek)
			{
				HRESULT hr = E_FAIL;
				IMediaSeeking *pIMediaSeeking = NULL;
				if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(
					IID_IMediaSeeking, (void **)&pIMediaSeeking)))
				{
					LONGLONG llCurrent = (LONGLONG)0;
					hr = pIMediaSeeking->SetPositions(&llCurrent, 
						AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
					pIMediaSeeking->Release();
				}
				if(SUCCEEDED(hr))
				{
					pIMediaControl->StopWhenReady();
				}
				else
				{
					if(SUCCEEDED(pIMediaControl->Stop()))
					{
						m_ISampleGrabberCB.InsertBlackFrame();
					}
				}
			}
			else
			{
				if(SUCCEEDED(pIMediaControl->Stop()))
				{
					m_ISampleGrabberCB.InsertBlackFrame();
				}
			}
			pIMediaControl->Release();

		}
	}
}

bool CVideoDirectShow::GetPosition(LONG *pLCurrent, LONG *pLStop)
{
	IMediaSeeking *pIMediaSeeking = NULL;
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder &&
		SUCCEEDED(m_pIGraphBuilder->QueryInterface(IID_IMediaSeeking, 
		(void **)&pIMediaSeeking)))
	{
//		LONG lCurrent = 0L, lStop = 0L;
		LONGLONG llCurrent = (LONGLONG)0, llStop = (LONGLONG)0;
		if(SUCCEEDED(pIMediaSeeking->GetCurrentPosition(&llCurrent)))
		{
			(*pLCurrent) = (LONG)(llCurrent / (LONGLONG)10000);
		}
		if(SUCCEEDED(pIMediaSeeking->GetStopPosition(&llStop)))
		{
			(*pLStop) = (LONG)(llStop / (LONGLONG)10000);
		}
		pIMediaSeeking->Release();
		if(!m_bCanSeek)
		{
			(*pLStop) = -(*pLStop);
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool CVideoDirectShow::GetFPS( int * pIfps)
{
		IBaseFilter *pIVideoRenderer = NULL;
		if(m_pIGraphBuilder && m_bCanUseGraphBuilder && IsPlaying() && 
			SUCCEEDED(m_pIGraphBuilder->FindFilterByName(L"Video Renderer", 
			&pIVideoRenderer)))
		{
			IQualProp *pIQualProp = NULL;
			if(SUCCEEDED(pIVideoRenderer->QueryInterface(IID_IQualProp, 
				(void **)&pIQualProp)))
			{
				if(SUCCEEDED(pIQualProp->get_AvgFrameRate(pIfps)))
				{
					return true;
				}
				else
				{
					return false;
				}
				pIQualProp->Release();
			}
			else
			{
				return false;
			}
			pIVideoRenderer->Release();
		}
		else
		{
			return false;
		}
}

bool CVideoDirectShow::IsPlaying()
{
	// Check state
	BOOL bRet = FALSE;
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IMediaControl *pIMediaControl = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(IID_IMediaControl, 
			(void **)&pIMediaControl)))
		{
			OAFilterState State = 0;
			pIMediaControl->GetState(0L, &State);
			bRet = State == State_Running;
			pIMediaControl->Release();
		}
	}

	return bRet;
}

bool CVideoDirectShow::GetMediaEvent(long *plEventCode, long *plParam1, long *plParam2)
{
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IMediaEvent *pIMediaEvent = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(
			IID_IMediaEvent, (void **)&pIMediaEvent)))
		{
			bool hl;
			hl = SUCCEEDED(pIMediaEvent->GetEvent(plEventCode, plParam1, plParam2, 0L));
			if(hl)
			{
				pIMediaEvent->FreeEventParams(*plEventCode, *plParam1, *plParam2);		
			}
			pIMediaEvent->Release();
			return hl;
		}

		return false;
	}
	else
	{
		return false;
	}
}

bool CVideoDirectShow::GetState(OAFilterState *pOAFstate)
{
	bool hr = false;
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IMediaControl *pIMediaControl = NULL;
		hr = SUCCEEDED(m_pIGraphBuilder->QueryInterface(IID_IMediaControl, 
			(void **)&pIMediaControl));
		if(hr)
		{
			OAFilterState State = 0;
			pIMediaControl->GetState(0L, pOAFstate);
			pIMediaControl->Release();
		}
	}
	return hr;
}

bool CVideoDirectShow::CanStep()
{
	bool bCanStep = false;
	if(m_pIGraphBuilder && m_bCanUseGraphBuilder)
	{
		IVideoFrameStep *pIVideoFrameStep = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(IID_IVideoFrameStep, 
			(void **)&pIVideoFrameStep)))
		{
			bCanStep = SUCCEEDED(pIVideoFrameStep->CanStep(0L, NULL));
			pIVideoFrameStep->Release();
		}
	}
	return bCanStep;
}

bool CVideoDirectShow::CaptureVideo(IMoniker *pMoniker, HWND hWnd, int BitsPixel, CRect * pRect)
{
	ReleaseFilterGraph();

	// Necessary parameters
	HRESULT hr = S_OK;
	AM_MEDIA_TYPE amMediaType;
	IBaseFilter *pISourceFilter = NULL;
	IBaseFilter *pISampleGrabber = NULL;
	IBaseFilter *pISampleRenderer = NULL;
	IBaseFilter *pIVideoRenderer = NULL;
	ICaptureGraphBuilder2 *pICaptureGraphBuilder2 = NULL;
	IUnknown *pIUnkSourceFilter = NULL;
	ISampleGrabber *pISampleGrabber2 = NULL;
	ISampleGrabber *pISampleRenderer2 = NULL;
	IVideoWindow *pIVideoWindow = NULL;
	IMediaEventEx *pIMediaEventEx = NULL;
	IMediaControl *pIMediaControl = NULL;

	// Create GraphBuilder object
	if(SUCCEEDED(hr))
	{
		hr = ::CoCreateInstance(CLSID_FilterGraph, NULL, 
			CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (LPVOID *)&m_pIGraphBuilder);
	}

	// Create source filter, sample grabber and video renderer
	if(SUCCEEDED(hr) && pMoniker)
	{
		SUCCEEDED(hr = pMoniker->BindToObject(NULL, NULL, 
			IID_IBaseFilter, (void **)&pISourceFilter)) && 
		SUCCEEDED(hr = ::CoCreateInstance(CLSID_SampleGrabber, NULL, 
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pISampleGrabber)) && 
		SUCCEEDED(hr = ::CoCreateInstance(CLSID_SampleGrabber, NULL, 
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pISampleRenderer)) && 
		SUCCEEDED(hr = ::CoCreateInstance(CLSID_VideoRenderer, NULL, 
			CLSCTX_INPROC_SERVER, IID_IBaseFilter, (LPVOID *)&pIVideoRenderer));
	}

	// Add them into filter graph
	if(SUCCEEDED(hr))
	{
		SUCCEEDED(hr = m_pIGraphBuilder->AddFilter(
			pISourceFilter, L"Source Filter")) && 
		SUCCEEDED(hr = m_pIGraphBuilder->AddFilter(
			pISampleGrabber, L"Sample Grabber")) && 
		SUCCEEDED(hr = m_pIGraphBuilder->AddFilter(
			pISampleRenderer, L"Sample Renderer")) && 
		SUCCEEDED(hr = m_pIGraphBuilder->AddFilter(
			pIVideoRenderer, L"Video Renderer"));
	}

	// Create and initialize capture graph builder object
	if(SUCCEEDED(hr) && SUCCEEDED(hr = ::CoCreateInstance(CLSID_CaptureGraphBuilder2, 
		NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, 
		(LPVOID *)&pICaptureGraphBuilder2)))
	{
		hr = pICaptureGraphBuilder2->SetFiltergraph(m_pIGraphBuilder);
	}

	// Let user initialize video format
	if(SUCCEEDED(hr) && SUCCEEDED(hr = pISourceFilter->QueryInterface(
		IID_IUnknown, (void **)&pIUnkSourceFilter)))
	{
		IPin *pIPin = NULL;
		GUID Type = MEDIATYPE_Video;
		GUID Category = PIN_CATEGORY_PREVIEW;
		if((Category = PIN_CATEGORY_PREVIEW, SUCCEEDED(pICaptureGraphBuilder2->FindPin(
			pIUnkSourceFilter, PINDIR_OUTPUT, &Category, &Type, TRUE, 0, &pIPin))) || 
			(Category = PIN_CATEGORY_VIDEOPORT, SUCCEEDED(pICaptureGraphBuilder2->FindPin(
			pIUnkSourceFilter, PINDIR_OUTPUT, &Category, &Type, TRUE, 0, &pIPin))) || 
			(Category = PIN_CATEGORY_CAPTURE, SUCCEEDED(pICaptureGraphBuilder2->FindPin(
			pIUnkSourceFilter, PINDIR_OUTPUT, &Category, &Type, TRUE, 0, &pIPin))))
		{
			ISpecifyPropertyPages *pISpecifyPropertyPages = NULL;
			if(SUCCEEDED(pIPin->QueryInterface(IID_ISpecifyPropertyPages, 
				(void **)&pISpecifyPropertyPages)))
			{
				IUnknown *pIUnkPin = NULL;
				if(SUCCEEDED(pIPin->QueryInterface(IID_IUnknown, (void **)&pIUnkPin)))
				{
					CAUUID Pages;
					memset(&Pages, 0, sizeof(CAUUID));
					if(SUCCEEDED(pISpecifyPropertyPages->GetPages(&Pages)))
					{
						if(AfxGetMainWnd()->GetStyle() & WS_MINIMIZE)
						{
							AfxGetMainWnd()->ShowWindow(SW_SHOWNOACTIVATE);
						}
//						GetSafeOwner()->SetForegroundWindow();
						m_bCanUseGraphBuilder = FALSE;
						::OleCreatePropertyFrame(hWnd, 
							0u, 0u, L"ÊÓÆµ¸ñÊ½", 1u, &pIUnkPin, Pages.cElems, 
							Pages.pElems, 0L, 0u, NULL);
						m_bCanUseGraphBuilder = TRUE;
						::CoTaskMemFree(Pages.pElems);
						
						// Update window
//						AfxGetMainWnd()->SendMessage(VS_UPDATEWINDOW);
					}
					pIUnkPin->Release();
				}
				pISpecifyPropertyPages->Release();
			}
			pIPin->Release();
		}
	}

	// Initialize sample grabber
	if(SUCCEEDED(hr) && SUCCEEDED(hr = pISampleGrabber->QueryInterface(
		IID_ISampleGrabber, (void **)&pISampleGrabber2)))
	{
		memset(&amMediaType, 0, sizeof(AM_MEDIA_TYPE));
		amMediaType.majortype = MEDIATYPE_Video;
		amMediaType.subtype = MEDIASUBTYPE_RGB24;
		amMediaType.formattype = GUID_NULL;
		SUCCEEDED(hr = pISampleGrabber2->SetMediaType(&amMediaType)) && 
		SUCCEEDED(hr = pISampleGrabber2->SetCallback(&m_ISampleGrabberCB, 1L));
	}

	// Initialize sample renderer
	if(SUCCEEDED(hr) && SUCCEEDED(hr = pISampleRenderer->QueryInterface(
		IID_ISampleGrabber, (void **)&pISampleRenderer2)))
	{

		if(BitsPixel == 16 || BitsPixel == 24 || BitsPixel == 32)
		{
			memset(&amMediaType, 0, sizeof(AM_MEDIA_TYPE));
			amMediaType.majortype = MEDIATYPE_Video;
			amMediaType.subtype = (BitsPixel == 16 ? MEDIASUBTYPE_RGB565 : (BitsPixel == 24 ? 
				MEDIASUBTYPE_RGB24 : MEDIASUBTYPE_RGB32));
			amMediaType.formattype = GUID_NULL;
			SUCCEEDED(hr = pISampleRenderer2->SetMediaType(&amMediaType)) && 
			SUCCEEDED(hr = pISampleRenderer2->SetCallback(&m_ISampleRendererCB, 1L));
		}
		else
		{
			hr = E_FAIL;
		}
	}

	// Connect filters
	if(SUCCEEDED(hr))
	{
		GUID Type = MEDIATYPE_Video;
		GUID Category = PIN_CATEGORY_PREVIEW;
		SUCCEEDED(hr = pICaptureGraphBuilder2->RenderStream(&Category, &Type, 
			pIUnkSourceFilter, pISampleGrabber, pISampleRenderer)) && 
		SUCCEEDED(hr = ConnectFilters(pISampleRenderer, pIVideoRenderer));
	}

	// Get connected media type of sample grabber and initialize m_ISampleGrabberCB
	if(SUCCEEDED(hr))
	{
		memset(&amMediaType, 0, sizeof(AM_MEDIA_TYPE));
		if(SUCCEEDED(hr = pISampleGrabber2->GetConnectedMediaType(&amMediaType)))
		{
			if(amMediaType.formattype == FORMAT_VideoInfo && 
				amMediaType.cbFormat >= sizeof(VIDEOINFOHEADER) && 
				amMediaType.pbFormat)
			{
				// Initialize m_ISampleGrabberCB here
				hr = m_ISampleGrabberCB.Initialize(
					(VIDEOINFOHEADER *)amMediaType.pbFormat);
			}
			else
			{
				hr = E_FAIL;
			}
			if(amMediaType.pbFormat)
			{
				::CoTaskMemFree(amMediaType.pbFormat);
			}
		}
	}

	// Get connected media type of sample renderer and initialize m_ISampleRendererCB
	if(SUCCEEDED(hr))
	{
		memset(&amMediaType, 0, sizeof(AM_MEDIA_TYPE));
		if(SUCCEEDED(hr = pISampleRenderer2->GetConnectedMediaType(&amMediaType)))
		{
			if(amMediaType.formattype == FORMAT_VideoInfo && 
				amMediaType.cbFormat >= sizeof(VIDEOINFOHEADER) && 
				amMediaType.pbFormat)
			{
				// Initialize m_ISampleRendererCB here
				VIDEOINFOHEADER *pVideoInfoHdr = (VIDEOINFOHEADER *)amMediaType.pbFormat;
				if(SUCCEEDED(hr = m_ISampleRendererCB.Initialize(
					pVideoInfoHdr, hWnd)))
				{
					// Calculate clipping rect
					// Calculate clipping rect
					pRect->right = pVideoInfoHdr->bmiHeader.biWidth;
					pRect->left = 0;
					pRect->bottom = pVideoInfoHdr->bmiHeader.biHeight;
					pRect->top = 0;
				}
			}
			else
			{
				hr = E_FAIL;
			}
			if(amMediaType.pbFormat)
			{
				::CoTaskMemFree(amMediaType.pbFormat);
			}
		}
	}

	// Initialize video window
	if(SUCCEEDED(hr) && SUCCEEDED(hr = m_pIGraphBuilder->QueryInterface(
		IID_IVideoWindow, (void **)&pIVideoWindow)))
	{
		hr = pIVideoWindow->put_AutoShow(OAFALSE);
	}

	// Install event notification
	if(SUCCEEDED(hr) && SUCCEEDED(hr = m_pIGraphBuilder->QueryInterface(
		IID_IMediaEventEx, (void **)&pIMediaEventEx)))
	{
		hr = pIMediaEventEx->SetNotifyWindow((OAHWND)hWnd, VS_MEDIAEVENT, 0L);
	}

	// Get run-time tags
	if(SUCCEEDED(hr))
	{
		// Does the source filter provides property pages?
		ISpecifyPropertyPages *pISpecifyPropertyPages = NULL;
		if(m_bCanAdjust = SUCCEEDED(pISourceFilter->QueryInterface(
			IID_ISpecifyPropertyPages, (void **)&pISpecifyPropertyPages)))
		{
			pISpecifyPropertyPages->Release();
		}

		// Can the filter graph be seeked?
		IMediaSeeking *pIMediaSeeking = NULL;
		if(SUCCEEDED(m_pIGraphBuilder->QueryInterface(
			IID_IMediaSeeking, (void **)&pIMediaSeeking)))
		{
			DWORD dwCaps = AM_SEEKING_CanSeekAbsolute;
			m_bCanSeek = pIMediaSeeking->CheckCapabilities(&dwCaps) == S_OK;
			pIMediaSeeking->Release();
		}
	}

	// Run the graph
	if(SUCCEEDED(hr) && SUCCEEDED(hr = m_pIGraphBuilder->QueryInterface(
		IID_IMediaControl, (void **)&pIMediaControl)))
	{
		hr = pIMediaControl->Run();
	}

	// Clean up
	if(pIMediaControl)
	{
		pIMediaControl->Release();
	}
	if(pIMediaEventEx)
	{
		pIMediaEventEx->Release();
	}
	if(pIVideoWindow)
	{
		pIVideoWindow->Release();
	}
	if(pISampleRenderer2)
	{
		pISampleRenderer2->Release();
	}
	if(pISampleGrabber2)
	{
		pISampleGrabber2->Release();
	}
	if(pIUnkSourceFilter)
	{
		pIUnkSourceFilter->Release();
	}
	if(pICaptureGraphBuilder2)
	{
		pICaptureGraphBuilder2->Release();
	}
	if(pIVideoRenderer)
	{
		pIVideoRenderer->Release();
	}
	if(pISampleRenderer)
	{
		pISampleRenderer->Release();
	}
	if(pISampleGrabber)
	{
		pISampleGrabber->Release();
	}
	if(pISourceFilter)
	{
		pISourceFilter->Release();
	}

	if(FAILED(hr))
	{
		// Release filter graph and reset other parameters
		ReleaseFilterGraph();
	}
	return SUCCEEDED(hr);
}

void CVideoDirectShow::ObtainFrame(unsigned char *pDst)
{
	m_ISampleGrabberCB.ObtainFrame(pDst);
}
