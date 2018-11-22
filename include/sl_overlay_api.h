#pragma once
#include "stdafx.h"

class smg_overlays;

// char* params like url - functions get ownership of that pointer and clean memory when finish with it
int WINAPI start_overlays_thread();
int WINAPI stop_overlays_thread();
std::shared_ptr<smg_overlays> WINAPI get_overlays();
int WINAPI get_overlays_count();
int WINAPI show_overlays();
int WINAPI hide_overlays();
int WINAPI remove_overlay(int id);
int WINAPI add_webview(const char* url);
int WINAPI add_webview(const char* url, int x, int y, int width, int height);
bool WINAPI set_webview_position(int id, int x, int y, int width, int height);
bool WINAPI set_webview_url(int id, char* url);