#pragma once
#include "stdafx.h"

class smg_overlays;

// char* params like url - functions get ownership of that pointer and clean memory when finish with it
int WINAPI start_overlays_thread();
int WINAPI stop_overlays_thread();
std::string get_thread_status_name();

int WINAPI show_overlays();
int WINAPI hide_overlays();

int WINAPI add_webview(const char* url);
int WINAPI add_webview(const char* url, int x, int y, int width, int height);
int WINAPI add_overlay_by_hwnd(const void* hwnd_array, size_t array_size);
int WINAPI set_overlay_position(int id, int x, int y, int width, int height);
int WINAPI set_webview_url(int id, char* url);
int WINAPI paint_overlay_from_buffer(int overlay_id, const void* image_array, size_t array_size, int width, int height);
int WINAPI set_overlay_transparency(int id, int transparency);
int WINAPI call_webview_roload(int id);
std::shared_ptr<smg_overlays> WINAPI get_overlays();
int WINAPI get_overlays_count();
int WINAPI remove_overlay(int id);

int WINAPI set_callback_for_user_input(int (*ptr)(int ));
int WINAPI use_callback_for_user_input();