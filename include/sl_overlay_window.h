#pragma once
#include <mutex>
#include "stdafx.h"

enum class overlay_status : int
{
	creating = 1,
	source_ready,
	working,
	destroing
};

class overlay_window
{
	protected:
	RECT rect;
	bool manual_position;
	std::mutex rect_access;
	int overlay_transparency;

	HBITMAP hbmp;
	HDC hdc;
	bool content_updated;

	int autohide_after;
	ULONGLONG last_content_chage_ticks;
	bool autohidden;
	bool autohide_by_transparency;

	public:
	RECT get_rect();
	bool set_rect(RECT& new_rect);
	bool apply_new_rect(RECT& new_rect);
	bool set_new_position(int x, int y);
	bool apply_size_from_orig();

	bool create_window_content_buffer();
	bool ready_to_create_overlay();
	bool paint_window_from_buffer(const void* image_array, size_t array_size, int width, int height);
	void paint_to_window(HDC window_hdc);
	bool is_content_updated();
	void set_transparency(int transparency, bool save_as_normal = true);
	void clean_resources();
	
	void check_autohide();
	void reset_autohide();

	virtual ~overlay_window();
	overlay_window();

	overlay_status status;
	int id;
	HWND orig_handle;
	HWND overlay_hwnd;
};
