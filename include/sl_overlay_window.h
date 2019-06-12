#pragma once
#include <mutex>
#include "stdafx.h"
#include "overlay_paint_frame.h"

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
	bool overlay_visibility;

	HBITMAP hbmp;
	HDC hdc;
	bool content_updated;
	std::shared_ptr<overlay_frame> frame;
	std::mutex frame_access;

	int autohide_after;
	ULONGLONG last_content_chage_ticks;
	bool autohidden;
	int autohide_by_transparency;

	public:
	RECT get_rect();
	bool set_rect(RECT& new_rect);
	bool apply_new_rect(RECT& new_rect);
	bool set_new_position(int x, int y);
	bool apply_size_from_orig();

	bool create_window_content_buffer();
	bool ready_to_create_overlay();
	bool paint_window_from_buffer(const void* image_array, size_t array_size, int width, int height);
	bool set_cached_image(std::shared_ptr<overlay_frame> save_frame);
	void paint_to_window(HDC window_hdc);
	bool is_content_updated();
	void set_transparency(int transparency, bool save_as_normal = true);
	int get_transparency();
	void set_visibility(bool visibility,bool overlays_shown);
	bool is_visible() {return overlay_visibility;};
	void apply_interactive_mode(bool is_intercepting);
	void set_autohide(int timeout, int transparency);
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
