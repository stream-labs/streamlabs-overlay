#include "web_view.h"
#include "overlays.h"
#include "settings.h"

#include "webform.h"

#include <algorithm>
#include <iostream>

HANDLE web_views_thread = 0;
DWORD web_views_thread_id = 0;
HINSTANCE web_views_hInstance = 0;

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
		if (container != nullptr) {
			CloseWindow(container);
			container = nullptr;
		}
		if (web_view != nullptr) {
			CloseWindow(web_view);
			web_view = nullptr;
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
	case WM_DESTROY: {
		std::shared_ptr<web_view_wnd> closed_window = get_web_view_by_container(hwnd);
		if (closed_window != nullptr) {
			closed_window->container = nullptr;
			closed_window->close_windows();
			web_views.remove_if(
			    [closed_window](std::shared_ptr<web_view_wnd>& n) { return (closed_window->overlay_id == n->overlay_id); });

			PostThreadMessage(overlays_thread_id, WM_WEBVIEW_CLOSED, closed_window->overlay_id, NULL);
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
		case WM_WEBVIEW_CREATE: {
			web_view_overlay_settings* new_window_params = reinterpret_cast<web_view_overlay_settings*>(msg.lParam);

			std::shared_ptr<web_view_wnd> new_wnd = std::make_shared<web_view_wnd>((int)msg.wParam, new_window_params->url);
			std::cout << "APP:" << new_wnd->overlay_id << ", " << new_wnd->url << std::endl;
			web_views.push_back(new_wnd);
			create_container_window(new_wnd, new_window_params);
			delete new_window_params;
		} break;
		case WM_WEBVIEW_CLOSE: {
			HWND web_view_hwnd = nullptr;

			std::shared_ptr<web_view_wnd> web_view_window = get_web_view_by_id((int)msg.wParam);
			if (web_view_window != nullptr) {
				web_views.remove_if([web_view_window](std::shared_ptr<web_view_wnd>& n) {
					return (web_view_window->overlay_id == n->overlay_id);
				});
				web_view_window->close_windows();
			}
		}

		break;
			break;
		case WM_WEBVIEW_SET_URL: {
			HWND web_view_hwnd = nullptr;

			std::shared_ptr<web_view_wnd> web_view_window = get_web_view_by_id((int)msg.wParam);
			if (web_view_window != nullptr) {
				web_view_window->apply_url(std::string((char*)msg.lParam));
			}
		} break;
		};

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	OleUninitialize();
	return 0;
}

void create_container_window(std::shared_ptr<web_view_wnd> n, web_view_overlay_settings* new_window_params)
{
	HWND hMain; // Our main window
	hMain = CreateWindowEx(
	    0,
	    _T("Webview"),
	    _T("Webview Window"),
	    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	    new_window_params->x,
	    new_window_params->y,
	    new_window_params->width,
	    new_window_params->height,
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