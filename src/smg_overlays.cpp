 
#include "stdafx.h"
#include "smg_overlays.h"

#include <new>
#include <list>
#include <memory>
#include <algorithm>

#include "web_page.h"

#include "tlhelp32.h"
#pragma comment (lib, "uxtheme.lib")

std::list<std::shared_ptr<captured_window> > showing_windows;

HINSTANCE g_hInstance = nullptr;
wchar_t const g_szWindowClass[] = L"overlays";
BOOL g_bDblBuffered = FALSE;
 
bool hweb_overlay_crated = false;

//  Entry to the app
int APIENTRY main(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	webform_hInstance = hInstance;
	webform_thread = CreateThread(nullptr, 0, web_page_thread_func, nullptr, 0, &webform_thread_id);
	if (webform_thread) {
		// Optionally do stuff, such as wait on the thread.
	}

     g_hInstance = hInstance;

    //  Mark that this process is DPI aware.
    SetProcessDPIAware();

    // Init COM and double-buffered painting
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        hr = BufferedPaintInit();
        g_bDblBuffered = SUCCEEDED(hr);
		 
        WNDCLASSEX wcex = { sizeof(wcex) };
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = WndProc;
        wcex.hInstance      = g_hInstance;
        wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszClassName  = g_szWindowClass;

        RegisterClassEx(&wcex);
				
		get_windows_list();
				
		register_hotkeys();

		create_windows_overlays();
		
		SetTimer(0, OVERLAY_UPDATE_TIMER, 300, (TIMERPROC)nullptr);

		// Main message loop
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			switch (msg.message)
			{
			case WM_HOTKEY:
			{
				switch (msg.wParam)
				{
				case HOTKEY_SHOW_OVERLAY:
				{
					std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<captured_window> &n) {
						ShowWindow(n->overlay_hwnd, SW_SHOW);
					});
				}
				break;
				case HOTKEY_HIDE_ALL:
				{
					std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<captured_window> &n) {
						ShowWindow(n->overlay_hwnd, SW_HIDE);
					});
					PostThreadMessage((DWORD)webform_thread_id, WM_HOTKEY, HOTKEY_HIDE_ALL, 0);
				}
				break;
				case HOTKEY_UPDATE_OVERLAY:
				{
					std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<captured_window> &n) {
						n->update_window_screenshot();
						InvalidateRect(n->overlay_hwnd, nullptr, TRUE);
					});
				} break;
				case HOTKEY_SHOW_WEB:
				{
					PostThreadMessage((DWORD)webform_thread_id, WM_HOTKEY, HOTKEY_SHOW_WEB, 0);
				} break;

				case HOTKEY_QUITE:
				{
					PostQuitMessage(0);
				}break;
				};
			}
			break;
			case WM_TIMER:
				std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<captured_window> &n) {
					n->update_window_screenshot();
					InvalidateRect(n->overlay_hwnd, nullptr, TRUE);
				});
				if (!hweb_overlay_crated && webform_hwnd != nullptr)
				{
					hweb_overlay_crated = true;
					
					std::shared_ptr<captured_window> found_window = std::make_shared<captured_window>();
					found_window->orig_handle = webform_hwnd;
					found_window->use_method = window_grab_method::bitblt;
					found_window->get_window_screenshot();
					showing_windows.push_back(found_window);
					create_windows_overlays();
				}
				break;
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				break;
			};
		}
        
		if (g_bDblBuffered)
            BufferedPaintUnInit();
  
		KillTimer(0, OVERLAY_UPDATE_TIMER);

        CoUninitialize();
    }

    return 0;
}

void create_windows_overlays()
{
	std::for_each(showing_windows.begin(), showing_windows.end(), [](std::shared_ptr<captured_window> &n) 
	{
		if (n->overlay_hwnd == nullptr)
		{
			DWORD const dwStyle = WS_POPUP;     // no border or title bar
			DWORD const dwStyleEx = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;   // transparent, topmost, with no taskbar item
			n->overlay_hwnd = CreateWindowEx(dwStyleEx, g_szWindowClass, NULL, dwStyle, 0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);

			if (n->overlay_hwnd)
			{
				SetLayeredWindowAttributes(n->overlay_hwnd, RGB(0xFF, 0xFF, 0xFF), 0xD0, LWA_ALPHA);
				//SetLayeredWindowAttributes(n->overlay_hwnd, RGB(0xFF, 0xFF, 0xFF), 0xD0, LWA_COLORKEY);

				SetWindowPos(n->overlay_hwnd, HWND_TOPMOST, n->x, n->y, n->width, n->height, SWP_NOREDRAW);

				ShowWindow(n->overlay_hwnd, SW_HIDE);
			}
		}
	});
}

void register_hotkeys()
{
	RegisterHotKey(NULL, HOTKEY_SHOW_OVERLAY, MOD_ALT, 0x53);  //'S'how
	RegisterHotKey(NULL, HOTKEY_HIDE_ALL, MOD_ALT, 0x48);  //'H'ide all
	RegisterHotKey(NULL, HOTKEY_SHOW_WEB, MOD_ALT, 0x57);  //show 'W'ebview
	RegisterHotKey(NULL, HOTKEY_UPDATE_OVERLAY, MOD_ALT, 0x55);  //'U'pdate
	RegisterHotKey(NULL, HOTKEY_QUITE, MOD_ALT, 0x51);  //'Q'uit
}

void get_windows_list()
{
	if (0)
	{
		EnumWindows(get_overlayed_windows, 0);
	}else {
		FindRunningProcess(L"notepad");
	}
}

