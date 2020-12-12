// cameraCapture.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "cameraCapture.h"
#include "cameraDevice.h"
#include <vector>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
CameraDevice camera;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

BITMAPINFO Info;
HBITMAP hbitmap;
HDC memdc;
std::vector<COLORREF> PixelData = std::vector<COLORREF>();
std::vector<COLORREF> PixelDataPrev = std::vector<COLORREF>();
std::vector<COLORREF> OutputData = std::vector<COLORREF>();

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CAMERACAPTURE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CAMERACAPTURE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   
   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   RegisterHotKey(hWnd, ID_HOTKEY_0, 0, '0');
   RegisterHotKey(hWnd, ID_HOTKEY_1, 0, '1');
   RegisterHotKey(hWnd, ID_HOTKEY_2, 0, '2');
   RegisterHotKey(hWnd, ID_HOTKEY_3, 0, '3');
   RegisterHotKey(hWnd, ID_HOTKEY_4, 0, '4');
   RegisterHotKey(hWnd, ID_HOTKEY_5, 0, '5');
   RegisterHotKey(hWnd, ID_HOTKEY_6, 0, '6');
   RegisterHotKey(hWnd, ID_HOTKEY_7, 0, '7');
   RegisterHotKey(hWnd, ID_HOTKEY_8, 0, '8');
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

void CreateBitmap()
{
	BITMAP Bmp;
	HDC hdesktop = GetDC(0);
	memdc = CreateCompatibleDC(hdesktop);
	hbitmap = CreateCompatibleBitmap(hdesktop, camera.getFrameWidth(), camera.getFrameHeight());
	SelectObject(memdc, hbitmap);
	GetObject(hbitmap, sizeof(Bmp), &Bmp);
	Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	Info.bmiHeader.biWidth = camera.getFrameWidth();
	Info.bmiHeader.biHeight = camera.getFrameHeight();
	Info.bmiHeader.biPlanes = 1;
	Info.bmiHeader.biBitCount = Bmp.bmBitsPixel;
	Info.bmiHeader.biCompression = BI_RGB;
	Info.bmiHeader.biSizeImage = ((camera.getFrameWidth() * Bmp.bmBitsPixel + 31) / 32)*4 * camera.getFrameHeight();
	PixelData.resize(Info.bmiHeader.biSizeImage/4);
	OutputData.resize(Info.bmiHeader.biSizeImage/4);
	PixelDataPrev.resize(Info.bmiHeader.biSizeImage/4);
	GetDIBits(memdc, hbitmap, 0, camera.getFrameHeight(), &PixelData[0], &Info, DIB_RGB_COLORS);
}

void desaturateFrame()
{
	for(unsigned int i=0;i<PixelData.size();i++)
	{
		DWORD avg = ((PixelData[i]&0xFF) + ((PixelData[i]>>8)&0xFF)+ ((PixelData[i]>>16)&0xFF))/3;
		OutputData[i]=avg | avg<<8 | avg<<16;
	}
}

void lowpassFilter()
{
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
	{
		int ystride = y*camera.getFrameWidth();
		for (unsigned int x = 1; x < camera.getFrameWidth()-1; x++)
		{
			char deltaR =  ((PixelData[x+ystride]&0xFF)-(OutputData[x+ystride]&0xFF))/2;//RC=0.5
			char deltaG =  char((((PixelData[x+ystride]&0xFF00)-(OutputData[x+ystride]&0xFF00))/2)>>8);
			char deltaB =  char((((PixelData[x+ystride]&0xFF0000)-(OutputData[x+ystride]&0xFF0000))/2)>>16);
			OutputData[x+ystride] = (deltaR + (OutputData[x+ystride]>>0)&0xFF) | (deltaG + (OutputData[x+ystride]>>8)&0xFF)<<8 | (deltaB + (OutputData[x+ystride]>>16)&0xFF)<<16;
		}
	}
	for (unsigned int x = 0; x < camera.getFrameWidth(); x++)
	{
		OutputData[x] = PixelData[x];
		OutputData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()] = PixelData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()];
	}
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
		OutputData[y*camera.getFrameWidth()] = PixelData[y*camera.getFrameWidth()];
}

void motionDetection()
{
	desaturateFrame();
	PixelData = OutputData;
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
	{
		int ystride = y*camera.getFrameWidth();
		for (unsigned int x = 1; x < camera.getFrameWidth()-1; x++)
			OutputData[x+ystride] = abs((long)((PixelData[x+ystride])-(PixelDataPrev[x+ystride])));
	}
	PixelDataPrev = PixelData;
	for (unsigned int x = 0; x < camera.getFrameWidth(); x++)
	{
		OutputData[x] = PixelData[x];
		OutputData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()] = PixelData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()];
	}
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
		OutputData[y*camera.getFrameWidth()] = PixelData[y*camera.getFrameWidth()];
}

