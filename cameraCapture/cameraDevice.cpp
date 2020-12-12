// cameraProgram.cpp : main project file.
#include "cameraDevice.h"
#include <fstream>
extern wchar_t* LogMediaType(IMFMediaType *pType);

HRESULT CameraDevice::CheckDeviceLost(DEV_BROADCAST_HDR *pHdr, BOOL *pbDeviceLost)
{
	DEV_BROADCAST_DEVICEINTERFACE *pDi = NULL;

	if (pbDeviceLost == NULL)
		return E_POINTER;
	*pbDeviceLost = FALSE;

	if (mediaSrc == NULL)
		return S_OK;
	if (pHdr == NULL)
		return S_OK;
	if (pHdr->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
		return S_OK;

	// Compare the device name with the symbolic link.
	pDi = (DEV_BROADCAST_DEVICEINTERFACE*)pHdr;

	if (g_pwszSymbolicLink)
	{
		if (_wcsicmp(g_pwszSymbolicLink, pDi->dbcc_name) == 0)
			*pbDeviceLost = TRUE;
	}
	return S_OK;
}


HRESULT CameraDevice::GetSymbolicLink(IMFActivate *pActivate)
{
	return pActivate->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,&g_pwszSymbolicLink,&g_cchSymbolicLink);
}

BOOL CameraDevice::RegisterForDeviceNotification(HWND hwnd)
{
	DEV_BROADCAST_DEVICEINTERFACE di = { 0 };
	di.dbcc_size = sizeof(di);
	di.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
	di.dbcc_classguid  = KSCATEGORY_CAPTURE; 

	g_hdevnotify = RegisterDeviceNotification(hwnd,&di,DEVICE_NOTIFY_WINDOW_HANDLE);

	if (g_hdevnotify == NULL)
		return FALSE;

	return TRUE;
}

HRESULT CameraDevice::CreateVideoCaptureDevice()
{
	UINT32 count = 0;

	IMFAttributes *pConfig = NULL;
	IMFActivate **ppDevices = NULL;

	HRESULT hr = MFStartup(MF_VERSION);
	if (FAILED(hr))
		return hr;
	// Create an attribute store to hold the search criteria.
	hr = MFCreateAttributes(&pConfig, 1);
	// Request video capture devices.
	if (FAILED(hr))
		return hr;
	hr = pConfig->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr))
		return hr;
	// Enumerate the devices,
	hr = MFEnumDeviceSources(pConfig, &ppDevices, &count);
	if (FAILED(hr))
		return hr;
	// Create a media source for the first device in the list.
	if (count > 0)
		hr = ppDevices[0]->ActivateObject(__uuidof(IMFMediaSource),(void**)&mediaSrc);
	else
		return E_FAIL;

	if (FAILED(hr))
		return hr;

	UINT32 cchName;
	hr = ppDevices[0]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,&devName, &cchName);

	if (FAILED(hr))
		return hr;

	for (DWORD i = 0; i < count; i++)
		ppDevices[i]->Release();
	CoTaskMemFree(ppDevices);

	hr = MFCreateSourceReaderFromMediaSource(mediaSrc,NULL,&srcReader);
	return hr;
}

HRESULT CameraDevice::EnumerateCaptureFormats()
{
	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	IMFMediaTypeHandler *pHandler = NULL;
	IMFMediaType *pType = NULL;
	HRESULT hr = mediaSrc->CreatePresentationDescriptor(&pPD);
	if (FAILED(hr))
		goto done;

	BOOL fSelected;
	hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
	if (FAILED(hr))
		goto done;

	hr = pSD->GetMediaTypeHandler(&pHandler);
	if (FAILED(hr))
		goto done;

	DWORD cTypes = 0;
	hr = pHandler->GetMediaTypeCount(&cTypes);
	if (FAILED(hr))
		goto done;
	int index;
	for (DWORD i = 0; i < cTypes; i++)
	{
		hr = pHandler->GetMediaTypeByIndex(i, &pType);
		if (FAILED(hr))
			goto done;
		logBuffer.append(LogMediaType(pType));
		logBuffer.append(L"\n");
		GUID subtype;
		hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
		if(subtype==MFVideoFormat_RGB24)
		{
			UINT32 width, height;
			hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
			if(width>frameWidth)
			{
				frameWidth = width;
				frameHeight = height;
				index = i;
			}
		}
		SafeRelease(pType);	
	}

	hr = pHandler->GetMediaTypeByIndex(index, &pType);
	if (FAILED(hr))
		goto done;
	PROPVARIANT var;
	if (SUCCEEDED(pType->GetItem(MF_MT_FRAME_RATE_RANGE_MAX, &var)))
	{
		hr = pType->SetItem(MF_MT_FRAME_RATE, var);
		PropVariantClear(&var);
		if (FAILED(hr))
			goto done;

		hr = pHandler->SetCurrentMediaType(pType);
		if (FAILED(hr))
			goto done;
	}
	//hr = srcReader->SetCurrentMediaType(index, NULL, pType);

done:
	SafeRelease(pPD);
	SafeRelease(pSD);
	SafeRelease(pHandler);
	SafeRelease(pType);
	//std::wofstream out("information.txt");
	//out << (wchar_t*)logBuffer.c_str();
	//out.close();
	return hr;
}

