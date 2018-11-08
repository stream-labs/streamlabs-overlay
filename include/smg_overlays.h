#pragma once
 
#include "stdafx.h"

const int OVERLAY_UPDATE_TIMER = 001;


// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void draw_overlay_gdi(HWND &hWnd);
BOOL CALLBACK get_overlayed_windows(HWND hwnd, LPARAM);
void register_hotkeys();
void create_windows_overlays();
void create_overlay_window_class();
void process_hotkeys(MSG &msg);
void get_windows_list();
bool FindRunningProcess(const WCHAR * process_name_part);

extern bool show_overlays;

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