void blurFilter()
{
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
	{
		int ystride = y*camera.getFrameWidth();
		int ystride1 = (y-1)*camera.getFrameWidth();
		int ystride2 = (y+1)*camera.getFrameWidth();
		for (unsigned int x = 1; x < camera.getFrameWidth()-1; x++)
		{
			OutputData[x+ystride] = ((PixelData[x+ystride1]&0xFF)>>2)&0xFF | ((PixelData[x+ystride1]&0xFF00)>>2)&0xFF00 | ((PixelData[x+ystride1]&0xFF0000)>>2)&0xFF0000;
			OutputData[x+ystride] += ((PixelData[x+ystride2]&0xFF)>>2)&0xFF | ((PixelData[x+ystride2]&0xFF00)>>2)&0xFF00 | ((PixelData[x+ystride2]&0xFF0000)>>2)&0xFF0000;
			OutputData[x+ystride] += ((PixelData[x+ystride-1]&0xFF)>>2)&0xFF | ((PixelData[x+ystride-1]&0xFF00)>>2)&0xFF00 | ((PixelData[x+ystride-1]&0xFF0000)>>2)&0xFF0000;
			OutputData[x+ystride] += ((PixelData[x+ystride+1]&0xFF)>>2)&0xFF | ((PixelData[x+ystride+1]&0xFF00)>>2)&0xFF00 | ((PixelData[x+ystride+1]&0xFF0000)>>2)&0xFF0000;
		}
	}
	for (unsigned int x = 0; x < camera.getFrameWidth(); x++)
	{
		OutputData[x] = PixelData[x];
		OutputData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()] = PixelData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()];
	}
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
		OutputData[y*camera.getFrameWidth()] = PixelData[y*camera.getFrameWidth()];
}

void sharpenFilter()
{
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
	{
		int ystride = y*camera.getFrameWidth();
		int ystride1 = (y-1)*camera.getFrameWidth();
		int ystride2 = (y+1)*camera.getFrameWidth();
		for (unsigned int x = 1; x < camera.getFrameWidth()-1; x++)
		{
			COLORREF redx5 =   (PixelData[x+ystride]&0xFF)*5;
			COLORREF greenx5 = (PixelData[x+ystride]&0xFF00)*5;
			COLORREF bluex5 =  (PixelData[x+ystride]&0xFF0000)*5;
			COLORREF redx4 =   ((PixelData[x+ystride1]&0x0000FF)) + ((PixelData[x+ystride2]&0x0000FF)) + ((PixelData[x+ystride-1]&0x0000FF)) + ((PixelData[x+ystride+1]&0x0000FF));
			COLORREF greenx4 = ((PixelData[x+ystride1]&0x00FF00)) + ((PixelData[x+ystride2]&0x00FF00)) + ((PixelData[x+ystride-1]&0x00FF00)) + ((PixelData[x+ystride+1]&0x00FF00));
			COLORREF bluex4 =  ((PixelData[x+ystride1]&0xFF0000)) + ((PixelData[x+ystride2]&0xFF0000)) + ((PixelData[x+ystride-1]&0xFF0000)) + ((PixelData[x+ystride+1]&0xFF0000));
			OutputData[x+ystride] = 0;
			OutputData[x+ystride] |= (redx5-redx4)&0xFF;
			OutputData[x+ystride] |= (greenx5-greenx4)&0xFF00;
			OutputData[x+ystride] |= (bluex5-bluex4)&0xFF0000;
		}
	}
	for (unsigned int x = 0; x < camera.getFrameWidth(); x++)
	{
		OutputData[x] = PixelData[x];
		OutputData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()] = PixelData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()];
	}
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
		OutputData[y*camera.getFrameWidth()] = PixelData[y*camera.getFrameWidth()];
}

void dilationFilter()
{
	desaturateFrame();
	PixelData = OutputData;
	PixelDataPrev = PixelData;
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
	{
		int ystride = y*camera.getFrameWidth();
		int ystride1 = (y-1)*camera.getFrameWidth();
		int ystride2 = (y+1)*camera.getFrameWidth();
		for (unsigned int x = 1; x < camera.getFrameWidth()-1; x++)
		{
			unsigned char avg = ((PixelDataPrev[x+ystride1-1]&0xFF) + (PixelDataPrev[x+ystride1]&0xFF) + (PixelDataPrev[x+ystride1+1]&0xFF) + (PixelDataPrev[x+ystride-1]&0xFF)
				+ (PixelDataPrev[x+ystride+1]&0xFF) + (PixelDataPrev[x+ystride2-1]&0xFF) + (PixelDataPrev[x+ystride2]&0xFF) + (PixelDataPrev[x+ystride2+1]&0xFF))/8;
			if((PixelData[x+ystride]&0xFF)>avg)
			{
				PixelDataPrev[x+ystride1-1] = PixelData[x+ystride];
				PixelDataPrev[x+ystride1] = PixelData[x+ystride];
				PixelDataPrev[x+ystride1+1] = PixelData[x+ystride];
				PixelDataPrev[x+ystride-1] = PixelData[x+ystride];
				PixelDataPrev[x+ystride+1] = PixelData[x+ystride];
				PixelDataPrev[x+ystride2-1] = PixelData[x+ystride];
				PixelDataPrev[x+ystride2] = PixelData[x+ystride];
				PixelDataPrev[x+ystride2+1] = PixelData[x+ystride];
			}
		}
	}
	OutputData = PixelDataPrev;
}

