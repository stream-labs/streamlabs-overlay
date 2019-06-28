#pragma once

struct overlay_frame_js;
struct overlay_frame_napi;

struct overlay_frame
{
	
public:
	void get_array( void ** array_ref, size_t * array_size);
	
	overlay_frame(overlay_frame_js * set_data);
	overlay_frame(overlay_frame_napi * set_data);
	virtual ~overlay_frame();

protected: 
	overlay_frame_js * data;
	overlay_frame_napi * data;
};
