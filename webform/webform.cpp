#include <windows.h>
#include <mshtmhst.h>
#include <mshtmdid.h>
#include <exdispid.h>
#include <tchar.h>
#include <list>
#include <string>
#include "webform.h"
using namespace std;
typedef basic_string<TCHAR> tstring;

#ifndef DOCHOSTUIFLAG_NO3DOUTERBORDER
#define DOCHOSTUIFLAG_NO3DOUTERBORDER 0x200000
#endif

// An HWEBF handle is actually just an (opaque) pointer to one of these.
//
struct TWebf : public IUnknown
{
	long ref;
	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv)
	{
		if (riid == IID_IUnknown) { *ppv = this; AddRef(); return S_OK; }
		if (riid == IID_IOleClientSite) { *ppv = &clientsite; AddRef(); return S_OK; }
		if (riid == IID_IOleWindow || riid == IID_IOleInPlaceSite) { *ppv = &site; AddRef(); return S_OK; }
		if (riid == IID_IOleInPlaceUIWindow || riid == IID_IOleInPlaceFrame) { *ppv = &frame; AddRef(); return S_OK; }
		if (riid == IID_IDispatch) { *ppv = &dispatch; AddRef(); return S_OK; }
		if (riid == IID_IDocHostUIHandler) { *ppv = &uihandler; AddRef(); return S_OK; }
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef() { return InterlockedIncrement(&ref); }
	ULONG STDMETHODCALLTYPE Release() { int tmp = InterlockedDecrement(&ref); if (tmp == 0) delete this; return tmp; }

	struct TOleClientSite : public IOleClientSite
	{
	public: TWebf *webf;
			// IUnknown
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) { return webf->QueryInterface(riid, ppv); }
			ULONG STDMETHODCALLTYPE AddRef() { return webf->AddRef(); }
			ULONG STDMETHODCALLTYPE Release() { return webf->Release(); }
			// IOleClientSite
			HRESULT STDMETHODCALLTYPE SaveObject() { return E_NOTIMPL; }
			HRESULT STDMETHODCALLTYPE GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk) { return E_NOTIMPL; }
			HRESULT STDMETHODCALLTYPE GetContainer(IOleContainer **ppContainer) { *ppContainer = 0; return E_NOINTERFACE; }
			HRESULT STDMETHODCALLTYPE ShowObject() { return S_OK; }
			HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL fShow) { return E_NOTIMPL; }
			HRESULT STDMETHODCALLTYPE RequestNewObjectLayout() { return E_NOTIMPL; }
	} clientsite;

	struct TOleInPlaceSite : public IOleInPlaceSite
	{
		TWebf *webf;
		// IUnknown
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) { return webf->QueryInterface(riid, ppv); }
		ULONG STDMETHODCALLTYPE AddRef() { return webf->AddRef(); }
		ULONG STDMETHODCALLTYPE Release() { return webf->Release(); }
		// IOleWindow
		HRESULT STDMETHODCALLTYPE GetWindow(HWND *phwnd) { *phwnd = webf->hhost; return S_OK; }
		HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }
		// IOleInPlaceSite
		HRESULT STDMETHODCALLTYPE CanInPlaceActivate() { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnInPlaceActivate() { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnUIActivate() { return S_OK; }
		HRESULT STDMETHODCALLTYPE GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO info)
		{
			*ppFrame = &webf->frame; webf->frame.AddRef();
			*ppDoc = 0;
			info->fMDIApp = FALSE; info->hwndFrame = webf->hhost; info->haccel = 0; info->cAccelEntries = 0;
			GetClientRect(webf->hhost, lprcPosRect);
			GetClientRect(webf->hhost, lprcClipRect);
			return(S_OK);
		}
		HRESULT STDMETHODCALLTYPE Scroll(SIZE scrollExtant) { return E_NOTIMPL; }
		HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL fUndoable) { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate() { return S_OK; }
		HRESULT STDMETHODCALLTYPE DiscardUndoState() { return E_NOTIMPL; }
		HRESULT STDMETHODCALLTYPE DeactivateAndUndo() { return E_NOTIMPL; }
		HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT lprcPosRect)
		{
			IOleInPlaceObject *iole = 0;
			webf->ibrowser->QueryInterface(IID_IOleInPlaceObject, (void**)&iole);
			if (iole != 0) { iole->SetObjectRects(lprcPosRect, lprcPosRect); iole->Release(); }
			return S_OK;
		}
	} site;

	struct TOleInPlaceFrame : public IOleInPlaceFrame
	{
		TWebf *webf;
		// IUnknown
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) { return webf->QueryInterface(riid, ppv); }
		ULONG STDMETHODCALLTYPE AddRef() { return webf->AddRef(); }
		ULONG STDMETHODCALLTYPE Release() { return webf->Release(); }
		// IOleWindow
		HRESULT STDMETHODCALLTYPE GetWindow(HWND *phwnd) { *phwnd = webf->hhost; return S_OK; }
		HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }
		// IOleInPlaceUIWindow
		HRESULT STDMETHODCALLTYPE GetBorder(LPRECT lprectBorder) { return E_NOTIMPL; }
		HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS pborderwidths) { return E_NOTIMPL; }
		HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS pborderwidths) { return E_NOTIMPL; }
		HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName) { return S_OK; }\
			// IOleInPlaceFrame
			HRESULT STDMETHODCALLTYPE InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) { return E_NOTIMPL; }
		HRESULT STDMETHODCALLTYPE SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject) { return S_OK; }
		HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU hmenuShared) { return E_NOTIMPL; }
		HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR pszStatusText) { return S_OK; }
		HRESULT STDMETHODCALLTYPE EnableModeless(BOOL fEnable) { return S_OK; }
		HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpmsg, WORD wID) { return E_NOTIMPL; }
	} frame;

	struct TDispatch : public IDispatch
	{
		TWebf *webf;
		// IUnknown
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) { return webf->QueryInterface(riid, ppv); }
		ULONG STDMETHODCALLTYPE AddRef() { return webf->AddRef(); }
		ULONG STDMETHODCALLTYPE Release() { return webf->Release(); }
		// IDispatch
		HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) { *pctinfo = 0; return S_OK; }
		HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) { return E_FAIL; }
		HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) { return E_FAIL; }
		HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *Params, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
		{
			switch (dispIdMember)
			{ // DWebBrowserEvents2
			case DISPID_BEFORENAVIGATE2: webf->BeforeNavigate2(Params->rgvarg[5].pvarVal->bstrVal, Params->rgvarg[0].pboolVal); break;
			case DISPID_DOCUMENTCOMPLETE: webf->DocumentComplete(Params->rgvarg[0].pvarVal->bstrVal); break;
				// http://msdn2.microsoft.com/en-us/library/ms671911(VS.80).aspx
			case DISPID_AMBIENT_DLCONTROL:
			{ pVarResult->vt = VT_I4;
			pVarResult->lVal = DLCTL_DLIMAGES | DLCTL_VIDEOS | DLCTL_BGSOUNDS | DLCTL_SILENT;
			} break;
			default: return DISP_E_MEMBERNOTFOUND;
			}
			return S_OK;
		}
	} dispatch;


	struct TDocHostUIHandler : public IDocHostUIHandler
	{
		TWebf *webf;
		// IUnknown
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) { return webf->QueryInterface(riid, ppv); }
		ULONG STDMETHODCALLTYPE AddRef() { return webf->AddRef(); }
		ULONG STDMETHODCALLTYPE Release() { return webf->Release(); }
		// IDocHostUIHandler
		HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved) { return S_OK; }
		HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO *pInfo) { pInfo->dwFlags = (webf->hasscrollbars ? 0 : DOCHOSTUIFLAG_SCROLL_NO) | DOCHOSTUIFLAG_NO3DOUTERBORDER; return S_OK; }
		HRESULT STDMETHODCALLTYPE ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc) { return S_OK; }
		HRESULT STDMETHODCALLTYPE HideUI() { return S_OK; }
		HRESULT STDMETHODCALLTYPE UpdateUI() { return S_OK; }
		HRESULT STDMETHODCALLTYPE EnableModeless(BOOL fEnable) { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnDocWindowActivate(BOOL fActivate) { return S_OK; }
		HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(BOOL fActivate) { return S_OK; }
		HRESULT STDMETHODCALLTYPE ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow) { return S_OK; }
		HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID) { return S_FALSE; }
		HRESULT STDMETHODCALLTYPE GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw) { return S_FALSE; }
		HRESULT STDMETHODCALLTYPE GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget) { return S_FALSE; }
		HRESULT STDMETHODCALLTYPE GetExternal(IDispatch **ppDispatch) { *ppDispatch = 0; return S_FALSE; }
		HRESULT STDMETHODCALLTYPE TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut) { *ppchURLOut = 0; return S_FALSE; }
		HRESULT STDMETHODCALLTYPE FilterDataObject(IDataObject *pDO, IDataObject **ppDORet) { *ppDORet = 0; return S_FALSE; }
	} uihandler;

	struct TDocHostShowUI : public IDocHostShowUI
	{
		TWebf *webf;
		// IUnknown
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) { return webf->QueryInterface(riid, ppv); }
		ULONG STDMETHODCALLTYPE AddRef() { return webf->AddRef(); }
		ULONG STDMETHODCALLTYPE Release() { return webf->Release(); }
		// IDocHostShowUI
		HRESULT STDMETHODCALLTYPE ShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult) { *plResult = IDCANCEL; return S_OK; }
		HRESULT STDMETHODCALLTYPE ShowHelp(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, POINT ptMouse, IDispatch *pDispatchObjectHit) { return S_OK; }
	} showui;


	TWebf(HWND hhost);
	~TWebf();
	void CloseThread();
	void Close();
	//
	HANDLE Ready();
	DWORD Set(const TCHAR *buf);
	void Go(const TCHAR *fn);
	const TCHAR *LastClick();
	const TCHAR *GetItem(const TCHAR *name);
	IHTMLDocument2 *GetDoc();
	IHTMLElement *GetElement(const TCHAR *id);
	void DeleteOldFiles();
	//
	void BeforeNavigate2(const wchar_t *url, short *cancel);
	void DocumentComplete(const wchar_t *url);
	unsigned int isnaving;    // bitmask: 4=haven't yet finished Navigate call, 2=haven't yet received DocumentComplete, 1=haven't yet received BeforeNavigate
	//
	HWND hhost;               // This is the window that hosts us
	IWebBrowser2 *ibrowser;   // Our pointer to the browser itself. Released in Close().
	DWORD cookie;             // By this cookie shall the watcher be known
	//
	bool hasscrollbars;       // This is read from WS_VSCROLL|WS_HSCROLL at WM_CREATE
	TCHAR *url;               // This was the url that the user just clicked on
	TCHAR *kurl;              // Key\0Value\0Key2\0Value2\0\0 arguments for the url just clicked on
	HANDLE hf; TCHAR fn[MAX_PATH]; // if we're in the middle of a Ready/Set/Go, then these say what we're writing to
	list<tstring> oldfiles;   // temp files that we must delete
};

