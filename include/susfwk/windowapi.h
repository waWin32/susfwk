// windowapi.h
//
#ifndef _SUS_WINDOW_API_
#define _SUS_WINDOW_API_

#include "memory.h"

#define SUS_WCNAMEA "suswndclass"
#define SUS_WCNAMEW L"suswndclass"

#define SUS_DEFWNDNAMEA ""
#define SUS_DEFWNDNAMEW L""

#ifndef SUSNOTCOMMCTRL
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")

// Initialize the comctl32.lib library
SUS_INLINE VOID SUSAPI susInitCommonControls(DWORD dwIcc) {
	INITCOMMONCONTROLSEX icex = {
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC = dwIcc
	};
	InitCommonControlsEx(&icex);
}

#endif // !SUSNOTCOMMCTRL

// -------------------------------------------------

// Get the location of an object located in the center of the area
SUS_INLINE POINT SUSAPI susGetCenterPos(_In_ SIZE objSize, _In_ SIZE areaSize) {
	return (POINT) { (areaSize.cx - objSize.cx) / 2, (areaSize.cy - objSize.cy) / 2 };
}
// Getting the screen size X
SUS_INLINE INT SUSAPI susGetScreenSizeX() {
	return GetSystemMetrics(SM_CXSCREEN);
}
// Getting the screen size Y
SUS_INLINE INT SUSAPI susGetScreenSizeY() {
	return GetSystemMetrics(SM_CYSCREEN);
}
// Getting the screen size
SUS_INLINE SIZE SUSAPI susGetScreenSize() {
	return (SIZE) {
		.cx = susGetScreenSizeX(),
		.cy = susGetScreenSizeY()
	};
}

// -------------------------------------------------

// Window structure
typedef struct sus_window_structA {
	HWND					hWnd;		// Window Descriptor
	WNDCLASSEXA				wcEx;		// Window Class structure
	CREATESTRUCTA			wStruct;	// Window creation structure
} SUS_WINDOW_STRUCTA, *SUS_PWINDOW_STRUCTA, *SUS_LPWINDOW_STRUCTA;
// Window structure
typedef struct sus_window_structW {
	HWND					hWnd;		// Window Descriptor
	WNDCLASSEXW				wcEx;		// Window Class structure
	CREATESTRUCTW			wStruct;	// Window creation structure
} SUS_WINDOW_STRUCTW, *SUS_PWINDOW_STRUCTW, *SUS_LPWINDOW_STRUCTW;

#ifdef UNICODE
#define SUS_WINDOW_STRUCT	SUS_WINDOW_STRUCTW
#define SUS_PWINDOW_STRUCT	SUS_PWINDOW_STRUCTW
#define SUS_LPWINDOW_STRUCT	SUS_LPWINDOW_STRUCTW
#else // ELSE UNICODE
#define SUS_WINDOW_STRUCT	SUS_WINDOW_STRUCTA
#define SUS_PWINDOW_STRUCT	SUS_PWINDOW_STRUCTA
#define SUS_LPWINDOW_STRUCT	SUS_LPWINDOW_STRUCTA
#endif // !UNICODE

typedef SUS_WINDOW_STRUCTA SUS_WINDOWA, *SUS_PWINDOWA, *SUS_LPWINDOWA;
typedef SUS_WINDOW_STRUCTW SUS_WINDOWW, *SUS_PWINDOWW, *SUS_LPWINDOWW;
typedef SUS_WINDOW_STRUCT SUS_WINDOW, *SUS_PWINDOW, *SUS_LPWINDOW;

