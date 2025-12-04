// windowapi.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/memory.h"
#include "include/susfwk/windowapi.h"

// Standard window class
static const WNDCLASSEXA susDefWndClassA = {
	.cbClsExtra = 0,
	.cbWndExtra = 0,
	.lpfnWndProc = NULL,
	.hInstance = NULL,
	.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
	.hCursor = NULL,
	.hIcon = NULL,
	.hIconSm = NULL,
	.lpszClassName = SUS_WCNAMEA,
	.lpszMenuName = NULL,
	.style = 0,
	.cbSize = sizeof(WNDCLASSEXA)
};
// Standard window class
static const WNDCLASSEXW susDefWndClassW = {
	.cbClsExtra = 0,
	.cbWndExtra = 0,
	.lpfnWndProc = NULL,
	.hInstance = NULL,
	.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
	.hCursor = NULL,
	.hIcon = NULL,
	.hIconSm = NULL,
	.lpszClassName = SUS_WCNAMEW,
	.lpszMenuName = NULL,
	.style = 0,
	.cbSize = sizeof(WNDCLASSEXW)
};
static const CREATESTRUCTA susDefWndStructA = {
	.dwExStyle = 0,
	.lpszClass = SUS_WCNAMEA,
	.lpszName = NULL,
	.hMenu = NULL,
	.hwndParent = NULL,
	.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	.x = CW_USEDEFAULT,
	.y = CW_USEDEFAULT,
	.cx = CW_USEDEFAULT,
	.cy = CW_USEDEFAULT,
	.hInstance = NULL,
	.lpCreateParams = NULL,
};
static const CREATESTRUCTW susDefWndStructW = {
	.dwExStyle = 0,
	.lpszClass = SUS_WCNAMEW,
	.lpszName = NULL,
	.hMenu = NULL,
	.hwndParent = NULL,
	.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	.x = CW_USEDEFAULT,
	.y = CW_USEDEFAULT,
	.cx = CW_USEDEFAULT,
	.cy = CW_USEDEFAULT,
	.hInstance = NULL,
	.lpCreateParams = NULL,
};
static const CREATESTRUCTA susDefWidgetStructA = {
	.dwExStyle = 0,
	.lpszClass = NULL,
	.lpszName = NULL,
	.hMenu = NULL,
	.hwndParent = NULL,
	.style = WS_VISIBLE | WS_CHILD,
	.x = 0,
	.y = 0,
	.cx = 100,
	.cy = 50,
	.hInstance = NULL,
	.lpCreateParams = NULL,
};
static const CREATESTRUCTW susDefWidgetStructW = {
	.dwExStyle = 0,
	.lpszClass = NULL,
	.lpszName = NULL,
	.hMenu = NULL,
	.hwndParent = NULL,
	.style = WS_VISIBLE | WS_CHILD,
	.x = 0,
	.y = 0,
	.cx = 100,
	.cy = 50,
	.hInstance = NULL,
	.lpCreateParams = NULL,
};

