#pragma once

#include <shared_mutex>
#include "stdafx.h"

DWORD WINAPI overlay_thread_func(void* data);

class overlay_window;

class smg_overlays
{
	static std::shared_ptr<smg_overlays> instance;
	bool quiting;

	void hide_overlays();
	void showup_overlays();
	void apply_interactive_mode_view();

	public:
	mutable std::shared_mutex overlays_list_access;
	bool showing_overlays;

	static std::shared_ptr<smg_overlays> get_instance();

	public:
	std::list<std::shared_ptr<overlay_window>> showing_windows;

	smg_overlays();
	virtual ~smg_overlays();
	void init();
	void deinit();

	void create_windows_overlays();
	void create_window_for_overlay(std::shared_ptr<overlay_window>& overlay);
	void create_overlay_window_class();

	int create_overlay_window_by_hwnd(HWND hwnd);

	size_t get_count();
	std::shared_ptr<overlay_window> get_overlay_by_id(int overlay_id);
	std::shared_ptr<overlay_window> get_overlay_by_window(HWND overlay_window);
	std::vector<int> get_ids();
	bool is_inside_overlay(int x , int y);

	bool remove_overlay(std::shared_ptr<overlay_window> overlay);
	bool on_window_destroy(HWND window);
	bool on_overlay_destroy(std::shared_ptr<overlay_window> overlay);

	void quit();

	//redirect user input
	bool is_intercepting = false;
	void hook_user_input();
	void unhook_user_input();

	//commands
	bool process_commands(MSG& msg);

	//events
	void on_update_timer();

	void draw_overlay_gdi(HWND& hWnd);
	void draw_overlay_direct2d(HWND& hWnd);

	BOOL g_bDblBuffered = FALSE;
	ID2D1Factory* m_pDirect2dFactory;
};