// Basic initialization of the window
SUS_WINDOWA SUSAPI susWindowSetupA(_In_opt_ LPCSTR lpTitle, _In_opt_ LPVOID lParam);
// Basic initialization of the window
SUS_WINDOWW SUSAPI susWindowSetupW(_In_opt_ LPCWSTR lpTitle, _In_opt_ LPVOID lParam);
// Build a window
BOOL SUSAPI susBuildWindowA(_Inout_ SUS_LPWINDOW_STRUCTA window);
// Build a window
BOOL SUSAPI susBuildWindowW(_Inout_ SUS_LPWINDOW_STRUCTW window);
// The main Window cycle
INT SUSAPI susWindowMainLoopA();
// The main Window cycle
INT SUSAPI susWindowMainLoopW();

// Clean all window data
SUS_INLINE VOID SUSAPI susWindowCleanupA(_In_ SUS_LPWINDOW_STRUCTA window) {
	if (lstrcmpA(window->wStruct.lpszClass, SUS_DEFWNDNAMEA) != 0) {
		UnregisterClassA(window->wcEx.lpszClassName, window->wcEx.hInstance);
		sus_free((LPVOID)window->wStruct.lpszClass);
	}
}
// Clean all window data
SUS_INLINE VOID SUSAPI susWindowCleanupW(_In_ SUS_LPWINDOW_STRUCTW window) {
	if (lstrcmpW(window->wStruct.lpszClass, SUS_DEFWNDNAMEW) != 0) {
		UnregisterClassW(window->wcEx.lpszClassName, window->wcEx.hInstance);
		sus_free((LPVOID)window->wStruct.lpszClass);
	}
}

#ifdef UNICODE
#define susWindowSetup		susWindowSetupW
#define susWindowCleanup	susWindowCleanupW
#define susBuildWindow		susBuildWindowW
#define susWindowMainLoop	susWindowMainLoopW
#else // ELSE UNICODE
#define susWindowSetup		susWindowSetupA
#define susWindowCleanup	susWindowCleanupA
#define susBuildWindow		susBuildWindowA
#define susWindowMainLoop	susWindowMainLoopA
#endif // !UNICODE


// =================================================================================================

// Set the window position
SUS_INLINE VOID SUSAPI susWindowSetPos(SUS_LPWINDOW window, POINT pos) {
	window->wStruct.x = pos.x;
	window->wStruct.y = pos.y;
}
// Set the window size
SUS_INLINE VOID SUSAPI susWindowSetSize(SUS_LPWINDOW window, SIZE size) {
	window->wStruct.cx = size.cx;
	window->wStruct.cy = size.cy;
}
// Set the window size and position
SUS_INLINE VOID SUSAPI susWindowSetBounds(SUS_LPWINDOW window, RECT bounds) {
	susWindowSetPos(window, (POINT) { bounds.left, bounds.top });
	susWindowSetSize(window, (SIZE) { bounds.right, bounds.bottom });
}
// Set the window frame in the center of the screen
SUS_INLINE VOID SUSAPI susWindowSetBoundsCenter(SUS_LPWINDOW window, SIZE size) {
	susWindowSetSize(window, size);
	susWindowSetPos(window, susGetCenterPos(size, susGetScreenSize()));
}
// Set styles for the window
SUS_INLINE VOID SUSAPI susWindowSetStyle(SUS_LPWINDOW window, DWORD style) {
	window->wStruct.style = style;
}
// Add styles for the window
SUS_INLINE VOID SUSAPI susAddWindowStyle(SUS_LPWINDOW window, DWORD style) {
	window->wStruct.style |= style;
}
// Remove styles for the window
SUS_INLINE VOID SUSAPI susRemoveWindowStyle(SUS_LPWINDOW window, DWORD style) {
	window->wStruct.style &= ~style;
}

// Set the extended styles for the window
SUS_INLINE VOID SUSAPI susWindowSetExStyle(SUS_LPWINDOW window, DWORD dwExStyle) {
	window->wStruct.dwExStyle = dwExStyle;
}
// Add extended styles for the window
SUS_INLINE VOID SUSAPI susAddWindowExStyle(SUS_LPWINDOW window, DWORD dwExStyle) {
	window->wStruct.dwExStyle |= dwExStyle;
}
// Remove extended styles for the window
SUS_INLINE VOID SUSAPI susRemoveWindowExStyle(SUS_LPWINDOW window, DWORD dwExStyle) {
	window->wStruct.dwExStyle &= ~dwExStyle;
}

