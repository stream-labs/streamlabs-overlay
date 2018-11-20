#pragma once
#include "stdafx.h"
 

extern HANDLE web_views_thread;
extern DWORD web_views_thread_id;
extern HINSTANCE web_views_hInstance;

DWORD WINAPI web_views_thread_func(void* data);