// Basic initialization of the window
SUS_WINDOWA SUSAPI susWindowSetupA(_In_opt_ LPCSTR lpTitle, _In_opt_ LPVOID lParam)
{
	SUS_PRINTDL("Setting up the window");
	SUS_ASSERT(lpTitle);
	SUS_WINDOWA window = { 0 };
	window.wcEx = susDefWndClassA;
	window.wStruct = susDefWndStructA;
	window.wcEx.lpfnWndProc = DefWindowProcA;
	window.wStruct.hInstance = GetModuleHandleA(NULL);
	window.wStruct.lpszName = lpTitle;
	window.wStruct.lpCreateParams = lParam;
	sus_formattingA(window.classNameBuffer, "%s-%s%pA", SUS_WCNAMEA, lpTitle, &window);
	return window;
}
// Basic initialization of the window
SUS_WINDOWW SUSAPI susWindowSetupW(_In_opt_ LPCWSTR lpTitle, _In_opt_ LPVOID lParam)
{
	SUS_PRINTDL("Setting up the window");
	SUS_ASSERT(lpTitle);
	SUS_WINDOWW window = { 0 };
	window.wcEx = susDefWndClassW;
	window.wStruct = susDefWndStructW;
	window.wcEx.lpfnWndProc = DefWindowProcW;
	window.wStruct.hInstance = GetModuleHandleW(NULL);
	window.wStruct.lpszName = lpTitle;
	window.wStruct.lpCreateParams = lParam;
	sus_formattingW(window.classNameBuffer, L"%s-%s%pW", SUS_WCNAMEW, lpTitle, &window);
	return window;
}
// Build a window
HWND SUSAPI susBuildWindowA(_Inout_ SUS_LPWINDOWA window)
{
	SUS_PRINTDL("Creating a window");
	SUS_ASSERT(window);
	WNDCLASSA wcData;
	if (!GetClassInfoA(window->wStruct.hInstance, window->wcEx.lpszClassName, &wcData)) {
		if (!RegisterClassExA(&window->wcEx)) {
			SUS_PRINTDE("Failed to register window class");
			SUS_PRINTDC(GetLastError());
			return FALSE;
		}
	}
	window->hWnd = CreateWindowExA(
		window->wStruct.dwExStyle,
		window->wStruct.lpszClass,
		window->wStruct.lpszName,
		window->wStruct.style,
		window->wStruct.x,
		window->wStruct.y,
		window->wStruct.cx,
		window->wStruct.cy,
		window->wStruct.hwndParent,
		window->wStruct.hMenu,
		window->wStruct.hInstance,
		window->wStruct.lpCreateParams
	);
	if (!window->hWnd) {
		SUS_PRINTDE("Couldn't create a window");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The window has been successfully created");
	return window->hWnd;
}
// Build a window
HWND SUSAPI susBuildWindowW(_Inout_ SUS_LPWINDOWW window)
{
	SUS_PRINTDL("Creating a window");
	SUS_ASSERT(window);
	WNDCLASSW wcData;
	if (!GetClassInfoW(window->wStruct.hInstance, window->wcEx.lpszClassName, &wcData)) {
		if (!RegisterClassExW(&window->wcEx)) {
			SUS_PRINTDE("Failed to register window class");
			SUS_PRINTDC(GetLastError());
			return FALSE;
		}
	}
	window->hWnd = CreateWindowExW(
		window->wStruct.dwExStyle,
		window->wStruct.lpszClass,
		window->wStruct.lpszName,
		window->wStruct.style,
		window->wStruct.x,
		window->wStruct.y,
		window->wStruct.cx,
		window->wStruct.cy,
		window->wStruct.hwndParent,
		window->wStruct.hMenu,
		window->wStruct.hInstance,
		window->wStruct.lpCreateParams
	);
	if (!window->hWnd) {
		SUS_PRINTDE("Couldn't create a window");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The window has been successfully created");
	return window->hWnd;
}
// Create a simple window
HWND SUSAPI susCreateWindowExA(_In_opt_ LPCSTR lpTitle, _In_ SIZE size, _In_opt_ WNDPROC handler, _In_opt_ HICON hIcon, _In_opt_ LPCSTR lpMenuName, _In_opt_ LPVOID lParam)
{
	SUS_WINDOWA wnd = susWindowSetupA(lpTitle, lParam);
	susWindowSetSize(&wnd, size);
	if (handler) susWindowSetHandler(&wnd, handler);
	if (hIcon) susWindowSetIcon(&wnd, hIcon);
	if (lpMenuName) susWindowSetMenu(&wnd, lpMenuName);
	return susBuildWindowA(&wnd);
}
// Create a simple window
HWND SUSAPI susCreateWindowExW(_In_opt_ LPCWSTR lpTitle, _In_ SIZE size, _In_opt_ WNDPROC handler, _In_opt_ HICON hIcon, _In_opt_ LPCWSTR lpMenuName, _In_opt_ LPVOID lParam)
{
	SUS_WINDOWW wnd = susWindowSetupW(lpTitle, lParam);
	susWindowSetSizeW(&wnd, size);
	if (handler) susWindowSetHandlerW(&wnd, handler);
	if (hIcon) susWindowSetIconW(&wnd, hIcon);
	if (lpMenuName) susWindowSetMenuW(&wnd, lpMenuName);
	return susBuildWindowW(&wnd);
}
// The main Window cycle
INT SUSAPI susWindowMainLoopA()
{
	SUS_PRINTDL("The main window cycle");
	MSG msg;
	while (GetMessageA(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
	return (int)msg.wParam;
}
// The main Window cycle
INT SUSAPI susWindowMainLoopW()
{
	SUS_PRINTDL("The main window cycle");
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return (int)msg.wParam;
}

// ===============================================

// Setup a custom widget
SUS_WIDGETA SUSAPI susWidgetSetupA(_In_opt_ LPCSTR lpTitle, _In_ INT id, _In_ LPCSTR lpszClass, _In_opt_ DWORD style)
{
	SUS_PRINTDL("Setting up the widget");
	SUS_WIDGETA wg = { 0 };
	wg.wStruct = susDefWidgetStructA;
	wg.wStruct.style |= WS_CHILD | WS_VISIBLE | style;
	wg.wStruct.lpszName = lpTitle;
	wg.wStruct.lpszClass = lpszClass;
	wg.wStruct.hMenu = (HMENU)(INT_PTR)id;
	return wg;
}
// Setup a custom widget
SUS_WIDGETW SUSAPI susWidgetSetupW(_In_opt_ LPCWSTR lpTitle, _In_ INT id, _In_ LPCWSTR lpszClass, _In_opt_ DWORD style)
{
	SUS_PRINTDL("Setting up the widget");
	SUS_WIDGETW wg = { 0 };
	wg.wStruct = susDefWidgetStructW;
	wg.wStruct.style |= WS_CHILD | WS_VISIBLE | style;
	wg.wStruct.lpszName = lpTitle;
	wg.wStruct.lpszClass = lpszClass;
	wg.wStruct.hMenu = (HMENU)(INT_PTR)id;
	return wg;
}

// Build a widget
HWND SUSAPI susBuildWidgetA(_In_ HWND hWnd, _Inout_ SUS_LPWIDGETA widget)
{
	SUS_PRINTDL("Creating a widget");
	SUS_ASSERT(hWnd && widget);
	widget->wStruct.hwndParent = hWnd;
	widget->wStruct.hInstance = GetModuleHandleA(NULL);
	widget->hWnd = CreateWindowExA(
		widget->wStruct.dwExStyle,
		widget->wStruct.lpszClass,
		widget->wStruct.lpszName,
		widget->wStruct.style,
		widget->wStruct.x,
		widget->wStruct.y,
		widget->wStruct.cx,
		widget->wStruct.cy,
		widget->wStruct.hwndParent,
		widget->wStruct.hMenu,
		widget->wStruct.hInstance,
		(LPVOID)susWindowLoadData(hWnd)
	);
	if (!widget->hWnd) {
		SUS_PRINTDE("Couldn't create a widget");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The widget has been successfully created");
	return widget->hWnd;
}
// Build a widget
HWND SUSAPI susBuildWidgetW(_In_ HWND hWnd, _Inout_ SUS_LPWIDGETW widget)
{
	SUS_PRINTDL("Creating a widget");
	SUS_ASSERT(hWnd && widget);
	widget->wStruct.hwndParent = hWnd;
	widget->wStruct.hInstance = GetModuleHandleW(NULL);
	widget->hWnd = CreateWindowExW(
		widget->wStruct.dwExStyle,
		widget->wStruct.lpszClass,
		widget->wStruct.lpszName,
		widget->wStruct.style,
		widget->wStruct.x,
		widget->wStruct.y,
		widget->wStruct.cx,
		widget->wStruct.cy,
		widget->wStruct.hwndParent,
		widget->wStruct.hMenu,
		widget->wStruct.hInstance,
		(LPVOID)susWindowLoadData(hWnd)
	);
	if (!widget->hWnd) {
		SUS_PRINTDE("Couldn't create a widget");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The widget has been successfully created");
	return widget->hWnd;
}
