#include "sl_overlay_window.h"

#include "sl_overlays_settings.h"
#include "stdafx.h"

#include <iostream>
#include "overlay_logging.h"

void overlay_window::set_transparency(int transparency)
{
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
	manual_position = false;
	status = overlay_status::creating;
	rect = {0};
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
	return set_new_position(new_rect.left, new_rect.top);
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
	log_cout << "APP: paint_window_from_buffer array_size = " << array_size << ", w " << width << ", h " << height << std::endl;

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

		log_cout << "APP: paint_window_from_buffer workedout = " << workedout << std::endl;
		if (!IsWindowVisible(overlay_hwnd))
		{
			ShowWindow(overlay_hwnd, SW_SHOWNA);
		}
	} else
	{
		log_cout << "APP: paint_window_from_buffer no hbmp " << std::endl;
	}

	return true;
}

bool overlay_window::create_window_content_buffer()
{
	bool created = false;
	BOOL ret = false;
	RECT client_rect = {0};
	HDC hdcScreen = GetDC(orig_handle);

	ret = GetWindowRect(orig_handle, &client_rect);
	if (ret && hdcScreen != nullptr)
	{
		int new_x = client_rect.left;
		int new_y = client_rect.top;
		int new_width = client_rect.right - client_rect.left;
		int new_height = client_rect.bottom - client_rect.top;
		RECT cur_rect = get_rect();

		HDC new_hdc = nullptr;
		HBITMAP new_hbmp = nullptr;
		bool keep_gdi = false;

		if (new_width == cur_rect.right - cur_rect.left && new_height == cur_rect.bottom - cur_rect.top && hdc != nullptr)
		{
			keep_gdi = true;
			new_hdc = hdc;
			new_hbmp = hbmp;
		} else
		{
			new_hdc = CreateCompatibleDC(hdcScreen);
			new_hbmp = CreateCompatibleBitmap(hdcScreen, new_width, new_height);
			SelectObject(new_hdc, new_hbmp);
		}

		if (new_hdc == nullptr || new_hbmp == nullptr)
		{
			DeleteDC(new_hdc);
			DeleteObject(new_hbmp);
		} else
		{
			if (!keep_gdi)
			{
				DeleteDC(hdc);
				DeleteObject(hbmp);

				hdc = new_hdc;
				hbmp = new_hbmp;
			}
			if (manual_position)
			{
				// if we have
				if (new_width == client_rect.right - client_rect.left && new_height == client_rect.bottom - client_rect.top)
				{
				} else
				{
					RECT new_rect = cur_rect;
					new_rect.right = cur_rect.left + new_width;
					new_rect.bottom = cur_rect.top + new_height;
					set_rect(new_rect);

					if (overlay_hwnd)
					{
						MoveWindow(overlay_hwnd, new_rect.left, new_rect.top, new_width, new_height, FALSE);
					}
				}

			} else
			{
				if (client_rect.left == cur_rect.left && client_rect.right == cur_rect.right &&
					client_rect.top == cur_rect.top && client_rect.bottom == cur_rect.bottom)
				{
				} else
				{
					set_rect(client_rect);

					if (overlay_hwnd)
					{
						MoveWindow(overlay_hwnd, new_x, new_y, new_width, new_height, FALSE);
					}
				}
			}
			created = true;
 
		}
	} else
	{
		log_cout << "APP: get_window_screenshot failed to get rect from orig window " << GetLastError() << std::endl;
	}

	ReleaseDC(NULL, hdcScreen);

	return created;
}