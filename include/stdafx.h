// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include <tchar.h>

//#include <atlbase.h>
#include <objbase.h>
//#include <atlstr.h>
//#include <atlsync.h>

#include <list>
#include <memory>
#include <vector>

#include <uxtheme.h> // for dbl-buffered painting

#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>

/*
Concepts:
overlay window - window drawing over all other windows 
source window - window from what content will be taken for overlay window 

Threads: 
All work with overlays in main thread. Node js api called in its own thread. Some communication between thread made by PostThreadMessages. 
Also have mutex to control access to overlays thread data. 
one for changes in list of overlays "overlays_list_access" and each overlay object have own mutex for data what can be accessed by other thread. 

Modes:
Node module. - control by node module api. 
*/

enum class sl_overlay_thread_state : int
{
	starting = 0x0020,
	runing = 0x0040,
	stopping = 0x0080,
	destoyed = 0x0100
};

const int COMMAND_SHOW_OVERLAYS = 1;
const int COMMAND_HIDE_OVERLAYS = 2;
const int COMMAND_QUIT = 4;
const int COMMAND_TAKE_INPUT = 7;
const int COMMAND_RELEASE_INPUT = 8;

//command for web view thread to close web view
//wParam id
#define WM_SLO_OVERLAY_CLOSE (WM_USER + 33)

//command for overlay thread to set overlay transparency
//wParam id
//lParam transparency
#define WM_SLO_OVERLAY_TRANSPARENCY (WM_USER + 35)

//command for overlay thread to set overlay transparency
//wParam id
//lParam transparency
#define WM_SLO_OVERLAY_VISIBILITY (WM_USER + 39)

//command for web view thread to set overlay position and size
//wParam id
//lParam LPRECT. have to delete it after recive
#define WM_SLO_OVERLAY_POSITION (WM_USER + 36)

//command for web view thread to update hbmp for new size 
//wParam id
//lParam LPRECT. have to delete it after recive
#define WM_SLO_OVERLAY_SIZE_CHANGED (WM_USER + 37)

//command for overlay thread to set overlay autohide timeout 
//wParam id
//lParam timeout
#define WM_SLO_OVERLAY_SET_AUTOHIDE (WM_USER + 38)

//signal for overlay thread about source window created and can be used to make overlay for it
#define WM_SLO_SOURCE_CREATED (WM_USER + 40)

//signal for overlay thread what overlay do not have window anymore and can be deleted
//wParam id
#define WM_SLO_OVERLAY_WINDOW_DESTOYED (WM_USER + 41)

//signal for overlay thread that it can create window for new overlay
#define WM_SLO_HWND_SOURCE_READY (WM_USER + 43)

//signal for overlay thread that it can create window for new overlay
#define WM_SLO_OVERLAY_COMMAND (WM_USER + 44)


bool set_dpi_awareness();

int try_to_get_dpi(HWND window_handle);