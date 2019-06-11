#include "sl_overlays.h"
#include "stdafx.h"

#include <algorithm>
#include <iostream>

#include "overlay_logging.h"
#include "sl_overlay_window.h"
#include "sl_overlays_settings.h"

std::shared_ptr<smg_settings> app_settings;

HANDLE overlays_thread = nullptr;
DWORD overlays_thread_id = 0;
sl_overlay_thread_state thread_state = sl_overlay_thread_state::destoyed;
std::mutex thread_state_mutex;

BOOL g_bDblBuffered = FALSE;

UINT_PTR OVERLAY_UPDATE_TIMER = 0;

DWORD WINAPI overlay_thread_func(void* data)
{
	app_settings = std::make_shared<smg_settings>();

	std::shared_ptr<smg_overlays> app = smg_overlays::get_instance();

	// Init COM and double-buffered painting
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY);
	
	if (SUCCEEDED(hr))
	{
		hr = BufferedPaintInit();
		g_bDblBuffered = SUCCEEDED(hr);

		app->init();

		OVERLAY_UPDATE_TIMER = SetTimer(0, 0, app_settings->redraw_timeout, (TIMERPROC) nullptr);

		thread_state_mutex.lock();
		thread_state = sl_overlay_thread_state::runing;
		thread_state_mutex.unlock();

		// Main message loop
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			//log_cout << "APP: wnd proc msg id " << msg.message << " for hwnd " << msg.hwnd << std::endl;
			bool catched = false;

			switch (msg.message)
			{
			case WM_SLO_OVERLAY_CLOSE:
			{
				log_cout << "APP: WM_SLO_OVERLAY_CLOSE" << (int)msg.wParam << std::endl;
				auto closed = app->get_overlay_by_id((int)msg.wParam);
				app->remove_overlay(closed);
				catched = true;
			}
			break;
			case WM_SLO_OVERLAY_POSITION:
			{
				log_cout << "APP: WM_SLO_OVERLAY_POSITION " << (int)msg.wParam << std::endl;
				std::shared_ptr<overlay_window> overlay = app->get_overlay_by_id((int)msg.wParam);
				RECT* new_rect = reinterpret_cast<RECT*>(msg.lParam);
				if (new_rect != nullptr)
				{
					if (overlay != nullptr)
					{
						overlay->apply_new_rect(*new_rect);
					}
					delete new_rect;
				}
				catched = true;
			}
			break;
			case WM_SLO_OVERLAY_SIZE_CHANGED:
			{
				std::shared_ptr<overlay_window> overlay = app->get_overlay_by_id((int)msg.wParam);
				if(overlay) {
					overlay->create_window_content_buffer();
				}
			}
			break;
			case WM_SLO_OVERLAY_TRANSPARENCY:
			{
				log_cout << "APP: WM_SLO_OVERLAY_TRANSPARENCY " << (int)msg.wParam << ", " << (int)msg.lParam << std::endl;
				std::shared_ptr<overlay_window> overlay = app->get_overlay_by_id((int)msg.wParam);

				if (overlay != nullptr)
				{
					overlay->set_transparency((int)msg.lParam);
				} 
				catched = true;
			}
			break;
			case WM_SLO_OVERLAY_VISIBILITY:
			{
				log_cout << "APP: WM_SLO_OVERLAY_VISIBILITY " << (int)msg.wParam << ", " << (int)msg.lParam << std::endl;
				std::shared_ptr<overlay_window> overlay = app->get_overlay_by_id((int)msg.wParam);

				if (overlay != nullptr)
				{
					
					overlay->set_visibility((bool)msg.lParam, app->showing_overlays);
				} 
				catched = true;
			}
			break;
			case WM_SLO_OVERLAY_SET_AUTOHIDE:
			{
				log_cout << "APP: WM_SLO_OVERLAY_SET_AUTOHIDE " << (int)msg.wParam << ", " << (int)msg.lParam << std::endl;
				std::shared_ptr<overlay_window> overlay = app->get_overlay_by_id((int)msg.wParam);

				if (overlay != nullptr)
				{
					const int autohide_timeout = (int)msg.lParam >> 10;
					const int autohide_transparency = (int)msg.lParam % 512;
					log_debug << "APP: WM_SLO_OVERLAY_SET_AUTOHIDE " << autohide_timeout << ", " << autohide_transparency << std::endl;
					overlay->set_autohide(autohide_timeout, autohide_transparency);
				}
				catched = true;
			}
			break;
			case WM_SLO_OVERLAY_WINDOW_DESTOYED:
			{
				log_cout << "APP: WM_OVERLAY_WINDOW_DESTOYED " << (int)msg.wParam << std::endl;
				std::shared_ptr<overlay_window> overlay = app->get_overlay_by_id((int)msg.wParam);
				app->on_overlay_destroy(overlay);
				catched = true;
			}
			break;
			case WM_SLO_OVERLAY_COMMAND:
			{
				catched = app->process_commands(msg);
			}
			break;
			case WM_SLO_HWND_SOURCE_READY:
			{
				log_cout << "APP: WM_SLO_HWND_SOURCE_READY " << (int)msg.wParam << std::endl;
				std::shared_ptr<overlay_window> overlay = app->get_overlay_by_id((int)msg.wParam);
				app->create_window_for_overlay(overlay);
				catched = true;
			}
			break;
			case WM_TIMER:
				//log_cout << "APP: WM_TIMER id = " << (int)msg.wParam << std::endl;
				if (static_cast<int>(msg.wParam) == OVERLAY_UPDATE_TIMER)
				{
					app->on_update_timer();
					catched = true;
				}
				break;
			default:
				break;
			};

			if (!catched)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if (g_bDblBuffered)
			BufferedPaintUnInit();

		KillTimer(0, OVERLAY_UPDATE_TIMER);
		OVERLAY_UPDATE_TIMER = 0;

		CoUninitialize();
	}

	app->deinit(); //todo clean singleton in case some one start thread another time after stop

	log_cout << "APP: exit from thread " << std::endl;

	thread_state_mutex.lock();

	CloseHandle(overlays_thread);

	overlays_thread = nullptr;
	overlays_thread_id = 0;
	thread_state = sl_overlay_thread_state::destoyed;
	
	thread_state_mutex.unlock();

	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{}
	break;
	case WM_SIZE:
	{
		auto overlay = smg_overlays::get_instance()->get_overlay_by_window(hWnd);
		if(overlay) {
			PostThreadMessage(overlays_thread_id, WM_SLO_OVERLAY_SIZE_CHANGED, overlay->id, NULL);		
		}		
	}
	break;
	case WM_CLOSE:
	{
		//todo some how window wants to be closed so need to remove its overlay object
		log_cout << "APP: WndProc WM_CLOSE for " << hWnd << std::endl;
	}
	break;
	case WM_DESTROY:
	{
		log_cout << "APP: WndProc WM_DESTROY for " << hWnd << std::endl;
		smg_overlays::get_instance()->on_window_destroy(hWnd);

		return 0;
	}
	break;
	case WM_ERASEBKGND:
	{
		// Don't do any erasing here.  It's done in WM_PAINT to avoid flicker.
		return 1;
	}
	break;

	case WM_QUIT:
	{
		log_cout << "APP: WndProc WM_QUIT for " << hWnd << std::endl;
	}
	break;
	case WM_PAINT:
	{
		smg_overlays::get_instance()->draw_overlay_gdi(hWnd, g_bDblBuffered);
		return 0;
	}
	break;

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
