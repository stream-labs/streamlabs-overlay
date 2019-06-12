#include "overlay_paint_frame.h"
#include "overlay_paint_frame_js.h"

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
	data->get_array(array_ref, array_size);
}
