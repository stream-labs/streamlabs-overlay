#include "user_input_callback.h"
#include <errno.h>
#include <iostream>

callback_keyboard_method_t user_keyboard_callback_info;
callback_mouse_method_t user_mouse_callback_info;

struct wm_event_t
{
	WPARAM wParam;
	LPARAM lParam;

	wm_event_t(WPARAM _wParam, LPARAM _lParam) : wParam(_wParam), lParam(_lParam) {}
};

callback_method_t::callback_method_t()
{
	ready = false;
}

void callback_method_t::callback_method_reset()
{
	initialized = false;
	completed = false;
	success = false;
	error = 0;
	result_int = 0;
}

napi_status callback_method_t::callback_method_call_tsf(bool block)
{
	std::cout << "APP: callback_method_call_tsf " << std::endl;

	initialized = true;
	completed = false;
	success = false;

	napi_status status;

	status = napi_call_threadsafe_function(threadsafe_function, 0, block ? napi_tsfn_blocking : napi_tsfn_nonblocking);
	if (status == napi_ok)
	{
		if (block)
		{
			while (!completed)
			{
			}
		}
	}

	return status;
}

napi_value callback_method_set_return_int(callback_method_t* method, napi_env env, napi_callback_info info)
{
	size_t argc = 1;
	napi_value argv[2];
	napi_value js_this;
	napi_value then;
	napi_value result;
	bool ispromise;
	std::cout << "APP: callback_method_set_return_int " << std::endl;

	if (napi_ok != napi_get_cb_info(env, info, &argc, &result, &js_this, 0))
		napi_throw_error(env, 0, "Could not get callback info");

	if (napi_ok != napi_is_promise(env, result, &ispromise))
		napi_throw_error(env, 0, "Could not check whether a promise was returned");

	if (ispromise)
	{
		argc = 2;

		if (napi_get_named_property(env, result, "then", &then))
			napi_throw_error(env, 0, "Could not get 'then' from the returned promise");
		else if (napi_ok != napi_get_reference_value(env, method->set_return_ref, &argv[0]))
			napi_throw_error(env, 0, "Could not get referenced value 'set_return'");
		else if (napi_ok != napi_get_reference_value(env, method->fail_ref, &argv[1]))
			napi_throw_error(env, 0, "Could not get referenced value 'fail'");
		else if (napi_ok != napi_call_function(env, result, then, argc, argv, &result))
			napi_throw_error(env, 0, "Could not call 'then'");
	}

	if (napi_ok != napi_get_value_int32(env, result, &method->result_int))
	{
		napi_throw_error(env, 0, "Could not get return value");
	} else
	{
		method->success = true;
		method->completed = true;
	}

	return 0;
}

napi_value callback_method_fail(callback_method_t* method, napi_env env, napi_callback_info info)
{
	std::cout << "APP: callback_method_fail " << std::endl;

	method->success = false;
	method->completed = true;

	return 0;
}

napi_status callback_method_t::set_args_and_call_callback(napi_env env, napi_value callback, napi_value* result)
{
	napi_value local_this;
	napi_status status;
	std::cout << "APP: set_args_and_call_callback" << std::endl;

	status = set_callback_args_values(env);
	if (status == napi_ok)
	{
		status = napi_get_reference_value(env, js_this, &local_this);
		if (status == napi_ok)
		{
			status = napi_call_function(env, local_this, callback, get_argc_to_cb(), get_argv_to_cb(), result);
		}
	}
	return status;
}

bool is_intercept_active = false;
bool callback_method_t::set_intercept_active(bool new_state)
{
	is_intercept_active = new_state;
	return new_state;
}

bool callback_method_t::get_intercept_active()
{
	return is_intercept_active;
}

