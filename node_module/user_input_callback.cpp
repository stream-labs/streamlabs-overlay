#include "user_input_callback.h"
#include <errno.h>
#include <iostream>
#include "overlay_logging.h"

callback_keyboard_method_t* user_keyboard_callback_info = nullptr;
callback_mouse_method_t* user_mouse_callback_info = nullptr;

struct wm_event_t
{
	WPARAM wParam;
	LPARAM lParam;

	wm_event_t(WPARAM _wParam, LPARAM _lParam) : wParam(_wParam), lParam(_lParam) {}
};

callback_method_t::callback_method_t()
{
	log_cout << "APP: callback_method_t()" << std::endl;
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
	log_cout << "APP: callback_method_call_tsf " << std::endl;

	initialized = true;
	completed = false;
	success = false;

	napi_status status = napi_ok;

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

napi_status callback_method_t::set_args_and_call_callback(napi_env env, napi_value callback, napi_value* result)
{
	napi_value local_this;
	napi_status status;
	log_cout << "APP: set_args_and_call_callback" << std::endl;

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

napi_status callback_keyboard_method_t::set_callback_args_values(napi_env env)
{
	log_cout << "APP: callback_keyboard_method_t::set_callback_args_values" << std::endl;
	napi_status status = napi_ok;

	std::shared_ptr<wm_event_t> event;

	{
		std::lock_guard<std::mutex> lock(send_queue_mutex);
		event = to_send.front();
		to_send.pop();
	}

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
		}
	}

	if (!send_key)
	{
		status = napi_create_int32(env, 0, &argv_to_cb[1]);
	}

	return status;
}

napi_status callback_mouse_method_t::set_callback_args_values(napi_env env)
{
	log_cout << "APP:  callback_mouse_method_t::set_callback_args_values" << std::endl;
	napi_status status = napi_ok;

	std::shared_ptr<wm_event_t> event;

	{
		std::lock_guard<std::mutex> lock(send_queue_mutex);
		event = to_send.front();
		to_send.pop();
	}

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
	case WM_LBUTTONUP:
		event_type = "mouseUp";
		mouse_modifiers = "leftButtonUp";
		send_mouse = true;
		break;
	case WM_RBUTTONDOWN:
		event_type = "mouseDown";
		mouse_modifiers = "rightButtonDown";
		send_mouse = true;
		break;
	case WM_RBUTTONUP:
		event_type = "mouseUp";
		mouse_modifiers = "rightButtonUp";
		send_mouse = true;
		break;
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		event_type = "mouseWheel";
		mouse_modifiers = "";
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
		if (status == napi_ok)
		{
			status = napi_create_string_utf8(env, mouse_modifiers.c_str(), mouse_modifiers.size(), &argv_to_cb[3]);
		}
	}

	if (!send_mouse)
	{
		status = napi_create_int32(env, 0, &argv_to_cb[1]);
		status = napi_create_int32(env, 0, &argv_to_cb[2]);
	}

	return status;
}

int callback_method_t::use_callback(WPARAM wParam, LPARAM lParam)
{
	log_cout << "APP: use_callback called" << std::endl;

	int ret = -1;

	{
		std::lock_guard<std::mutex> lock(send_queue_mutex);
		to_send.push(std::make_shared<wm_event_t>(wParam, lParam));
	}

	uv_async_send(&uv_async_this);
	
	if (to_send.size() > 256) 
	{
		log_cout << "APP: Failed to send too many events, will switch input interception off" << std::endl;
		switch_input();
	}

	return ret;
}

int switch_input()
{
	log_cout << "APP: switch_input " << std::endl;

	int ret = -1;

	callback_method_t::set_intercept_active(!callback_method_t::get_intercept_active());
	switch_overlays_user_input(callback_method_t::get_intercept_active());

	return ret;
}

int use_callback_keyboard(WPARAM wParam, LPARAM lParam)
{
	log_cout << "APP: use_callback_keyboard  " << std::endl;

	int ret = -1;

	callback_method_t* method = user_keyboard_callback_info;
	if (method != nullptr)
	{
		ret = method->use_callback(wParam, lParam);
	}

	return ret;
}

int use_callback_mouse(WPARAM wParam, LPARAM lParam)
{
	log_cout << "APP: use_callback_mouse  " << std::endl;

	int ret = -1;

	callback_method_t* method = user_mouse_callback_info;
	if (method != nullptr)
	{
		ret = method->use_callback(wParam, lParam);
	}

	return ret;
}

void callback_finalize(napi_env env, void* data, void* hint)
{
	log_cout << "APP: callback_finalize " << std::endl;
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
		status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &async_name);
	}
	
	if (status == napi_ok)
	{
		
		status = napi_async_destroy(env_this, async_context); 

		status = napi_async_init(env, argv[0], async_name, &async_context);
		uv_async_init(uv_default_loop(), &uv_async_this, &static_async_callback);
		uv_async_this.data = this;
		env_this = env;

		log_cout << "APP: user_input_callback_method_init status = " << status << std::endl;

		if (status == napi_ok)
		{
			set_callback();
			ready = true;
		}
	}

	return status;
}

void callback_method_t::static_async_callback(uv_async_t* handle)
{
	try
	{
		static_cast<callback_method_t*>(handle->data)->async_callback();
	} catch (std::exception& e)
	{
	} catch (...)
	{}
}

void callback_method_t::async_callback()
{
	napi_status status;
	napi_value js_cb;
	napi_value ret_value;
	napi_value recv;

	napi_handle_scope scope;

	status = napi_open_handle_scope(env_this, &scope);
	if (status == napi_ok)
	{
		status = napi_get_reference_value(env_this, js_this, &js_cb);
		if (status == napi_ok)
		{
			while (to_send.size() > 0)
			{
				status = set_callback_args_values(env_this);
				if (status == napi_ok)
				{
					status = napi_make_callback(env_this, async_context, recv, js_cb, get_argc_to_cb(), get_argv_to_cb(), &ret_value);
				}
			}
		}

		napi_close_handle_scope(env_this, scope);
	}

	if (status != napi_ok)
	{
		log_cout << "APP: failed async_callback to send callback with status " << status << std::endl;
		while (to_send.size() > 0)
		{
			to_send.pop();
		}
	}
}

void callback_mouse_method_t::set_callback()
{
	set_callback_for_mouse_input(&use_callback_mouse);
}

void callback_keyboard_method_t::set_callback()
{
	set_callback_for_keyboard_input(&use_callback_keyboard);
}