TWebf::TWebf(HWND hhost)
{
	ref = 0; clientsite.webf = this; site.webf = this; frame.webf = this; dispatch.webf = this; uihandler.webf = this; showui.webf = this;
	this->hhost = hhost; hf = INVALID_HANDLE_VALUE;
	ibrowser = 0; cookie = 0; isnaving = 0; url = 0; kurl = 0;
	hasscrollbars = (GetWindowLongPtr(hhost, GWL_STYLE)&(WS_HSCROLL | WS_VSCROLL)) != 0;
	RECT rc; GetClientRect(hhost, &rc);
	//
	HRESULT hr; IOleObject* iole = 0;
	hr = CoCreateInstance(CLSID_WebBrowser, 0, CLSCTX_INPROC_SERVER, IID_IOleObject, (void**)&iole); if (iole == 0) return;
	hr = iole->SetClientSite(&clientsite); if (hr != S_OK) { iole->Release(); return; }
	hr = iole->SetHostNames(L"MyHost", L"MyDoc"); if (hr != S_OK) { iole->Release(); return; }
	hr = OleSetContainedObject(iole, TRUE); if (hr != S_OK) { iole->Release(); return; }
	hr = iole->DoVerb(OLEIVERB_SHOW, 0, &clientsite, 0, hhost, &rc); if (hr != S_OK) { iole->Release(); return; }
	bool connected = false;
	IConnectionPointContainer *cpc = 0; iole->QueryInterface(IID_IConnectionPointContainer, (void**)&cpc);
	if (cpc != 0)
	{
		IConnectionPoint *cp = 0;  cpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &cp);
		if (cp != 0) { cp->Advise(&dispatch, &cookie); cp->Release(); connected = true; }
		cpc->Release();
	}
	if (!connected) { iole->Release(); return; }
	iole->QueryInterface(IID_IWebBrowser2, (void**)&ibrowser); iole->Release();
}


