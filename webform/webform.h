#ifndef __webform_H
#define __webform_H
#include <mshtml.h>
#include <exdisp.h>
#include <tchar.h>



#define WEBFORM_CLASS (_T("WebformClass"))
// Create a Webfrom control either by calling HWND hwebf=WebformCreate(hparent,id);
// or with CreateWindow(WEBFORM_CLASS,_T("initial-url"),...)
// If you specify WS_VSCROLL style then the webform will be created with scrollbars.

#define WEBFN_CLICKED      2 
// This notification is sent via WM_COMMAND to the parent window when a link
// has been clicked in the webform. Use WebformLastClick(hwebf) to obtain the url.

#define WEBFN_LOADED       3    
// This notification is sent via WM_COMMAND when you have called WebformGo(hwebf,url).
// It indicates that the page has finished loading.


#define WEBFM_READY        (WM_USER+0)
// HANDLE hf = WebformReady(hwebf);
// HANDLE hf = (HANDLE)SendMessage(hwebf,WEBFM_READY,0,0);
// Both are equivalent. Call this when you want to create a dynamic page.
// Build the dynamic page either with WriteFile(hf) or with WebformSet(hwebf,text)
// There is no need to close the handle. It will be closed automatically when
// you call WebformGo, or when the webform control closes.

#define WEBFM_SET          (WM_USER+1)
// WebformSet(hwebf,_T(text));
// SendMessage(hwebf,WEBFM_SET,0,(LPARAM)tchar);
// Both are equivalent. Call this to build the dynamic page.

// WebformGo(hwebf,_T("http://www.wischik.com/lu/Programmer"));
// WebformGo(hwebf,_T("about:blank"));
// WebformGo(hwebf,_T("c:\\temp\\file.html"));
// SetWindowText(hwebf,_T("http://www.wischik.com"));
// WebformGo(hwebf,0);
// All these are equivalent ways of navigating the browser.
// If the navigation destination is 0, as in the last one, then
// it will navigate to the dynamic page you have been building with Ready/Set.

#define WEBFM_GETLASTCLICK (WM_USER+2)
// TCHAR *url = WebformGetLastClick(hwebf);
// TCHAR *url = (TCHAR*)SendMessage(hwebf,WEBFM_GETLASTCLICK,0,0);
// Both are equivalent. This retrieves the URL that the user just clicked on.

#define WEBFM_GETITEM      (WM_USER+3)
// TCHAR *val = WebformGetItem(hwebf,_T("key"));
// TCHAR *val = (TCHAR*)SendMessage(hwebf,WEBFM_GETITEM,0,(LPARAM)key);
// Both are equivalent. If the user had clicked on a form-submission link,
// forming a get-request like mypage.html?key=val&x=y
// then call WebformGetItem(hwebf,_T("key")) to retrieve "val",
// and similarly for the other key/val pairs in the get-request

#define WEBFM_GETDOC       (WM_USER+4)
// IHTMLDocument2 *doc = WebformGetDoc(hwebf);
// IHTMLDocument2 *doc = (IHTMLDocument2*)SendMessage(hwebf,WEBFM_GETDOC,0,0);
// Both are equivalent. They retrieve the "doc" that's currently
// being displayed. While the page is being loaded, doc will return NULL.
// If you call this, you are responsible for doing doc->Release() afterwards.

#define WEBFM_GETBROWSER   (WM_USER+5)
// IWebBrowser2 *ibrowser = WebformGetBrowser(hwebf);
// IWebBrowser2 *ibrowser = (IWebBrowser2*)SendMessage(hwebf,WEBFM_GETBROWSER,0,0);
// Both are equivalent. They retrieve the "browser" object.
// This will always be valid. If you call this, you are responsible for
// doing ibrowser->Release() afterwards.

#define WEBFM_GETELEMENT   (WM_USER+6)
// IHTMLElement *elem = WebformGetElement<IHTMLElement>(hwebf,"id");
// IHTMLDivElement *div = WebformGetElement<IHTMLDivElement>(hwebf,"id");
// IHTMLElement *elem = (IHTMLElement*)SendMessage(hwebf,WEBFM_GETELEMENT,0,(LPARAM)id);
// This function obtains an html element from the document. This will
// only work once the document is loaded; before then it will return NULL.
// If you call this and get a non-NULL result then you are responsible
// for calling Release() afterwards.
// The Sendmessage form always returns the IHTMLElement which has html id="id".
// The WebformGetElement form is templated. It fetches the html element,
// then does QueryInterface to obtain the indicated interface type on it.
// If the element is of the wrong type (e.g. you try to get IHTMLDivElement
// on something that's really an IHTMLTextElement) then it will return 0.



#pragma warning(suppress:4312)
inline HWND WebformCreate(HWND hparent, UINT id) { return CreateWindowEx(0, WEBFORM_CLASS, _T("about:blank"), WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, 0, 100, 100, hparent, (HMENU)id, GetModuleHandle(0), 0); }
inline void WebformDestroy(HWND hwebf) { DestroyWindow(hwebf); }

inline HANDLE WebformReady(HWND hwebf) { return (HANDLE)SendMessage(hwebf, WEBFM_READY, 0, 0); }
inline void WebformSet(HWND hwebf, const TCHAR *buf) { SendMessage(hwebf, WEBFM_SET, 0, (LPARAM)buf); }
inline void WebformGo(HWND hwebf, const TCHAR *fn) { SetWindowText(hwebf, fn); }

inline const TCHAR* WebformLastClick(HWND hwebf) { return (TCHAR*)SendMessage(hwebf, WEBFM_GETLASTCLICK, 0, 0); }
inline const TCHAR* WebformGetItem(HWND hwebf, const TCHAR *name) { return (TCHAR*)SendMessage(hwebf, WEBFM_GETITEM, 0, (LPARAM)name); }

inline IHTMLDocument2 *WebformGetDoc(HWND hwebf) { return (IHTMLDocument2*)SendMessage(hwebf, WEBFM_GETDOC, 0, 0); }
inline IWebBrowser2 *WebformGetBrowser(HWND hwebf) { return (IWebBrowser2*)SendMessage(hwebf, WEBFM_GETBROWSER, 0, 0); }

template<class T> inline T* WebformGetElement(HWND hwebf, const TCHAR *id)
{
	IHTMLElement *e = (IHTMLElement*)SendMessage(hwebf, WEBFM_GETELEMENT, 0, (LPARAM)id); if (e == 0) return 0;
	T *r = 0; e->QueryInterface(__uuidof(T), (void**)&r); e->Release();
	return r;
}
template<> inline IHTMLElement* WebformGetElement<IHTMLElement>(HWND hwebf, const TCHAR *id)
{
	return (IHTMLElement*)SendMessage(hwebf, WEBFM_GETELEMENT, 0, (LPARAM)id);
}



#endif
