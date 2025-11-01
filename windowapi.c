// windowapi.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/string.h"
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
	.style = WS_OVERLAPPEDWINDOW,
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
	.style = WS_OVERLAPPEDWINDOW,
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
	sus_memcpy((LPBYTE)&window.wcEx, (CONST LPBYTE)&susDefWndClassA, sizeof(WNDCLASSEXA));
	sus_memcpy((LPBYTE)&window.wStruct, (CONST LPBYTE) &susDefWndStructA, sizeof(CREATESTRUCTA));
	window.wcEx.lpfnWndProc = DefWindowProcA;
	window.wStruct.hInstance = GetModuleHandleA(NULL);
	window.wStruct.lpszName = lpTitle;
	window.wStruct.lpCreateParams = lParam;
	LPSTR className = sus_fmalloc((SIZE_T)(lstrlenA(SUS_WCNAMEA) + lstrlenA(lpTitle) + 30));
	if (className) {
		sus_formattingA(className, "%s%s%pA", SUS_WCNAMEA, lpTitle, &window);
		window.wcEx.lpszClassName = className;
		window.wStruct.lpszClass = className;
	}
	else {
		window.wcEx.lpszClassName = SUS_DEFWNDNAMEA;
		window.wStruct.lpszClass = SUS_DEFWNDNAMEA;
	}
	return window;
}
// Basic initialization of the window
SUS_WINDOWW SUSAPI susWindowSetupW(_In_opt_ LPCWSTR lpTitle, _In_opt_ LPVOID lParam)
{
	SUS_PRINTDL("Setting up the window");
	SUS_ASSERT(lpTitle);
	SUS_WINDOWW window = { 0 };
	sus_memcpy((LPBYTE)&window.wcEx, (CONST LPBYTE)&susDefWndClassW, sizeof(WNDCLASSEXW));
	sus_memcpy((LPBYTE)&window.wStruct, (CONST LPBYTE)&susDefWndStructW, sizeof(CREATESTRUCTW));
	window.wcEx.lpfnWndProc = DefWindowProcW;
	window.wStruct.hInstance = GetModuleHandleW(NULL);
	window.wStruct.lpszName = lpTitle;
	window.wStruct.lpCreateParams = lParam;
	LPWSTR className = sus_fmalloc((SIZE_T)(lstrlenW(SUS_WCNAMEW) + lstrlenW(lpTitle) + 30));
	if (className) {
		sus_formattingW(className, L"%s%s%pW", SUS_WCNAMEW, lpTitle,  &window);
		window.wcEx.lpszClassName = className;
		window.wStruct.lpszClass = className;
	}
	else {
		window.wcEx.lpszClassName = SUS_DEFWNDNAMEW;
		window.wStruct.lpszClass = SUS_DEFWNDNAMEW;
	}
	return window;
}
// Build a window
BOOL SUSAPI susBuildWindowA(_Inout_ SUS_LPWINDOW_STRUCTA window)
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
		return FALSE;
	}
	SUS_PRINTDL("The window has been successfully created");
	return TRUE;
}
// Build a window
BOOL SUSAPI susBuildWindowW(_Inout_ SUS_LPWINDOW_STRUCTW window)
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
		return FALSE;
	}
	SUS_PRINTDL("The window has been successfully created");
	return TRUE;
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
	sus_memcpy((LPBYTE)&wg.wStruct, (CONST LPBYTE) & susDefWidgetStructA, sizeof(CREATESTRUCTA));
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
	sus_memcpy((LPBYTE)&wg.wStruct, (CONST LPBYTE) & susDefWidgetStructW, sizeof(CREATESTRUCTA));
	wg.wStruct.style |= WS_CHILD | WS_VISIBLE | style;
	wg.wStruct.lpszName = lpTitle;
	wg.wStruct.lpszClass = lpszClass;
	wg.wStruct.hMenu = (HMENU)(INT_PTR)id;
	return wg;
}

// Build a widget
BOOL SUSAPI susBuildWidgetA(_In_ HWND hWnd, _Inout_ SUS_LPWIDGETA widget)
{
	SUS_PRINTDL("Creating a widget");
	SUS_ASSERT(widget);
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
		return FALSE;
	}
	SUS_PRINTDL("The widget has been successfully created");
	return TRUE;
}
// Build a widget
BOOL SUSAPI susBuildWidgetW(_In_ HWND hWnd, _Inout_ SUS_LPWIDGETW widget)
{
	SUS_PRINTDL("Creating a widget");
	SUS_ASSERT(widget);
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
		return FALSE;
	}
	SUS_PRINTDL("The widget has been successfully created");
	return TRUE;
}