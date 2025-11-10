// windowapi.h
//
#ifndef _SUS_WINDOW_API_
#define _SUS_WINDOW_API_

#include "memory.h"

#define SUS_WCNAMEA "suswc"
#define SUS_WCNAMEW L"suswc"

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
} SUS_WINDOWA, *SUS_PWINDOW_STRUCTA, *SUS_LPWINDOWA;
// Window structure
typedef struct sus_window_structW {
	HWND					hWnd;		// Window Descriptor
	WNDCLASSEXW				wcEx;		// Window Class structure
	CREATESTRUCTW			wStruct;	// Window creation structure
} SUS_WINDOWW, *SUS_LPWINDOWW;

#ifdef UNICODE
#define SUS_WINDOW		SUS_WINDOWW
#define SUS_LPWINDOW	SUS_LPWINDOWW
#else // ELSE UNICODE
#define SUS_WINDOW		SUS_WINDOWA
#define SUS_LPWINDOW	SUS_LPWINDOWA
#endif // !UNICODE

// Basic initialization of the window
SUS_WINDOWA SUSAPI susWindowSetupA(_In_opt_ LPCSTR lpTitle, _In_opt_ LPVOID lParam);
// Basic initialization of the window
SUS_WINDOWW SUSAPI susWindowSetupW(_In_opt_ LPCWSTR lpTitle, _In_opt_ LPVOID lParam);

#ifdef UNICODE
#define susWindowSetup		susWindowSetupW
#else // ELSE UNICODE
#define susWindowSetup		susWindowSetupA
#endif // !UNICODE

// Build a window
BOOL SUSAPI susBuildWindowA(_Inout_ SUS_LPWINDOWA window);
// Build a window
BOOL SUSAPI susBuildWindowW(_Inout_ SUS_LPWINDOWW window);

#ifdef UNICODE
#define susBuildWindow		susBuildWindowW
#else // ELSE UNICODE
#define susBuildWindow		susBuildWindowA
#endif // !UNICODE

// Create a simple window
BOOL SUSAPI susCreateWindowExA(_In_opt_ LPCSTR lpTitle, _In_ SIZE size, _In_opt_ WNDPROC handler, _In_opt_ HICON hIcon, _In_opt_ LPCSTR lpMenuName, _In_opt_ LPVOID lParam);
// Create a simple window
SUS_INLINE BOOL SUSAPI susCreateWindowA(_In_opt_ LPCSTR lpTitle, _In_ SIZE size, _In_opt_ WNDPROC handler, _In_opt_ LPVOID lParam) {
	return susCreateWindowExA(lpTitle, size, handler, NULL, NULL, lParam);
}
// Create a simple window
BOOL SUSAPI susCreateWindowExW(_In_opt_ LPCWSTR lpTitle, _In_ SIZE size, _In_opt_ WNDPROC handler, _In_opt_ HICON hIcon, _In_opt_ LPCWSTR lpMenuName, _In_opt_ LPVOID lParam);
// Create a simple window
SUS_INLINE BOOL SUSAPI susCreateWindowW(_In_opt_ LPCWSTR lpTitle, _In_ SIZE size, _In_opt_ WNDPROC handler, _In_opt_ LPVOID lParam) {
	return susCreateWindowExW(lpTitle, size, handler, NULL, NULL, lParam);
}

#ifdef UNICODE
#define susCreateWindowEx	susCreateWindowExW
#define susCreateWindow	susCreateWindowW
#else // ELSE UNICODE
#define susCreateWindowEx	susCreateWindowExA
#define susCreateWindow	susCreateWindowA
#endif // !UNICODE

// The main Window cycle
INT SUSAPI susWindowMainLoopA();
// The main Window cycle
INT SUSAPI susWindowMainLoopW();

#ifdef UNICODE
#define susWindowMainLoop	susWindowMainLoopW
#else // ELSE UNICODE
#define susWindowMainLoop	susWindowMainLoopA
#endif // !UNICODE

// =================================================================================================

