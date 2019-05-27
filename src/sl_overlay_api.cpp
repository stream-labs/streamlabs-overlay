#include "sl_overlay_api.h"

#include "sl_overlay_window.h"
#include "sl_overlays.h"
#include "sl_overlays_settings.h"

extern HANDLE overlays_thread;
extern DWORD overlays_thread_id;
extern std::mutex thread_state_mutex;
extern sl_overlay_thread_state thread_state;

//==== node api ====
//when used as a "plugin" we have to start our own thread to work with windows events loop
int WINAPI start_overlays_thread()
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::destoyed)
	{
		thread_state_mutex.unlock();
		return 0;
	} else
	{
		thread_state = sl_overlay_thread_state::starting;
		thread_state_mutex.unlock();

		overlays_thread = CreateThread(nullptr, 0, overlay_thread_func, nullptr, 0, &overlays_thread_id);
		if (overlays_thread)
		{
			return 1;
		} else
		{
			thread_state_mutex.lock();
			thread_state = sl_overlay_thread_state::destoyed;
			thread_state_mutex.unlock();
			return 0;
		}
	}
}

int WINAPI stop_overlays_thread()
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		return 0;
	} else
	{
		thread_state = sl_overlay_thread_state::stopping;
		thread_state_mutex.unlock();

		BOOL ret = PostThreadMessage((DWORD)overlays_thread_id, WM_HOTKEY, HOTKEY_QUIT, 0);

		if (ret)
		{
			return 1;
		} else {
			return 0;
		}
	}
}

std::string get_thread_status_name()
{
	std::lock_guard<std::mutex> lock(thread_state_mutex);
	std::string ret = "nop";
	switch (thread_state)
	{
	case sl_overlay_thread_state::starting:
		ret = "starting";
		break;
	case sl_overlay_thread_state::runing:
		ret = "runing";
		break;
	case sl_overlay_thread_state::stopping:
		ret = "stopping";
		break;
	case sl_overlay_thread_state::destoyed:
		ret = "destoyed";
		break;
	}
	return ret;
}

int WINAPI get_overlays_count()
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		return 0;
	} else
	{
		int ret = smg_overlays::get_instance(in_standalone_mode)->get_count();

		thread_state_mutex.unlock();
		return ret;
	}
}

int WINAPI show_overlays()
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		return 0;
	} else
	{
		BOOL ret = PostThreadMessage((DWORD)overlays_thread_id, WM_HOTKEY, HOTKEY_SHOW_OVERLAYS, 0);

		thread_state_mutex.unlock();
		return ret;
	}
}

int WINAPI hide_overlays()
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		return 0;
	} else
	{
		BOOL ret = PostThreadMessage((DWORD)overlays_thread_id, WM_HOTKEY, HOTKEY_HIDE_OVERLAYS, 0);

		thread_state_mutex.unlock();
		return ret;
	}
}

int WINAPI remove_overlay(int id)
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		return 0;
	} else
	{
		BOOL ret = PostThreadMessage(overlays_thread_id, WM_SLO_WEBVIEW_CLOSE, id, NULL);

		thread_state_mutex.unlock();
		return ret;
	}
}

static int (*callback_keyboard_ptr)(WPARAM, LPARAM) = nullptr;
static int (*callback_mouse_ptr)(WPARAM, LPARAM) = nullptr;
static int (*callback_switch_ptr)() = nullptr;

int WINAPI set_callback_for_keyboard_input(int (*ptr)(WPARAM, LPARAM))
{
	callback_keyboard_ptr = ptr;

	return 0;
}

int WINAPI set_callback_for_mouse_input(int (*ptr)(WPARAM, LPARAM))
{
	callback_mouse_ptr = ptr;

	return 0;
}

int WINAPI set_callback_for_switching_input(int (*ptr)())
{
	callback_switch_ptr = ptr;

	return 0;
}

int WINAPI use_callback_for_keyboard_input(WPARAM wParam, LPARAM lParam)
{
	if (callback_keyboard_ptr != nullptr)
	{
		callback_keyboard_ptr(wParam, lParam);
	}
	return 0;
}

int WINAPI use_callback_for_mouse_input(WPARAM wParam, LPARAM lParam)
{
	if (callback_mouse_ptr != nullptr)
	{
		callback_mouse_ptr(wParam, lParam);
	}
	return 0;
}

int WINAPI use_callback_for_switching_input()
{
	if (callback_switch_ptr != nullptr)
	{
		callback_switch_ptr();
	}
	return 0;
}

