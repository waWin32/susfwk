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
SUS_INLINE VOID SUSAPI susInitCommonControls() {
	InitCommonControls();
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

// Mouse Events
typedef struct sus_mouse_event {
	POINT pos;
	UINT button;
#define MOUSE_WHEEL_UP 1 << 0
#define MOUSE_WHEEL_DOWN 1 << 1
#define MOUSE_WHEEL_LEFT 1 << 2
#define MOUSE_WHEEL_RIGHT 1 << 3
	BYTE flags;
#define MOUSE_EVENT_IS_ALT_PRESSED 1 << 0
#define MOUSE_EVENT_IS_CTRL_PRESSED 1 << 1
#define MOUSE_EVENT_IS_SHIFT_PRESSED 1 << 2
#define MOUSE_EVENT_DRAGGED	1 << 3
} SUS_MOUSE_EVENT, *SUS_PMOUSE_EVENT, *SUS_LPMOUSE_EVENT;
// Keyboard Events
typedef struct sus_key_event {
	UINT key;
	UINT scanCode;
	BYTE flags;
#define KEY_EVENT_IS_EXTENDED 1 << 0
#define KEY_EVENT_WAS_PRESSED_BEFORE 1 << 1
#define KEY_EVENT_IS_ALT_PRESSED 1 << 2
#define KEY_EVENT_IS_CTRL_PRESSED 1 << 3
#define KEY_EVENT_IS_SHIFT_PRESSED 1 << 4
} SUS_KEY_EVENT, *SUS_PKEY_EVENT, *SUS_LPKEY_EVENT;

// ---------------------------------

typedef struct sus_graphics {
	HDC			hdc;	// Double buffer
	HBITMAP		hbmMem;	// current bitmap
	HBITMAP		hbmOld;	// the old bitmap
	sus_flag8	flags;	// Graphics flags
#define SUS_GFLAG_DONT_USE_DOUBLE_BUFFERING 1 << 0
} SUS_GRAPHICS_STRUCT, *SUS_PGRAPHICS_STRUCT, *SUS_LPGRAPHICS_STRUCT;

// Window structure
typedef struct sus_window_structA {
	HWND					hWnd;		// Window Descriptor
	WNDCLASSEXA				wcEx;		// Window Class structure
	CREATESTRUCTA			wStruct;	// Window creation structure
	SUS_GRAPHICS_STRUCT		graphics;	// graphics context
} SUS_WINDOW_STRUCTA, *SUS_PWINDOW_STRUCTA, *SUS_LPWINDOW_STRUCTA;
// Window structure
typedef struct sus_window_structW {
	HWND					hWnd;		// Window Descriptor
	WNDCLASSEXW				wcEx;		// Window Class structure
	CREATESTRUCTW			wStruct;	// Window creation structure
	SUS_GRAPHICS_STRUCT		graphics;	// graphics context
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
typedef SUS_GRAPHICS_STRUCT SUS_GRAPHICS, *SUS_PGRAPHICS, *SUS_LPGRAPHICS;

// Basic initialization of the window
SUS_WINDOWA SUSAPI susWindowSetupA(_In_opt_ LPCSTR lpTitle, _In_opt_ LPVOID lParam);
// Basic initialization of the window
SUS_WINDOWW SUSAPI susWindowSetupW(_In_opt_ LPCWSTR lpTitle, _In_opt_ LPVOID lParam);
// Build a window
BOOL SUSAPI susBuildWindowA(_Inout_ SUS_LPWINDOW_STRUCTA window);
// Build a window
BOOL SUSAPI susBuildWindowW(_Inout_ SUS_LPWINDOW_STRUCTW window);
// The main Window cycle
INT SUSAPI susWindowMainLoopA(_In_ SUS_LPWINDOW_STRUCTA window);
// The main Window cycle
INT SUSAPI susWindowMainLoopW(_In_ SUS_LPWINDOW_STRUCTW window);

#ifdef UNICODE
#define susWindowSetup		susWindowSetupW
#define susBuildWindow		susBuildWindowW
#define susWindowMainLoop	susWindowMainLoopW
#else // ELSE UNICODE
#define susWindowSetup		susWindowSetupA
#define susBuildWindow		susBuildWindowA
#define susWindowMainLoop	susWindowMainLoopA
#endif // !UNICODE

// =================================================================================================

// Set the window position
SUS_INLINE VOID SUSAPI susSetWindowPos(SUS_LPWINDOW window, POINT pos) {
	window->wStruct.x = pos.x;
	window->wStruct.y = pos.y;
}
// Set the window size
SUS_INLINE VOID SUSAPI susSetWindowSize(SUS_LPWINDOW window, SIZE size) {
	window->wStruct.cx = size.cx;
	window->wStruct.cy = size.cy;
}
// Set the window size and position
SUS_INLINE VOID SUSAPI susSetWindowBounds(SUS_LPWINDOW window, RECT bounds) {
	susSetWindowPos(window, (POINT) { bounds.left, bounds.top });
	susSetWindowSize(window, (SIZE) { bounds.right, bounds.bottom });
}
// Set the window frame in the center of the screen
SUS_INLINE VOID SUSAPI susSetWindowBoundsCenter(SUS_LPWINDOW window, SIZE size) {
	susSetWindowSize(window, size);
	susSetWindowPos(window, susGetCenterPos(size, susGetScreenSize()));
}
// Set styles for the window
SUS_INLINE VOID SUSAPI susSetWindowStyle(SUS_LPWINDOW window, DWORD style) {
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
SUS_INLINE VOID SUSAPI susSetWindowExStyle(SUS_LPWINDOW window, DWORD dwExStyle) {
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
SUS_INLINE VOID SUSAPI susSetWindowTitleA(SUS_LPWINDOWA window, LPCSTR title) {
	window->wStruct.lpszName = title;
}
// Set the window title
SUS_INLINE VOID SUSAPI susSetWindowTitleW(SUS_LPWINDOWW window, LPCWSTR title) {
	window->wStruct.lpszName = title;
}

#ifdef UNICODE
#define susSetWindowTitle	susSetWindowTitleW
#else // ELSE UNICODE
#define susSetWindowTitle	susSetWindowTitleA
#endif // !UNICODE

// ===============================================

SUS_INLINE VOID susSetWindowMenu(SUS_LPWINDOW window, LPCSTR lpszMenuName) {
	window->wcEx.lpszMenuName = lpszMenuName;
}
SUS_INLINE VOID susSetWindowHandler(SUS_LPWINDOW window, WNDPROC wndProc) {
	window->wcEx.lpfnWndProc = wndProc;
}
// Set the background color
SUS_INLINE VOID SUSAPI susSetWindowBackgroundColor(SUS_LPWINDOW window, HBRUSH color) {
	window->wcEx.hbrBackground = color;
}
// Set the window icon
SUS_INLINE VOID SUSAPI susSetWindowIcon(SUS_LPWINDOW window, HICON icon) {
	window->wcEx.hIcon = icon;
}
// Set the window cursor
SUS_INLINE VOID SUSAPI susSetWindowCursor(SUS_LPWINDOW window, HCURSOR cursor) {
	window->wcEx.hCursor = cursor;
}
// Set window visibility
SUS_INLINE VOID SUSAPI susSetWindowVisible(SUS_LPWINDOW window, BOOLEAN visible) {
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
SUS_INLINE SUS_OBJECT susWriteIncomingWindowData(HWND hWnd, LPARAM lParam) {
	CONST LPCREATESTRUCT pCreate = (CONST LPCREATESTRUCT)lParam;
	susWriteDataToWindow(hWnd, (LONG_PTR)pCreate->lpCreateParams);
	return (SUS_OBJECT)pCreate->lpCreateParams;
}
// Get a window from a window procedure
SUS_INLINE SUS_OBJECT susLoadWindowData(HWND hWnd) {
	return (SUS_OBJECT)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

// Scaling the graphics after resizing the window
SUS_INLINE VOID susScalingGraphics(HWND hWnd, SUS_LPGRAPHICS_STRUCT graphics, LPARAM lParam) {
	if (graphics->flags & SUS_GFLAG_DONT_USE_DOUBLE_BUFFERING)
		return;
	HDC hdc = GetDC(hWnd);
	if (graphics->hbmMem) {
		SelectObject(graphics->hdc, graphics->hbmOld);
		DeleteObject(graphics->hbmMem);
		graphics->hbmMem = NULL;
	}
	graphics->hdc = CreateCompatibleDC(hdc);
	SIZE bounds = (SIZE){
		.cx = LOWORD(lParam),
		.cy = HIWORD(lParam)
	};
	graphics->hbmMem = CreateCompatibleBitmap(hdc, bounds.cx, bounds.cy);
	if (graphics->hbmMem) {
		graphics->hbmOld = SelectObject(graphics->hdc, graphics->hbmMem);
	}
	ReleaseDC(hWnd, hdc);
}
SUS_INLINE VOID susCleanupGraphics(SUS_LPGRAPHICS_STRUCT graphics) {
	if (graphics->hbmOld) {
		SelectObject(graphics->hdc, graphics->hbmOld);
		graphics->hbmOld = NULL;
	}
	if (graphics->hbmMem) {
		DeleteObject(graphics->hbmMem);
		graphics->hbmMem = NULL;
	}
	if (graphics->hdc) {
		DeleteDC(graphics->hdc);
		graphics->hdc = NULL;
	}
}
SUS_INLINE PAINTSTRUCT susBeginPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	BeginPaint(hWnd, &ps);
	return ps;
}
SUS_INLINE VOID susEndPaint(SUS_GRAPHICS_STRUCT graphics, HWND hWnd, PAINTSTRUCT *ps)
{
	if (!(graphics.flags & SUS_GFLAG_DONT_USE_DOUBLE_BUFFERING)) {
		BitBlt(ps->hdc,
			ps->rcPaint.left, ps->rcPaint.top,
			ps->rcPaint.right - ps->rcPaint.left,
			ps->rcPaint.bottom - ps->rcPaint.top,
			graphics.hdc, ps->rcPaint.left, ps->rcPaint.top,
			SRCCOPY
		);
	}
	EndPaint(hWnd, ps);
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

typedef SUS_WIDGET SUS_WGSTATIC;
typedef SUS_WIDGETA SUS_WGSTATICA;
typedef SUS_WIDGETW SUS_WGSTATICW;

// Setup a button
SUS_WGBUTTONA SUSAPI susWgButtonSetupA(
	_In_ LPCSTR lpTitle,
	_In_ INT id
);
// Setup a button
SUS_WGBUTTONW SUSAPI susWgButtonSetupW(
	_In_ LPCWSTR lpTitle,
	_In_ INT id
);

#ifdef UNICODE
#define susWgButtonSetup	susWgButtonSetupW
#else
#define susWgButtonSetup	susWgButtonSetupA
#endif // !UNICODE

// Setup a edit widget
SUS_WGEDITA SUSAPI susWgEditSetupA(
	_In_ LPCSTR lpTitle,
	_In_ INT id
);
// Setup a edit widget
SUS_WGEDITW SUSAPI susWgEditSetupW(
	_In_ LPCWSTR lpTitle,
	_In_ INT id
);

#ifdef UNICODE
#define susWgEditSetup	susWgEditSetupW
#else
#define susWgEditSetup	susWgEditSetupA
#endif // !UNICODE

// Setup a static widget
SUS_WGSTATICA SUSAPI susWgStaticSetupA(
	_In_ LPCSTR lpTitle,
	_In_ INT id
);
// Setup a static widget
SUS_WGSTATICW SUSAPI susWgStaticSetupW(
	_In_ LPCWSTR lpTitle,
	_In_ INT id
);

#ifdef UNICODE
#define susWgStaticSetup	susWgStaticSetupW
#else
#define susWgStaticSetup	susWgStaticSetupA
#endif // !UNICODE

// Build a widget
BOOL SUSAPI susBuildWidgetA(
	_In_ HWND hWnd,
	_In_ SUS_LPWIDGETA widget
);
// Build a widget
BOOL SUSAPI susBuildWidgetW(
	_In_ HWND hWnd,
	_In_ SUS_LPWIDGETW widget
);

#ifdef UNICODE
#define susBuildWidget	susBuildWidgetW
#else
#define susBuildWidget	susBuildWidgetA
#endif // !UNICODE

// =================================================================================================

// Set the widget position
SUS_INLINE VOID SUSAPI susSetWidgetPos(SUS_LPWIDGET widget, POINT pos) {
	widget->wStruct.x = pos.x;
	widget->wStruct.y = pos.y;
}
// Set the widget size
SUS_INLINE VOID SUSAPI susSetWidgetSize(SUS_LPWIDGET widget, SIZE size) {
	widget->wStruct.cx = size.cx;
	widget->wStruct.cy = size.cy;
}
#ifndef SUSNOTCOMMCTRL
// Install an event handler for the widget
SUS_INLINE VOID SUSAPI susSetWidgetHandler(SUS_LPWIDGET widget, SUBCLASSPROC widgetProc) {
	SetWindowSubclass(widget->hWnd, widgetProc, (UINT_PTR)widget->wStruct.hMenu, (DWORD_PTR)0);
}
// Delete a widget Handler
SUS_INLINE VOID SUSAPI susRemoveWidgetHandler(HWND hWnd, SUBCLASSPROC widgetProc, UINT_PTR uIdSubclass) {
	RemoveWindowSubclass(hWnd, widgetProc, uIdSubclass);
}
#endif // !SUSNOTCOMMCTRL
// Set the widget size and position
SUS_INLINE VOID SUSAPI susSetWidgetBounds(SUS_LPWIDGET widget, RECT bounds) {
	susSetWidgetPos(widget, (POINT) { bounds.left, bounds.top });
	susSetWidgetSize(widget, (SIZE) { bounds.right - bounds.left, bounds.bottom - bounds.top });
}
// Set styles for the widget
SUS_INLINE VOID SUSAPI susSetWidgetStyle(SUS_LPWIDGET widget, DWORD style) {
	widget->wStruct.style = style;
}
// Set the extended styles for the widget
SUS_INLINE VOID SUSAPI susSetWidgetExStyle(SUS_LPWIDGET widget, DWORD dwExStyle) {
	widget->wStruct.dwExStyle = dwExStyle;
}
// Set the extended styles for the widget
SUS_INLINE VOID SUSAPI susSetWidgetId(SUS_LPWIDGET widget, UINT id) {
	widget->wStruct.hMenu = (HMENU)((SIZE_T)id);
}
// Set the widget title
SUS_INLINE VOID SUSAPI susSetWidgetTitleA(SUS_LPWIDGETA widget, LPCSTR title) {
	widget->wStruct.lpszName = title;
}
// Set the widget title
SUS_INLINE VOID SUSAPI susSetWidgetTitleW(SUS_LPWIDGETW widget, LPCWSTR title) {
	widget->wStruct.lpszName = title;
}

#endif /* !_SUS_WINDOW_API_ */