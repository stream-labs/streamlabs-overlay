#pragma once

#include <mutex>
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
bool FindRunningProcess(const WCHAR* process_name_part);
DWORD WINAPI overlay_thread_func(void* data);
extern bool in_standalone_mode;

enum class overlay_status
{
	creating,
	source_ready,
	working,
	destroing
};

class captured_window
{
	protected:
	RECT rect;
	std::mutex rect_access;

	public:
	RECT get_rect();
	bool set_rect(RECT& new_rect);

	window_grab_method use_method;
	overlay_status status;

	bool get_window_screenshot();

	virtual bool save_state_to_settings();
	virtual std::string get_url();
	virtual bool ready_to_create_overlay();
	HWND orig_handle;
	HBITMAP hbmp;
	HDC hdc;

	HWND overlay_hwnd;

	int id;

	~captured_window();
	captured_window();

	virtual void clean_resources();
};

class web_view_window : public captured_window
{
	public:
	std::string url;
	bool overlay_crated = false;

	virtual bool save_state_to_settings();
	virtual bool ready_to_create_overlay();
	virtual void clean_resources();
	virtual std::string get_url();
};

class app_window : public captured_window
{
	public:
};

class web_view_overlay_settings;

class smg_overlays
{
	std::mutex block_access;
	bool showing_overlays;

	public:
	std::list<std::shared_ptr<captured_window>> showing_windows;

	smg_overlays();
	~smg_overlays(){};

	void register_hotkeys();
	void original_window_ready(int overlay_id, HWND orig_window);
	void create_windows_overlays();
	void create_window_for_overlay(std::shared_ptr<captured_window>& overlay);
	void create_overlay_window_class();

	int create_web_view_window(web_view_overlay_settings& n);

	void hide_overlays();
	void create_windows_for_apps();

	size_t get_count();
	std::shared_ptr<captured_window> get_overlay_by_id(int overlay_id);

	bool remove_overlay(std::shared_ptr<captured_window> overlay);

	std::vector<int> get_ids();

	void init();

	void process_hotkeys(MSG& msg);
	void on_update_timer();

	void deinit();

	BOOL process_found_window(HWND hwnd, LPARAM param);

	void draw_overlay_gdi(HWND& hWnd, bool g_bDblBuffered);

	void update_settings();
};