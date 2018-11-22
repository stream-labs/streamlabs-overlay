#pragma once

#include <shared_mutex>
#include "stdafx.h"


DWORD WINAPI overlay_thread_func(void* data);
extern bool in_standalone_mode;

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
		delete [] url ;
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

class web_view_overlay_settings;

class smg_overlays
{
	static std::shared_ptr<smg_overlays> instance;
	bool quiting;

	public:
	mutable std::shared_mutex overlays_list_access;
	bool showing_overlays;

	static std::shared_ptr<smg_overlays> get_instance()
	{
		if (instance == nullptr) {
			instance = std::make_shared<smg_overlays>();
		}
		return instance;
	};

	public:
	std::list<std::shared_ptr<overlay_window>> showing_windows;

	smg_overlays();
	virtual ~smg_overlays(){};

	void register_hotkeys();
	void original_window_ready(int overlay_id, HWND orig_window);
	void create_windows_overlays();
	void create_window_for_overlay(std::shared_ptr<overlay_window>& overlay);
	void create_overlay_window_class();

	int create_web_view_window(web_view_overlay_settings& n);

	void hide_overlays();
	void create_windows_for_apps();

	size_t get_count();
	std::shared_ptr<overlay_window> get_overlay_by_id(int overlay_id);

	bool remove_overlay(std::shared_ptr<overlay_window> overlay);

	std::vector<int> get_ids();

	void init();

	void process_hotkeys(MSG& msg);
	void on_update_timer();

	void deinit();

	BOOL process_found_window(HWND hwnd, LPARAM param);

	void draw_overlay_gdi(HWND& hWnd, bool g_bDblBuffered);

	void update_settings();
};