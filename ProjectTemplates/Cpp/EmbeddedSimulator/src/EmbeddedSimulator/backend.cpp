#include "backend.h"

using namespace Gdiplus;

// Helper function
INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// GDI Token necessary for cleanup
ULONG_PTR gdiToken;

// Window DC
HDC dcWin;

// Bitmap DC
HDC dcMem;

// Bitmap
HBITMAP bMem;

// Init flag
BOOL IsInitialized = FALSE;

// State flag
BOOL State = FALSE;

void Backend_Initialize()
{
  GdiplusStartupInput gdiStartup;

  GdiplusStartup(&gdiToken, &gdiStartup, NULL);
}

void Backend_SetContext(HWND hWnd)
{
  dcWin = GetDC(hWnd);
  dcMem = CreateCompatibleDC(dcWin);
  bMem = (HBITMAP)SelectObject(dcMem, CreateCompatibleBitmap(dcWin, 320, 240));
  IsInitialized = TRUE;
}

void Backend_Shutdown()
{
  GdiplusShutdown(gdiToken);

  DeleteObject(bMem);
}

void Backend_Refresh()
{
  if (IsInitialized)
  {
    BOOL res = BitBlt(dcWin, 0, 0, 320, 240, dcMem, 0, 0, SRCCOPY);
    if (res == FALSE)
    {
      DWORD err = GetLastError();
      ;
    }
  }
}

BOOL Backend_Capture(HWND hWnd, LPWSTR name)
{
  BOOL result = FALSE;

  if (IsInitialized)
  {
    HDC dcEx = CreateCompatibleDC(dcWin);
    HBITMAP bEx = CreateCompatibleBitmap(dcWin, 320, 240);
    SelectObject(dcEx, bEx);
    BOOL res = BitBlt(dcEx, 0, 0, 320, 240, dcWin, 0, 0, SRCCOPY);

    CLSID   encoderClsid;
    GetEncoderClsid(L"image/png", &encoderClsid);

    if (res == TRUE)
    {
      Bitmap *b = Bitmap::FromHBITMAP(bEx, NULL);
      Gdiplus::Status s = b->Save(name, &encoderClsid, NULL);
      if (s == 0)
        result = TRUE;

      DeleteObject(b);
    }

    DeleteDC(dcEx);
  }

  return result;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
  UINT  num = 0;          // number of image encoders
  UINT  size = 0;         // size of the image encoder array in bytes

  ImageCodecInfo* pImageCodecInfo = NULL;

  GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;  // Failure

  pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
  if (pImageCodecInfo == NULL)
    return -1;  // Failure

  GetImageEncoders(num, size, pImageCodecInfo);

  for (UINT j = 0; j < num; ++j)
  {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
    {
      *pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success
    }
  }

  free(pImageCodecInfo);
  return -1;  // Failure
}

void Backend_FillScreen(char r, char g, char b)
{
  SelectObject(dcMem, GetStockObject(DC_PEN));
  SetDCPenColor(dcMem, RGB(r, g, b));
  SelectObject(dcMem, GetStockObject(DC_BRUSH));
  SetDCBrushColor(dcMem, RGB(r, g, b));
  Rectangle(dcMem, 0, 0, 321, 241);
}

void Backend_DrawRectangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char r, char g, char b)
{
  SelectObject(dcMem, GetStockObject(DC_PEN));
  SetDCPenColor(dcMem, RGB(r, g, b));
  SelectObject(dcMem, GetStockObject(DC_BRUSH));
  SetDCBrushColor(dcMem, RGB(r, g, b));

  Rectangle(dcMem, x1, y1, x2 + 1, y2 + 1);
}

void Backend_DrawPixel(unsigned int x, unsigned int y, char r, char g, char b)
{
  SetPixel(dcMem, x, y, RGB(r, g, b));
}

void Backend_DrawLine(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, char r, char g, char b)
{
  SelectObject(dcMem, GetStockObject(DC_PEN));
  SetDCPenColor(dcMem, RGB(r, g, b));
  MoveToEx(dcMem, x1, y1, NULL);
  LineTo(dcMem, x2, y2);
}