void TWebf::Close()
{
	if (ibrowser != 0)
	{
		IConnectionPointContainer *cpc = 0; ibrowser->QueryInterface(IID_IConnectionPointContainer, (void**)&cpc);
		if (cpc != 0)
		{
			IConnectionPoint *cp = 0; cpc->FindConnectionPoint(DIID_DWebBrowserEvents2, &cp);
			if (cp != 0) { cp->Unadvise(cookie); cp->Release(); }
			cpc->Release();
		}
		IOleObject *iole = 0; ibrowser->QueryInterface(IID_IOleObject, (void**)&iole); ibrowser->Release(); ibrowser = 0;
		if (iole != 0) { iole->Close(OLECLOSE_NOSAVE); iole->Release(); }
	}
}

TWebf::~TWebf()
{
	DeleteOldFiles();
	if (hf != INVALID_HANDLE_VALUE) CloseHandle(hf); hf = INVALID_HANDLE_VALUE;
	if (url != 0) delete[] url;
	if (kurl != 0) delete[] kurl;
}

void TWebf::DeleteOldFiles()
{ // The timing of DeleteOldFiles is sensitive. We can't delete
  // a file that is currently being displayed. That's because if
  // it's deleted, then subsequent GET requests to it will fail to
  // populat the GET query. Therefore, we delete old files only
  // at the start of Go() when the old file will surely not be needed.
	list<tstring> newfiles;
	for (list<tstring>::const_iterator i = oldfiles.begin(); i != oldfiles.end(); i++)
	{
		tstring tfn = *i;
		BOOL res = DeleteFile(tfn.c_str());
		if (!res) newfiles.push_back(tfn);
	}
	oldfiles = newfiles;
}



