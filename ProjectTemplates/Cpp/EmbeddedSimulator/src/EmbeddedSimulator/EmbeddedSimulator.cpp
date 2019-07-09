// EmbeddedSimulator.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "EmbeddedSimulator.h"
#include <chrono>
#include <future>
#include <thread>

#define MAX_LOADSTRING 100

// Prototypes
void SaveScreenshot(HWND hWnd);
void ProcessBackground(void);

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

char TimerRunning = 0x00;
char TimerProcessing = 0x00;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Backend_Initialize();
    Simulation_Initialize();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EMBEDDEDSIMULATOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMBEDDEDSIMULATOR));

    MSG msg;

    // TODO: Place custom initialization code here.
    Backend_DrawLine(0, 0, APP_SCREEN_WIDTH, APP_SCREEN_HEIGHT, 255, 0, 0);
    Backend_DrawLine(0, APP_SCREEN_HEIGHT, APP_SCREEN_WIDTH, 0, 255, 0, 0);

    TimerRunning = 0x01;
    std::thread t(ProcessBackground);

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Process simulation loop. However, check if processing runs on parallel thread to prevent race conditions.
        if (TimerProcessing == 0x00)
          Simulation_ProcessLoop();
    }

    // Wait for thread to finish
    t.join();

    // Shutdown backend
    Backend_Shutdown();

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EMBEDDEDSIMULATOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EMBEDDEDSIMULATOR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
  // Determine desktop resolution
  HWND hDesktop = GetDesktopWindow();
  RECT desktop;
  GetWindowRect(hDesktop, &desktop);
  int res_x = desktop.right;
  int res_y = desktop.bottom;

  int left = CW_USEDEFAULT;
  int top = CW_USEDEFAULT;

  if (res_x > (APP_SCREEN_WIDTH + 20))
  {
    if (res_x > (APP_SCREEN_HEIGHT + 40))
    {
      left = (res_x - (APP_SCREEN_WIDTH + 20)) >> 1;
      top = (res_y - (APP_SCREEN_HEIGHT + 40)) >> 1;
    }
  }

  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindow(
    TEXT(WINDOW_CLASSNAME),
    TEXT(WINDOW_WINDOWNAME),
    WS_OVERLAPPEDWINDOW,
    left,
    top,
    APP_SCREEN_WIDTH + 20,
    APP_SCREEN_HEIGHT + 40,
    NULL,
    NULL,
    hInstance,
    NULL
  );

  if (!hWnd)
  {
    return FALSE;
  }

  SetMenu(hWnd, NULL);
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  Backend_SetContext(hWnd);
  Backend_Refresh();

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    {
      switch (wParam)
      {
        case VK_RETURN: SaveScreenshot(hWnd); break;
        default:
          break;
      }
      break;
    }
    case WM_CREATE:
    {
      // Timer 1: Display refresh timer
      SetTimer(hWnd, 1, 100, NULL);
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        TimerRunning = 0x00;
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        break;
    case WM_TIMER:
    {
      switch (wParam)
      {
        case 1:
        {
          // Window refresh timer
          Backend_Refresh();
        }
        default: break;
      }
      break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void SaveScreenshot(HWND hWnd)
{
  OPENFILENAME ofn;
  char szFileName[128] = "";
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = L"PNG-Datei (*.png)\0*.png\0\0";
  ofn.lpstrFile = (LPWSTR)szFileName;
  ofn.nMaxFile = 128;
  ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NONETWORKBUTTON;
  ofn.lpstrDefExt = L"png";

  if (GetSaveFileName(&ofn))
  {
    // Take screenshot, save file
    if (Backend_Capture(hWnd, ofn.lpstrFile))
      MessageBox(hWnd, L"Erfolgreich exportiert.", L"Screenshot", 0);
    else
      MessageBox(hWnd, L"Fehler beim Export.", L"Screenshot", 0);
  }
}

void ProcessBackground()
{
  while (TimerRunning)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    TimerProcessing = 0x01;
    Simulation_ProcessTimer();
    TimerProcessing = 0x00;
  }
}