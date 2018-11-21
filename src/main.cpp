#include "overlays.h"
#include "stdafx.h"

#include <algorithm>
#include <iostream>

#include "settings.h"
#include "web_view.h"

smg_settings app_settings;

HANDLE overlays_thread = nullptr;
DWORD overlays_thread_id = 0;
BOOL g_bDblBuffered = FALSE;

const int OVERLAY_UPDATE_TIMER = 001;
bool in_standalone_mode = false;

//  Regular entry to the app
int APIENTRY main(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/);

//  Windows entry to the app
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	main(hInstance, hPrevInstance, pCmdLine, nCmdShow);
}

int APIENTRY main(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	in_standalone_mode = true;
	overlays_thread = GetModuleHandle(NULL);
	overlays_thread_id = GetCurrentThreadId();

	return overlay_thread_func(NULL);
}

DWORD WINAPI overlay_thread_func(void* data)
{
	std::shared_ptr<smg_overlays> app;
	app = smg_overlays::get_instance();

	web_views_hInstance = GetModuleHandle(NULL);

	web_views_thread = CreateThread(nullptr, 0, web_views_thread_func, nullptr, 0, &web_views_thread_id);
	if (web_views_thread) {
		// Optionally do stuff, such as wait on the thread.
	}

	//  Mark that this process is DPI aware.
	SetProcessDPIAware();

	// Init COM and double-buffered painting
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr)) {
		hr = BufferedPaintInit();
		g_bDblBuffered = SUCCEEDED(hr);

		app->init();

		SetTimer(0, OVERLAY_UPDATE_TIMER, app_settings.redraw_timeout, (TIMERPROC) nullptr);

		// Main message loop
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0)) {
			//std::cout << "APP:"  << "wnd proc msg id " << msg.message << " for hwnd " << msg.hwnd << std::endl;

			switch (msg.message) {
			case WM_WEBVIEW_CREATED:
				app->original_window_ready((int)msg.wParam, *(reinterpret_cast<HWND*>(msg.lParam)));
				break;
			case WM_WEBVIEW_CLOSED: {
				std::cout << "APP:"
				          << "WM_WEBVIEW_CLOSED " << (int)msg.wParam << std::endl;
				auto closed = app->get_overlay_by_id((int)msg.wParam);
				app->remove_overlay(closed);
			} break;
			case WM_HOTKEY: {
				app->process_hotkeys(msg);
			} break;
			case WM_TIMER:
				app->on_update_timer();
				break;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				break;
			};
		}

		if (g_bDblBuffered)
			BufferedPaintUnInit();

		KillTimer(0, OVERLAY_UPDATE_TIMER);

		CoUninitialize();
	}

	app->deinit();

	return 0;
}

BOOL CALLBACK get_overlayed_windows(HWND hwnd, LPARAM param)
{
	return smg_overlays::get_instance()->process_found_window(hwnd, param);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CREATE: {
	} break;
	case WM_SIZE: {
	} break;
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	} break;
	case WM_ERASEBKGND: {
		// Don't do any erasing here.  It's done in WM_PAINT to avoid flicker.
		return 1;
	} break;
	case WM_TIMER: {
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	} break;
	case WM_PAINT: {
		smg_overlays::get_instance()->draw_overlay_gdi(hWnd, g_bDblBuffered);
		return 0;
	} break;

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

//==== node api ====
//when used as a "plugin" we have to start our own thread to work with windows events loop
int WINAPI start_overlays_thread()
{
	overlays_thread = CreateThread(nullptr, 0, overlay_thread_func, nullptr, 0, &overlays_thread_id);
	if (overlays_thread) {
		// Optionally do stuff, such as wait on the thread.
	}
	return 0;
}

int WINAPI stop_overlays_thread()
{
	PostThreadMessage((DWORD)overlays_thread_id, WM_HOTKEY, HOTKEY_QUIT, 0);
	return 0;
}

int WINAPI show_overlays()
{
	PostThreadMessage((DWORD)overlays_thread_id, WM_HOTKEY, HOTKEY_SHOW_OVERLAYS, 0);
	return 0;
}

int WINAPI hide_overlays()
{
	PostThreadMessage((DWORD)overlays_thread_id, WM_HOTKEY, HOTKEY_HIDE_OVERLAYS, 0);
	return 0;
}

int WINAPI add_webview(const char* url, int x, int y, int width, int height)
{
	web_view_overlay_settings n;
	n.x = x;
	n.y = y;
	n.width = width;
	n.height = height;
	n.url = std::string(url);

	int ret = smg_overlays::get_instance()->create_web_view_window(n);

	return ret;
}

int WINAPI add_webview(const char* url)
{
	return add_webview(url, 100, 100, 400, 300);
}

bool WINAPI set_webview_params(int id, const char* url, int x, int y, int width, int height)
{
	//set params to overlay 
	return true;
}

std::shared_ptr<smg_overlays> get_overlays()
{
	return smg_overlays::get_instance();
}