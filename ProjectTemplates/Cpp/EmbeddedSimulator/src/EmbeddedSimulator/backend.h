#ifndef __BACKEND_H
#define __BACKEND_H

#include <Windows.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
#include <objidl.h>

#include "globals.h"

// Initialization prototypes
void Backend_Initialize(void);
void Backend_SetContext(HWND hWnd);
void Backend_Shutdown(void);
void Backend_Refresh(void);

// Drawing functions
void Backend_FillScreen(char r, char g, char b);
void Backend_DrawRectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char r, char g, char b);
void Backend_DrawPixel(unsigned int x, unsigned int y, char r, char g, char b);
void Backend_DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char r, char g, char b);

// Capture screenshot
BOOL Backend_Capture(HWND hWnd, LPWSTR name);

#endif // !__BACKEND_H