void erosionFilter()
{
	desaturateFrame();
	PixelData = OutputData;
	PixelDataPrev = PixelData;
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
	{
		int ystride = y*camera.getFrameWidth();
		int ystride1 = (y-1)*camera.getFrameWidth();
		int ystride2 = (y+1)*camera.getFrameWidth();
		for (unsigned int x = 1; x < camera.getFrameWidth()-1; x++)
		{
			unsigned char avg = ((PixelDataPrev[x+ystride1-1]&0xFF) + (PixelDataPrev[x+ystride1]&0xFF) + (PixelDataPrev[x+ystride1+1]&0xFF) + (PixelDataPrev[x+ystride-1]&0xFF)
				+ (PixelDataPrev[x+ystride+1]&0xFF) + (PixelDataPrev[x+ystride2-1]&0xFF) + (PixelDataPrev[x+ystride2]&0xFF) + (PixelDataPrev[x+ystride2+1]&0xFF))/8;
			if((PixelData[x+ystride]&0xFF)>avg)
				PixelDataPrev[x+ystride]=avg | avg<<8 | avg<<16;
		}
	}
	OutputData = PixelDataPrev;
}

void sobelFilter()
{
	desaturateFrame();
	PixelData = OutputData;
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
	{
		int ystride = y*camera.getFrameWidth();
		int ystride1 = (y-1)*camera.getFrameWidth();
		int ystride2 = (y+1)*camera.getFrameWidth();
		for (unsigned int x = 1; x < camera.getFrameWidth()-1; x++)
		{
			COLORREF left =  ((PixelData[x+ystride1-1]&0xFF)) + ((PixelData[x+ystride-1]&0xFF))*2 + ((PixelData[x+ystride2-1]&0xFF));
			COLORREF right = ((PixelData[x+ystride1+1]&0xFF)) + ((PixelData[x+ystride+1]&0xFF))*2 + ((PixelData[x+ystride2+1]&0xFF));
			COLORREF up =    ((PixelData[x+ystride1-1]&0xFF)) + ((PixelData[x+ystride1] &0xFF))*2 + ((PixelData[x+ystride1+1]&0xFF));
			COLORREF down =  ((PixelData[x+ystride2-1]&0xFF)) + ((PixelData[x+ystride2] &0xFF))*2 + ((PixelData[x+ystride2+1]&0xFF));
			unsigned char sum = (unsigned char)abs((long)((right-left)+(down-up)))/2;
			OutputData[x+ystride] = 0;
			OutputData[x+ystride] |= sum;
			OutputData[x+ystride] |= sum<<8;
			OutputData[x+ystride] |= sum<<16;
		}
	}
	for (unsigned int x = 0; x < camera.getFrameWidth(); x++)
	{
		OutputData[x] = PixelData[x];
		OutputData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()] = PixelData[x+(camera.getFrameHeight()-1)*camera.getFrameWidth()];
	}
	for (unsigned int y = 1; y < camera.getFrameHeight()-1; y++)
		OutputData[y*camera.getFrameWidth()] = PixelData[y*camera.getFrameWidth()];
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int wmId;
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_CREATE:
		if(SUCCEEDED(camera.CreateDevice()))
		{
			CreateBitmap();
			SetTimer(hWnd,ID_TIMER,33,NULL);		
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, camera.getFrameWidth(), camera.getFrameHeight(), memdc, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	case WM_DEVICECHANGE:
		if (lParam != 0)
		{
			HRESULT hr = S_OK;
			BOOL bDeviceLost = FALSE;
			hr = camera.CheckDeviceLost((PDEV_BROADCAST_HDR)lParam, &bDeviceLost);
			if (FAILED(hr) || bDeviceLost)
			{
				camera.CloseDevice();
				MessageBox(hWnd, L"Lost the capture device.", NULL, MB_OK);
			}
		}
		return TRUE;
	case WM_TIMER:
		camera.CaptureFrame(PixelData);
		switch(wmId)
		{
		case ID_HOTKEY_1: desaturateFrame();break;
		case ID_HOTKEY_2: dilationFilter();break;
		case ID_HOTKEY_3: erosionFilter();break;
		case ID_HOTKEY_4: sobelFilter();break;
		case ID_HOTKEY_5: motionDetection();break;
		case ID_HOTKEY_6: lowpassFilter();break;
		case ID_HOTKEY_7: blurFilter();break;
		case ID_HOTKEY_8: sharpenFilter();break;
		default:OutputData=PixelData;break;
		}
		SetDIBits(memdc, hbitmap, 0, camera.getFrameHeight(), &OutputData[0], &Info, DIB_RGB_COLORS);
		InvalidateRect(hWnd,NULL,FALSE);
		break;
	case WM_DESTROY:
		KillTimer(hWnd,ID_TIMER);
		camera.CloseDevice();
		PostQuitMessage(0);
		break;
	case WM_HOTKEY :
		wmId    = LOWORD(wParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
