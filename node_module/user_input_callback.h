#pragma once

#include <mutex>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <queue>
#include <memory>

#define NAPI_EXPERIMENTAL 1
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

	napi_status set_args_and_call_callback(napi_env env, napi_value callback, napi_value* result);

	bool ready;
	bool intercept_active;
	std::mutex send_queue_mutex;
	std::queue<std::shared_ptr<wm_event_t>> to_send;

	size_t argc_to_cb;
	napi_value argv_to_cb[3];
	napi_status set_callback_args_values(napi_env env);
	
	callback_method_t();
};

extern callback_method_t user_input_callback_info;

int callback_method_func_get_args(callback_method_t* method, napi_env env);
napi_value callback_method_func_set_return(napi_env env, napi_callback_info info);
napi_value callback_method_func_fail(napi_env env, napi_callback_info info);

napi_status user_input_callback_init(callback_method_t* method, napi_env env, napi_callback_info info, const char* name);