void TCharToWide(const char *src, wchar_t *dst, int dst_size_in_wchars) { MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dst, dst_size_in_wchars); }
#pragma warning(suppress:4996)
void TCharToWide(const wchar_t *src, wchar_t *dst, int dst_size_in_wchars) { wcscpy(dst, src); }
void WideToTChar(const wchar_t *src, char *dst, int dst_size_in_tchars) { WideCharToMultiByte(CP_ACP, 0, src, -1, dst, dst_size_in_tchars, NULL, NULL); }
#pragma warning(suppress:4996)
void WideToTChar(const wchar_t *src, wchar_t *dst, int dst_size_in_tchars) { wcscpy(dst, src); }


HANDLE TWebf::Ready()
{
	if (hf != INVALID_HANDLE_VALUE) { OutputDebugString(_T("Webform error: asked for Ready(), but something already is.")); return hf; }
	GetTempPath(MAX_PATH, fn); TCHAR *fnp = fn + _tcslen(fn);
	for (unsigned long t = GetTickCount() / 100; hf == INVALID_HANDLE_VALUE; t++)
	{
		wsprintf(fnp, _T("wbf%lu.html"), (unsigned long)t % 100000);
		hf = CreateFile(fn, GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY, 0);
	}
	return hf;
}

DWORD TWebf::Set(const TCHAR *buf)
{
	if (hf == INVALID_HANDLE_VALUE) { OutputDebugString(_T("Webform error: tried to Set, but haven't Ready()ed")); return 0; }
	DWORD writ; WriteFile(hf, buf, (DWORD)(_tcslen(buf) * sizeof(TCHAR)), &writ, 0);
	return writ;
}

