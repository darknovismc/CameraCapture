# CameraCapture
Program written in Windows API using Windows Media Foundation to connect to any web camera and to process and capture video streams. 

By pressing different hotkeys you can apply different filters:
1-desaturate frame,2-RGB dilation filter,3- RGB erosion filter, 4- sobel filter, 5-motion detection, 6-low pass filter,7-blur,8-sharpen,0-no filter

cameraCapture.cpp - creates main window class ,registers and creates window instance.
Paints frames on a HBITMAP using HDC in WM_PAINT message for a performance reason.All frame filters are also implemented here

cameraDevice.cpp - creates camera device using Windows Media Foundation to connect to any webcam.
Enumerates all capture formats and chooses one with the highest frame resolution and framerate. 
Initializes SinkWriter for writing a video capture into .wmv file after it has been filtered.

mediaDebug.cpp - optional class for loging and debugging camera capabilities.
