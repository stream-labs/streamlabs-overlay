#include "sl_overlays_settings.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>

void smg_settings::default_init() {}

smg_settings::smg_settings() : settings_version(0x0002)
{
	transparency = 0xD0;
	use_color_key = false;
	redraw_timeout = 300;
}