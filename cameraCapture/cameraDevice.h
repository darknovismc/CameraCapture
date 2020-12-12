#include "stdafx.h"
#include <string>
#include <vector>
struct rgbData
{
	unsigned char r,g,b;
};
class CameraDevice
{
public:
	CameraDevice();
	void CloseDevice();
	HRESULT CreateDevice();
	HRESULT CheckDeviceLost(DEV_BROADCAST_HDR *pHdr, BOOL *pbDeviceLost);
	void CaptureFrame(std::vector<COLORREF>& pixelData);
	rgbData getFrameData(int index){return frameData[index];}
	unsigned int getFrameWidth(){return frameWidth;}
	unsigned int getFrameHeight(){return frameHeight;}
private:
	unsigned long long	startTime;
	const long long frameDuration;
	rgbData* frameData;
	std::wstring logBuffer;
	WCHAR	*g_pwszSymbolicLink;
	UINT32	g_cchSymbolicLink;
	IMFMediaSource *mediaSrc;
	IMFSinkWriter *sinkWriter;
	IMFSourceReader *srcReader;
	HDEVNOTIFY  g_hdevnotify;
	wchar_t *devName;
	unsigned int frameWidth,frameHeight;
	HRESULT GetSymbolicLink(IMFActivate *pActivate);
	BOOL RegisterForDeviceNotification(HWND hwnd);
	HRESULT CreateVideoCaptureDevice();
	HRESULT EnumerateCaptureFormats();
	HRESULT InitializeSinkWriter();
};