void TWebf::Go(const TCHAR *url)
{
	DeleteOldFiles();
	if (url == 0)
	{
		if (hf == INVALID_HANDLE_VALUE) { OutputDebugString(_T("Webform error: tried to Go, but haven't yet Ready/Set")); return; }
		url = fn;
	}
	if (hf != INVALID_HANDLE_VALUE) { CloseHandle(hf); hf = INVALID_HANDLE_VALUE; oldfiles.push_back(fn); }
	// Navigate to the new one and delete the old one
	wchar_t ws[MAX_PATH]; TCharToWide(url, ws, MAX_PATH);
	isnaving = 7;
	VARIANT v; v.vt = VT_I4; v.lVal = 0; //v.lVal=navNoHistory;
	ibrowser->Navigate(ws, &v, NULL, NULL, NULL);
	// nb. the events know not to bother us for currentlynav.
	// (Special case: maybe it's already loaded by the time we get here!)
	if ((isnaving & 2) == 0)
	{
		WPARAM w = (GetWindowLong(hhost, GWL_ID) & 0xFFFF) | ((WEBFN_LOADED & 0xFFFF) << 16);
		PostMessage(GetParent(hhost), WM_COMMAND, w, (LPARAM)hhost);
	}
	isnaving &= ~4;
	return;
}


void TWebf::DocumentComplete(const wchar_t *)
{
	isnaving &= ~2;
	if (isnaving & 4) return; // "4" means that we're in the middle of Go(), so the notification will be handled there
	WPARAM w = (GetWindowLong(hhost, GWL_ID) & 0xFFFF) | ((WEBFN_LOADED & 0xFFFF) << 16);
	PostMessage(GetParent(hhost), WM_COMMAND, w, (LPARAM)hhost);
}



void TWebf::BeforeNavigate2(const wchar_t *cu, short *cancel)
{
	int oldisnav = isnaving; isnaving &= ~1; if (oldisnav & 1) return; // ignore events that came from our own Go()
	*cancel = TRUE;
	//
	if (url != 0) delete[] url; if (kurl != 0) delete[] kurl;
	int len = (int)wcslen(cu);
	url = new TCHAR[len * 2];
	kurl = new TCHAR[len * 2];
	WideToTChar(cu, url, len * 2);
	// parse into kurl. From http://a.com/b.html?key=val&keyb=valb%3F+t
	// we generate "key\0val\0keyb\0valb! t\0\0"
	const TCHAR *c = url; TCHAR *d = kurl;
	while (*c != 0 && *c != '?') c++; // http://a.com/b.html?key=val&key2=val2&key3=val%3D
	if (*c == '?') c++; // key=val&key2=val2
	if (*c == 0) { *d = 0; d++; }
	while (*c != 0)
	{
		while (*c != '=' && *c != 0) { *d = *c; d++; c++; } *d = 0; d++;
		if (*c == '=') c++;
		while (*c != 0 && *c != '&')
		{
			if (*c == '%' && c[1] != 0 && c[2] != 0)
			{
				int hi = c[1]; if (hi >= '0' && hi <= '9') hi -= '0'; else if (hi >= 'A' && hi <= 'F') hi = hi + 10 - 'A'; else if (hi >= 'a' && hi <= 'f') hi = hi + 10 - 'a'; else hi = 0;
				int lo = c[2]; if (lo >= '0' && lo <= '9') lo -= '0'; else if (lo >= 'A' && lo <= 'F') lo = lo + 10 - 'A'; else if (lo >= 'a' && lo <= 'f') lo = lo + 10 - 'a'; else lo = 0;
				int i = hi * 16 + lo; *d = (TCHAR)i; if (*d == 0) *d = '%';
				d++; c += 3;
			}
			else if (*c == '+') { *d = ' '; d++; c++; }
			else { *d = *c; d++; c++; }
		}
		if (*c == '&') c++;
		*d = 0; d++;
	}
	*d = 0;
	//
	WPARAM w = (GetWindowLong(hhost, GWL_ID) & 0xFFFF) | ((WEBFN_CLICKED & 0xFFFF) << 16);
	PostMessage(GetParent(hhost), WM_COMMAND, w, (LPARAM)hhost);
}

