#include "sl_overlay_api.h"

#include "sl_overlays_settings.h"
#include "sl_overlays.h"

extern HANDLE overlays_thread;
extern DWORD overlays_thread_id;

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

int WINAPI get_overlays_count()
{
	return smg_overlays::get_instance()->get_count();
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

int WINAPI remove_overlay(int id)
{
	BOOL ret = PostThreadMessage(overlays_thread_id, WM_WEBVIEW_CLOSE, id, NULL);
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
	delete[] url;

	return ret;
}

int WINAPI add_webview(const char* url)
{
	return add_webview(url, 100, 100, 400, 300);
}

bool WINAPI set_webview_position(int id, int x, int y, int width, int height)
{
	RECT* n = new RECT;
	n->left = x;
	n->top = y;
	n->right = x + width;
	n->bottom = y + height;

	BOOL ret = PostThreadMessage(overlays_thread_id, WM_WEBVIEW_SET_POSITION, id, reinterpret_cast<LPARAM>(n));
	if (!ret) {
		delete n;
		return false;
	}

	return true;
}

bool WINAPI set_webview_url(int id, char* url)
{
	BOOL ret = PostThreadMessage(overlays_thread_id, WM_WEBVIEW_SET_URL, id, reinterpret_cast<LPARAM>(url));
	if (!ret) {
		delete[] url;
		return false;
	}

	return true;
}

std::shared_ptr<smg_overlays> get_overlays()
{
	return smg_overlays::get_instance();
}