// Set the window position
SUS_INLINE VOID SUSAPI susWindowSetPos(SUS_LPWINDOW window, POINT pos) {
	window->wStruct.x = pos.x;
	window->wStruct.y = pos.y;
}
// Set the window position
SUS_INLINE VOID SUSAPI susWindowSetPosW(SUS_LPWINDOWW window, POINT pos) {
	window->wStruct.x = pos.x;
	window->wStruct.y = pos.y;
}
// Set the window size
SUS_INLINE VOID SUSAPI susWindowSetSize(SUS_LPWINDOWA window, SIZE size) {
	window->wStruct.cx = size.cx;
	window->wStruct.cy = size.cy;
}
// Set the window size
SUS_INLINE VOID SUSAPI susWindowSetSizeW(SUS_LPWINDOWW window, SIZE size) {
	window->wStruct.cx = size.cx;
	window->wStruct.cy = size.cy;
}
// Set the window size and position
SUS_INLINE VOID SUSAPI susWindowSetBounds(SUS_LPWINDOWA window, RECT bounds) {
	susWindowSetPos(window, (POINT) { bounds.left, bounds.top });
	susWindowSetSize(window, (SIZE) { bounds.right, bounds.bottom });
}
// Set the window size and position
SUS_INLINE VOID SUSAPI susWindowSetBoundsW(SUS_LPWINDOWW window, RECT bounds) {
	susWindowSetPosW(window, (POINT) { bounds.left, bounds.top });
	susWindowSetSizeW(window, (SIZE) { bounds.right, bounds.bottom });
}
// Set the window frame in the center of the screen
SUS_INLINE VOID SUSAPI susWindowSetBoundsCenter(SUS_LPWINDOWA window, SIZE size) {
	susWindowSetSize(window, size);
	susWindowSetPos(window, susGetCenterPos(size, susGetScreenSize()));
}
// Set the window frame in the center of the screen
SUS_INLINE VOID SUSAPI susWindowSetBoundsCenterW(SUS_LPWINDOWW window, SIZE size) {
	susWindowSetSizeW(window, size);
	susWindowSetPosW(window, susGetCenterPos(size, susGetScreenSize()));
}
// Set styles for the window
SUS_INLINE VOID SUSAPI susWindowSetStyle(SUS_LPWINDOW window, DWORD style) {
	window->wStruct.style = style;
}
// Add styles for the window
SUS_INLINE VOID SUSAPI susWindowAddStyle(SUS_LPWINDOW window, DWORD style) {
	window->wStruct.style |= style;
}
// Remove styles for the window
SUS_INLINE VOID SUSAPI susWindowRemoveStyle(SUS_LPWINDOW window, DWORD style) {
	window->wStruct.style &= ~style;
}

