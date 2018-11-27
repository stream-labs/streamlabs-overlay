#include "sl_web_view.h"
#include "sl_overlays.h"
#include "sl_overlays_settings.h"

#include "webform.h"

#include <algorithm>
#include <iostream>

HANDLE web_views_thread = 0;
DWORD web_views_thread_id = 0;
HINSTANCE web_views_hInstance = 0;
bool quiting = false;

extern DWORD overlays_thread_id;

class web_view_wnd
{
	public:
	int overlay_id;
	HWND container;
	HWND web_view;
	std::string url;

	web_view_wnd(int _overlay_id, std::string _url)
	    : overlay_id(_overlay_id), url(_url), container(nullptr), web_view(nullptr){};

	void close_windows()
	{
		std::cout << "WEBVIEW: close_windows " << std::endl;
		if (container != nullptr) {
			std::cout << "WEBVIEW: close_windows container" << std::endl;
			auto temp = container;
			container = nullptr;
			DestroyWindow(temp);
		}
		if (web_view != nullptr) {
			std::cout << "WEBVIEW: close_windows web_view" << std::endl;
			auto temp = web_view;
			web_view = nullptr;
			DestroyWindow(temp);
		}
	};

	void apply_url(std::string& _url);
};
std::list<std::shared_ptr<web_view_wnd>> web_views;

void create_container_window(std::shared_ptr<web_view_wnd> n, web_view_overlay_settings* new_window_params);
std::shared_ptr<web_view_wnd> get_web_view_by_container(HWND);
std::shared_ptr<web_view_wnd> get_web_view_by_id(int id);

LRESULT CALLBACK PlainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE: {
		std::cout << "WEBVIEW: WM_CREATE " << std::endl;
		CREATESTRUCT* params = (CREATESTRUCT*)lParam;
		if (params != nullptr) {
			web_view_wnd* wnd_params = (web_view_wnd*)params->lpCreateParams;
			if (wnd_params != nullptr) {
				wnd_params->container = hwnd;
				wnd_params->web_view = WebformCreate(hwnd, 103);

				PostThreadMessage(
				    overlays_thread_id,
				    WM_WEBVIEW_CREATED,
				    wnd_params->overlay_id,
				    reinterpret_cast<LPARAM>(&(wnd_params->web_view)));
				wnd_params->apply_url(wnd_params->url);
			}
		}

	} break;
	case WM_SIZE: {
		std::cout << "WEBVIEW: WM_SIZE " << LOWORD(lParam) << " x " << HIWORD(lParam) << std::endl;
		HWND web_view_hwnd = nullptr;

		std::shared_ptr<web_view_wnd> web_view_window = get_web_view_by_container(hwnd);
		if (web_view_window != nullptr) {
			web_view_hwnd = web_view_window->web_view;
		}

		if (web_view_hwnd != nullptr) {
			MoveWindow(web_view_hwnd, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		}
	} break;
	case WM_COMMAND: {
		int id = LOWORD(wParam), code = HIWORD(wParam);
		if (id == 103 && code == WEBFN_CLICKED) {
			HWND web_view_hwnd = nullptr;

			std::shared_ptr<web_view_wnd> web_view_window = get_web_view_by_container(hwnd);
			if (web_view_window != nullptr) {
				web_view_hwnd = web_view_window->web_view;
			}

			if (web_view_hwnd != nullptr) {
				const TCHAR* url = WebformLastClick(web_view_hwnd);
				//todo convert url to std::string and save
				//web_view_window->apply_url(url);
				WebformGo(web_view_hwnd, url);
			}
		}
	} break;
	case WM_CLOSE: {
		std::cout << "WEBVIEW: WM_CLOSE for " << hwnd << std::endl;
	} break;
	case WM_DESTROY: {
		std::cout << "WEBVIEW: WM_DESTROY for " << hwnd << std::endl;
		std::shared_ptr<web_view_wnd> closed_window = get_web_view_by_container(hwnd);
		if (closed_window != nullptr) {
			std::cout << "WEBVIEW: webview object found and be deleted " << std::endl;
			closed_window->container = nullptr;

			web_views.remove_if(
			    [closed_window](std::shared_ptr<web_view_wnd>& n) { return (closed_window->overlay_id == n->overlay_id); });
			closed_window->close_windows();

			PostThreadMessage(overlays_thread_id, WM_WEBVIEW_CLOSED, closed_window->overlay_id, NULL);
		} else {
			std::cout << "WEBVIEW: WM_DESTROY for webview object not found " << std::endl;
		}

		if (quiting) {
			std::cout << "WEBVIEW: WM_DESTROY and web_views size " << web_views.size() << std::endl;
			if (web_views.size() == 0) {
				PostQuitMessage(0);
			}
		} else {
			std::cout << "WEBVIEW: WM_DESTROY and not quiting " << std::endl;
		}

	} break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD WINAPI web_views_thread_func(void* data)
{
	HRESULT ole_ret = OleInitialize(0);
	if (ole_ret != S_FALSE && ole_ret != S_OK) {
		return 0;
	}

	WNDCLASSEX wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)PlainWndProc;
	wcex.hInstance = web_views_hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = _T("Webview");
	ATOM res = RegisterClassEx(&wcex);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		//std::cout << "APP:" << "web page proc msg id " << msg.message << " for hwnd " << msg.hwnd << std::endl;

		switch (msg.message) {
		case WM_QUIT: {
			std::cout << "WEBVIEW: WM_QUIT " << std::endl;
		} break;
		case WM_WEBVIEW_CREATE: {
			web_view_overlay_settings* new_window_params = reinterpret_cast<web_view_overlay_settings*>(msg.lParam);

			std::shared_ptr<web_view_wnd> new_wnd = std::make_shared<web_view_wnd>((int)msg.wParam, new_window_params->url);
			std::cout << "WEBVIEW: WM_WEBVIEW_CREATE " << new_wnd->overlay_id << ", " << new_wnd->url << std::endl;
			web_views.push_back(new_wnd);
			create_container_window(new_wnd, new_window_params);
			delete new_window_params;
		} break;
		case WM_WEBVIEW_CLOSE: {
			HWND web_view_hwnd = nullptr;
			std::cout << "WEBVIEW: WM_WEBVIEW_CLOSE " << (int)msg.wParam << std::endl;
			std::shared_ptr<web_view_wnd> web_view_window = get_web_view_by_id((int)msg.wParam);
			if (web_view_window != nullptr) {
				std::cout << "WEBVIEW: window found " << std::endl;
				web_views.remove_if([web_view_window](std::shared_ptr<web_view_wnd>& n) {
					std::cout << "WEBVIEW: remove window from list " << (web_view_window->overlay_id == n->overlay_id)
					          << std::endl;
					return (web_view_window->overlay_id == n->overlay_id);
				});
				web_view_window->close_windows();
			} else {
				std::cout << "WEBVIEW: window not found " << std::endl;
				if (quiting) {
					if (web_views.size() == 0) {
						PostQuitMessage(0);
					}
				}
			}
		} break;
		case WM_WEBVIEW_SET_URL: {
			HWND web_view_hwnd = nullptr;

			std::shared_ptr<web_view_wnd> web_view_window = get_web_view_by_id((int)msg.wParam);
			if (web_view_window != nullptr) {
				web_view_window->apply_url(std::string((char*)msg.lParam));
			}
		} break;
		case WM_WEBVIEW_SET_POSITION: {
			std::cout << "WEBVIEW: WM_WEBVIEW_SET_POSITION " << (int)msg.wParam << std::endl;

			RECT* new_rect = reinterpret_cast<RECT*>(msg.lParam);
			if (new_rect != nullptr) {
				std::shared_ptr<web_view_wnd> web_view_window = get_web_view_by_id((int)msg.wParam);
				if (web_view_window != nullptr) {
					MoveWindow(
					    web_view_window->container,
					    new_rect->left,
					    new_rect->top,
					    new_rect->right - new_rect->left,
					    new_rect->bottom - new_rect->top,
					    TRUE);
				}
				delete new_rect;
			}
		} break;
		case WM_WEBVIEW_CLOSE_THREAD: {
			std::cout << "WEBVIEW: WM_WEBVIEW_CLOSE_THREAD" << std::endl;
			if (quiting == false) {
				//destroy all windows without notifying overlay thread
				//todo but windows will be closed by
				if (web_views.size() == 0) {
					PostQuitMessage(0);
				}
			}
		} break;
		};

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	std::cout << "WEBVIEW: Exit from loop " << std::endl;
	OleUninitialize();
	return 0;
}

