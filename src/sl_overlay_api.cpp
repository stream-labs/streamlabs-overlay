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

		return ret;
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
		int ret = smg_overlays::get_instance()->get_count();

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

static int (*callback_ptr)(int) = nullptr;
int WINAPI set_callback_for_user_input(int (*ptr)(int))
{
	callback_ptr = ptr;

	return 0;
}

int WINAPI use_callback_for_user_input()
{
	static int send = 1122;
	if (callback_ptr != nullptr)
	{
		callback_ptr(send);
		//callback_ptr = nullptr;
		send += 123;
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
		int ret = smg_overlays::get_instance()->create_web_view_window(n);

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

			ret = smg_overlays::get_instance()->create_overlay_window_by_hwnd(hwnd);

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

			std::shared_ptr<overlay_window> overlay = smg_overlays::get_instance()->get_overlay_by_id(overlay_id);
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

		return smg_overlays::get_instance();
	}
}