#pragma once
 
#include "stdafx.h"
class smg_overlays;

int WINAPI start_overlays_thread();
int WINAPI stop_overlays_thread();
std::shared_ptr<smg_overlays> WINAPI get_overlays();
int WINAPI show_overlays();
int WINAPI hide_overlays();
int WINAPI add_webview(const char* url);

BOOL CALLBACK get_overlayed_windows(HWND hwnd, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
bool FindRunningProcess(const WCHAR * process_name_part);
DWORD WINAPI overlay_thread_func(void* data);
extern bool in_standalone_mode;
 
class captured_window
{
public:
	window_grab_method use_method;

	bool get_window_screenshot();
	bool update_window_screenshot();
	virtual bool check_orig_and_create_overlay() { return false; };
	virtual bool save_state_to_settings() { return false; };
	virtual std::string get_url() { return ""; };
	virtual bool is_web_view() { return false; };
	virtual bool ready_to_create_overlay() { return orig_handle != nullptr; }
	HWND orig_handle;
	HBITMAP hbmp;
	HDC hdc;
	
	HWND overlay_hwnd;

	int width;
	int height;
	
	int x; 
	int y;

	int id;

	~captured_window();
	captured_window();
	
	void clean_resources();

	
};

class web_view_window : public captured_window
{
public:
	std::string url;
	HWND web_view_hwnd = 0;
	HWND container_hwnd = 0;
	bool overlay_crated = false;
	int some_id;
		
	virtual bool check_orig_and_create_overlay();
	virtual bool save_state_to_settings();
	virtual std::string get_url() { return url; };
	virtual bool is_web_view() { return true; };
	virtual bool ready_to_create_overlay() { return orig_handle != nullptr && web_view_hwnd != nullptr; }
};

class app_window : public captured_window
{
public:

};

class web_view_overlay_settings;

class smg_overlays
{
	bool showing_overlays;

public:	
	std::list<std::shared_ptr<captured_window> > showing_windows;

	smg_overlays();
	~smg_overlays() {};

	void register_hotkeys();
	void create_windows_overlays();
	void create_overlay_window_class();

	int create_empty_web_view_window( web_view_overlay_settings& n);
	std::shared_ptr<web_view_window> get_web_view_by_container(HWND container);

	void hide_overlays();
	void create_windows_for_apps();

	size_t get_count();
	std::shared_ptr<captured_window> get_overlay_by_id(int overlay_id);

	std::vector<int> get_ids();

	void init();

	void process_hotkeys(MSG &msg);
	void on_update_timer();

	void deinit();

	BOOL process_found_window(HWND hwnd, LPARAM param);

	void draw_overlay_gdi(HWND & hWnd, bool g_bDblBuffered);
	
	void update_settings();
};