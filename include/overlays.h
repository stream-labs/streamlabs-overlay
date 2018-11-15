#pragma once
 
#include "stdafx.h"

BOOL CALLBACK get_overlayed_windows(HWND hwnd, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool FindRunningProcess(const WCHAR * process_name_part);
DWORD WINAPI overlay_thread_func(void* data);
int WINAPI start_overlays_thread();

extern bool showing_overlays;

class captured_window
{
public:
	window_grab_method use_method;

	bool get_window_screenshot();
	bool update_window_screenshot();

	HWND orig_handle;
	HBITMAP hbmp;
	HDC hdc;
	
	HWND overlay_hwnd;

	int width;
	int height;
	
	int x; 
	int y;

	~captured_window();
	captured_window();

	void clean_resources();
};

class smg_overlays
{
	std::list<std::shared_ptr<captured_window> > showing_windows;
	bool showing_overlays;
	//HINSTANCE g_hInstance = nullptr;

public:	
	void register_hotkeys();
	void create_windows_overlays();
	void create_overlay_window_class();

	void hide_overlays();
	void get_windows_list();

	smg_overlays();

	void init();

	void process_hotkeys(MSG &msg);
	void on_update_timer();

	void deinit();

	BOOL process_found_window(HWND hwnd, LPARAM param);

	void draw_overlay_gdi(HWND & hWnd, bool g_bDblBuffered);
	
	void update_settings();
};