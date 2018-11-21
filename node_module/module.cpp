//#include <node.h>
#include <node_api.h>

#include <iostream>
#include <vector>
#include "overlays.h" // NOLINT(build/include)

namespace demo
{
	napi_value Start(napi_env env, napi_callback_info args)
	{
		start_overlays_thread();
		return nullptr;
	}

	napi_value Stop(napi_env env, napi_callback_info args)
	{
		stop_overlays_thread();
		return nullptr;
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
		int count = get_overlays()->get_count();
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

		if (argc == 1) {
			char * url = new char[512];
			size_t result;
			status = napi_get_value_string_utf8(env, argv[0], url, 256, &result);
			std::cout << "APP:"
			          << "AddOverlay " << argc << ", " << url << ", " << std::string(url).size() << std::endl;
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
		if (argc == 5) {
			char *url = new char[512];
			size_t result;
			status = napi_get_value_string_utf8(env, argv[0], url, 256, &result);
			std::cout << "APP:"
			          << "AddOverlay " << argc << ", " << url << ", " << std::string(url).size() << std::endl;

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

	napi_value RemoveOverlay(napi_env env, napi_callback_info args)
	{
		napi_status status;
		size_t argc = 1;
		napi_value argv[1];
		int32_t overlay_id;
		status = napi_get_cb_info(env, args, &argc, argv, NULL, NULL);
		status = napi_get_value_int32(env, argv[0], &overlay_id);
		std::cout << "APP:"
		          << "RemoveOverlay " << overlay_id << std::endl;
		remove_overlay(overlay_id);

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
		std::cout << "APP:"
		          << "GetOverlayInfo look for " << overlay_id << std::endl;
		std::shared_ptr<captured_window> requested_overlay = get_overlays()->get_overlay_by_id(overlay_id);

		if (requested_overlay == nullptr) {
			return nullptr;
		} else {
			napi_value ret;
			status = napi_create_object(env, &ret);
			if (status != napi_ok)
				return nullptr;

			napi_value id;
			status = napi_create_int32(env, requested_overlay->id, &id);
			status = napi_set_named_property(env, ret, "id", id);

			std::string url_str = requested_overlay->get_url();
			if (url_str.size() != 0) {
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
		for (int i = 0; i < ids.size(); i++) {
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
		if (argc == 5) {
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

			function_ret = set_webview_position(id, x, y, width, height);
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

		if (argc == 2) {
			int overlay_id;
			status = napi_get_value_int32(env, argv[0], &overlay_id);

			char * url = new char[512];
			size_t result;
			status = napi_get_value_string_utf8(env, argv[1], url, 256, &result);
			std::cout << "APP:"
			          << "AddOverlay " << argc << ", " << url << ", " << std::string(url).size() << std::endl;
			overlay_id = set_webview_url(overlay_id, url);
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

		status = napi_create_function(env, nullptr, 0, GetOverlaysCount, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "get_overlays_count", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, GetOverlaysIDs, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "get_overlays_ids", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, GetOverlayInfo, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "get_overlay_info", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, ShowOverlays, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "show_overlays", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, HideOverlays, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "hide_overlays", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, AddOverlay, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "add_overlay", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, AddOverlayEx, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "add_overlay_ex", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, SetOverlayPosition, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "set_overlay_position", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, SetOverlayUrl, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "set_overlay_url", fn);
		if (status != napi_ok)
			return nullptr;

		status = napi_create_function(env, nullptr, 0, RemoveOverlay, nullptr, &fn);
		if (status != napi_ok)
			return nullptr;
		status = napi_set_named_property(env, exports, "remove_overlay", fn);
		if (status != napi_ok)
			return nullptr;
		 
		//set transparency
		return exports;
	}

	NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
} // namespace demo