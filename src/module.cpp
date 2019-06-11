#include "user_input_callback.h"

#include "sl_overlay_api.h" // NOLINT(build/include)
#include "sl_overlay_window.h"
#include "sl_overlays.h"

#include <iostream>
#include <vector>

#include <node_api.h>
#include "overlay_logging.h"

const napi_value failed_ret = nullptr;

napi_value Start(napi_env env, napi_callback_info args)
{
	int thread_start_status = 0;
	napi_value ret = nullptr;

	thread_start_status = start_overlays_thread();
	if (thread_start_status != 0)
	{
		if (user_keyboard_callback_info == nullptr)
		{
			user_keyboard_callback_info = new callback_keyboard_method_t();
		}

		if (user_mouse_callback_info == nullptr)
		{
			user_mouse_callback_info = new callback_mouse_method_t();
		}
	}

	if (napi_create_int32(env, thread_start_status, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value Stop(napi_env env, napi_callback_info args)
{
	if (user_keyboard_callback_info != nullptr)
	{
		delete user_keyboard_callback_info;
		user_keyboard_callback_info = nullptr;
	}
	if (user_mouse_callback_info != nullptr)
	{
		delete user_mouse_callback_info;
		user_mouse_callback_info = nullptr;
	}

	int thread_stop_status = 0;
	napi_value ret = nullptr;

	thread_stop_status = stop_overlays_thread();

	if (napi_create_int32(env, thread_stop_status, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value GetStatus(napi_env env, napi_callback_info args)
{
	std::string thread_status = get_thread_status_name();

	napi_value ret = nullptr;
	if (napi_create_string_utf8(env, thread_status.c_str(), thread_status.size(), &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value ShowOverlays(napi_env env, napi_callback_info args)
{
	show_overlays();
	return failed_ret;
}

napi_value HideOverlays(napi_env env, napi_callback_info args)
{
	hide_overlays();
	return failed_ret;
}

napi_value GetOverlaysCount(napi_env env, napi_callback_info args)
{
	int count = get_overlays_count();
	napi_value ret = nullptr;

	if (napi_create_int32(env, count, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value AddOverlayHWND(napi_env env, napi_callback_info args)
{
	napi_value ret = nullptr;

	size_t argc = 1;
	napi_value argv[1];
	if (napi_get_cb_info(env, args, &argc, argv, NULL, NULL) != napi_ok)
		return failed_ret;

	int crated_overlay_id = -1;
	if (argc == 1)
	{
		void* incoming_array = nullptr;
		size_t array_lenght = 0;
		if (napi_get_buffer_info(env, argv[0], &incoming_array, &array_lenght) != napi_ok)
			return failed_ret;

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

	if (napi_create_int32(env, crated_overlay_id, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value RemoveOverlay(napi_env env, napi_callback_info args)
{
	size_t argc = 1;
	napi_value argv[1];
	int32_t overlay_id;
	napi_value ret = nullptr;

	if (napi_get_cb_info(env, args, &argc, argv, NULL, NULL) != napi_ok)
		return failed_ret;

	if (napi_get_value_int32(env, argv[0], &overlay_id) != napi_ok)
		return failed_ret;

	log_cout << "APP: RemoveOverlay " << overlay_id << std::endl;
	remove_overlay(overlay_id);

	return ret;
}

napi_value SwitchToInteractive(napi_env env, napi_callback_info args)
{
	if (!user_keyboard_callback_info->ready || !user_mouse_callback_info->ready)
	{
		log_cout << "APP: SwitchToInteractive rejected as callbacks not set" << std::endl;
		return failed_ret;
	}

	napi_value ret = nullptr;
	size_t argc = 1;
	napi_value argv[1];
	bool switch_to;
	int switched = -1;

	if (napi_get_cb_info(env, args, &argc, argv, NULL, NULL) != napi_ok)
		return failed_ret;

	if (napi_get_value_bool(env, argv[0], &switch_to) != napi_ok)
		return failed_ret;
	
	bool check_visibility_for_switch = (!is_overlays_hidden()) && switch_to || (!switch_to);

	if (callback_method_t::get_intercept_active() != switch_to && check_visibility_for_switch)
	{
		set_callback_for_switching_input(&switch_input); // so module can switch itself off by some command

		switch_input();

		log_cout << "APP: SwitchToInteractive " << callback_method_t::get_intercept_active() << std::endl;

		switched = 1;
	}

	if (napi_create_int32(env, switched, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value SetKeyboardCallback(napi_env env, napi_callback_info args)
{
	log_cout << "APP: SetKeyboardCallback " << std::endl;
	if (user_keyboard_callback_info->ready)
	{
		user_keyboard_callback_info->ready = false;
		napi_delete_reference(env, user_keyboard_callback_info->js_this);
	} else
	{}

	size_t argc = 1;
	napi_value argv[1];
	napi_value js_this;
	napi_value js_callback;
	napi_valuetype is_function = napi_undefined;

	if (napi_get_cb_info(env, args, &argc, argv, &js_this, 0) != napi_ok)
		return failed_ret;

	//check if js side of callback is valid
	if (napi_get_prototype(env, argv[0], &js_callback) != napi_ok)
		return failed_ret;

	if (napi_typeof(env, js_callback, &is_function) != napi_ok)
		return failed_ret;

	if (is_function == napi_function)
	{
		//save reference and go to creating threadsafe function
		if (napi_create_reference(env, argv[0], 0, &user_keyboard_callback_info->js_this) != napi_ok)
			return failed_ret;

		user_keyboard_callback_info->callback_init(env, args, "func_keyboard");
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
	{}

	size_t argc = 1;
	napi_value argv[1];
	napi_value js_this;
	napi_value js_callback;
	napi_valuetype is_function = napi_undefined;

	if (napi_get_cb_info(env, args, &argc, argv, &js_this, 0) != napi_ok)
		return failed_ret;

	//check if js side of callback is valid
	if (napi_get_prototype(env, argv[0], &js_callback) != napi_ok)
		return failed_ret;

	if (napi_typeof(env, js_callback, &is_function) != napi_ok)
		return failed_ret;

	if (is_function == napi_function)
	{
		//save reference and go to creating threadsafe function
		if (napi_create_reference(env, argv[0], 0, &user_mouse_callback_info->js_this) != napi_ok)
			return failed_ret;

		user_mouse_callback_info->callback_init(env, args, "func_mouse");
	}

	return nullptr;
}

napi_value GetOverlayInfo(napi_env env, napi_callback_info args)
{
	size_t argc = 1;
	napi_value argv[1];
	int32_t overlay_id;
	if (napi_get_cb_info(env, args, &argc, argv, NULL, NULL) != napi_ok)
		return failed_ret;

	if (napi_get_value_int32(env, argv[0], &overlay_id) != napi_ok)
		return failed_ret;

	log_cout << "APP: GetOverlayInfo look for " << overlay_id << std::endl;
	std::shared_ptr<overlay_window> requested_overlay = get_overlays()->get_overlay_by_id(overlay_id);

	if (requested_overlay)
	{
		napi_value ret;
		if (napi_create_object(env, &ret) != napi_ok)
			return failed_ret;

		RECT overlay_rect = requested_overlay->get_rect();

		if (napi_create_and_set_named_property(env, ret, "id", requested_overlay->id) != napi_ok)
			return failed_ret;

		if (napi_create_and_set_named_property(env, ret, "width", overlay_rect.right - overlay_rect.left) != napi_ok)
			return failed_ret;

		if (napi_create_and_set_named_property(env, ret, "height", overlay_rect.bottom - overlay_rect.top) != napi_ok)
			return failed_ret;

		if (napi_create_and_set_named_property(env, ret, "x", overlay_rect.left) != napi_ok)
			return failed_ret;

		if (napi_create_and_set_named_property(env, ret, "y", overlay_rect.top) != napi_ok)
			return failed_ret;

		return ret;
	}

	return failed_ret;
}

napi_value GetOverlaysIDs(napi_env env, napi_callback_info args)
{
	std::vector<int> ids = get_overlays()->get_ids();
	napi_value ret = nullptr;

	if (napi_create_array(env, &ret) != napi_ok)
		return failed_ret;

	for (int i = 0; i < ids.size(); i++)
	{
		napi_value id;
		if (napi_create_int32(env, ids[i], &id) != napi_ok)
			return failed_ret;

		if (napi_set_element(env, ret, i, id) != napi_ok)
			return failed_ret;
	}

	return ret;
}

napi_value SetOverlayPosition(napi_env env, napi_callback_info args)
{
	napi_value ret = nullptr;

	size_t argc = 5;
	napi_value argv[5];
	if (napi_get_cb_info(env, args, &argc, argv, NULL, NULL) != napi_ok)
		return failed_ret;

	int position_set_result = -1;
	if (argc == 5)
	{
		int id, x, y, width, height;

		if (napi_get_value_int32(env, argv[0], &id) != napi_ok)
			return failed_ret;

		if (napi_get_value_int32(env, argv[1], &x) != napi_ok)
			return failed_ret;

		if (napi_get_value_int32(env, argv[2], &y) != napi_ok)
			return failed_ret;

		if (napi_get_value_int32(env, argv[3], &width) != napi_ok)
			return failed_ret;

		if (napi_get_value_int32(env, argv[4], &height) != napi_ok)
			return failed_ret;
		
		log_cout << "APP: SetOverlayPosition " << id << ", size " << width << "x" << height << " at [" << x << ", " << y << "] " << std::endl;

		position_set_result = set_overlay_position(id, x, y, width, height);
	}

	if (napi_create_int32(env, position_set_result, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value PaintOverlay(napi_env env, napi_callback_info args)
{
	napi_value ret = nullptr;

	size_t argc = 4;
	napi_value argv[4];
	if (napi_get_cb_info(env, args, &argc, argv, NULL, NULL) != napi_ok)
		return failed_ret;

	int painted = -1;
	if (argc == 4)
	{
		int overlay_id = -1;
		int width = 0;
		int height = 0;

		if (napi_get_value_int32(env, argv[0], &overlay_id) != napi_ok)
			return failed_ret;
		if (napi_get_value_int32(env, argv[1], &width) != napi_ok)
			return failed_ret;
		if (napi_get_value_int32(env, argv[2], &height) != napi_ok)
			return failed_ret;

		void* incoming_array = nullptr;
		size_t array_lenght = 0;
		if (napi_get_buffer_info(env, argv[3], &incoming_array, &array_lenght) != napi_ok)
			return failed_ret;

		if (incoming_array != nullptr)
		{
			log_cout << "APP: PaintOverlay " << overlay_id << ", size " << width << "x" << height << " and buffer size " << array_lenght << std::endl;

			painted = paint_overlay_from_buffer(overlay_id, incoming_array, array_lenght, width, height);
			incoming_array = nullptr;
		} else
		{
			log_cout << "APP: PaintOverlay failed to get buffer" << argc << std::endl;
		}
	}

	if (napi_create_int32(env, painted, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value SetOverlayTransparency(napi_env env, napi_callback_info args)
{
	napi_value ret = nullptr;

	size_t argc = 2;
	napi_value argv[2];

	if (napi_get_cb_info(env, args, &argc, argv, NULL, NULL) != napi_ok)
		return failed_ret;

	int set_transparency_result = -1;
	if (argc == 2)
	{
		int overlay_id = -1;
		int overlay_transparency;

		if (napi_get_value_int32(env, argv[0], &overlay_id) != napi_ok)
			return failed_ret;

		if (napi_get_value_int32(env, argv[1], &overlay_transparency) != napi_ok)
			return failed_ret;

		log_cout << "APP: SetOverlayTransparency " << overlay_transparency << std::endl;
		set_transparency_result = set_overlay_transparency(overlay_id, overlay_transparency);
	}

	if (napi_create_int32(env, set_transparency_result, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value SetOverlayAutohide(napi_env env, napi_callback_info args)
{
	napi_value ret = nullptr;

	size_t argc = 2;
	napi_value argv[2];

	if (napi_get_cb_info(env, args, &argc, argv, NULL, NULL) != napi_ok)
		return failed_ret;

	int set_autohide_result = -1;
	if (argc == 2 || argc == 3)
	{
		int overlay_id = -1;
		int overlay_autohide;

		if (napi_get_value_int32(env, argv[0], &overlay_id) != napi_ok)
			return failed_ret;

		if (napi_get_value_int32(env, argv[1], &overlay_autohide) != napi_ok)
			return failed_ret;
	
		log_cout << "APP: SetOverlayAutohide " << overlay_autohide << std::endl;
		set_autohide_result = set_overlay_autohide(overlay_id, overlay_autohide);
	}

	if (napi_create_int32(env, set_autohide_result, &ret) != napi_ok)
		return failed_ret;

	return ret;
}

napi_value init(napi_env env, napi_value exports)
{
	napi_value fn;

	if (napi_create_function(env, nullptr, 0, Start, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "start", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, Stop, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "stop", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, GetStatus, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "getStatus", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, GetOverlaysCount, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "getCount", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, GetOverlaysIDs, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "getIds", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, GetOverlayInfo, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "getInfo", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, ShowOverlays, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "show", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, HideOverlays, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "hide", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, AddOverlayHWND, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "addHWND", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, SetOverlayPosition, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "setPosition", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, PaintOverlay, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "paintOverlay", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, SetOverlayTransparency, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "setTransparency", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, SetOverlayAutohide, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "setAutohide", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, RemoveOverlay, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "remove", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, SwitchToInteractive, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "switchInteractiveMode", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, SetKeyboardCallback, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "setKeyboardCallback", fn) != napi_ok)
		return failed_ret;

	if (napi_create_function(env, nullptr, 0, SetMouseCallback, nullptr, &fn) != napi_ok)
		return failed_ret;
	if (napi_set_named_property(env, exports, "setMouseCallback", fn) != napi_ok)
		return failed_ret;

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
