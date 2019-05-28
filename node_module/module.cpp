#include "user_input_callback.h"

#include "sl_overlay_api.h" // NOLINT(build/include)
#include "sl_overlay_window.h"
#include "sl_overlays.h"

#include <iostream>
#include <vector>

#include <node_api.h>
#include "overlay_logging.h"


napi_value Start(napi_env env, napi_callback_info args)
{
	int thread_start_status = 0;
	napi_value ret;
	napi_status status;
	
	thread_start_status = start_overlays_thread();
	if (thread_start_status != 0)
	{
		if (user_keyboard_callback_info == nullptr)
		{
			user_keyboard_callback_info = new callback_keyboard_method_t();
		}

		if( user_mouse_callback_info == nullptr)
		{
			user_mouse_callback_info = new callback_mouse_method_t();
		}
	}

	status = napi_create_int32(env, thread_start_status, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value Stop(napi_env env, napi_callback_info args)
{
	if (user_keyboard_callback_info!=nullptr)
	{
		delete user_keyboard_callback_info;
		user_keyboard_callback_info = nullptr;
	}
	if (user_mouse_callback_info!=nullptr)
	{
		delete user_mouse_callback_info;
		user_mouse_callback_info = nullptr;
	}

	int thread_stop_status = 0;
	napi_value ret;
	napi_status status;

	thread_stop_status  = stop_overlays_thread();
	status = napi_create_int32(env, thread_stop_status, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value GetStatus(napi_env env, napi_callback_info args)
{
	std::string thread_status = get_thread_status_name();
	napi_status status;

	napi_value ret;
	status = napi_create_string_utf8(env, thread_status.c_str(), thread_status.size(), &ret);

	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value ShowOverlays(napi_env env, napi_callback_info args)
{
	show_overlays();
	return nullptr;
}

napi_value HideOverlays(napi_env env, napi_callback_info args)
{
	hide_overlays();
	return nullptr;
}

napi_value GetOverlaysCount(napi_env env, napi_callback_info args)
{
	int count = get_overlays_count();
	napi_value ret;
	napi_status status;

	status = napi_create_int32(env, count, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value AddOverlay(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 1;
	napi_value argv[1];
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	int crated_overlay_id = -1;

	if (argc == 1)
	{
		char* url = new char[512];
		size_t result;
		status = napi_get_value_string_utf8(env, argv[0], url, 256, &result);
		log_cout << "APP: AddOverlay " << argc << ", " << url << ", " << std::string(url).size() << std::endl;
		crated_overlay_id = add_webview(url);
	}

	napi_value ret;
	status = napi_create_int32(env, crated_overlay_id, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value AddOverlayEx(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 5;
	napi_value argv[5];
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	int crated_overlay_id = -1;
	if (argc == 5)
	{
		char* url = new char[512];
		size_t result;
		status = napi_get_value_string_utf8(env, argv[0], url, 256, &result);
		log_cout << "APP: AddOverlayEx " << argc << ", " << url << ", " << std::string(url).size() << std::endl;

		int x;
		status = napi_get_value_int32(env, argv[1], &x);
		int y;
		status = napi_get_value_int32(env, argv[2], &y);
		int width;
		status = napi_get_value_int32(env, argv[3], &width);
		int height;
		status = napi_get_value_int32(env, argv[4], &height);

		crated_overlay_id = add_webview(url, x, y, width, height);
	}

	napi_value ret;
	status = napi_create_int32(env, crated_overlay_id, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value AddOverlayHWND(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 1;
	napi_value argv[1];
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	int crated_overlay_id = -1;
	if (argc == 1)
	{
		void* incoming_array = nullptr;
		size_t array_lenght = 0;
		status = napi_get_buffer_info(env, argv[0], &incoming_array, &array_lenght);

		if (incoming_array != nullptr)
		{
			log_cout << "APP: AddOverlayHWND " << argc << std::endl;

			crated_overlay_id = add_overlay_by_hwnd(incoming_array, array_lenght);
			incoming_array = nullptr;
		} else
		{
			log_cout << "APP: AddOverlayHWND failed to get hwnd" << argc << std::endl;
		}
	}

	napi_value ret;
	status = napi_create_int32(env, crated_overlay_id, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value RemoveOverlay(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 1;
	napi_value argv[1];
	int32_t overlay_id;

	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	status = napi_get_value_int32(env, argv[0], &overlay_id);
	
	log_cout << "APP: RemoveOverlay " << overlay_id << std::endl;
	remove_overlay(overlay_id);

	return nullptr;
}

napi_value SwitchToInteractive(napi_env env, napi_callback_info args)
{
	if (!user_keyboard_callback_info->ready || !user_mouse_callback_info->ready)
	{
		log_cout << "APP: SwitchToInteractive rejected as callbacks not set" << std::endl;
		return nullptr;
	}

	napi_status status;
	size_t argc = 1;
	napi_value argv[1];
	bool switch_to;

	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	if (status == napi_ok) {
		status = napi_get_value_bool(env, argv[0], &switch_to);
		if (status == napi_ok)
		{
			if (callback_method_t::get_intercept_active() != switch_to)
			{
				set_callback_for_switching_input(&switch_input); // so module can switch itself off by some hotkey 
	
				switch_input();

				log_cout << "APP: SwitchToInteractive " << callback_method_t::get_intercept_active() << std::endl;
			}
		}
	}
	return nullptr;
}

napi_value SetKeyboardCallback(napi_env env, napi_callback_info args)
{
	log_cout << "APP: SetKeyboardCallback " << std::endl;
	if (user_keyboard_callback_info->ready)
	{
		user_keyboard_callback_info->ready = false;
		napi_delete_reference(env, user_keyboard_callback_info->js_this);
	} else {
	}

	napi_status status;
	size_t argc = 1;
	napi_value argv[1];
	napi_value js_this;
	napi_value js_callback;
	napi_valuetype is_function = napi_undefined;

	status = napi_get_cb_info(env, args, &argc, argv, &js_this, 0);
	if (status == napi_ok)
	{
		//check if js side of callback is valid
		status = napi_get_prototype(env, argv[0], &js_callback);
		if (status == napi_ok)
		{
			status = napi_typeof(env, js_callback, &is_function);
		}
	}

	if (is_function == napi_function)
	{
		//save reference and go to creating threadsafe function
		status = napi_create_reference(env, argv[0], 0, &user_keyboard_callback_info->js_this);
		if (status == napi_ok)
		{

			status = user_keyboard_callback_info->callback_init( env, args, "func_keyboard");
		}
	}

	return nullptr;
}

napi_value SetMouseCallback(napi_env env, napi_callback_info args)
{
	log_cout << "APP: SetMouseCallback " << std::endl;
	if (user_mouse_callback_info->ready)
	{
		user_mouse_callback_info->ready = false;
		napi_delete_reference(env, user_mouse_callback_info->js_this);
	} else
	{
	}

	napi_status status;
	size_t argc = 1;
	napi_value argv[1];
	napi_value js_this;
	napi_value js_callback;
	napi_valuetype is_function = napi_undefined;

	status = napi_get_cb_info(env, args, &argc, argv, &js_this, 0);
	if (status == napi_ok)
	{
		//check if js side of callback is valid
		status = napi_get_prototype(env, argv[0], &js_callback);
		if (status == napi_ok)
		{
			status = napi_typeof(env, js_callback, &is_function);
		}
	}

	if (is_function == napi_function)
	{
		//save reference and go to creating threadsafe function
		status = napi_create_reference(env, argv[0], 0, &user_mouse_callback_info->js_this);
		if (status == napi_ok)
		{
			status = user_mouse_callback_info->callback_init(env, args, "func_mouse");
		}
	}

	return nullptr;
}

napi_value GetOverlayInfo(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 1;
	napi_value argv[1];
	int32_t overlay_id;
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	status = napi_get_value_int32(env, argv[0], &overlay_id);
	log_cout << "APP: GetOverlayInfo look for " << overlay_id << std::endl;
	std::shared_ptr<overlay_window> requested_overlay = get_overlays()->get_overlay_by_id(overlay_id);

	if (requested_overlay == nullptr)
	{
		return nullptr;
	} else
	{
		napi_value ret;
		status = napi_create_object(env, &ret);
		if (status != napi_ok)
			return nullptr;

		napi_value id;
		status = napi_create_int32(env, requested_overlay->id, &id);
		status = napi_set_named_property(env, ret, "id", id);

		std::string url_str = requested_overlay->get_url();
		if (url_str.size() != 0)
		{
			napi_value url;
			status = napi_create_string_utf8(env, url_str.c_str(), url_str.size() + 1, &url);
			status = napi_set_named_property(env, ret, "url", url);
		}
		RECT overlay_rect = requested_overlay->get_rect();

		napi_value width;
		status = napi_create_int32(env, overlay_rect.right - overlay_rect.left, &width);
		status = napi_set_named_property(env, ret, "width", width);

		napi_value height;
		status = napi_create_int32(env, overlay_rect.bottom - overlay_rect.top, &height);
		status = napi_set_named_property(env, ret, "height", width);

		napi_value x;
		status = napi_create_int32(env, overlay_rect.left, &x);
		status = napi_set_named_property(env, ret, "x", x);

		napi_value y;
		status = napi_create_int32(env, overlay_rect.top, &y);
		status = napi_set_named_property(env, ret, "y", y);

		return ret;
	}
}

napi_value GetOverlaysIDs(napi_env env, napi_callback_info args)
{
	std::vector<int> ids = get_overlays()->get_ids();
	napi_value ret;
	napi_status status;
	status = napi_create_array(env, &ret);
	for (int i = 0; i < ids.size(); i++)
	{
		napi_value id;
		napi_create_int32(env, ids[i], &id);
		napi_set_element(env, ret, i, id);
	}

	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value SetOverlayPosition(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 5;
	napi_value argv[5];
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	int function_ret = -1;
	if (argc == 5)
	{
		int id;
		status = napi_get_value_int32(env, argv[0], &id);

		int x;
		status = napi_get_value_int32(env, argv[1], &x);
		int y;
		status = napi_get_value_int32(env, argv[2], &y);
		int width;
		status = napi_get_value_int32(env, argv[3], &width);
		int height;
		status = napi_get_value_int32(env, argv[4], &height);

		function_ret = set_overlay_position(id, x, y, width, height);
	}

	napi_value ret;
	status = napi_create_int32(env, function_ret, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value SetOverlayUrl(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 2;
	napi_value argv[2];
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	int overlay_id = -1;

	if (argc == 2)
	{
		int overlay_id;
		status = napi_get_value_int32(env, argv[0], &overlay_id);

		char* url = new char[512];
		size_t result;
		status = napi_get_value_string_utf8(env, argv[1], url, 256, &result);
		log_cout << "APP: SetOverlayUrl " << url << ", " << std::string(url).size() << std::endl;
		overlay_id = set_webview_url(overlay_id, url);
	}

	napi_value ret;
	status = napi_create_int32(env, overlay_id, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value PaintOverlay(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 4;
	napi_value argv[4];
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	int overlay_id = -1;

	if (argc == 4)
	{
		int width = 0;
		int height = 0;
		status = napi_get_value_int32(env, argv[0], &overlay_id);
		status = napi_get_value_int32(env, argv[1], &width);
		status = napi_get_value_int32(env, argv[2], &height);

		void* incoming_array = nullptr;
		size_t array_lenght = 0;
		status = napi_get_buffer_info(env, argv[3], &incoming_array, &array_lenght);

		if (incoming_array != nullptr)
		{
			log_cout << "APP: PaintOverlay " << argc << ", image buffer size " << array_lenght << ", w " << width << ", h "
			          << height << std::endl;

			paint_overlay_from_buffer(overlay_id, incoming_array, array_lenght, width, height);
			incoming_array = nullptr;
		} else
		{
			log_cout << "APP: PaintOverlay failed to get buffer" << argc << std::endl;
		}
	}

	napi_value ret;
	status = napi_create_int32(env, overlay_id, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value SetOverlayTransparency(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 2;
	napi_value argv[2];
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	int overlay_id = -1;

	if (argc == 2)
	{
		int overlay_id;
		status = napi_get_value_int32(env, argv[0], &overlay_id);

		int overlay_transparency;
		status = napi_get_value_int32(env, argv[1], &overlay_transparency);
		log_cout << "APP: SetOverlayTransparency " << overlay_transparency << std::endl;
		overlay_id = set_overlay_transparency(overlay_id, overlay_transparency);
	}

	napi_value ret;
	status = napi_create_int32(env, overlay_id, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value CallOverlayReload(napi_env env, napi_callback_info args)
{
	napi_status status;
	size_t argc = 1;
	napi_value argv[1];
	status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
	int overlay_id = -1;

	if (argc == 1)
	{
		int overlay_id;
		status = napi_get_value_int32(env, argv[0], &overlay_id);
		log_cout << "APP: CallOverlayReload " << std::endl;
		overlay_id = call_webview_roload(overlay_id);
	}

	napi_value ret;
	status = napi_create_int32(env, overlay_id, &ret);
	if (status != napi_ok)
		return nullptr;
	return ret;
}

napi_value init(napi_env env, napi_value exports)
{
	napi_status status;
	napi_value fn;

	status = napi_create_function(env, nullptr, 0, Start, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "start", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, Stop, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "stop", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, GetStatus, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "getStatus", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, GetOverlaysCount, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "getCount", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, GetOverlaysIDs, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "getIds", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, GetOverlayInfo, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "getInfo", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, ShowOverlays, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "show", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, HideOverlays, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "hide", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, AddOverlay, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "add", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, AddOverlayEx, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "addEx", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, AddOverlayHWND, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "addHWND", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, SetOverlayPosition, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "setPosition", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, SetOverlayUrl, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "setUrl", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, PaintOverlay, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "paintOverlay", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, CallOverlayReload, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "reload", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, SetOverlayTransparency, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "setTransparency", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, RemoveOverlay, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "remove", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, SwitchToInteractive, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "switchInteractiveMode", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, SetKeyboardCallback, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "setKeyboardCallback", fn);
	if (status != napi_ok)
		return nullptr;

	status = napi_create_function(env, nullptr, 0, SetMouseCallback, nullptr, &fn);
	if (status != napi_ok)
		return nullptr;
	status = napi_set_named_property(env, exports, "setMouseCallback", fn);
	if (status != napi_ok)
		return nullptr;

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)

