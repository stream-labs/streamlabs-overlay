#pragma once
#include "stdafx.h"
#include <mutex>

enum class overlay_status
{
	creating,
	source_ready,
	working,
	destroing
};

class overlay_window
{
	protected:
	RECT rect;
	std::mutex rect_access;

	public:
	RECT get_rect();
	virtual bool set_rect(RECT& new_rect);
	virtual bool apply_new_rect(RECT& new_rect);

	window_grab_method use_method;
	overlay_status status;

	bool get_window_screenshot();

	virtual bool save_state_to_settings();
	virtual std::string get_url();
	virtual void set_url(char* url)
	{
		delete[] url;
	};
	virtual bool ready_to_create_overlay();
	HWND orig_handle;
	HBITMAP hbmp;
	HDC hdc;

	HWND overlay_hwnd;

	int id;

	virtual ~overlay_window();
	overlay_window();

	virtual void clean_resources();
};

class web_view_window : public overlay_window
{
	public:
	std::string url;
	bool overlay_crated = false;

	virtual bool save_state_to_settings();
	virtual bool ready_to_create_overlay();
	virtual void clean_resources();
	virtual std::string get_url();
	virtual void set_url(char* new_url);
	virtual bool apply_new_rect(RECT& new_rect);
};

class app_window : public overlay_window
{
	public:
};