void callback_method_threadsafe_callback(napi_env env, napi_value callback, void* in_context, void* data)
{
	callback_method_t* method;
	method = (callback_method_t*)in_context;
	napi_value then;
	napi_value result;
	bool ispromise;
	size_t argc = 2;
	napi_value global;
	napi_value argv[2];
	napi_status status;
	int ret = 0;
	std::cout << "APP: callback_method_threadsafe_callback" << std::endl;

	status = method->set_args_and_call_callback(env, callback, &result);

	if (status == napi_ok)
	{
		status = napi_is_promise(env, result, &ispromise);
	}

	if (status == napi_ok)
	{
		if (ispromise)
		{
			status = napi_get_named_property(env, result, "then", &then);
			if (status == napi_ok)
			{
				status = napi_get_reference_value(env, method->set_return_ref, &argv[0]);
				if (status == napi_ok)
				{
					status = napi_get_reference_value(env, method->fail_ref, &argv[1]);
					if (status == napi_ok)
					{
						status = napi_call_function(env, result, then, argc, argv, &result);
					}
				}
			}
		} else
		{
			status = napi_get_global(env, &global);

			if (status == napi_ok)
			{
				status = napi_get_value_int32(env, result, &ret);
				if (status == napi_ok)
				{
					if (ret != 0)
					{
						method->success = true;
						method->completed = true;
					}
					std::cout << "APP: callback_method_threadsafe_callback ret " << ret << std::endl;
				} else
				{
					//if it function then call it
					//status = napi_call_function(env, global, *argv, 1, &result, &result);
					//std::cout << "APP: callback_method_threadsafe_callback " << status << std::endl;
					//std::cout << "APP: callback_method_threadsafe_callback " << result << std::endl;
				}
			}
		}
	}
}

napi_status callback_keyboard_method_t::set_callback_args_values(napi_env env)
{
	std::cout << "APP: callback_keyboard_method_t::set_callback_args_values" << std::endl;
	napi_status status = napi_ok;

	std::shared_ptr<wm_event_t> event;

	{
		std::lock_guard<std::mutex> lock(send_queue_mutex);
		event = to_send.front();
		to_send.pop();
	}

	argc_to_cb = 3;

	bool send_key = false;

	std::string event_type = "unknown";

	switch (event->wParam)
	{
	case WM_KEYDOWN:
		event_type = "keyDown";
		send_key = true;
		break;
	case WM_KEYUP:
		event_type = "keyUp";
		send_key = true;
		break;
	case WM_CHAR:
		event_type = "char";
		send_key = true;
		break;
	default:
		break;
	};

	if (status == napi_ok)
	{
		status = napi_create_string_utf8(env, event_type.c_str(), event_type.size(), &argv_to_cb[0]);
	}

	if (send_key)
	{
		if (status == napi_ok)
		{
			LPKBDLLHOOKSTRUCT key = (LPKBDLLHOOKSTRUCT)event->lParam;
			status = napi_create_int32(env, key->vkCode, &argv_to_cb[1]);
			status = napi_create_int32(env, key->vkCode, &argv_to_cb[2]);
		}
	}


	if (!send_key)
	{
		status = napi_create_int32(env, 0, &argv_to_cb[1]);
		status = napi_create_int32(env, 0, &argv_to_cb[2]);
	}

	return status;
}

napi_status callback_mouse_method_t::set_callback_args_values(napi_env env)
{
	std::cout << "APP:  callback_mouse_method_t::set_callback_args_values" << std::endl;
	napi_status status = napi_ok;

	std::shared_ptr<wm_event_t> event;

	{
		std::lock_guard<std::mutex> lock(send_queue_mutex);
		event = to_send.front();
		to_send.pop();
	}

	argc_to_cb = 4;

	bool send_mouse = false;
	std::string event_type = "unknown";
	std::string mouse_modifiers = "";

	switch (event->wParam)
	{
	
	case WM_MOUSEMOVE:
		event_type = "mouseMove";
		send_mouse = true;
		break;
	case WM_LBUTTONDOWN:
		event_type = "mouseDown";
		mouse_modifiers = "leftButtonDown";
		send_mouse = true;
		break;
	default:
		break;
	};

	if (status == napi_ok)
	{
		status = napi_create_string_utf8(env, event_type.c_str(), event_type.size(), &argv_to_cb[0]);
	}

	if (send_mouse)
	{
		if (status == napi_ok)
		{
			status = napi_create_int32(env, 100, &argv_to_cb[1]);
		}
		if (status == napi_ok)
		{
			status = napi_create_int32(env, 100, &argv_to_cb[2]);
		}
	}

	if ( !send_mouse)
	{
		status = napi_create_int32(env, 0, &argv_to_cb[1]);
		status = napi_create_int32(env, 0, &argv_to_cb[2]);
	}

	return status;
}

