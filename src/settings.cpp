#include "settings.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>

void smg_settings::test_init()
{
	apps_names.push_back("notepad.exe");

	web_view_overlay_settings web_page;
	web_page.x      = 100;
	web_page.y      = 100;
	web_page.width  = 600;
	web_page.height = 600;
	web_page.url    = "help.html";
	web_pages.push_back(web_page);
}

void smg_settings::default_init() {}

bool smg_settings::read()
{
	std::cout << "APP:"
	          << "read settings from file " << config_file_name << ", settings ver " << settings_version << std::endl;
	std::ifstream infile(config_file_name);
	if (infile.fail()) {
		std::cout << "APP:"
		          << "read settings failed " << std::endl;
		return false;
	}

	std::string line;
	int         apps_count      = 0;
	int         web_pages_count = 0;

	std::getline(infile, line);
	std::istringstream apps_count_line(line);
	apps_count_line >> apps_count;

	for (int i = 0; i < apps_count; i++) {
		std::getline(infile, line);
		apps_names.push_back(line);
	}

	std::getline(infile, line);
	std::istringstream web_pages_count_line(line);
	web_pages_count_line >> web_pages_count;

	for (int i = 0; i < web_pages_count; i++) {
		web_view_overlay_settings web_page;
		web_page.read(infile);
		web_pages.push_back(web_page);
	}
	return true;
}

void smg_settings::write()
{
	std::cout << "APP:"
	          << "write settings to file " << config_file_name << std::endl;

	std::ofstream outfile(config_file_name);
	outfile << apps_names.size() << std::endl;
	std::for_each(apps_names.begin(), apps_names.end(), [&outfile](std::string& n) { outfile << n << std::endl; });

	outfile << web_pages.size() << std::endl;
	std::for_each(web_pages.begin(), web_pages.end(), [&outfile](web_view_overlay_settings& n) { n.write(outfile); });

	outfile.close();
}

smg_settings::smg_settings() : settings_version(0x0001)
{
	transparency   = 0xD0;
	use_color_key  = false;
	redraw_timeout = 300;
}

void web_view_overlay_settings::write(std::ofstream& outfile)
{
	outfile << x << std::endl;
	outfile << y << std::endl;
	outfile << width << std::endl;
	outfile << height << std::endl;
	outfile << url << std::endl;
}

void web_view_overlay_settings::read(std::ifstream& infile)
{
	std::string line;

	std::getline(infile, line);
	std::istringstream x_line(line);
	x_line >> x;

	std::getline(infile, line);
	std::istringstream y_line(line);
	y_line >> y;

	std::getline(infile, line);
	std::istringstream width_line(line);
	width_line >> width;

	std::getline(infile, line);
	std::istringstream height_line(line);
	height_line >> height;

	std::getline(infile, line);
	url = line;

	std::cout << "APP:"
	          << "read webview from config " << x << ", " << y << ", " << width << ", " << height << ", " << url
	          << std::endl;
}