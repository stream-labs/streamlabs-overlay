#pragma once

#include <shared_mutex>
#include "stdafx.h"

DWORD WINAPI overlay_thread_func(void* data);

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
	void init();
	void deinit();

	void original_window_ready(int overlay_id, HWND orig_window);
	void create_windows_overlays();
	void create_window_for_overlay(std::shared_ptr<overlay_window>& overlay);
	void create_overlay_window_class();

	int create_overlay_window_by_hwnd(HWND hwnd);
	void create_windows_for_apps();

	size_t get_count();
	std::shared_ptr<overlay_window> get_overlay_by_id(int overlay_id);
	std::shared_ptr<overlay_window> get_overlay_by_window(HWND overlay_window);
	std::vector<int> get_ids();

	bool remove_overlay(std::shared_ptr<overlay_window> overlay);
	bool on_window_destroy(HWND window);
	bool on_overlay_destroy(std::shared_ptr<overlay_window> overlay);

	void hide_overlays();
	void quit();

	//redirect user input
	HIMC our_IMC = nullptr;
	HIMC original_IMC = nullptr;
	bool is_intercepting = false;
	HWND game_hwnd = nullptr;
	void hook_user_input();
	void unhook_user_input();

	//commands
	bool process_commands(MSG& msg);

	//events
	void on_update_timer();

	BOOL process_found_window(HWND hwnd, LPARAM param);

	void draw_overlay_gdi(HWND& hWnd, bool g_bDblBuffered);

	void update_settings();
};