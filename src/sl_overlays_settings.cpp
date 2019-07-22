/******************************************************************************
    Copyright (C) 2016-2019 by Streamlabs (General Workings Inc)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

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