// Set the extended styles for the window
SUS_INLINE VOID SUSAPI susWindowSetExStyle(SUS_LPWINDOW window, DWORD dwExStyle) {
	window->wStruct.dwExStyle = dwExStyle;
}
// Add extended styles for the window
SUS_INLINE VOID SUSAPI susWindowAddExStyle(SUS_LPWINDOW window, DWORD dwExStyle) {
	window->wStruct.dwExStyle |= dwExStyle;
}
// Remove extended styles for the window
SUS_INLINE VOID SUSAPI susWindowRemoveExStyle(SUS_LPWINDOW window, DWORD dwExStyle) {
	window->wStruct.dwExStyle &= ~dwExStyle;
}
// Set the window parent
SUS_INLINE VOID SUSAPI susWindowSetParent(SUS_LPWINDOWA window, HWND hWnd) {
	window->wStruct.hwndParent = hWnd;
}
// Set the window parent
SUS_INLINE VOID SUSAPI susWindowSetParentW(SUS_LPWINDOWW window, HWND hWnd) {
	window->wStruct.hwndParent = hWnd;
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

SUS_INLINE VOID susWindowSetMenu(SUS_LPWINDOWA window, LPCSTR lpszMenuName) {
	window->wcEx.lpszMenuName = lpszMenuName;
}
SUS_INLINE VOID susWindowSetMenuW(SUS_LPWINDOWW window, LPCWSTR lpszMenuName) {
	window->wcEx.lpszMenuName = lpszMenuName;
}
SUS_INLINE VOID susWindowSetHandler(SUS_LPWINDOWA window, WNDPROC wndProc) {
	window->wcEx.lpfnWndProc = wndProc;
}
SUS_INLINE VOID susWindowSetHandlerW(SUS_LPWINDOWW window, WNDPROC wndProc) {
	window->wcEx.lpfnWndProc = wndProc;
}
// Set the background color
SUS_INLINE VOID SUSAPI susWindowSetBackgroundColor(SUS_LPWINDOWA window, HBRUSH color) {
	window->wcEx.hbrBackground = color;
}
// Set the background color
SUS_INLINE VOID SUSAPI susWindowSetBackgroundColorW(SUS_LPWINDOWW window, HBRUSH color) {
	window->wcEx.hbrBackground = color;
}
// Set the window icon
SUS_INLINE VOID SUSAPI susWindowSetIcon(SUS_LPWINDOWA window, HICON icon) {
	window->wcEx.hIcon = icon;
}
// Set the window icon
SUS_INLINE VOID SUSAPI susWindowSetIconW(SUS_LPWINDOWW window, HICON icon) {
	window->wcEx.hIcon = icon;
}
// Set the window cursor
SUS_INLINE VOID SUSAPI susWindowSetCursor(SUS_LPWINDOWA window, HCURSOR cursor) {
	window->wcEx.hCursor = cursor;
}
// Set the window cursor
SUS_INLINE VOID SUSAPI susWindowSetCursorw(SUS_LPWINDOWW window, HCURSOR cursor) {
	window->wcEx.hCursor = cursor;
}
// Set window visibility
SUS_INLINE VOID SUSAPI susWindowSetVisible(HWND hWnd, BOOLEAN visible) {
	ShowWindow(hWnd, visible ? SW_SHOW : SW_HIDE);
}

// ===============================================

// Save data to a window
SUS_INLINE SUS_OBJECT susWindowWriteData(HWND hWnd, LONG_PTR dwNewLong) {
	SetWindowLongPtr(hWnd, GWLP_USERDATA, dwNewLong);
	return (SUS_OBJECT)dwNewLong;
}
// Write the window's incoming data to its internal memory
SUS_INLINE SUS_OBJECT susWindowWriteContextData(HWND hWnd, LPARAM lParam) {
	CONST LPCREATESTRUCT pCreate = (CONST LPCREATESTRUCT)lParam;
	susWindowWriteData(hWnd, (LONG_PTR)pCreate->lpCreateParams);
	return (SUS_OBJECT)pCreate->lpCreateParams;
}
// Get a window from a window procedure
SUS_INLINE SUS_OBJECT susWindowLoadData(HWND hWnd) {
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
SUS_INLINE SUS_WIDGETA SUSAPI susWgButtonSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "BUTTON", BS_DEFPUSHBUTTON | BS_CENTER | BS_VCENTER);
}
// Setup a button
SUS_INLINE SUS_WIDGETW SUSAPI susWgButtonSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"BUTTON", BS_DEFPUSHBUTTON | BS_CENTER | BS_VCENTER);
}

#ifdef UNICODE
#define susWgButtonSetup	susWgButtonSetupW
#else
#define susWgButtonSetup	susWgButtonSetupA
#endif // !UNICODE

// Setup a edit widget
SUS_INLINE SUS_WIDGETA SUSAPI susWgEditSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "EDIT", ES_AUTOHSCROLL);
}
// Setup a edit widget
SUS_INLINE SUS_WIDGETW SUSAPI susWgEditSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"EDIT", ES_AUTOHSCROLL);
}

#ifdef UNICODE
#define susWgEditSetup	susWgEditSetupW
#else
#define susWgEditSetup	susWgEditSetupA
#endif // !UNICODE

