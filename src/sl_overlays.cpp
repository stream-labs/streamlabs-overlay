/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "sl_overlays.h"
#include "stdafx.h"

#include <algorithm>
#include <iostream>

#include "sl_overlay_window.h"
#include "sl_overlays_settings.h"

#include "overlay_logging.h"
#include "sl_overlay_api.h"

#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "shcore.lib")

wchar_t const g_szWindowClass[] = L"overthetop_overlay";

std::shared_ptr<smg_overlays> smg_overlays::instance = nullptr;

extern HANDLE overlays_thread;
extern DWORD overlays_thread_id;
extern std::mutex thread_state_mutex;
extern sl_overlay_thread_state thread_state;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool smg_overlays::process_commands(MSG& msg)
{
	bool ret = false;
	log_cout << "APP: process_commands id " << msg.wParam << std::endl;
	switch (msg.wParam)
	{
	case COMMAND_SHOW_OVERLAYS:
	{
		if (showing_overlays)
		{
			// need to hide befor show. or show can be ignored.
			showing_overlays = false;
			hide_overlays();
		}

		showup_overlays();
		showing_overlays = true;
		ret = true;
	}
	break;
	case COMMAND_HIDE_OVERLAYS:
	{
		showing_overlays = false;

		hide_overlays();
		ret = true;
	}
	break;
	case COMMAND_QUIT:
	{
		thread_state_mutex.lock();

		log_cout << "APP: COMMAND_QUIT " << static_cast<int>(thread_state) << std::endl;
		if (thread_state != sl_overlay_thread_state::runing)
		{
			thread_state_mutex.unlock();
		} else
		{
			thread_state = sl_overlay_thread_state::stopping;
			thread_state_mutex.unlock();
		}

		if (!quiting)
		{
			quit();
		}
		ret = true;
	}
	break;

	case COMMAND_TAKE_INPUT:
	{
		hook_user_input();

		showup_overlays();
		apply_interactive_mode_view();
	}
	break;
	case COMMAND_RELEASE_INPUT:
	{
		unhook_user_input();

		apply_interactive_mode_view();
	}
	break;
	};

	return ret;
}

void smg_overlays::quit()
{
	log_cout << "APP: quit " << std::endl;
	quiting = true;

	if (showing_windows.size() != 0)
	{
		std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<overlay_window>& n) {
			PostMessage(0, WM_SLO_OVERLAY_CLOSE, n->id, 0);
		});
	} else
	{
		on_overlay_destroy(nullptr);
	}

	//it's not all. after last windows will be destroyed then thread quits
}

int smg_overlays::create_overlay_window_by_hwnd(HWND hwnd)
{
	std::shared_ptr<overlay_window> new_overlay_window;
	if (direct2d_paint)
	{
		new_overlay_window = std::make_shared<overlay_window_direct2d>();
	} else
	{
		std::shared_ptr<overlay_window_gdi> new_overlay_window_gdi = std::make_shared<overlay_window_gdi>();
		new_overlay_window_gdi->set_dbl_buffering(g_bDblBuffered);
		new_overlay_window = new_overlay_window_gdi;
	}
	
	new_overlay_window->orig_handle = hwnd;
	new_overlay_window->apply_size_from_orig();

	{
		std::unique_lock<std::shared_mutex> lock(overlays_list_access);
		showing_windows.push_back(new_overlay_window);
	}

	PostThreadMessage(
	    overlays_thread_id,
	    WM_SLO_HWND_SOURCE_READY,
	    new_overlay_window->id,
	    reinterpret_cast<LPARAM>(&(new_overlay_window->orig_handle)));

	return new_overlay_window->id;
}

void smg_overlays::on_update_timer()
{
	if (showing_overlays)
	{
		std::shared_lock<std::shared_mutex> lock(overlays_list_access);
		std::for_each(
		    showing_windows.begin(),
		    showing_windows.end(),
		    [is_intercepting = this->is_intercepting](std::shared_ptr<overlay_window>& n) {
			    if (n->is_visible())
			    {
				    if (n->is_content_updated())
				    {
					    InvalidateRect(n->overlay_hwnd, nullptr, TRUE);
				    } else
				    {
					    if (!is_intercepting)
					    {
						    n->check_autohide();
					    }
				    }
			    }
		    });
	}
}

void smg_overlays::deinit()
{
	if (g_bDblBuffered)
		BufferedPaintUnInit();

	log_cout << "APP: deinit " << std::endl;
	quiting = false;
}

void smg_overlays::showup_overlays()
{
	log_cout << "APP: showup_overlays " << std::endl;
	{
		std::shared_lock<std::shared_mutex> lock(overlays_list_access);
		std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<overlay_window>& n) {
			if (n->overlay_hwnd != 0 && n->is_visible())
			{
				ShowWindow(n->overlay_hwnd, SW_SHOW);
				n->reset_autohide();
			}
		});
	}
}