void create_container_window(std::shared_ptr<web_view_wnd> n, web_view_overlay_settings* new_window_params)
{
	RECT window_rect;
	window_rect.left = new_window_params->x;
	window_rect.top = new_window_params->y;
	window_rect.right = new_window_params->x + new_window_params->width;
	window_rect.bottom = new_window_params->y + new_window_params->height;

	std::cout << "WEBVIEW: create container x " << new_window_params->x << " y " << new_window_params->y << ", size "
	          << new_window_params->width << " x " << new_window_params->height << std::endl;

	AdjustWindowRect(
	    &window_rect, WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CLIPCHILDREN, false);

	HWND hMain; // Our main window
	hMain = CreateWindowEx(
	    0,
	    _T("Webview"),
	    _T("Webview Window"),
	    WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
	        WS_CLIPCHILDREN, //WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	    window_rect.left,
	    window_rect.top,
	    window_rect.right - window_rect.left,
	    window_rect.bottom - window_rect.top,
	    NULL,
	    NULL,
	    web_views_hInstance,
	    n.get());

	if (hMain != NULL) {
		ShowWindow(hMain, SW_SHOW);
	}
}

std::shared_ptr<web_view_wnd> get_web_view_by_container(HWND container)
{
	std::shared_ptr<web_view_wnd> ret = nullptr;
	auto finded_wnd = std::find_if(web_views.begin(), web_views.end(), [&container](std::shared_ptr<web_view_wnd>& n) {
		return container == n->container;
	});
	if (finded_wnd != web_views.end()) {
		ret = *(finded_wnd);
	}
	return ret;
}

std::shared_ptr<web_view_wnd> get_web_view_by_id(int id)
{
	std::shared_ptr<web_view_wnd> ret = nullptr;
	auto finded_wnd = std::find_if(
	    web_views.begin(), web_views.end(), [&id](std::shared_ptr<web_view_wnd>& n) { return id == n->overlay_id; });
	if (finded_wnd != web_views.end()) {
		ret = *(finded_wnd);
	}
	return ret;
}

void web_view_wnd::apply_url(std::string& _url)
{
	url = _url;
	if (web_view != nullptr) {
		WCHAR* wide_url = new wchar_t[url.size() + 1];
		mbstowcs(&wide_url[0], url.c_str(), url.size() + 1);
		WebformGo(web_view, wide_url);
		delete[] wide_url;
	}
}