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

#include "overlay_paint_frame_js.h"
#include "overlay_logging.h"
#include <mutex>


overlay_frame_js::overlay_frame_js(napi_env env, napi_value array)
{
    env_ref = env;
    napi_create_reference(env, array, 1, &array_cache_ref);
}

void overlay_frame_js::get_array( void ** array_ref, size_t * array_size)
{
    napi_status status = napi_ok;
    napi_value buffer_cache = nullptr;;
    try {
        if( napi_get_reference_value(env_ref, array_cache_ref, &buffer_cache) == napi_ok)
        {
            status = napi_get_buffer_info(env_ref, buffer_cache, array_ref, array_size) ;
        }
    } catch (...) {
    }

    return;
}

void overlay_frame_js::clean()
{
    napi_delete_reference(env_ref, array_cache_ref);
}