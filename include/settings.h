#pragma once

#include <list>
#include <stdlib.h>
#include <string>

const std::string config_file_name = "settings.cfg";

class web_view_overlay_settings
{
	public:
	std::string url;
	int x;
	int y;
	int width;
	int height;

	void read(std::ifstream& infile);
	void write(std::ofstream& outfile);
};

class smg_settings
{
	public:
	const int settings_version;
	std::list<std::string> apps_names;
	std::list<web_view_overlay_settings> web_pages;

	int transparency; // o - 255
	bool use_color_key;
	int redraw_timeout; //ms

	void test_init();
	void default_init();

	bool read();
	void write();

	smg_settings();
};

extern smg_settings app_settings;
