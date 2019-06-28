#pragma once
#include <mutex>
#include "overlay_paint_frame.h"
#include "stdafx.h"

extern wchar_t const g_szWindowClass[];

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

	bool content_updated;
	std::shared_ptr<overlay_frame> frame;
	std::mutex frame_access;

	int autohide_after;
	ULONGLONG last_content_chage_ticks;
	bool autohidden;
	int autohide_by_transparency;

	overlay_window();

	public:
	RECT get_rect();
	bool set_rect(RECT& new_rect);
	bool apply_new_rect(RECT& new_rect);
	bool set_new_position(int x, int y);
	bool apply_size_from_orig();

	bool create_window();
	bool ready_to_create_overlay();
	bool set_cached_image(std::shared_ptr<overlay_frame> save_frame);
	virtual bool create_window_content_buffer() = 0;
	virtual bool paint_window_from_buffer(const void* image_array, size_t array_size, int width, int height) = 0;
	virtual void paint_to_window(HDC window_hdc) = 0;
	virtual void create_render_target(ID2D1Factory* m_pDirect2dFactory){};
	bool is_content_updated();
	void set_transparency(int transparency, bool save_as_normal = true);
	int get_transparency();
	void set_visibility(bool visibility, bool overlays_shown);
	bool is_visible();
	void apply_interactive_mode(bool is_intercepting);
	void set_autohide(int timeout, int transparency);
	virtual void clean_resources();

	void check_autohide();
	void reset_autohide();

	virtual ~overlay_window();

	overlay_status status;
	int id;
	HWND orig_handle;
	HWND overlay_hwnd;
};

class overlay_window_gdi : public overlay_window
{
	HBITMAP hbmp;
	HDC hdc;
	bool g_bDblBuffered = false;

	public:
	overlay_window_gdi();
	virtual void clean_resources() override;

	virtual bool paint_window_from_buffer(const void* image_array, size_t array_size, int width, int height) override;
	virtual bool create_window_content_buffer() override;
	virtual void paint_to_window(HDC window_hdc) override;
	void set_dbl_buffering(bool enable);
};

class overlay_window_direct2d : public overlay_window
{
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1Bitmap* m_pBitmap;
	ID2D1SolidColorBrush* m_pLightSlateGrayBrush;

	public:
	overlay_window_direct2d();
	virtual void clean_resources() override;

	virtual bool paint_window_from_buffer(const void* image_array, size_t array_size, int width, int height) override;
	virtual bool create_window_content_buffer() override;
	virtual void paint_to_window(HDC window_hdc) override;
	virtual void create_render_target(ID2D1Factory* m_pDirect2dFactory) override;
};