napi_value keyboard_callback_return(napi_env env, napi_callback_info info)
{
	return callback_method_set_return_int(&user_keyboard_callback_info, env, info);
}

napi_value keyboard_callback_fail(napi_env env, napi_callback_info info)
{
	return callback_method_fail(&user_keyboard_callback_info, env, info);
}

napi_value mouse_callback_return(napi_env env, napi_callback_info info)
{
	return callback_method_set_return_int(&user_mouse_callback_info, env, info);
}

napi_value mouse_callback_fail(napi_env env, napi_callback_info info)
{
	return callback_method_fail(&user_mouse_callback_info, env, info);
}

static void example_finalize(napi_env env, void* data, void* hint) {}

int callback_method_t::use_callback(WPARAM wParam, LPARAM lParam)
{
	std::cout << "APP: use_callback with " << std::endl;

	int ret = -1;

	{
		std::lock_guard<std::mutex> lock(send_queue_mutex);
		to_send.push(std::make_shared<wm_event_t>(wParam, lParam));
	}
	while (to_send.size() > 0)
	{
		if (!initialized)
		{
			if (callback_method_call_tsf(false) != napi_ok)
			{
				ret = -1;
			} else
			{
				ret = 1;
			}
		}

		if (completed)
		{
			if (success)
			{
				ret = result_int;
			}

			callback_method_reset();
		}
	}

	return ret;
}

int use_callback_keyboard(WPARAM wParam, LPARAM lParam)
{
	std::cout << "APP: use_callback with " << std::endl;

	int ret = -1;

	callback_method_t* method = &user_keyboard_callback_info;
	if (method != nullptr)
	{
		ret = method->use_callback(wParam, lParam);
	}

	return ret;
}

int use_callback_mouse(WPARAM wParam, LPARAM lParam)
{
	std::cout << "APP: use_callback with " << std::endl;

	int ret = -1;

	callback_method_t* method = &user_mouse_callback_info;
	if (method != nullptr)
	{
		ret = method->use_callback(wParam, lParam);
	}

	return ret;
}

void callback_finalize(napi_env env, void* data, void* hint)
{
	std::cout << "APP: callback_finalize " << std::endl;
}

napi_status callback_method_t::callback_init(napi_env env, napi_callback_info info, const char* name)
{
	size_t argc = 1;
	napi_value argv[1];
	napi_value async_name;
	napi_value local_return;
	napi_value local_fail;
	napi_status status;

	status = napi_get_cb_info(env, info, &argc, argv, NULL, 0);
	if (status == napi_ok)
	{
		status = napi_create_function(env, "set_return", NAPI_AUTO_LENGTH, set_return, NULL, &local_return);
		if (status == napi_ok)
		{
			status = napi_create_reference(env, local_return, 0, &set_return_ref);
		}
	}

	if (status == napi_ok)
	{
		status = napi_create_function(env, "fail", NAPI_AUTO_LENGTH, fail, NULL, &local_fail);
		if (status == napi_ok)
		{
			status = napi_create_reference(env, local_fail, 0, &fail_ref);
		}
	}

	if (status == napi_ok)
	{
		status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &async_name);
	}

	if (status == napi_ok)
	{
		status = napi_create_threadsafe_function(
		    env,
		    argv[0],
		    0,
		    async_name,
		    0,
		    1,
		    0,
		    callback_finalize,
		    this,
		    callback_method_threadsafe_callback,
		    &threadsafe_function);

		std::cout << "APP: user_input_callback_method_init status = " << status << std::endl;

		if (status == napi_ok)
		{
			set_callback();
		}
	}

	return status;
}

void callback_mouse_method_t::set_callback()
{
	set_callback_for_mouse_input(&use_callback_mouse);
}

void callback_keyboard_method_t::set_callback()
{
	set_callback_for_keyboard_input(&use_callback_keyboard);
}
