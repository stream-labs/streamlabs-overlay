#include "web_view.h"
#include "overlays.h"
#include "settings.h"

#include "webform.h"

#include <algorithm>
#include <iostream>


HANDLE web_views_thread = 0;
DWORD  web_views_thread_id = 0;
HINSTANCE web_views_hInstance = 0;
extern std::shared_ptr<smg_overlays> app;

void create_container_window(std::shared_ptr<web_view_window> & n);

LRESULT CALLBACK PlainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
	} break;
	case WM_SIZE:
	{
		std::shared_ptr<web_view_window> new_web_view = app->get_web_view_by_container(hwnd);
		if (new_web_view != nullptr)
		{
			MoveWindow(new_web_view->web_view_hwnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		}

	} break;
	case WM_COMMAND:
	{
		int id = LOWORD(wParam), code = HIWORD(wParam);
		if (id == 103 && code == WEBFN_CLICKED)
		{
			const TCHAR *url = WebformLastClick(app->get_web_view_by_container(hwnd)->web_view_hwnd);
			WebformGo(app->get_web_view_by_container(hwnd)->web_view_hwnd, url);
			//WebformGo(get_web_view_hwnd(hwnd), _T("http://bbc.co.uk"));
		}
	} break;
	case WM_DESTROY:
	{
		app->get_web_view_by_container(hwnd)->web_view_hwnd = nullptr;
		app->get_web_view_by_container(hwnd)->container_hwnd = nullptr;

	} break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD WINAPI web_views_thread_func(void* data)
{
	HRESULT ole_ret = OleInitialize(0);
	if (ole_ret != S_FALSE && ole_ret != S_OK)
	{
		return 0;
	}

	WNDCLASSEX wcex = { 0 }; wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)PlainWndProc;
	wcex.hInstance = web_views_hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = _T("Webview");
	ATOM res = RegisterClassEx(&wcex);
	   
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		std::cout << "APP:"  << "web page proc msg id " << msg.message << " for hwnd "<< msg.hwnd << std::endl;

		if (msg.message == WM_HOTKEY)
		{
			switch (msg.wParam)
			{
			case HOTKEY_ADD_WEB:
			{
				int created = 0;
				std::for_each(app->showing_windows.begin(), app->showing_windows.end(), [&created](std::shared_ptr<captured_window> &n)
				{ 
					std::cout << "APP:" << "add webviews test 2" << std::endl;
					if (n->is_web_view())
					{
						std::cout << "APP:" << "add webviews test 1" << std::endl;
						std::shared_ptr<web_view_window> derived = std::static_pointer_cast<web_view_window> (n);
						if (derived->container_hwnd == nullptr)
						{
							created++;
							create_container_window(derived);
						}

					}
					return; 
				}

				);

				std::cout << "APP:"  << "add webviews " << created << std::endl;
			}break;
			case HOTKEY_HIDE_OVERLAYS:
			{
				//do not hide window. hidden windows does not paint 
			}
			break;
			};
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	OleUninitialize();
	return 0;
}

void create_container_window(std::shared_ptr<web_view_window> & n)
{
	HWND hMain;         // Our main window
	hMain = CreateWindowEx(0, _T("Webview"), _T("Webview Window"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, n->x, n->y, n->width, n->height, NULL, NULL, web_views_hInstance, NULL);
	if (hMain != NULL)
	{
		n->container_hwnd = hMain;
		n->web_view_hwnd = WebformCreate(hMain, 103);
		n->orig_handle = n->web_view_hwnd;

		WCHAR * url = new wchar_t[n->url.size() + 1];
		mbstowcs(&url[0], n->url.c_str(), n->url.size() + 1);
		WebformGo(n->web_view_hwnd, url);
		delete url;
		ShowWindow(hMain, SW_SHOW);
	}
}
