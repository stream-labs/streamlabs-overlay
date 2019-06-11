#include "sl_overlay_window.h"

#include "sl_overlays_settings.h"
#include "stdafx.h"

#include <iostream>
#include "overlay_logging.h"

void overlay_window::set_transparency(int transparency, bool save_as_normal)
{
	if (overlay_hwnd != 0)
	{
		if (save_as_normal)
		{
			overlay_transparency = transparency;
		}
		SetLayeredWindowAttributes(overlay_hwnd, RGB(0xFF, 0xFF, 0xFF), transparency, LWA_ALPHA);
	}
}

void overlay_window::set_visibility(bool visibility, bool overlays_shown)
{
	if (overlay_hwnd != 0)
	{
		overlay_visibility = visibility;
		if (IsWindowVisible(overlay_hwnd))
		{
			if(!overlay_visibility)
			{
				ShowWindow(overlay_hwnd, SW_HIDE);
			}
		} else {
			if(overlay_visibility && overlays_shown)
			{
				ShowWindow(overlay_hwnd, SW_SHOW);
			}
		}
	}
}

int overlay_window::get_transparency()
{
	return overlay_transparency;
}

void overlay_window::set_autohide(int timeout, int transparency)
{
	autohide_after = timeout;
	autohide_by_transparency = transparency;
}

bool overlay_window::ready_to_create_overlay()
{
	return orig_handle != nullptr;
}

bool overlay_window::is_content_updated()
{
	return content_updated;
}

void overlay_window::apply_interactive_mode(bool is_intercepting)
{
	if (overlay_hwnd != 0)
	{
		if (is_intercepting)
		{
			set_transparency(255, false);
		} else
		{
			set_transparency(get_transparency(), false);
		}
	}
}

void overlay_window::check_autohide()
{
	ULONGLONG current_ticks = GetTickCount64();

	if (autohide_after > 0 && !autohidden)
	{
		if (current_ticks > (last_content_chage_ticks + autohide_after * 1000))
		{
			autohidden = true;
			if (autohide_by_transparency > 0)
			{
				set_transparency(autohide_by_transparency, false);
			} else
			{
				ShowWindow(overlay_hwnd, SW_HIDE);
			}
		}
	}
}

void overlay_window::reset_autohide()
{
	if (autohidden)
	{
		if (autohide_by_transparency > 0)
		{
			set_transparency(overlay_transparency, false);
		}
		autohidden = false;
	}
	last_content_chage_ticks = GetTickCount64();
}

overlay_window::~overlay_window()
{
	clean_resources();
}

overlay_window::overlay_window()
{
	content_updated = false;
	static int id_counter = 128;
	id = id_counter++;
	orig_handle = nullptr;
	overlay_hwnd = nullptr;
	hdc = nullptr;
	hbmp = nullptr;
	manual_position = false;
	status = overlay_status::creating;
	rect = {0};

	overlay_transparency = -1;

	autohide_after = 0;
	autohidden = false;
	autohide_by_transparency = 50;
}

