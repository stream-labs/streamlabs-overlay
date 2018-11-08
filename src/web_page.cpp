#include "web_page.h"
#include "webform.h"
#include <algorithm>


std::list<std::shared_ptr<web_forms_window>>  web_forms_windows;

HANDLE webform_thread = 0;
DWORD  webform_thread_id = 0;
HINSTANCE webform_hInstance = 0;

HWND get_webform_hwnd(HWND container)
{
	HWND ret = NULL;
	std::for_each(web_forms_windows.begin(), web_forms_windows.end(), [&ret, container](std::shared_ptr<web_forms_window> &n)
	{ 
		if (n->container_hwnd == container)
		{
			ret = n->webform_hwnd;
		}
	}
		);
	return ret;
}

std::shared_ptr<web_forms_window> get_webform(HWND container)
{
	std::shared_ptr<web_forms_window> ret = nullptr;
	std::for_each(web_forms_windows.begin(), web_forms_windows.end(), [&ret, container](std::shared_ptr<web_forms_window> &n)
	{
		if (n->container_hwnd == container)
		{
			ret = n;
		}
	}
	);
	return ret;
}

LRESULT CALLBACK PlainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		std::shared_ptr<web_forms_window> new_web_view = std::make_shared<web_forms_window>();
		new_web_view->container_hwnd = hwnd;
		new_web_view->webform_hwnd = WebformCreate(hwnd, 103);  // We pick 103 as the id for our child control
		web_forms_windows.push_back(new_web_view);
	} break;
	case WM_SIZE:
	{
		MoveWindow(get_webform_hwnd(hwnd), 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
	} break;
	case WM_COMMAND:
	{
		int id = LOWORD(wParam), code = HIWORD(wParam);
		if (id == 103 && code == WEBFN_CLICKED)
		{
			const TCHAR *url = WebformLastClick(get_webform_hwnd(hwnd));
			if (_tcscmp(url, _T("http://cnn.com")) == 0) 
				WebformGo(get_webform_hwnd(hwnd), _T("http://bbc.co.uk"));
			else 
				WebformGo(get_webform_hwnd(hwnd), url);
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

	std::for_each(app_settings.web_pages.begin(), app_settings.web_pages.end(), [](web_page_overlay_settings &n)
	{
		create_container_window(n);
	}
	);
	   
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.message == WM_HOTKEY)
		{
			switch (msg.wParam)
			{
			case HOTKEY_ADD_WEB:
			{
				web_page_overlay_settings new_web_view;
				
				new_web_view.url = "https://google.com";
				new_web_view.x = 100;
				new_web_view.y = 100;
				new_web_view.width = 400;
				new_web_view.height = 250;
				
				create_container_window(new_web_view);

				app_settings.web_pages.push_back(new_web_view);			
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

	OleUninitialize();
	return 0;
}

void create_container_window(web_page_overlay_settings & n)
{
	HWND hMain;         // Our main window
	hMain = CreateWindowEx(0, _T("PlainClass"), _T("Plain Window"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, n.x, n.y, n.width, n.height, NULL, NULL, webform_hInstance, NULL);
	if (hMain != NULL)
	{
		std::shared_ptr<web_forms_window> cur_web_view_window = get_webform(hMain);
		if (cur_web_view_window != nullptr)
		{
			cur_web_view_window->url = n.url;
			WCHAR * url = new wchar_t[cur_web_view_window->url.size() + 1];
			mbstowcs(&url[0], cur_web_view_window->url.c_str(), cur_web_view_window->url.size() + 1);

			WebformGo(cur_web_view_window->webform_hwnd, url);
		}

		ShowWindow(hMain, SW_SHOW);
	}
}
