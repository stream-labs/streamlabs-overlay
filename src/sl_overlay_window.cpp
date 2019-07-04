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
			if (!overlay_visibility)
			{
				ShowWindow(overlay_hwnd, SW_HIDE);
			}
		} else
		{
			if (overlay_visibility && overlays_shown)
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

bool overlay_window::is_visible()
{
	return overlay_visibility;
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
		if (current_ticks > (last_content_chage_ticks + 1000 * autohide_after))
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
	last_content_chage_ticks = 0;
	overlay_visibility = true;
	content_updated = false;
	content_set = false;
	static int id_counter = 128;
	id = id_counter++;
	orig_handle = nullptr;
	overlay_hwnd = nullptr;
	manual_position = false;
	status = overlay_status::creating;
	rect = {0};

	overlay_transparency = -1;

	autohide_after = 0;
	autohidden = false;
	autohide_by_transparency = 50;
}

overlay_window_gdi::overlay_window_gdi()
{
	hdc = nullptr;
	hbmp = nullptr;
}

overlay_window_direct2d::overlay_window_direct2d()
{
	m_pRenderTarget = nullptr;
	m_pBitmap = nullptr;
}

void overlay_window::clean_resources()
{
	if (status != overlay_status::destroing)
	{
		status = overlay_status::destroing;
		log_cout << "APP: clean_resources for " << id << std::endl;

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

void overlay_window_gdi::clean_resources()
{
	if (status != overlay_status::destroing)
	{
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
	}

	overlay_window::clean_resources();
}

void overlay_window_direct2d::clean_resources()
{
	if (status != overlay_status::destroing)
	{
		if (m_pRenderTarget != nullptr)
		{
			m_pRenderTarget->Release();
			m_pRenderTarget = nullptr;
		}
	}

	overlay_window::clean_resources();
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

	if (orig_handle)
	{
		int iDpi = try_to_get_dpi(orig_handle);
		int dpiScaledX = MulDiv(new_rect.left, iDpi, 96);
		int dpiScaledY = MulDiv(new_rect.top, iDpi, 96);

		log_debug << "APP: apply_new_rect " << new_rect.left << " to " << dpiScaledX << std::endl;

		SetWindowPos( overlay_hwnd,HWND_TOPMOST, dpiScaledX, dpiScaledY, new_rect.right - new_rect.left, new_rect.bottom - new_rect.top, SWP_NOREDRAW);
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
		SetWindowPos(overlay_hwnd, HWND_TOPMOST, x, y, ret.right - ret.left, ret.bottom - ret.top, SWP_NOREDRAW);
	}

	return set_rect(ret);
}

bool overlay_window::set_rect(RECT& new_rect)
{
	std::lock_guard<std::mutex> lock(rect_access);
	rect = new_rect;
	return true;
}

bool overlay_window::set_cached_image(std::shared_ptr<overlay_frame> save_frame)
{
	{
		std::lock_guard<std::mutex> lock(frame_access);
		frame = save_frame;

		const RECT overlay_rect = get_rect();
		void* image_array = nullptr;
		size_t image_array_size = 0;
		size_t expected_array_size = (overlay_rect.right - overlay_rect.left) * (overlay_rect.bottom - overlay_rect.top) * 4;

		frame->get_array(&image_array, &image_array_size);
		if (image_array_size != expected_array_size)
		{
			log_error << "APP: Saving image from electron array_size = " << image_array_size << ", expected = " << expected_array_size << std::endl;
			frame = nullptr;
			return false;
		} else
		{
			if (apply_image_from_buffer(image_array,image_array_size,overlay_rect.right - overlay_rect.left,overlay_rect.bottom - overlay_rect.top))
			{
				content_updated = true;
			}
			frame = nullptr;
		}
	}

	if (autohidden)
	{
		if (!IsWindowVisible(overlay_hwnd))
		{
			ShowWindow(overlay_hwnd, SW_SHOW);
		}

		if (autohide_by_transparency > 0)
		{
			set_transparency(overlay_transparency, false);
		}

		autohidden = false;
	}

	return true;
}

bool overlay_window_gdi::apply_image_from_buffer(const void* image_array, size_t array_size, int width, int height)
{
	log_debug << "APP: Saving image from electron array_size = " << array_size << ", w " << width << ", h " << height
	          << std::endl;
	bool ret = true;
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
			ret = false;
		} else
		{
			content_set = true;
		}
	} else
	{
		log_error << "APP: Saving image from electron failed. no hbmp to save to." << std::endl;
	}

	return ret;
}

bool overlay_window_direct2d::apply_image_from_buffer(const void* image_array, size_t array_size, int width, int height)
{
	log_debug << "APP: Saving image from electron array_size = " << array_size << ", w " << width << ", h " << height
	          << std::endl;
	bool ret = true;

	if (m_pBitmap != nullptr)
	{
		D2D1_RECT_U bits_size = {0, 0, width, height};
		m_pBitmap->CopyFromMemory(&bits_size, image_array, width * 4);
		content_set = true;
	}

	return ret;
}

void overlay_window_direct2d::create_render_target(ID2D1Factory* m_pDirect2dFactory)
{
	if (!m_pRenderTarget)
	{
		HRESULT hr = S_OK;
		RECT rc;
		GetClientRect(overlay_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// Create a Direct2D render target.
		hr = m_pDirect2dFactory->CreateHwndRenderTarget( D2D1::RenderTargetProperties( D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)), D2D1::HwndRenderTargetProperties(overlay_hwnd, size), &m_pRenderTarget);
	}
}

std::string overlay_window_direct2d::get_status()
{
	if (!overlay_hwnd)
	{
		return "no_overlay_window";
	} else if (!m_pRenderTarget || !m_pBitmap)
	{
		return "failed_drawing";
	}
	return "ok";
}

std::string overlay_window_gdi::get_status()
{
	if (!overlay_hwnd)
	{
		return "no_overlay_window";
	} else if (!hdc)
	{
		return "failed_drawing";
	}
	return "ok";
}

void overlay_window_gdi::set_dbl_buffering(bool enable)
{
	g_bDblBuffered = enable;
}

void overlay_window_gdi::paint_to_window(HDC window_hdc)
{
	const RECT overlay_rect = get_rect();
	BOOL ret = true;
	PAINTSTRUCT ps;
	HPAINTBUFFER hBufferedPaint = nullptr;
	RECT rc;

	window_hdc = hdc;

	GetClientRect(overlay_hwnd, &rc);
	HDC display_hdc = BeginPaint(overlay_hwnd, &ps);

	if (g_bDblBuffered)
	{
		HDC hdcMem;
		hBufferedPaint = BeginBufferedPaint(display_hdc, &rc, BPBF_COMPOSITED, nullptr, &hdcMem);
		if (hBufferedPaint)
		{
			display_hdc = hdcMem;
		}
	}

	ret = BitBlt(
	    window_hdc,
	    0,
	    0,
	    overlay_rect.right - overlay_rect.left,
	    overlay_rect.bottom - overlay_rect.top,
	    display_hdc,
	    0,
	    0,
	    SRCCOPY);

	if (!ret)
	{
		log_error << "APP: paint_to_window had issue " << GetLastError() << std::endl;
	}

	if (hBufferedPaint)
	{
		BufferedPaintMakeOpaque(hBufferedPaint, nullptr);
		EndBufferedPaint(hBufferedPaint, TRUE);
	}

	EndPaint(overlay_hwnd, &ps);

	ValidateRect(overlay_hwnd, &rect);

	last_content_chage_ticks = GetTickCount64();

	content_updated = false;
}

void overlay_window_direct2d::paint_to_window(HDC window_hdc)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(overlay_hwnd, &ps);

	if (m_pRenderTarget)
	{
		HRESULT hr = S_OK;

		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		// Draw a grid background.
		int width = static_cast<int>(rtSize.width);
		int height = static_cast<int>(rtSize.height);
		
		if (m_pBitmap != nullptr && content_set )
		{
			m_pRenderTarget->DrawBitmap( m_pBitmap, D2D1::RectF(0, 0, width, height), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, nullptr);
		} else
		{
			m_pRenderTarget->Clear(D2D1::ColorF(0.0f, 0.0f));
		}

		hr = m_pRenderTarget->EndDraw();
	} 

	EndPaint(overlay_hwnd, &ps);

	ValidateRect(overlay_hwnd, &rect);

	last_content_chage_ticks = GetTickCount64();

	content_updated = false;
}

