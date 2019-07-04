// stdafx.cpp : source file that includes just the standard includes

#include "stdafx.h"

int try_to_get_dpi(HWND window_handle)
{
	int get_dpi = 0;

    static HMODULE user32_dll = 0;
    if (!user32_dll)
    {
        user32_dll = LoadLibrary(L"user32.dll");
    }

    if (user32_dll)
    {
        typedef UINT(WINAPI * GetDpiForWindow_Fn)(HWND);
        GetDpiForWindow_Fn pfnGetDpiForWindow = (GetDpiForWindow_Fn)GetProcAddress(user32_dll, "GetDpiForWindow");
        if (pfnGetDpiForWindow)
        {
            get_dpi = pfnGetDpiForWindow(window_handle); 
        }
    }

    if(get_dpi == 0)
    {
        get_dpi = 96;
    }

	return get_dpi;
}

bool set_dpi_awareness()
{
	HMODULE user32_dll = LoadLibrary(L"user32.dll");
	if (user32_dll)
	{
		typedef DPI_AWARENESS_CONTEXT(WINAPI * SetThreadDpiAwarenessContext_Fn)(DPI_AWARENESS_CONTEXT);
		SetThreadDpiAwarenessContext_Fn pfnSetDPIAwareness =
		    (SetThreadDpiAwarenessContext_Fn)GetProcAddress(user32_dll, "SetThreadDpiAwarenessContext");
		if (pfnSetDPIAwareness)
		{
			pfnSetDPIAwareness(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		}
		FreeLibrary(user32_dll);
	}

	return true;
}