#include "stdafx.h"
#include "overlays.h"

#include <algorithm>
#include <iostream>
#include <algorithm>

#include "web_view.h"
#include "settings.h"

std::shared_ptr<smg_overlays> app;
smg_settings app_settings;

HANDLE overlays_thread = nullptr;
DWORD  overlays_thread_id = 0;
BOOL g_bDblBuffered = FALSE;

const int OVERLAY_UPDATE_TIMER = 001;

//  Regular entry to the app
int APIENTRY main(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/);

//  Windows entry to the app
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	main(hInstance, hPrevInstance, pCmdLine, nCmdShow);
}

int APIENTRY main(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	web_views_hInstance = hInstance;
	//g_hInstance = hInstance;

	return overlay_thread_func(NULL);
}

//when used as a "plugin" we have to start our own thread to work with windows events loop
int WINAPI start_overlays_thread()
{
	overlays_thread = CreateThread(nullptr, 0, overlay_thread_func, nullptr, 0, &overlays_thread_id);
	if (overlays_thread) {
		// Optionally do stuff, such as wait on the thread.
	}
	return 0;
}

DWORD WINAPI overlay_thread_func(void* data)
{
	app = std::make_shared<smg_overlays>();

	//if (g_hInstance == NULL)
	{
		HINSTANCE g_hInstance = GetModuleHandle(NULL);
		web_views_hInstance = g_hInstance;
	}

	web_views_thread = CreateThread(nullptr, 0, web_views_thread_func, nullptr, 0, &web_views_thread_id);
	if (web_views_thread) {
		// Optionally do stuff, such as wait on the thread.
	}

	//  Mark that this process is DPI aware.
	SetProcessDPIAware();

	// Init COM and double-buffered painting
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		hr = BufferedPaintInit();
		g_bDblBuffered = SUCCEEDED(hr);

		app->init();

		SetTimer(0, OVERLAY_UPDATE_TIMER, app_settings.redraw_timeout, (TIMERPROC)nullptr);
		// Main message loop
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			std::cout << "wnd proc msg id " << msg.message << " for hwnd " << msg.hwnd << std::endl;

			switch (msg.message)
			{
			case WM_HOTKEY:
			{
				app->process_hotkeys(msg);
			}
			break;
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
	return app->process_found_window(hwnd, param);
}

// ----------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{

	}
	break;
	case WM_SIZE:
	{
	} break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	break;
	case WM_ERASEBKGND:
	{
		// Don't do any erasing here.  It's done in WM_PAINT to avoid flicker.
		return 1;
	}
	break;
	case WM_TIMER:
	{
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;
	}
	break;
	case WM_PAINT:
	{
		app->draw_overlay_gdi(hWnd, g_bDblBuffered);
		return 0;
	}
	break;

	default:break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
