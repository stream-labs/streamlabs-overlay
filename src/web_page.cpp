#include "web_page.h"
#include "webform.h"

HWND webform_hwnd = 0;
HANDLE webform_thread = 0;
DWORD  webform_thread_id = 0;
HINSTANCE webform_hInstance = 0;

LRESULT CALLBACK PlainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		webform_hwnd = WebformCreate(hwnd, 103);  // We pick 103 as the id for our child control
		WebformGo(webform_hwnd, _T("https://google.com"));
	} break;
	case WM_SIZE:
	{
		MoveWindow(webform_hwnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
	} break;
	case WM_COMMAND:
	{
		int id = LOWORD(wParam), code = HIWORD(wParam);
		if (id == 103 && code == WEBFN_CLICKED)
		{
			const TCHAR *url = WebformLastClick(webform_hwnd);
			if (_tcscmp(url, _T("http://cnn.com")) == 0) 
				WebformGo(webform_hwnd, _T("http://bbc.co.uk"));
			else 
				WebformGo(webform_hwnd, url);
		}
	} break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	} break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD WINAPI web_page_thread_func(void* data)
{
	HWND hMain;         // Our main window
	HRESULT ole_ret = OleInitialize(0);
	if (ole_ret != S_FALSE && ole_ret != S_OK)
	{
		return 0;
	}

	WNDCLASSEX wcex = { 0 }; wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)PlainWndProc;
	wcex.hInstance = webform_hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = _T("PlainClass");
	ATOM res = RegisterClassEx(&wcex);

	hMain = CreateWindowEx(0, _T("PlainClass"), _T("Plain Window"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 400, 400, NULL, NULL, webform_hInstance, NULL);
	if (hMain != NULL)
	{
		ShowWindow(hMain, SW_SHOW);

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (msg.message == WM_HOTKEY)
			{
				switch (msg.wParam)
				{
				case HOTKEY_SHOW_WEB:
				{
					ShowWindow(hMain, SW_SHOW);
					ShowWindow(webform_hwnd, SW_SHOW);
				}break;
				case HOTKEY_HIDE_ALL:
				{
					//do not hide window. hidden windows does not paint 
				}
				break;
				};
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	OleUninitialize();
	return 0;
}
