#include "sl_overlay_window.h"

#include "sl_overlays_settings.h"
#include "stdafx.h"

#include <iostream>
#include "overlay_logging.h"

bool overlay_window::save_state_to_settings()
{
	return false;
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
	update_from_original = true;
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
	if (status != overlay_status::destroing) {
		status = overlay_status::destroing;
		log_cout << "APP: clean_resources for " << id << std::endl;
		if (hdc != nullptr) {

			DeleteDC(hdc);
			hdc = nullptr;
		}

		if (hbmp != nullptr) {
			DeleteObject(hbmp);
			hbmp = nullptr;
		}

		if (overlay_hwnd != nullptr) {
			log_cout << "APP: clean_resources close overlay window hwnd " << overlay_hwnd << std::endl;
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
	return set_new_position(new_rect.left, new_rect.top);
}

bool overlay_window::set_new_position(int x, int y)
{
	RECT ret = get_rect();
	
	int shift = ret.left-x;
	ret.left-=shift;
	ret.right-=shift;

	shift = ret.top-y;
	ret.top-=shift;
	ret.bottom-=shift;
	
	manual_position = true;	

	if (overlay_hwnd) {
		MoveWindow(overlay_hwnd, x, y, ret.right-ret.left, ret.bottom-ret.top, FALSE);
	}

	return set_rect(ret);
}

bool overlay_window::set_rect(RECT& new_rect)
{
	std::lock_guard<std::mutex> lock(rect_access);
	rect = new_rect;
	return true;
}

bool  overlay_window::paint_window_from_buffer(const void* image_array, size_t array_size, int width, int height)
{
	log_cout << "APP: paint_window_from_buffer array_size = " << array_size << ", w " << width << ", h " << height  << std::endl;
	// HDC new_hdc = nullptr;
	// HBITMAP new_hbmp = nullptr;
	// int new_width = width;
	// int new_height = height;
	
	// HDC hdcScreen = GetDC(orig_handle);
	// new_hdc = CreateCompatibleDC(hdcScreen);
	// new_hbmp = CreateCompatibleBitmap(hdcScreen, new_width, new_height);
	// SelectObject(new_hdc, new_hbmp);
	
	if(hbmp != nullptr) {
		LONG workedout = 0 ;
		ULONGLONG ticks_before = GetTickCount64();
		if(false) {
		if(true) {
			// Marked as outdate in msdn with note.
			// "This function is provided only for compatibility with 16-bit versions of Windows. Applications should use the SetDIBits function."
			// but 30-50 percent faster than SetDIBits
			// and blit speed about 10000 times per second for 600x800 image 
			workedout = SetBitmapBits(hbmp, width * height * 4, image_array);
		} else {
			BITMAPINFO phmi; 
			phmi.bmiHeader.biSize = sizeof(phmi.bmiHeader);
			phmi.bmiHeader.biWidth = width;
			phmi.bmiHeader.biHeight = -height;
			phmi.bmiHeader.biPlanes = 1;
			phmi.bmiHeader.biBitCount = 32;
			phmi.bmiHeader.biCompression = BI_RGB;
			workedout = SetDIBits(hdc, hbmp, 0, height, image_array, &phmi, false);
		}	
		} else {
			BITMAPINFO phmi; 
			phmi.bmiHeader.biSize = sizeof(phmi.bmiHeader);
			phmi.bmiHeader.biWidth = width;
			phmi.bmiHeader.biHeight = -height;
			phmi.bmiHeader.biPlanes = 1;
			phmi.bmiHeader.biBitCount = 32;
			phmi.bmiHeader.biCompression = BI_RGB;
			//for(int i =0 ; i<1000; i++ ) 
			{
				workedout = SetDIBitsToDevice(hdc, 0,0, width, height, 0,0, 0, height, image_array, &phmi, false);
			}
		}
		
		ULONGLONG ticks_after = GetTickCount64();

		log_cout << "APP: paint_window_from_buffer ticks " << ticks_after-ticks_before <<  std::endl;

		log_cout << "APP: paint_window_from_buffer workedout = " << workedout <<  std::endl;
		if (!IsWindowVisible(overlay_hwnd)) {
				ShowWindow(overlay_hwnd, SW_SHOWNA);
		}
		update_from_original = false;
	} else {
		log_cout << "APP: paint_window_from_buffer no hbmp " << std::endl;
	}

	return true;
}

bool overlay_window::get_window_screenshot()
{
	if( !update_from_original )
	{
		return true;
	}

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

		//log_cout << "APP: get_window_screenshot new_x " << new_x << ", new_y " << new_y << ", new_width " << new_width << ", new_height " << new_height << std::endl;

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
				if(manual_position)
				{
					// if we have 
					if ( new_width == client_rect.right-client_rect.left &&
						new_height == client_rect.bottom-client_rect.top) {
					} else {
						RECT new_rect = cur_rect;
						new_rect.right = cur_rect.left + new_width; 
						new_rect.bottom = cur_rect.top + new_height;
						set_rect(new_rect);

						if (overlay_hwnd) {
							MoveWindow(overlay_hwnd, new_rect.left, new_rect.top, new_width, new_height, FALSE);
						}
					}

				} else {
					if (client_rect.left == cur_rect.left && client_rect.right == cur_rect.right &&
						client_rect.top == cur_rect.top && client_rect.bottom == cur_rect.bottom) {
					} else {
						set_rect(client_rect);

						if (overlay_hwnd) {
							MoveWindow(overlay_hwnd, new_x, new_y, new_width, new_height, FALSE);
						}
					}
				}
				updated = true;
			} else {
				log_cout << "APP: get_window_screenshot failed to get bitmap from orig window "
				          << GetLastError() << std::endl;
				if (!keep_gdi) {
					DeleteDC(new_hdc);
					DeleteObject(new_hbmp);
				}
			}
		}
	} else {
		//log_cout << "APP: get_window_screenshot failed to get rect from orig window " << GetLastError() << std::endl;
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