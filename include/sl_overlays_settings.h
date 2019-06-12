#pragma once

#include <list>
#include <stdlib.h>
#include <string>
#include <memory>

class smg_settings
{
	public:
	const int settings_version;

	int transparency; // o - 255
	bool use_color_key;
	int redraw_timeout; //ms

	void default_init();

	smg_settings();
};

extern std::shared_ptr<smg_settings> app_settings;
