#include "sl_overlay_window.h"

#include "sl_overlays_settings.h"
#include "sl_web_view.h"
#include "stdafx.h"

#include <iostream>

bool overlay_window::save_state_to_settings()
{
	return false;
}

std::string overlay_window::get_url()
{
	return "";
}

void overlay_window::set_url(char* url)
{
	delete[] url;
}

void overlay_window::set_transparency(int transparency) {
	if (overlay_hwnd != 0)
	{
		SetLayeredWindowAttributes(overlay_hwnd, RGB(0xFF, 0xFF, 0xFF), transparency, LWA_ALPHA);
	}
}

bool overlay_window::ready_to_create_overlay()
{
	return orig_handle != nullptr;
}

overlay_window::~overlay_window()
{
	clean_resources();
}

overlay_window::overlay_window()
{
	static int id_counter = 128;
	id = id_counter++;
	use_method = sl_window_capture_method::print;
	orig_handle = nullptr;
	overlay_hwnd = nullptr;
	hdc = nullptr;
	hbmp = nullptr;

	status = overlay_status::creating;
}

void overlay_window::clean_resources()
{
	if (status != overlay_status::destroing) {
		status = overlay_status::destroing;
		std::cout << "APP: clean_resources for " << id << std::endl;
		if (hdc != nullptr) {

			DeleteDC(hdc);
			hdc = nullptr;
		}

		if (hbmp != nullptr) {
			DeleteObject(hbmp);
			hbmp = nullptr;
		}

		if (overlay_hwnd != nullptr) {
			std::cout << "APP: clean_resources close overlay window hwnd " << overlay_hwnd << std::endl;
			DestroyWindow(overlay_hwnd);
		} else {
			PostMessage(0, WM_SLO_OVERLAY_WINDOW_DESTOYED, id, NULL);
		}
	}
}

RECT overlay_window::get_rect()
{
	std::lock_guard<std::mutex> lock(rect_access);
	RECT ret = rect;
	return ret;
}

bool overlay_window::apply_new_rect(RECT& new_rect)
{
	return set_rect(new_rect);
}

bool overlay_window::set_rect(RECT& new_rect)
{
	std::lock_guard<std::mutex> lock(rect_access);
	rect = new_rect;
	return true;
}

bool overlay_window::get_window_screenshot()
{
	bool updated = false;
	BOOL ret = false;
	RECT client_rect = {0};
	HDC hdcScreen = GetDC(orig_handle);

	ret = GetWindowRect(orig_handle, &client_rect);
	if (ret && hdcScreen != nullptr) {
		int new_x = client_rect.left;
		int new_y = client_rect.top;
		int new_width = client_rect.right - client_rect.left;
		int new_height = client_rect.bottom - client_rect.top;
		RECT cur_rect = get_rect();

		HDC new_hdc = nullptr;
		HBITMAP new_hbmp = nullptr;
		bool keep_gdi = false;

		if (new_width == cur_rect.right - cur_rect.left && new_height == cur_rect.bottom - cur_rect.top && hdc != nullptr) {
			keep_gdi = true;
			new_hdc = hdc;
			new_hbmp = hbmp;
		} else {
			new_hdc = CreateCompatibleDC(hdcScreen);
			new_hbmp = CreateCompatibleBitmap(hdcScreen, new_width, new_height);
			SelectObject(new_hdc, new_hbmp);
		}

		if (new_hdc == nullptr || new_hbmp == nullptr) {
			DeleteDC(new_hdc);
			DeleteObject(new_hbmp);
		} else {
			switch (use_method) {
			case sl_window_capture_method::bitblt:
				ret = BitBlt(new_hdc, 0, 0, new_width, new_height, hdcScreen, 0, 0, SRCCOPY);
				break;
			case sl_window_capture_method::print:
				ret = PrintWindow(orig_handle, new_hdc, 0);
				break;
			case sl_window_capture_method::message_print:
				LRESULT msg_ret = SendMessage(
				    orig_handle,
				    WM_PAINT,
				    (WPARAM)new_hdc,
				    PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED);
				ret = (msg_ret == S_OK);
				break;
			};

			if (ret) {
				if (!keep_gdi) {
					DeleteDC(hdc);
					DeleteObject(hbmp);

					hdc = new_hdc;
					hbmp = new_hbmp;
				}

				if (client_rect.left == cur_rect.left && client_rect.right == cur_rect.right &&
				    client_rect.top == cur_rect.top && client_rect.bottom == cur_rect.bottom) {
				} else {
					set_rect(client_rect);

					if (overlay_hwnd) {
						MoveWindow(overlay_hwnd, new_x, new_y, new_width, new_height, FALSE);
					}
				}
				updated = true;
			} else {
				std::cout << "APP: get_window_screenshot failed to get bitmap from orig window "
				          << GetLastError() << std::endl;
				if (!keep_gdi) {
					DeleteDC(new_hdc);
					DeleteObject(new_hbmp);
				}
			}
		}
	}

	if (overlay_hwnd) {
		if (!updated) {
			if (IsWindow(orig_handle)) {
				// it is still a window we can show it later
				ShowWindow(overlay_hwnd, SW_HIDE);
			} else {
				// it is not a window anymore . should close our
				// overlay
				ShowWindow(overlay_hwnd, SW_HIDE);
			}
		} else {
			if (!IsWindowVisible(overlay_hwnd)) {
				ShowWindow(overlay_hwnd, SW_SHOWNA);
			}
		}
	}
	ReleaseDC(NULL, hdcScreen);

	return updated;
}

bool web_view_window::save_state_to_settings()
{
	web_view_overlay_settings wnd_settings;
	wnd_settings.url = url;

	RECT client_rect = {0};
	GetWindowRect(orig_handle, &client_rect);
	wnd_settings.x = client_rect.left;
	wnd_settings.y = client_rect.top;
	wnd_settings.width = client_rect.right - client_rect.left;
	wnd_settings.height = client_rect.bottom - client_rect.top;

	app_settings->web_pages.push_back(wnd_settings);
	return true;
}

std::string web_view_window::get_url()
{
	return url;
}

void web_view_window::set_url(char* new_url)
{
	std::string save_url = new_url;
	BOOL ret = PostThreadMessage(web_views_thread_id, WM_SLO_WEBVIEW_SET_URL, id, reinterpret_cast<LPARAM>(new_url));
	if (!ret) {
		delete[] new_url;
	} else {
		url = save_url;
	}
}

bool web_view_window::ready_to_create_overlay()
{
	return orig_handle != nullptr;
}

void web_view_window::clean_resources()
{
	overlay_window::clean_resources();
	PostThreadMessage(web_views_thread_id, WM_SLO_WEBVIEW_CLOSE, id, NULL);
}

bool web_view_window::apply_new_rect(RECT& new_rect)
{
	RECT* send_rect = new RECT(new_rect);
	BOOL ret = PostThreadMessage(web_views_thread_id, WM_SLO_OVERLAY_POSITION, id, reinterpret_cast<LPARAM>(send_rect));
	if (!ret) {
		delete send_rect;
		return false;
	}

	return true;
}
