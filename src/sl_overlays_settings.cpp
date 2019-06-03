#include "sl_overlays_settings.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>

void smg_settings::default_init() {}

smg_settings::smg_settings() : settings_version(0x0001)
{
	transparency = 0xD0;
	use_color_key = false;
	redraw_timeout = 300;
}

web_view_overlay_settings::web_view_overlay_settings() : x(0), y(0), width(0), height(0) {}
