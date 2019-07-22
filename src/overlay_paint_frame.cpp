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

#include "overlay_paint_frame.h"
#include "overlay_paint_frame_js.h"
#include "overlay_logging.h"

#include <node_api.h>

overlay_frame::overlay_frame(overlay_frame_js * set_data) :data( set_data)
{
}

overlay_frame::~overlay_frame()
{
	data->clean();
	delete data;
	data = nullptr;
}

void overlay_frame::get_array( void ** array_ref, size_t * array_size)
{
	if(data != nullptr)
	{
		data->get_array(array_ref, array_size);
	}
}