// Set the window title
SUS_INLINE VOID SUSAPI susWindowSetTitleA(SUS_LPWINDOWA window, LPCSTR title) {
	window->wStruct.lpszName = title;
}
// Set the window title
SUS_INLINE VOID SUSAPI susWindowSetTitleW(SUS_LPWINDOWW window, LPCWSTR title) {
	window->wStruct.lpszName = title;
}

#ifdef UNICODE
#define susWindowSetTitle	susWindowSetTitleW
#else // ELSE UNICODE
#define susWindowSetTitle	susWindowSetTitleA
#endif // !UNICODE

// ===============================================

SUS_INLINE VOID susWindowSetMenu(SUS_LPWINDOW window, LPCTSTR lpszMenuName) {
	window->wcEx.lpszMenuName = lpszMenuName;
}
SUS_INLINE VOID susWindowSetHandler(SUS_LPWINDOW window, WNDPROC wndProc) {
	window->wcEx.lpfnWndProc = wndProc;
}
// Set the background color
SUS_INLINE VOID SUSAPI susWindowSetBackgroundColor(SUS_LPWINDOW window, HBRUSH color) {
	window->wcEx.hbrBackground = color;
}
// Set the window icon
SUS_INLINE VOID SUSAPI susWindowSetIcon(SUS_LPWINDOW window, HICON icon) {
	window->wcEx.hIcon = icon;
}
// Set the window cursor
SUS_INLINE VOID SUSAPI susWindowSetCursor(SUS_LPWINDOW window, HCURSOR cursor) {
	window->wcEx.hCursor = cursor;
}
// Set window visibility
SUS_INLINE VOID SUSAPI susWindowSetVisible(SUS_LPWINDOW window, BOOLEAN visible) {
	ShowWindow(window->hWnd, visible ? SW_SHOW : SW_HIDE);
}

// ===============================================

// Get a rectangle graphic
SUS_INLINE RECT SUSAPI susGetGraphicsRect(PAINTSTRUCT* gr) {
	return gr->rcPaint;
}
// Get the width of the graph
SUS_INLINE LONG SUSAPI susGetGraphicsWidth(PAINTSTRUCT* gr) {
	return gr->rcPaint.right - gr->rcPaint.left;
}
// Get the height of the graph
SUS_INLINE LONG SUSAPI susGetGraphicsHeight(PAINTSTRUCT* gr) {
	return gr->rcPaint.bottom - gr->rcPaint.top;
}

// ===============================================

