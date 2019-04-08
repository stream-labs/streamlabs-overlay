#pragma once

#include <mutex>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <queue>
#include <memory>

#include "sl_overlay_api.h"

#define NAPI_EXPERIMENTAL 
#define NAPI_VERSION 3
#include <node_api.h>

struct wm_event_t;

struct callback_method_t
{
	napi_ref js_this;

	napi_threadsafe_function threadsafe_function;
	bool initialized;
	bool completed;
	bool success;

	int result_int;

	int error;
	napi_value result;
	napi_callback set_return;
	napi_callback fail;
	napi_ref set_return_ref;
	napi_ref fail_ref;
	
	void callback_method_reset();

	napi_status set_args_and_call_callback(napi_env env, napi_value callback, napi_value* result);
	napi_status callback_method_call_tsf(bool block);
	napi_status callback_init( napi_env env, napi_callback_info info, const char* name);
	virtual void set_callback() = 0;
	int use_callback(WPARAM wParam, LPARAM lParam);

	bool ready;
	static bool set_intercept_active(bool);;
	static bool get_intercept_active();

	std::mutex send_queue_mutex;
	std::queue<std::shared_ptr<wm_event_t>> to_send;
	virtual size_t get_argc_to_cb() = 0;
	virtual napi_value * get_argv_to_cb() = 0;
	virtual napi_status set_callback_args_values(napi_env env){return napi_ok;};
	
	callback_method_t();
};

struct callback_keyboard_method_t : callback_method_t
{
	size_t argc_to_cb;
	napi_value argv_to_cb[2];
	
	virtual size_t get_argc_to_cb() {return argc_to_cb;};
	virtual napi_value * get_argv_to_cb() {return argv_to_cb;} ;

	virtual napi_status set_callback_args_values(napi_env env);
	virtual void set_callback();
};

struct callback_mouse_method_t : callback_method_t
{
	size_t argc_to_cb;
	napi_value argv_to_cb[4];

	virtual size_t get_argc_to_cb() { return argc_to_cb; };
	virtual napi_value * get_argv_to_cb() { return argv_to_cb; };

	virtual napi_status set_callback_args_values(napi_env env);
	virtual void set_callback();
};

extern callback_keyboard_method_t user_keyboard_callback_info;
extern callback_mouse_method_t user_mouse_callback_info;

napi_value keyboard_callback_return(napi_env env, napi_callback_info info);
napi_value keyboard_callback_fail(napi_env env, napi_callback_info info);
napi_value mouse_callback_return(napi_env env, napi_callback_info info);
napi_value mouse_callback_fail(napi_env env, napi_callback_info info);

int switch_input();
