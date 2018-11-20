// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
 
#include <atlbase.h>
//#include <atlstr.h>
//#include <atlsync.h>

#include <list>
#include <vector>
#include <memory>

#include <uxtheme.h>    // for dbl-buffered painting


enum class window_grab_method { bitblt, print, message_print};

const int HOTKEY_SHOW_OVERLAYS = 1;
const int HOTKEY_HIDE_OVERLAYS = 2;
const int HOTKEY_UPDATE_OVERLAYS = 3;
const int HOTKEY_QUIT = 4;
const int HOTKEY_ADD_WEB = 5;
const int HOTKEY_CATCH_APP = 6;


#define WM_WEBVIEW_CREATE (WM_USER + 32)
//wParam id
//lParam url

#define WM_WEBVIEW_CLOSE (WM_USER + 33)
//wParam id

#define WM_WEBVIEW_SET_URL (WM_USER + 34)
//lParam url

#define WM_WEBVIEW_CREATED (WM_USER + 35)
//wParam id
//lParam hwnd

#define WM_WEBVIEW_CLOSED (WM_USER + 36)
//wParam id

#define WM_WINDOW_CREATED (WM_USER + 37)
//
//have window to what we ready to create overlay