void smg_overlays::hide_overlays()
{
	log_cout << "APP: hide_overlays " << std::endl;
	std::shared_lock<std::shared_mutex> lock(overlays_list_access);
	std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<overlay_window>& n) {
		if (n->overlay_hwnd != 0)
		{
			ShowWindow(n->overlay_hwnd, SW_HIDE);
		}
	});
}

void smg_overlays::apply_interactive_mode_view()
{
	log_cout << "APP: apply_interactive_mode_view " << std::endl;
	{
		std::shared_lock<std::shared_mutex> lock(overlays_list_access);
		std::for_each(
		    showing_windows.begin(),
		    showing_windows.end(),
		    [is_intercepting = this->is_intercepting](std::shared_ptr<overlay_window>& n) {
			    n->apply_interactive_mode(is_intercepting);
		    });
	}
}

void smg_overlays::create_overlay_window_class()
{
	WNDCLASSEX wcex = {sizeof(wcex)};
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = 0;
	wcex.lpszClassName = g_szWindowClass;

	RegisterClassEx(&wcex);
}

bool smg_overlays::is_inside_overlay(int x, int y)
{
	bool ret = false;
	std::shared_lock<std::shared_mutex> lock(overlays_list_access);
	std::for_each(showing_windows.begin(), showing_windows.end(), [&ret, &x, &y](std::shared_ptr<overlay_window>& n) {
		RECT o_rect = n->get_rect();
		if (n->is_visible())
		{
			if (x <= o_rect.right && x >= o_rect.left)
			{
				if (y <= o_rect.bottom && y >= o_rect.top)
				{
					ret = true;
				}
			}
		}
	});
	return ret;
}

void smg_overlays::create_window_for_overlay(std::shared_ptr<overlay_window>& overlay)
{
	if (overlay->create_window())
	{
		if (showing_overlays)
		{
			ShowWindow(overlay->overlay_hwnd, SW_SHOW);
		} else
		{
			ShowWindow(overlay->overlay_hwnd, SW_HIDE);
		}
	} 
}

HHOOK msg_hook = nullptr;
HHOOK llkeyboard_hook = nullptr;
HHOOK llmouse_hook = nullptr;

LRESULT CALLBACK CallWndMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	log_cout << "APP: CallWndMsgProc " << wParam << std::endl;

	return CallNextHookEx(msg_hook, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		KBDLLHOOKSTRUCT* event = (KBDLLHOOKSTRUCT*)lParam;
		log_cout << "APP: LowLevelKeyboardProc " << event->vkCode << ", " << event->dwExtraInfo << std::endl;

		if (event->vkCode == VK_ESCAPE)
		{
			use_callback_for_switching_input();
		} else

			use_callback_for_keyboard_input(wParam, lParam);
		return -1;
	}

	return CallNextHookEx(llkeyboard_hook, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		MSLLHOOKSTRUCT* event = (MSLLHOOKSTRUCT*)lParam;
		log_cout << "APP: LowLevelMouseProc " << wParam << ", " << event->pt.x << ", " << event->pt.y << ", "
		         << event->dwExtraInfo << std::endl;

		std::shared_ptr<smg_overlays> app = smg_overlays::get_instance();
		if (app->is_inside_overlay(event->pt.x, event->pt.y))
		{
			use_callback_for_mouse_input(wParam, lParam);
		} else
		{
			if (wParam != WM_MOUSEMOVE && wParam != WM_MOUSEWHEEL && wParam != WM_MOUSEHWHEEL)
			{
				use_callback_for_switching_input();
			}
		}

		if (wParam != WM_MOUSEMOVE)
		{
			return -1;
		}
	}
	return CallNextHookEx(llmouse_hook, nCode, wParam, lParam);
}

void smg_overlays::hook_user_input()
{
	log_cout << "APP: hook_user_input " << std::endl;

	if (!is_intercepting)
	{
		HWND game_hwnd = GetForegroundWindow();
		if (game_hwnd != nullptr)
		{
			//print window title
			TCHAR title[256];
			GetWindowText(game_hwnd, title, 256);
			std::wstring title_wstr(title);
			std::string title_str(title_wstr.begin(), title_wstr.end());
			log_debug << "APP: hook_user_input catch window - " << title_str << std::endl;
		}

		llkeyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
		llmouse_hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

		is_intercepting = true;

		log_cout << "APP: Input hooked" << std::endl;
	}
}

void smg_overlays::unhook_user_input()
{
	log_cout << "APP: unhook_user_input " << std::endl;
	if (is_intercepting)
	{
		if (msg_hook != nullptr)
		{
			UnhookWindowsHookEx(msg_hook);
			msg_hook = nullptr;
		}
		if (llkeyboard_hook != nullptr)
		{
			UnhookWindowsHookEx(llkeyboard_hook);
			llkeyboard_hook = nullptr;
		}
		if (llmouse_hook != nullptr)
		{
			UnhookWindowsHookEx(llmouse_hook);
			llmouse_hook = nullptr;
		}

		log_cout << "APP: Input unhooked" << std::endl;
		is_intercepting = false;
	}
}