bool FindRunningProcess(const WCHAR * process_name_part) 
{
	bool procRunning = false;

	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE) 
	{
		procRunning = false;
	} else {
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32)) 
		{ 
			while (true)
			{
				if (StrStrW(pe32.szExeFile, process_name_part) != nullptr)
				{
					unsigned long process_id = pe32.th32ProcessID;
					EnumWindows(get_overlayed_windows, (LPARAM)&process_id);
				}
				if (!Process32Next(hProcessSnap, &pe32))
				{ 
					break;
				}
			}

			// clean the snapshot object
			CloseHandle(hProcessSnap);
		}
	}

	return procRunning;
}

BOOL CALLBACK get_overlayed_windows(HWND hwnd, LPARAM param)
{
	char buffer[128];
	bool window_ok = false;
	if (param != NULL)
	{
		unsigned long process_id = 0;
		GetWindowThreadProcessId(hwnd, &process_id);
		if ( *((unsigned long*)param) == process_id  && (GetWindow(hwnd, GW_OWNER) == (HWND)nullptr && IsWindowVisible(hwnd) ) )//&& ))
		{
			window_ok = true;
		}
	} else {
		int written = GetWindowTextA(hwnd, buffer, 128);
		if (written && strstr(buffer, "Notepad.") != nullptr)
		{
			window_ok = true;
		}
	}
	
	if(window_ok )
	{
		std::shared_ptr<captured_window> found_window = std::make_shared<captured_window>();
		found_window->orig_handle = hwnd;
		found_window->get_window_screenshot();

		showing_windows.push_back(found_window);
	}
	return TRUE;
}

// ----------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
        case WM_CREATE:
        { 
			
        }
		break;
		case WM_SIZE:
		{ 
		} break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
		break;
        case WM_ERASEBKGND:
        {
            // Don't do any erasing here.  It's done in WM_PAINT to avoid flicker.
            return 1;
        }
		break;
        case WM_TIMER:
        {
			InvalidateRect(hWnd, NULL, TRUE);
            return 0;
        }
		break;
        case WM_PAINT:
        {
			draw_overlay_gdi(hWnd);
            return 0;
        }
		break;

		default:break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void draw_overlay_gdi(HWND &hWnd)
{
	PAINTSTRUCT     ps;
	HPAINTBUFFER    hBufferedPaint = NULL;
	RECT            rc;

	GetClientRect(hWnd, &rc);
	HDC hdc = BeginPaint(hWnd, &ps);

	if (g_bDblBuffered)
	{
		// Get doublebuffered DC
		HDC hdcMem;
		hBufferedPaint = BeginBufferedPaint(hdc, &rc, BPBF_COMPOSITED, NULL, &hdcMem);
		if (hBufferedPaint)
		{
			hdc = hdcMem;
		}
	}

	std::for_each(showing_windows.begin(), showing_windows.end(), [&hdc, &hWnd](std::shared_ptr<captured_window> &n) {
		if (hWnd == n->overlay_hwnd)
		{
			BOOL ret = BitBlt(hdc, 0, 0, n->width, n->height, n->hdc, 0, 0, SRCCOPY);
			if (!ret)
			{
				std::cout << "draw_overlay_gdi had issue " << GetLastError() << std::endl;
			}
		}
	});

	if (hBufferedPaint)
	{
		// end painting
		BufferedPaintMakeOpaque(hBufferedPaint, nullptr);
		EndBufferedPaint(hBufferedPaint, TRUE);
	}

	EndPaint(hWnd, &ps);
}

void captured_window::update_window_screenshot() 
{
	get_window_screenshot();
}

inline captured_window::~captured_window()
{
	clean_resources();
}

inline captured_window::captured_window()
{
	use_method = window_grab_method::print;
	orig_handle = nullptr;
	overlay_hwnd = nullptr;
	hdc = nullptr;
	hbmp = nullptr;
}

inline void captured_window::clean_resources()
{
	DeleteDC(hdc);
	DeleteObject(hbmp);
}

void captured_window::get_window_screenshot() 
{
	bool updated = false;
	BOOL ret = false;
	RECT client_rect = { 0 };
	HDC hdcScreen = GetDC(orig_handle);

	ret = GetWindowRect(orig_handle, &client_rect);
	if (ret && hdcScreen != nullptr)
	{
		int new_x = client_rect.left;
		int new_y = client_rect.top;
		int new_width = client_rect.right - client_rect.left;
		int new_height = client_rect.bottom - client_rect.top;
			
		HDC new_hdc = CreateCompatibleDC(hdcScreen);
		HBITMAP new_hbmp = CreateCompatibleBitmap(hdcScreen, new_width, new_height);

		if ( new_hdc == nullptr || new_hbmp == nullptr)
		{
			DeleteDC(new_hdc);
			DeleteObject(new_hbmp);
		} else {
			SelectObject(new_hdc, new_hbmp);

			switch (use_method)
			{
			case window_grab_method::bitblt:
				ret = BitBlt(new_hdc, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);
				break;
			case window_grab_method::print:
				ret = PrintWindow(orig_handle, new_hdc, 0);
				break;
			case window_grab_method::message_print:
				LRESULT msg_ret = SendMessage(orig_handle, WM_PAINT, (WPARAM)new_hdc, PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED);
				ret = (msg_ret == S_OK);
				break;
			};
			
			if (ret)
			{
				clean_resources();

				hdc = new_hdc;
				hbmp = new_hbmp;
				x = new_x;
				y = new_y;
				width = new_width;
				height = new_height;
				
				updated = true;
				MoveWindow(overlay_hwnd, x, y, width, height, FALSE);
			} else {
				DeleteDC(new_hdc);
				DeleteObject(new_hbmp);
			}
		}
	}
	if (!updated)
	{
		//std::cout << "get_window_screenshot had issue " << GetLastError() << std::endl; 
	}
	ReleaseDC(NULL, hdcScreen);
}