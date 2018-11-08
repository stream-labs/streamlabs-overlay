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
#include <atlstr.h>
#include <atlsync.h>

#include <list>
#include <memory>

#include <uxtheme.h>    // for dbl-buffered painting
 


enum class window_grab_method { bitblt, print, message_print};

const int HOTKEY_SHOW_OVERLAY = 1;
const int HOTKEY_HIDE_ALL = 2;
const int HOTKEY_UPDATE_OVERLAY = 3;
const int HOTKEY_QUITE = 4;
const int HOTKEY_ADD_WEB = 5;

const std::string config_file_name = "settings.cfg";

struct web_page_overlay_settings
{
	std::string url;
	int x;
	int y;
	int width;
	int height;

	void read(std::ifstream &infile);
	void write(std::ofstream & outfile);
};

struct smg_settings
{
	std::list<std::string> apps_names;
	std::list<web_page_overlay_settings> web_pages;
	
	void test_init();

	void read();
	void write();
};

extern smg_settings app_settings;