HRESULT CameraDevice::InitializeSinkWriter()
{
	IMFMediaType *pMediaTypeOut = NULL;   
	IMFMediaType *pMediaTypeIn = NULL;   
	DWORD streamIndex;     
	//
	IMFMediaTypeHandler *pHandler = NULL;
	IMFPresentationDescriptor *pPD = NULL;
	IMFStreamDescriptor *pSD = NULL;
	BOOL fSelected;
	HRESULT hr = MFCreateSinkWriterFromURL(L"output.wmv", NULL, NULL, &sinkWriter);
	if (SUCCEEDED(hr))
		hr = mediaSrc->CreatePresentationDescriptor(&pPD);
	if (SUCCEEDED(hr))
		hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
	if (SUCCEEDED(hr))
		hr = pSD->GetMediaTypeHandler(&pHandler);
	if (SUCCEEDED(hr))
		hr = pHandler->GetCurrentMediaType(&pMediaTypeIn);

	// Set the output media type.
	if (SUCCEEDED(hr))
		hr = MFCreateMediaType(&pMediaTypeOut);   
	if (SUCCEEDED(hr))
		hr = pMediaTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);     
	if (SUCCEEDED(hr))
		hr = pMediaTypeOut->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_WMV3);   
	if (SUCCEEDED(hr))
	{
		UINT32 bitrate = MFGetAttributeUINT32(pMediaTypeIn, MF_MT_AVG_BITRATE, 663552000);
		hr = pMediaTypeOut->SetUINT32(MF_MT_AVG_BITRATE, bitrate);   
	}
	if (SUCCEEDED(hr))
		hr = pMediaTypeOut->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);   
	if (SUCCEEDED(hr))
		hr = MFSetAttributeSize(pMediaTypeOut, MF_MT_FRAME_SIZE, frameWidth, frameHeight);   
	if (SUCCEEDED(hr))
	{
		UINT64 framerate = MFGetAttributeUINT64(pMediaTypeIn, MF_MT_FRAME_RATE, 30);
		hr = pMediaTypeOut->SetUINT64(MF_MT_FRAME_RATE, framerate);   
	}
	if (SUCCEEDED(hr))
	{
		UINT32 x,y;
		MFGetAttributeRatio(pMediaTypeIn, MF_MT_PIXEL_ASPECT_RATIO, &x, &y);
		hr = MFSetAttributeRatio(pMediaTypeOut, MF_MT_PIXEL_ASPECT_RATIO, x, y);   
	}
	if (SUCCEEDED(hr))
		hr = sinkWriter->AddStream(pMediaTypeOut, &streamIndex);   
	if (SUCCEEDED(hr))
		hr = sinkWriter->SetInputMediaType(streamIndex, pMediaTypeIn, NULL);   

	// Tell the sink writer to start accepting data.
	if (SUCCEEDED(hr))
		hr = sinkWriter->BeginWriting();

	SafeRelease(pMediaTypeOut);
	SafeRelease(pMediaTypeIn);
	return hr;
}

CameraDevice::CameraDevice():frameDuration(10 * 1000 * 1000 / 30)
{
	g_pwszSymbolicLink = NULL;
	g_cchSymbolicLink = 0;
	mediaSrc=NULL;
	g_hdevnotify = NULL;
	sinkWriter = NULL;
	frameWidth=0;frameHeight=0;
	startTime=0;
}

HRESULT CameraDevice::CreateDevice()
{
	HRESULT hr =CreateVideoCaptureDevice();
	if (FAILED(hr))
		return hr;
	hr = EnumerateCaptureFormats();
	if (FAILED(hr))
		return hr;
	hr = InitializeSinkWriter();
	if (FAILED(hr))
		return hr;
	frameData = new rgbData[frameWidth*frameHeight];
	return hr;
}

void CameraDevice::CloseDevice()
{
	if (g_hdevnotify)
		UnregisterDeviceNotification(g_hdevnotify);
	if(sinkWriter)
	{
		sinkWriter->Finalize();
		sinkWriter->Release();
	}
	if(srcReader)
		srcReader->Release();
	if (mediaSrc)
	{
		mediaSrc->Shutdown();
		mediaSrc->Release();
	}
	MFShutdown();
	delete[] frameData;
}

void CameraDevice::CaptureFrame(std::vector<COLORREF>& pixelData)
{
	HRESULT hr = S_OK;
	DWORD streamIndex, flags,i=0;
	LONGLONG llTimeStamp;
	IMFSample *pSample = NULL;
	hr = srcReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM,0,&streamIndex,&flags,&llTimeStamp,&pSample);
	if (SUCCEEDED(hr))
	{
		if(pSample)
		{
			IMFMediaBuffer* imgBuffer=NULL;
			pSample->GetBufferByIndex(0,&imgBuffer);
			//IMF2DBuffer     *m_p2DBuffer=NULL;
			//hr =imgBuffer->QueryInterface(IID_PPV_ARGS(&m_p2DBuffer));//returns E_NOINTERFACE 
			if(imgBuffer)
			{
				rgbData* imgData;
				unsigned long length;
				imgBuffer->Lock((BYTE**)&imgData,NULL,&length);
				if((length/3) == frameWidth*frameHeight)
				{
					for(unsigned int i=0;i<frameWidth*frameHeight;i++)
					{
						COLORREF color = imgData[i].r | imgData[i].g<<8 | imgData[i].b<<16;
						pixelData[i]=color;
					}
				}
				imgBuffer->Unlock();
				SafeRelease(imgBuffer);
			}
			//write to file
			hr = pSample->SetSampleTime(startTime);
			if (SUCCEEDED(hr))
				hr = pSample->SetSampleDuration(frameDuration);
			if (SUCCEEDED(hr))
				sinkWriter->WriteSample(0, pSample);
			startTime+=frameDuration;
			SafeRelease(pSample);
		}
	}
	
}