void overlay_window::clean_resources()
{
	if (status != overlay_status::destroing)
	{
		status = overlay_status::destroing;
		log_cout << "APP: clean_resources for " << id << std::endl;
		if (hdc != nullptr)
		{

			DeleteDC(hdc);
			hdc = nullptr;
		}

		if (hbmp != nullptr)
		{
			DeleteObject(hbmp);
			hbmp = nullptr;
		}

		if (overlay_hwnd != nullptr)
		{
			log_cout << "APP: clean_resources close overlay window hwnd " << overlay_hwnd << std::endl;
			DestroyWindow(overlay_hwnd);
		} else
		{
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
	manual_position = true;

	if (overlay_hwnd)
	{
		MoveWindow(
		    overlay_hwnd, new_rect.left, new_rect.top, new_rect.right - new_rect.left, new_rect.bottom - new_rect.top, FALSE);
	}

	return set_rect(new_rect);
}

bool overlay_window::set_new_position(int x, int y)
{
	RECT ret = get_rect();

	int shift = ret.left - x;
	ret.left -= shift;
	ret.right -= shift;

	shift = ret.top - y;
	ret.top -= shift;
	ret.bottom -= shift;

	manual_position = true;

	if (overlay_hwnd)
	{
		MoveWindow(overlay_hwnd, x, y, ret.right - ret.left, ret.bottom - ret.top, FALSE);
	}

	return set_rect(ret);
}

bool overlay_window::set_rect(RECT& new_rect)
{
	std::lock_guard<std::mutex> lock(rect_access);
	rect = new_rect;
	return true;
}

bool overlay_window::paint_window_from_buffer(const void* image_array, size_t array_size, int width, int height)
{
	log_debug << "APP: Saving image from electron array_size = " << array_size << ", w " << width << ", h " << height
	          << std::endl;

	if (hbmp != nullptr)
	{
		LONG workedout = 0;

		BITMAPINFO phmi;
		phmi.bmiHeader.biSize = sizeof(phmi.bmiHeader);
		phmi.bmiHeader.biWidth = width;
		phmi.bmiHeader.biHeight = -height;
		phmi.bmiHeader.biPlanes = 1;
		phmi.bmiHeader.biBitCount = 32;
		phmi.bmiHeader.biCompression = BI_RGB;
		workedout = SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, image_array, &phmi, false);
		if (workedout != height)
		{
			log_error << "APP: Saving image from electron with SetDIBitsToDevice failed with workedout = " << workedout
			          << std::endl;
		}
		content_updated = true;
		if (autohidden)
		{
			if (autohide_by_transparency > 0 )
			{
				set_transparency(overlay_transparency, false);
			} else
			{
				if (!IsWindowVisible(overlay_hwnd))
				{
					ShowWindow(overlay_hwnd, SW_SHOW);
				}
			}
			autohidden = false;
		}
	} else
	{
		log_error << "APP: Saving image from electron failed. no hbmp to save to." << std::endl;
	}

	return true;
}

void overlay_window::paint_to_window(HDC window_hdc)
{
	RECT overlay_rect = get_rect();
	BOOL ret = true;

	ret = BitBlt(
	    window_hdc, 0, 0, overlay_rect.right - overlay_rect.left, overlay_rect.bottom - overlay_rect.top, hdc, 0, 0, SRCCOPY);

	if (!ret)
	{
		log_cout << "APP: paint_to_window had issue " << GetLastError() << std::endl;
	}

	last_content_chage_ticks = GetTickCount64();

	content_updated = false;
}

bool overlay_window::apply_size_from_orig()
{
	BOOL ret = false;
	RECT client_rect = {0};
	ret = GetWindowRect(orig_handle, &client_rect);

	int new_x = client_rect.left;
	int new_y = client_rect.top;
	int new_width = client_rect.right - client_rect.left;
	int new_height = client_rect.bottom - client_rect.top;

	rect = client_rect;

	return true;
}

bool overlay_window::create_window_content_buffer()
{
	bool created = false;
	BOOL ret = false;
	RECT client_rect = {0};
	HDC hdcScreen = GetDC(overlay_hwnd);

	ret = GetWindowRect(overlay_hwnd, &client_rect);

	if (hdcScreen != nullptr)
	{
		int new_x = client_rect.left;
		int new_y = client_rect.top;
		int new_width = client_rect.right - client_rect.left;
		int new_height = client_rect.bottom - client_rect.top;
		RECT cur_rect = get_rect();

		HDC new_hdc = nullptr;
		HBITMAP new_hbmp = nullptr;
		bool keep_gdi = false;

		new_hdc = CreateCompatibleDC(hdcScreen);
		new_hbmp = CreateCompatibleBitmap(hdcScreen, new_width, new_height);
		SelectObject(new_hdc, new_hbmp);

		if (new_hdc == nullptr || new_hbmp == nullptr)
		{
			DeleteDC(new_hdc);
			DeleteObject(new_hbmp);
		} else
		{
			DeleteDC(hdc);
			DeleteObject(hbmp);

			hdc = new_hdc;
			hbmp = new_hbmp;
			created = true;
		}
	} else
	{
		log_cout << "APP: get_window_screenshot failed to get rect from orig window " << GetLastError() << std::endl;
	}

	ReleaseDC(NULL, hdcScreen);

	return created;
}