int WINAPI switch_overlays_user_input(bool mode_active)
{
	BOOL ret = false;

	if (mode_active)
	{
		ret = PostThreadMessage((DWORD)overlays_thread_id, WM_HOTKEY, HOTKEY_TAKE_INPUT, 0);
	} else
	{
		ret = PostThreadMessage((DWORD)overlays_thread_id, WM_HOTKEY, HOTKEY_RELEASE_INPUT, 0);
	}

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

	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		delete[] url;
		return -1;
	} else
	{
		int ret = smg_overlays::get_instance(in_standalone_mode)->create_web_view_window(n);

		thread_state_mutex.unlock();
		delete[] url;
		return ret;
	}
}

int WINAPI add_overlay_by_hwnd(const void* hwnd_array, size_t array_size)
{
	int ret = -1;
	if (hwnd_array != nullptr && array_size == sizeof(HWND))
	{
		thread_state_mutex.lock();
		if (thread_state != sl_overlay_thread_state::runing)
		{
			thread_state_mutex.unlock();
		} else
		{
			HWND hwnd;
			memcpy(&hwnd, hwnd_array, sizeof(HWND));

			ret = smg_overlays::get_instance(in_standalone_mode)->create_overlay_window_by_hwnd(hwnd);

			thread_state_mutex.unlock();
		}
	}
	return ret;
}

int WINAPI paint_overlay_from_buffer(int overlay_id, const void* image_array, size_t array_size, int width, int height)
{
	int ret = -1;
	if (true)
	{
		thread_state_mutex.lock();
		if (thread_state != sl_overlay_thread_state::runing)
		{
			thread_state_mutex.unlock();
		} else
		{

			std::shared_ptr<overlay_window> overlay = smg_overlays::get_instance(in_standalone_mode)->get_overlay_by_id(overlay_id);
			if (overlay != nullptr)
			{
				overlay->paint_window_from_buffer(image_array, array_size, width, height);
				//todo
				//BOOL ret = PostThreadMessage(overlays_thread_id, WM_SLO_OVERLAY_POSITION, id, reinterpret_cast<LPARAM>(n));
			}
			thread_state_mutex.unlock();
		}
	}
	return ret;
}

int WINAPI add_webview(const char* url)
{
	return add_webview(url, 100, 100, 400, 300);
}

int WINAPI set_overlay_position(int id, int x, int y, int width, int height)
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();

		return -1;
	} else
	{
		RECT* n = new RECT;
		n->left = x;
		n->top = y;
		n->right = x + width;
		n->bottom = y + height;

		BOOL ret = PostThreadMessage(overlays_thread_id, WM_SLO_OVERLAY_POSITION, id, reinterpret_cast<LPARAM>(n));
		thread_state_mutex.unlock();

		if (!ret)
		{
			delete n;
			return -1;
		}

		return id;
	}
}

int WINAPI set_webview_url(int id, char* url)
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		delete[] url;
		return -1;
	} else
	{
		BOOL ret = PostThreadMessage(overlays_thread_id, WM_SLO_WEBVIEW_SET_URL, id, reinterpret_cast<LPARAM>(url));
		thread_state_mutex.unlock();

		if (!ret)
		{
			delete[] url;
			return -1;
		}

		return id;
	}

	return id;
}

int WINAPI set_overlay_transparency(int id, int transparency)
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		return -1;
	} else
	{
		BOOL ret = PostThreadMessage(overlays_thread_id, WM_SLO_OVERLAY_TRANSPARENCY, id, (LPARAM)(transparency));
		thread_state_mutex.unlock();

		if (!ret)
		{
			return -1;
		}

		return id;
	}

	return id;
}

int WINAPI call_webview_roload(int id)
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		return -1;
	} else
	{
		BOOL ret = PostThreadMessage(overlays_thread_id, WM_SLO_WEBVIEW_RELOAD, id, NULL);
		thread_state_mutex.unlock();

		if (!ret)
		{
			return -1;
		}

		return id;
	}

	return id;
}

std::shared_ptr<smg_overlays> get_overlays()
{
	thread_state_mutex.lock();
	if (thread_state != sl_overlay_thread_state::runing)
	{
		thread_state_mutex.unlock();
		return nullptr;
	} else
	{
		thread_state_mutex.unlock();

		return smg_overlays::get_instance(in_standalone_mode);
	}
}