// Save data to a window
SUS_INLINE SUS_OBJECT susWriteDataToWindow(HWND hWnd, LONG_PTR dwNewLong) {
	SetWindowLongPtr(hWnd, GWLP_USERDATA, dwNewLong);
	return (SUS_OBJECT)dwNewLong;
}
// Write the window's incoming data to its internal memory
SUS_INLINE SUS_OBJECT susWriteWindowContextData(HWND hWnd, LPARAM lParam) {
	CONST LPCREATESTRUCT pCreate = (CONST LPCREATESTRUCT)lParam;
	susWriteDataToWindow(hWnd, (LONG_PTR)pCreate->lpCreateParams);
	return (SUS_OBJECT)pCreate->lpCreateParams;
}
// Get a window from a window procedure
SUS_INLINE SUS_OBJECT susLoadWindowData(HWND hWnd) {
	return (SUS_OBJECT)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

// ===============================================


// Widget structure
typedef struct sus_widget_structA {
	HWND					hWnd;		// Widget Descriptor
	CREATESTRUCTA			wStruct;	// Widget creation structure
} SUS_WIDGETA, *SUS_PWIDGETA, *SUS_LPWIDGETA;
// Widget structure
typedef struct sus_widget_structW {
	HWND					hWnd;		// Widget Descriptor
	CREATESTRUCTW			wStruct;	// Widget creation structure
} SUS_WIDGETW, *SUS_PWIDGETW, *SUS_LPWIDGETW;

#ifdef UNICODE
#define SUS_WIDGET SUS_WIDGETW
#define SUS_PWIDGET SUS_PWIDGETW
#define SUS_LPWIDGET SUS_LPWIDGETW
#else
#define SUS_WIDGET SUS_WIDGETA
#define SUS_PWIDGET SUS_PWIDGETA
#define SUS_LPWIDGET SUS_LPWIDGETA
#endif // !UNICODE

typedef SUS_WIDGET SUS_WGBUTTON;
typedef SUS_WIDGETA SUS_WGBUTTONA;
typedef SUS_WIDGETW SUS_WGBUTTONW;

typedef SUS_WIDGET SUS_WGEDIT;
typedef SUS_WIDGETA SUS_WGEDITA;
typedef SUS_WIDGETW SUS_WGEDITW;

typedef SUS_WIDGET SUS_WGPANEL;
typedef SUS_WIDGETA SUS_WGPANELA;
typedef SUS_WIDGETW SUS_WGPANELW;

typedef SUS_WIDGET SUS_WGTOOLBAR;
typedef SUS_WIDGETA SUS_WGTOOLBARA;
typedef SUS_WIDGETW SUS_WGTOOLBARW;

// Setup a custom widget
SUS_WIDGETA SUSAPI susWidgetSetupA(
	_In_opt_ LPCSTR lpTitle,
	_In_ INT id,
	_In_ LPCSTR lpszClass,
	_In_opt_ DWORD style
);
// Setup a custom widget
SUS_WIDGETW SUSAPI susWidgetSetupW(
	_In_opt_ LPCWSTR lpTitle,
	_In_ INT id,
	_In_ LPCWSTR lpszClass,
	_In_opt_ DWORD style
);

#ifdef UNICODE
#define susWidgetSetup	susWidgetSetupW
#else
#define susWidgetSetup	susWidgetSetupA
#endif // !UNICODE

// Setup a button
SUS_INLINE SUS_WGBUTTONA SUSAPI susWgButtonSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "BUTTON", BS_DEFPUSHBUTTON | BS_CENTER | BS_VCENTER);
}
// Setup a button
SUS_INLINE SUS_WGBUTTONW SUSAPI susWgButtonSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"BUTTON", BS_DEFPUSHBUTTON | BS_CENTER | BS_VCENTER);
}

#ifdef UNICODE
#define susWgButtonSetup	susWgButtonSetupW
#else
#define susWgButtonSetup	susWgButtonSetupA
#endif // !UNICODE

// Setup a edit widget
SUS_INLINE SUS_WGEDITA SUSAPI susWgEditSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "EDIT", ES_AUTOHSCROLL);
}
// Setup a edit widget
SUS_INLINE SUS_WGEDITW SUSAPI susWgEditSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"EDIT", ES_AUTOHSCROLL);
}

#ifdef UNICODE
#define susWgEditSetup	susWgEditSetupW
#else
#define susWgEditSetup	susWgEditSetupA
#endif // !UNICODE

// Setup a static widget
SUS_INLINE SUS_WGPANELA SUSAPI susWgPanelSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "STATIC", 0);
}
// Setup a static widget
SUS_INLINE SUS_WGPANELW SUSAPI susWgPanelSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"STATIC", 0);
}

#ifdef UNICODE
#define susWgPanelSetup	susWgPanelSetupW
#else
#define susWgPanelSetup	susWgPanelSetupA
#endif // !UNICODE

