#pragma once

#include <shared_mutex>
#include "stdafx.h"

DWORD WINAPI overlay_thread_func(void* data);
extern bool in_standalone_mode;

class web_view_overlay_settings;
class overlay_window;

class smg_overlays
{
	static std::shared_ptr<smg_overlays> instance;
	bool quiting;

	public:
	mutable std::shared_mutex overlays_list_access;
	bool showing_overlays;

	static std::shared_ptr<smg_overlays> get_instance();

	public:
	std::list<std::shared_ptr<overlay_window>> showing_windows;

	smg_overlays();
	virtual ~smg_overlays(){};
	void deinit();

	void original_window_ready(int overlay_id, HWND orig_window);
	void create_windows_overlays();
	void create_window_for_overlay(std::shared_ptr<overlay_window>& overlay);
	void create_overlay_window_class();

	int create_web_view_window(web_view_overlay_settings& n);

	void hide_overlays();
	void create_windows_for_apps();

	size_t get_count();
	std::shared_ptr<overlay_window> get_overlay_by_id(int overlay_id);
	std::shared_ptr<overlay_window> get_overlay_by_window(HWND overlay_window);

	bool remove_overlay(std::shared_ptr<overlay_window> overlay);
	bool on_window_destroy(HWND window);

	std::vector<int> get_ids();

	void init();

	void register_hotkeys();
	void deregister_hotkeys();
	bool process_hotkeys(MSG& msg);

	void quit();

	void on_update_timer();

	BOOL process_found_window(HWND hwnd, LPARAM param);

	void draw_overlay_gdi(HWND& hWnd, bool g_bDblBuffered);

	void update_settings();
};