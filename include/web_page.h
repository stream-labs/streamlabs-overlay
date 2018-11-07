#pragma once
#include "stdafx.h"

extern HWND webform_hwnd;
extern HANDLE webform_thread;
extern DWORD webform_thread_id;
extern HINSTANCE webform_hInstance;

DWORD WINAPI web_page_thread_func(void* data);