const TCHAR *TWebf::LastClick()
{
	return url;
}

const TCHAR *TWebf::GetItem(const TCHAR *key)
{
	const TCHAR *c = kurl;
	while (*c != 0)
	{
		if (_tcscmp(c, key) == 0) return c + _tcslen(key) + 1;
		while (*c != 0) c++; c++; while (*c != 0) c++; c++;
	}
	return 0;
}

IHTMLDocument2 *TWebf::GetDoc()
{
	IDispatch *dispatch = 0; ibrowser->get_Document(&dispatch); if (dispatch == 0) return 0;
	IHTMLDocument2 *doc;  dispatch->QueryInterface(IID_IHTMLDocument2, (void**)&doc);
	dispatch->Release(); return doc;
}

IHTMLElement *TWebf::GetElement(const TCHAR *id)
{
	IHTMLElement *ret = 0; if (id == 0) return 0;
	IHTMLDocument2 *doc = GetDoc(); if (doc == 0) return 0;
	IHTMLElementCollection* doc_all;
	HRESULT hr = doc->get_all(&doc_all);      // this is like doing document.all
	if (hr == S_OK)
	{
		VARIANT vid; vid.vt = VT_BSTR;
		int len = (int)_tcslen(id);
		wchar_t *ws = new wchar_t[len + 1]; TCharToWide(id, ws, len + 1);
		vid.bstrVal = ws;
		VARIANT v0; VariantInit(&v0);
		IDispatch* disp;
		hr = doc_all->item(vid, v0, &disp);       // this is like doing document.all["messages"]
		delete[] ws;
		if (hr == S_OK && disp != 0)
		{
			hr = disp->QueryInterface(IID_IHTMLElement, (void **)&ret); // it's the caller's responsibility to release ret
			disp->Release();
		}
		doc_all->Release();
	}
	doc->Release();
	return ret;
}



LRESULT CALLBACK WebformWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CREATE)
	{
		TWebf *webf = new TWebf(hwnd); if (webf->ibrowser == 0) { delete webf; webf = 0; }
		else webf->AddRef();
#pragma warning(suppress:4244)
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)webf);
		CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
		if (cs->style & (WS_HSCROLL | WS_VSCROLL)) SetWindowLongPtr(hwnd, GWL_STYLE, cs->style & ~(WS_HSCROLL | WS_VSCROLL));
		if (cs->lpszName != 0 && cs->lpszName[0] != 0) webf->Go(cs->lpszName);
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
#pragma warning(suppress:4312)
	TWebf *webf = (TWebf*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (webf == 0) return DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_DESTROY) { webf->Close(); webf->Release(); SetWindowLongPtr(hwnd, GWLP_USERDATA, 0); }
	//
	switch (msg)
	{
	case WM_SETTEXT: webf->Go((TCHAR*)lParam); break;
	case WM_SIZE:    webf->ibrowser->put_Width(LOWORD(lParam)); webf->ibrowser->put_Height(HIWORD(lParam)); break;
	case WEBFM_GETLASTCLICK: return (LRESULT)webf->LastClick();
	case WEBFM_READY: return (LRESULT)webf->Ready();
	case WEBFM_GETITEM: return (LRESULT)webf->GetItem((TCHAR*)lParam);
	case WEBFM_GETDOC: return (LRESULT)webf->GetDoc();
	case WEBFM_GETBROWSER: {webf->ibrowser->AddRef(); return (LRESULT)webf->ibrowser; }
	case WEBFM_GETELEMENT: return (LRESULT)webf->GetElement((TCHAR*)lParam);
	case WEBFM_SET: return (LRESULT)webf->Set((TCHAR*)lParam);
	};
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


struct TWebformAutoRegister
{
	TWebformAutoRegister()
	{
		WNDCLASSEX wcex = { 0 }; wcex.cbSize = sizeof(WNDCLASSEX); wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = (WNDPROC)WebformWndProc; wcex.hInstance = GetModuleHandle(0);;
		wcex.lpszClassName = WEBFORM_CLASS; RegisterClassEx(&wcex);
	}
} WebformAutoRegister;