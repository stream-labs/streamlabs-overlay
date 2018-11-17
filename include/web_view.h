#pragma once
#include "stdafx.h"
class web_view_overlay_settings;

extern HANDLE web_views_thread;
extern DWORD web_views_thread_id;
extern HINSTANCE web_views_hInstance;

DWORD WINAPI web_views_thread_func(void* data);