size_t smg_overlays::get_count()
{
	std::shared_lock<std::shared_mutex> lock(overlays_list_access);
	return showing_windows.size();
}

std::shared_ptr<overlay_window> smg_overlays::get_overlay_by_id(int overlay_id)
{
	std::shared_ptr<overlay_window> ret;
	std::shared_lock<std::shared_mutex> lock(overlays_list_access);

	std::list<std::shared_ptr<overlay_window>>::iterator findIter =
	    std::find_if(showing_windows.begin(), showing_windows.end(), [&overlay_id](std::shared_ptr<overlay_window>& n) {
		    return overlay_id == n->id;
	    });

	if (findIter != showing_windows.end())
	{
		ret = *findIter;
	}

	return ret;
}

std::shared_ptr<overlay_window> smg_overlays::get_overlay_by_window(HWND overlay_hwnd)
{
	std::shared_ptr<overlay_window> ret;
	std::shared_lock<std::shared_mutex> lock(overlays_list_access);

	std::list<std::shared_ptr<overlay_window>>::iterator findIter =
	    std::find_if(showing_windows.begin(), showing_windows.end(), [&overlay_hwnd](std::shared_ptr<overlay_window>& n) {
		    return overlay_hwnd == n->overlay_hwnd;
	    });

	if (findIter != showing_windows.end())
	{
		ret = *findIter;
	}

	return ret;
}

bool smg_overlays::remove_overlay(std::shared_ptr<overlay_window> overlay)
{
	log_cout << "APP: RemoveOverlay status " << static_cast<int>(overlay->status) << std::endl;
	if (overlay->status != overlay_status::destroing)
	{
		overlay->clean_resources();

		return true;
	}
	return false;
}

bool smg_overlays::on_window_destroy(HWND window)
{
	auto overlay = get_overlay_by_window(window);
	log_cout << "APP: on_window_destroy and overlay found " << (overlay != nullptr) << std::endl;
	const bool removed = on_overlay_destroy(overlay);
	return removed;
}

bool smg_overlays::on_overlay_destroy(std::shared_ptr<overlay_window> overlay)
{
	bool removed = false;
	if (overlay != nullptr)
	{
		log_cout << "APP: overlay status was " << static_cast<int>(overlay->status) << std::endl;
		if (overlay->status == overlay_status::destroing)
		{
			std::unique_lock<std::shared_mutex> lock(overlays_list_access);
			showing_windows.remove_if([&overlay](std::shared_ptr<overlay_window>& n) { return (overlay->id == n->id); });
			removed = true;
		}
	}

	log_cout << "APP: overlays count " << showing_windows.size() << " and quiting " << quiting << std::endl;
	if (showing_windows.size() == 0 && quiting)
	{
		PostQuitMessage(0);
	}

	return removed;
}

std::vector<int> smg_overlays::get_ids()
{
	std::vector<int> ret;
	int i = 0;
	std::shared_lock<std::shared_mutex> lock(overlays_list_access);
	ret.resize(showing_windows.size());
	std::for_each(showing_windows.begin(), showing_windows.end(), [&ret, &i](std::shared_ptr<overlay_window>& n) {
		ret[i] = n->id;
		i++;
	});

	return ret;
}

std::shared_ptr<smg_overlays> smg_overlays::get_instance()
{
	if (instance == nullptr)
	{
		instance = std::make_shared<smg_overlays>();
	}
	return instance;
}

smg_overlays::smg_overlays()
{
	showing_overlays = false;
	quiting = false;

	log_cout << "APP: start overlays " << std::endl;
}

smg_overlays::~smg_overlays()
{
	if (m_pDirect2dFactory != nullptr)
	{
		m_pDirect2dFactory->Release();

		m_pDirect2dFactory = nullptr;
	}
}

void smg_overlays::init()
{
	HRESULT hr;
	if (direct2d_paint)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pDirect2dFactory);
		if (!SUCCEEDED(hr))
		{
			direct2d_paint = false;
		}
	}

	if (!direct2d_paint)
	{
		HRESULT hr = BufferedPaintInit();
		g_bDblBuffered = SUCCEEDED(hr);
	}

	app_settings->default_init();

	create_overlay_window_class();
}

void smg_overlays::draw_overlay(HWND& hWnd)
{
	{
		std::shared_lock<std::shared_mutex> lock(overlays_list_access);
		std::for_each(showing_windows.begin(), showing_windows.end(), [&hWnd](std::shared_ptr<overlay_window>& n) {
			if (hWnd == n->overlay_hwnd)
			{
				n->paint_to_window(0);
			}
		});
	}
}

