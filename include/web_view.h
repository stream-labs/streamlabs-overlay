#pragma once
#include "stdafx.h"
class web_view_overlay_settings;

struct web_view_window
{
	std::string url;
	HWND web_view_hwnd = 0;
	HWND container_hwnd = 0;
	bool overlay_crated = false;
	int some_id;
};

extern std::list<std::shared_ptr<web_view_window>>  web_view_windows;
extern HANDLE web_views_thread;
extern DWORD web_views_thread_id;
extern HINSTANCE web_views_hInstance;

DWORD WINAPI web_views_thread_func(void* data);

void create_container_window(web_view_overlay_settings & n);