// Build a widget
BOOL SUSAPI susBuildWidgetA(
	_In_ HWND hWnd,
	_Inout_ SUS_LPWIDGETA widget
);
// Build a widget
BOOL SUSAPI susBuildWidgetW(
	_In_ HWND hWnd,
	_Inout_ SUS_LPWIDGETW widget
);

#ifdef UNICODE
#define susBuildWidget	susBuildWidgetW
#else
#define susBuildWidget	susBuildWidgetA
#endif // !UNICODE

// =================================================================================================

// Set the widget position
SUS_INLINE VOID SUSAPI susWidgetSetPos(SUS_LPWIDGET widget, POINT pos) {
	widget->wStruct.x = pos.x;
	widget->wStruct.y = pos.y;
}
// Set the widget size
SUS_INLINE VOID SUSAPI susWidgetSetSize(SUS_LPWIDGET widget, SIZE size) {
	widget->wStruct.cx = size.cx;
	widget->wStruct.cy = size.cy;
}
#ifndef SUSNOTCOMMCTRL
// Install an event handler for the widget
SUS_INLINE VOID SUSAPI susWidgetSetHandler(SUS_LPWIDGET widget, SUBCLASSPROC widgetProc) {
	SetWindowSubclass(widget->hWnd, widgetProc, (UINT_PTR)widget->wStruct.hMenu, (DWORD_PTR)0);
}
// Delete a widget Handler
SUS_INLINE VOID SUSAPI susRemoveWidgetHandler(HWND hWnd, SUBCLASSPROC widgetProc, UINT_PTR uIdSubclass) {
	RemoveWindowSubclass(hWnd, widgetProc, uIdSubclass);
}
#endif // !SUSNOTCOMMCTRL
// Set the widget size and position
SUS_INLINE VOID SUSAPI susWidgetSetBounds(SUS_LPWIDGET widget, RECT bounds) {
	susWidgetSetPos(widget, (POINT) { bounds.left, bounds.top });
	susWidgetSetSize(widget, (SIZE) { bounds.right, bounds.bottom });
}
// Set the widget size and position
SUS_INLINE VOID SUSAPI susWidgetSetRect(SUS_LPWIDGET widget, RECT bounds) {
	susWidgetSetPos(widget, (POINT) { bounds.left, bounds.top });
	susWidgetSetSize(widget, (SIZE) { bounds.right - bounds.left, bounds.bottom - bounds.top });
}
// Set styles for the widget
SUS_INLINE VOID SUSAPI susWidgetSetStyle(SUS_LPWIDGET widget, DWORD style) {
	widget->wStruct.style = style;
}
// Add styles for the widget
SUS_INLINE VOID SUSAPI susWidgetAddStyle(SUS_LPWIDGET widget, DWORD style) {
	widget->wStruct.style |= style;
}
// Set the extended styles for the widget
SUS_INLINE VOID SUSAPI susWidgetSetExStyle(SUS_LPWIDGET widget, DWORD dwExStyle) {
	widget->wStruct.dwExStyle = dwExStyle;
}
// Add the extended styles for the widget
SUS_INLINE VOID SUSAPI susWidgetAddExStyle(SUS_LPWIDGET widget, DWORD dwExStyle) {
	widget->wStruct.dwExStyle |= dwExStyle;
}
// Set the extended styles for the widget
SUS_INLINE VOID SUSAPI susWidgetSetId(SUS_LPWIDGET widget, UINT id) {
	widget->wStruct.hMenu = (HMENU)((SIZE_T)id);
}
// Set the widget title
SUS_INLINE VOID SUSAPI susWidgetSetTitleA(SUS_LPWIDGETA widget, LPCSTR title) {
	widget->wStruct.lpszName = title;
}
// Set the widget title
SUS_INLINE VOID SUSAPI susWidgetSetTitleW(SUS_LPWIDGETW widget, LPCWSTR title) {
	widget->wStruct.lpszName = title;
}

#endif /* !_SUS_WINDOW_API_ */