bool overlay_window::apply_size_from_orig()
{
	BOOL ret = false;

	ret = GetWindowRect(orig_handle, &rect);

	return ret;
}

bool overlay_window::create_window()
{
	if (overlay_hwnd == nullptr && ready_to_create_overlay())
	{
		DWORD const dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN; // no border or title bar
		DWORD const dwStyleEx = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | 0x00000800;
		//| 0x40000000
		//| 0x80000000
		//| 0x20000000;
		// transparent, topmost, with no taskbar

		overlay_hwnd =
		    CreateWindowEx(dwStyleEx, g_szWindowClass, NULL, dwStyle, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);

		if (overlay_hwnd)
		{
			if (app_settings->use_color_key)
			{
				SetLayeredWindowAttributes(overlay_hwnd, RGB(0xFF, 0xFF, 0xFF), 0xD0, LWA_COLORKEY);
			} else
			{
				set_transparency(app_settings->transparency);
			}

			SetWindowPos(
			    overlay_hwnd, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOREDRAW);
			return true;
		}
	}
	return false;
}

bool overlay_window_gdi::create_window_content_buffer()
{
	bool created = false;
	RECT client_rect = {0};
	GetWindowRect(overlay_hwnd, &client_rect);

	HDC hdcScreen = GetDC(overlay_hwnd);

	if (hdcScreen != nullptr)
	{
		int new_x = client_rect.left;
		int new_y = client_rect.top;
		int new_width = client_rect.right - client_rect.left;
		int new_height = client_rect.bottom - client_rect.top;

		log_cout << "APP: create_window_content_buffer  rect at [" << new_x << " , " << new_y << "]" << std::endl;

		HDC new_hdc = nullptr;
		HBITMAP new_hbmp = nullptr;

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
		log_error << "APP: create_window_content_buffer failed to get rect from orig window " << GetLastError() << std::endl;
	}
	content_set = false;

	ReleaseDC(nullptr, hdcScreen);

	return created;
}
bool overlay_window_direct2d::create_window_content_buffer()
{
	bool created = false;
	RECT client_rect = {0};
	GetWindowRect(overlay_hwnd, &client_rect);

	if (m_pRenderTarget)
	{
		HRESULT hr = S_OK;
		int new_width = client_rect.right - client_rect.left;
		int new_height = client_rect.bottom - client_rect.top;
		m_pRenderTarget->Resize(D2D1::SizeU(new_width, new_height));

		log_debug << "APP: create_window_content_buffer d2d width " << new_width << ", height " << new_height << std::endl;
		if (m_pBitmap != nullptr)
		{
			m_pBitmap->Release();
			m_pBitmap = nullptr;
		}

		D2D1_SIZE_U bitmap_size;
		bitmap_size.width = new_width;
		bitmap_size.height = new_height;

		float dpi_x, dpi_y;
		m_pRenderTarget->GetDpi(&dpi_x, &dpi_y);

		hr = m_pRenderTarget->CreateBitmap(
		    bitmap_size,
		    D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), dpi_x, dpi_y),
		    &m_pBitmap);

		log_debug << "APP: create_window_content_buffer d2d hr " << hr << std::endl;
		if (SUCCEEDED(hr))
		{
			created = true;
		}
	}

	content_set = false;

	return created;
}