// Setup a static widget
SUS_INLINE SUS_WIDGETA SUSAPI susWgPanelSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "STATIC", 0);
}
// Setup a static widget
SUS_INLINE SUS_WIDGETW SUSAPI susWgPanelSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"STATIC", 0);
}

#ifdef UNICODE
#define susWgPanelSetup	susWgPanelSetupW
#else
#define susWgPanelSetup	susWgPanelSetupA
#endif // !UNICODE

// Setup a radio button widget
SUS_INLINE SUS_WIDGETA SUSAPI susWgRadioButtonSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "BUTTON", BS_AUTORADIOBUTTON | WS_TABSTOP);
}
// Setup a radio button widget
SUS_INLINE SUS_WIDGETW SUSAPI susWgRadioButtonSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"BUTTON", BS_AUTORADIOBUTTON | WS_TABSTOP);
}

#ifdef UNICODE
#define susWgRadioButtonSetup	susWgRadioButtonSetupW
#else
#define susWgRadioButtonSetup	susWgRadioButtonSetupA
#endif // !UNICODE

// Setup a check box widget
SUS_INLINE SUS_WIDGETA SUSAPI susWgCheckBoxSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "BUTTON", BS_AUTOCHECKBOX | WS_TABSTOP);
}
// Setup a check box widget
SUS_INLINE SUS_WIDGETW SUSAPI susWgCheckBoxSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"BUTTON", BS_AUTOCHECKBOX | WS_TABSTOP);
}

#ifdef UNICODE
#define susWgCheckBoxSetup	susWgCheckBoxSetupW
#else
#define susWgCheckBoxSetup	susWgCheckBoxSetupA
#endif // !UNICODE

// Setup a list box widget
SUS_INLINE SUS_WIDGETA SUSAPI susWgListBoxSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "LISTBOX", LBS_NOTIFY | WS_VSCROLL | WS_BORDER);
}
// Setup a list box widget
SUS_INLINE SUS_WIDGETW SUSAPI susWgListBoxSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"LISTBOX", LBS_NOTIFY | WS_VSCROLL | WS_BORDER);
}

#ifdef UNICODE
#define susWgListBoxSetup	susWgListBoxSetupW
#else
#define susWgListBoxSetup	susWgListBoxSetupA
#endif // !UNICODE

// Setup a combo box widget
SUS_INLINE SUS_WIDGETA SUSAPI susWgComboBoxSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "COMBOBOX", CBS_DROPDOWN | WS_VSCROLL);
}
// Setup a combo box widget
SUS_INLINE SUS_WIDGETW SUSAPI susWgComboBoxSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"COMBOBOX", CBS_DROPDOWN | WS_VSCROLL);
}

#ifdef UNICODE
#define susWgComboBoxSetup  susWgComboBoxSetupW
#else
#define susWgComboBoxSetup  susWgComboBoxSetupA
#endif // !UNICODE

// Setup a scroll box widget
SUS_INLINE SUS_WIDGETA SUSAPI susWgScrollBarSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, "SCROLLBAR", SBS_HORZ);
}
// Setup a scroll box widget
SUS_INLINE SUS_WIDGETW SUSAPI susWgScrollBarSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, L"SCROLLBAR", SBS_HORZ);
}

#ifdef UNICODE
#define susWgScrollBarSetup  susWgScrollBarSetupW
#else
#define susWgScrollBarSetup  susWgScrollBarSetupA
#endif // !UNICODE

// Setup a list view widget
SUS_INLINE SUS_WIDGETA SUSAPI susWgListViewSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id) {
	return susWidgetSetupA(lpTitle, id, WC_LISTVIEWA, LVS_REPORT);
}
// Setup a list view widget
SUS_INLINE SUS_WIDGETW SUSAPI susWgListViewSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id) {
	return susWidgetSetupW(lpTitle, id, WC_LISTVIEWW, LVS_REPORT);
}

#ifdef UNICODE
#define susWgListViewSetup  susWgListViewSetupW
#else
#define susWgListViewSetup  susWgListViewSetupA
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
