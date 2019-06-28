#pragma once

#include <node_api.h>

#include <napi.h>


struct overlay_frame_js
{
	napi_ref array_cache_ref;
	napi_env env_ref;

	overlay_frame_js(napi_env env, napi_value array);
	void get_array( void ** array_ref, size_t * array_size);
	void clean();
};

struct overlay_frame_napi
{
	Napi::ObjectReference array_cache_ref;
	Napi::Env env_ref;

	overlay_frame_napi(Napi::Env env, Napi::Object array);
	void get_array( void ** array_ref, size_t * array_size);
	void clean();
};
