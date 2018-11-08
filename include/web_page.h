#pragma once
#include "stdafx.h"

struct web_forms_window
{
	std::string url;
	HWND webform_hwnd = 0;
	HWND container_hwnd = 0;
	bool hweb_overlay_crated = false;
	int some_id;
};

extern std::list<std::shared_ptr<web_forms_window>>  web_forms_windows;
extern HANDLE webform_thread;
extern DWORD webform_thread_id;
extern HINSTANCE webform_hInstance;

DWORD WINAPI web_page_thread_func(void* data);

void create_container_window(web_page_overlay_settings & n);


