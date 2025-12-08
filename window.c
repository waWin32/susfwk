// window.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/math.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/window.h"

// ================================================================================================= //
// ************************************************************************************************* //
//											BASIC DEFINITIONS										 //
/*										Basic structures for windows								 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//								 Internal system wrappers								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Get userData parameters by lParam
static SUS_OBJECT susWindowGetParam(LPARAM lParam) {
	CONST LPCREATESTRUCT pCreate = (CONST LPCREATESTRUCT)lParam;
	return pCreate->lpCreateParams;
}
// Save data to a window
static SUS_OBJECT susWindowWriteData(HWND hWnd, LONG_PTR dwNewLong) {
	SetWindowLongPtr(hWnd, GWLP_USERDATA, dwNewLong);
	return (SUS_OBJECT)dwNewLong;
}
// Get a window from a window procedure
static SUS_OBJECT susWindowLoadData(HWND hWnd) {
	return (SUS_OBJECT)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											 WINDOW BUILDERS										 //
/*									  Building and creating a window								 */
// ************************************************************************************************* //
// ================================================================================================= //

// -------------------------------------------------

// Window structure
struct sus_window {
	HWND						_PARENT_;
	SUS_WINDOW_CLOSE_OPERATION	closeOperation;
	SUS_WINDOW_HANDLER			handler;
	SUS_MOUSE_LISTENER			mouseListener;
	SUS_KEYBOARD_LISTENER		keyboardListener;
	SUS_OBJECT					userData;
};

// -------------------------------------------------


// -------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//							Working with the Input window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Get global modifiers
DWORD SUSAPI susSystemGetModifier()
{
	DWORD modifier = 0;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		if (GetAsyncKeyState(VK_RSHIFT) & 0x8000) modifier |= SUS_MODIFIER_KEY_RSHIFT;
		if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) modifier |= SUS_MODIFIER_KEY_LSHIFT;
	}
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
		if (GetAsyncKeyState(VK_RCONTROL) & 0x8000) modifier |= SUS_MODIFIER_KEY_RCTRL;
		if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) modifier |= SUS_MODIFIER_KEY_LCTRL;
	}
	if (GetAsyncKeyState(VK_MENU) & 0x8000) {
		if (GetAsyncKeyState(VK_RMENU) & 0x8000) modifier |= SUS_MODIFIER_KEY_RALT;
		if (GetAsyncKeyState(VK_LMENU) & 0x8000) modifier |= SUS_MODIFIER_KEY_LALT;
	}
	if (GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000) modifier |= SUS_MODIFIER_KEY_WIN;
	if (GetKeyState(VK_CAPITAL) & 0x0001) modifier |= SUS_MODIFIER_KEY_CAPSLOCK;
	if (GetKeyState(VK_NUMLOCK) & 0x0001) modifier |= SUS_MODIFIER_KEY_NUMLOCK;
	return modifier;
}

// -------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 28159)

// Checking whether a click is pressed
static BOOL SUSAPI susSystemMouseIsClick(_In_ POINT pos, _In_ SUS_MOUSE_BUTTON button, _In_ SUS_MOUSE_MESSAGE msg) {
	static struct { DWORD time; POINT pos; SUS_MOUSE_BUTTON button; } lastPress = { 0 };
	switch (msg)
	{
	case SUS_MOUSE_MESSAGE_PRESS: {
		lastPress.time = GetTickCount();
		lastPress.pos = pos;
		lastPress.button = button;
	} return FALSE;
	case SUS_MOUSE_MESSAGE_RELEASE: {
		SIZE ñlickRound = (SIZE){ GetSystemMetrics(SM_CXDOUBLECLK) / 2, GetSystemMetrics(SM_CYDOUBLECLK) / 2 };
		return (button == lastPress.button) && ((sus_abs(lastPress.pos.x - pos.x) <= ñlickRound.cx) && (sus_abs(lastPress.pos.y - pos.y) <= ñlickRound.cy)) && (GetTickCount() - lastPress.time < GetDoubleClickTime() / 2);
	}
	default: return FALSE;
	}
}
// Get the number of clicks in a series
static UINT SUSAPI susSystemGetClickCount(_In_ POINT pos, _In_ SUS_MOUSE_BUTTON button) {
	static struct {
		DWORD time; UINT count;
		POINT pos; SUS_MOUSE_BUTTON button;
	} lastClick = { 0 };
	DWORD currTime = GetTickCount();
	if (!lastClick.time) {
		lastClick.time = currTime; lastClick.pos = pos;
		lastClick.button = button; lastClick.count = 0;
	}
	SIZE dbClickRound = (SIZE){ GetSystemMetrics(SM_CXDOUBLECLK), GetSystemMetrics(SM_CYDOUBLECLK) };
	if ((lastClick.button == button) && (currTime - lastClick.time < GetDoubleClickTime()) &&
		((sus_abs(lastClick.pos.x - pos.x) <= dbClickRound.cx) && (sus_abs(lastClick.pos.y - pos.y) <= dbClickRound.cy))) {
		return ++lastClick.count;
	}
	lastClick.time = 0;
	return susSystemGetClickCount(pos, button);
}
// Call the mouse handler
static LRESULT SUSAPI susWindowTrackMouseEvent(_In_ SUS_WINDOW window, _In_ SUS_MOUSE_MESSAGE msg, _In_ SUS_MOUSE_BUTTON button, _In_ LPARAM lParam) {
	SUS_ASSERT(window);
	if (!window->mouseListener) return 0;
	SUS_MOUSE_EVENT event = { .position = (POINT) { LOWORD(lParam), HIWORD(lParam) }, .button = button, .time = GetTickCount64(), .modifier = susSystemGetModifier() };
	if (susSystemMouseIsClick(event.position, button, msg)) msg = SUS_MOUSE_MESSAGE_CLICK;
	if (msg == SUS_MOUSE_MESSAGE_CLICK) event.clickCount = susSystemGetClickCount(event.position, button);
	window->mouseListener(window, msg, event);
	return 0;
}

#pragma warning(pop)

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Call the keyboard handler
static LRESULT SUSAPI susWindowTrackKeyboardEvent(_In_ SUS_WINDOW window, _In_ SUS_KEYBOARD_MESSAGE msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	SUS_ASSERT(window);
	if (!window->keyboardListener) return 0;
	SUS_KEYBOARD_EVENT event = { .key.as_virtual = (DWORD)wParam, .repeatCount = LOWORD(lParam), .scanCode = (lParam >> 16) & 0xFF, .modifier = susSystemGetModifier() };
	window->keyboardListener(window, msg, event);
	return 0;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							The Layout system for the window							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The layout system on the window
struct sus_window_layout {
	SUS_LAYOUT_TYPE type;

};

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Window Event handler implementation							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Call the main window handler
#define susWindowCallHandler(window, msg, param) (LRESULT)((window)->handler(window, msg, (LPARAM)(param)))

// The main window handler
LRESULT WINAPI susWindowHandler(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	SUS_WINDOW wnd = susWindowLoadData(hWnd);
	switch (uMsg)
	{
	case WM_NCCREATE: {
		wnd = susWindowWriteData(hWnd, (LONG_PTR)susWindowGetParam(lParam));
		wnd->super = hWnd;
	} return TRUE;
	// ---------------- Main handler ----------------
	case WM_CREATE:		return susWindowCallHandler(wnd, SUS_WNDMSG_ON_CREATE, NULL);
	case WM_SIZE:		return susWindowCallHandler(wnd, SUS_WNDMSG_ON_RESIZE, lParam);
	case WM_MOVE:		return susWindowCallHandler(wnd, SUS_WNDMSG_ON_MOVE, lParam);
	case WM_SETFOCUS:	return susWindowCallHandler(wnd, SUS_WNDMSG_ON_FOCUS, TRUE);
	case WM_KILLFOCUS:	return susWindowCallHandler(wnd, SUS_WNDMSG_ON_FOCUS, FALSE);
	case WM_CLOSE: {
		if (susWindowCallHandler(wnd, SUS_WNDMSG_ON_CLOSE, NULL)) return 1;
		if (wnd->closeOperation == SUS_WINDOW_HIDE_ON_CLOSE) susWindowSetVisible(wnd, FALSE);
		else if (wnd->closeOperation == SUS_WINDOW_EXIT_ON_CLOSE || wnd->closeOperation == SUS_WINDOW_DELETE_ON_CLOSE) return !DestroyWindow(hWnd);
	} return 0;
	case WM_DESTROY:	return susWindowCallHandler(wnd, SUS_WNDMSG_ON_DESTROY, NULL);
	// ---------------- Mouse handler ----------------
	case WM_LBUTTONDOWN: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_LEFT, lParam);
	case WM_RBUTTONDOWN: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_RIGHT, lParam);
	case WM_MBUTTONDOWN: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_MIDDLE, lParam);
	case WM_LBUTTONUP: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_LEFT, lParam);
	case WM_RBUTTONUP: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_RIGHT, lParam);
	case WM_MBUTTONUP: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_MIDDLE, lParam);
	case WM_MOUSELEAVE: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_EXIT, SUS_MOUSE_BUTTON_NONE, lParam);
	case WM_MOUSEHOVER: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_ENTER, SUS_MOUSE_BUTTON_NONE, lParam);
	case WM_MOUSEMOVE: return susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_MOVE, SUS_MOUSE_BUTTON_NONE, lParam);
	// ---------------- Keyboard handler ----------------
	case WM_KEYDOWN: return susWindowTrackKeyboardEvent(wnd, SUS_KEYBOARD_MESSAGE_PRESS, wParam, lParam);
	case WM_KEYUP: return susWindowTrackKeyboardEvent(wnd, SUS_KEYBOARD_MESSAGE_RELEASE, wParam, lParam);
	case WM_CHAR: return susWindowTrackKeyboardEvent(wnd, SUS_KEYBOARD_MESSAGE_TYPE, wParam, lParam);
	// ---------------------------------------------
	case WM_NCDESTROY: {
		if (wnd->closeOperation == SUS_WINDOW_EXIT_ON_CLOSE) PostQuitMessage(0);
		sus_free(wnd);
	} return 0;
	default: return uMsg >= WM_USER ? susWindowCallHandler(wnd, uMsg, lParam) : DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

// -------------------------------------------------

// Default window handler
static LRESULT SUSAPI susDefWindowHandler(SUS_WINDOW widget, SUS_WINDOW_MESSAGE msg, LPARAM param) {
	UNREFERENCED_PARAMETER(widget);
	UNREFERENCED_PARAMETER(msg);
	UNREFERENCED_PARAMETER(param);
	return 0;
}

// -------------------------------------------------

// The main Window cycle
INT SUSAPI susWindowMainLoopA()
{
	SUS_PRINTDL("The main window cycle");
	MSG msg = { 0 };
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
	MSG msg = { 0 };
	while (GetMessageW(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return (int)msg.wParam;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//								 Working with the window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set a standard action when closing a window
VOID SUSAPI susWindowSetCloseOperation(_Inout_ SUS_WINDOW window, _In_ SUS_WINDOW_CLOSE_OPERATION closeOperation) {
	SUS_ASSERT(window);
	window->closeOperation = closeOperation;
}

// -------------------------------------------------

// Set extended window styles
BOOL SUSAPI susWindowSetExStyle(_Inout_ SUS_WINDOW window, _In_ DWORD styles) {
	SUS_ASSERT(window && window->super);
	return (BOOL)(SetWindowLongPtr(window->super, GWL_EXSTYLE, styles) ? TRUE : FALSE);
}
// Get extended window styles
DWORD SUSAPI susWindowGetExStyle(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window && window->super);
	return (DWORD)GetWindowLongPtr(window->super, GWL_EXSTYLE);
}
// Set window styles
BOOL SUSAPI susWindowSetStyle(_Inout_ SUS_WINDOW window, _In_ DWORD styles) {
	SUS_ASSERT(window && window->super);
	return (BOOL)(SetWindowLongPtr(window->super, GWL_STYLE, styles) ? TRUE : FALSE);
}
// Get window styles
DWORD SUSAPI susWindowGetStyle(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window && window->super);
	return (DWORD)GetWindowLongPtr(window->super, GWL_STYLE);
}

// -------------------------------------------------

// Set window visibility
BOOL SUSAPI susWindowSetVisible(_In_ SUS_WINDOW window, _In_ BOOL enabled) {
	SUS_ASSERT(window && window->super);
	return ShowWindow(window->super, enabled ? SW_SHOW : SW_HIDE);
}
// Minimize the window
BOOL SUSAPI susWindowMinimize(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window && window->super);
	return ShowWindow(window->super, SW_MINIMIZE);
}
// Expand the window
BOOL SUSAPI susWindowMaximize(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window && window->super);
	return ShowWindow(window->super, SW_MAXIMIZE);
}
// Restore the window after maximizing/minimizing
BOOL SUSAPI susWindowRestore(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window && window->super);
	return ShowWindow(window->super, SW_RESTORE);
}
// Set Fullscreen for the window
BOOL SUSAPI susWindowFullscreen(_In_ SUS_WINDOW window, _In_ BOOL enabled) {
	SUS_ASSERT(window && window->super);
	if (enabled) {
		susWindowSetStyle(window, susWindowGetStyle(window) | WS_POPUP);
		susWindowSetExStyle(window, susWindowGetExStyle(window) | WS_EX_TOPMOST);
		return SetWindowPos(window->super, HWND_TOPMOST, 0, 0, susGetScreenSizeX(), susGetScreenSizeY(), SWP_FRAMECHANGED);
	}
	susWindowSetStyle(window, susWindowGetStyle(window) & ~WS_POPUP);
	susWindowSetExStyle(window, susWindowGetExStyle(window) & ~WS_EX_TOPMOST);
	return SetWindowPos(window->super, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}
// Set the status - always on top
BOOL SUSAPI susWindowSetAlwaysOnTop(_In_ SUS_WINDOW window, _In_ BOOL enabled) {
	SUS_ASSERT(window && window->super);
	susWindowSetExStyle(window, susWindowGetExStyle(window) | WS_EX_TOPMOST);
	return SetWindowPos(window->super, enabled ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

// -------------------------------------------------

// Set the window position
BOOL SUSAPI susWindowSetPosition(_In_ SUS_WINDOW window, _In_ POINT position) {
	SUS_ASSERT(window && window->super);
	return SetWindowPos(window->super, NULL, position.x, position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
// Set the window size
BOOL SUSAPI susWindowSetSize(_In_ SUS_WINDOW window, _In_ SIZE size) {
	SUS_ASSERT(window && window->super);
	return SetWindowPos(window->super, NULL, 0, 0, size.cx, size.cy, SWP_NOMOVE | SWP_NOZORDER);
}
// Set the position and size of the window
BOOL SUSAPI susWindowSetBounds(_In_ SUS_WINDOW window, _In_ RECT bounds) {
	SUS_ASSERT(window && window->super);
	return SetWindowPos(window->super, NULL, bounds.left, bounds.top, bounds.right, bounds.bottom, SWP_NOZORDER);
}
// Get Window frames
RECT SUSAPI susWindowGetBounds(_In_ SUS_WINDOW window) {
	RECT rect = { 0 };
	GetWindowRect(window->super, &rect);
	return (RECT) { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
}
// Get the window position
POINT SUSAPI susWindowGetPosition(_In_ SUS_WINDOW window) {
	RECT rect = susWindowGetBounds(window);
	return (POINT) { rect.left, rect.top };
}
// Get the window size
SIZE SUSAPI susWindowGetSize(_In_ SUS_WINDOW window) {
	RECT rect = susWindowGetBounds(window);
	return (SIZE) { rect.right, rect.bottom };
}

// -------------------------------------------------

// Set the cursor for the window
HCURSOR SUSAPI susWindowSetCursor(_In_ SUS_WINDOW window, _In_ HCURSOR hCursor) {
	SUS_ASSERT(window && window->super && hCursor);
	return SetCursor(hCursor);
}
// Set the icon for the window
HICON SUSAPI susWindowSetIcon(_In_ SUS_WINDOW window, _In_ HICON hIcon) {
	SUS_ASSERT(window && window->super && hIcon);
	return (HICON)SendMessage(window->super, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}
// Set the Menu for the window
BOOL SUSAPI susWindowSetMenuBar(_In_ SUS_WINDOW window, _In_ HMENU hMenu) {
	SUS_ASSERT(window && window->super && hMenu);
	return SetMenu(window->super, hMenu);
}
// Set transparency for the window (From 0 to 1)
BOOL SUSAPI susWindowSetTransparency(_In_ SUS_WINDOW window, _In_ sus_float transparency) {
	SUS_ASSERT(window && window->super);
	transparency = sus_clamp(transparency, 0.0f, 1.0f);
	DWORD exstyle = (DWORD)GetWindowLongPtr(window->super, GWL_EXSTYLE);
	if (transparency >= 1.0f - SUS_EPSILON) return SetWindowLongPtr(window->super, GWL_EXSTYLE, exstyle & ~WS_EX_LAYERED) ? TRUE : FALSE;
	if (!(exstyle & WS_EX_LAYERED)) SetWindowLongPtr(window->super, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);
	return SetLayeredWindowAttributes(window->super, 0, (BYTE)((1.0f - transparency) * 255.0f), LWA_ALPHA);
}
// Set the title for the window
BOOL SUSAPI susWindowSetTitleA(_In_ SUS_WINDOW window, _In_ LPCSTR title) {
	SUS_ASSERT(window && window->super);
	return SetWindowTextA(window->super, title);
}
// Set the title for the window
BOOL SUSAPI susWindowSetTitleW(_In_ SUS_WINDOW window, _In_ LPCWSTR title) {
	SUS_ASSERT(window && window->super);
	return SetWindowTextW(window->super, title);
}
// Get a static window title
LPCSTR SUSAPI susWindowGetTitleA(_In_ SUS_WINDOW window) {
	static CHAR titleBuffer[128] = { 0 };
	GetWindowTextA(window->super, titleBuffer, sizeof(titleBuffer) / sizeof(CHAR) - 1);
	return titleBuffer;
}
// Get a static window title
LPCWSTR SUSAPI susWindowGetTitleW(_In_ SUS_WINDOW window) {
	static WCHAR titleBuffer[128] = { 0 };
	GetWindowTextW(window->super, titleBuffer, sizeof(titleBuffer) / sizeof(WCHAR) - 1);
	return titleBuffer;
}

// -------------------------------------------------

// Set a timer
BOOL SUSAPI susWindowSetTimer(_In_ SUS_WINDOW window, _In_ UINT uId, _In_ UINT Elapse, _In_ TIMERPROC lpTimerFunc) {
	SUS_ASSERT(window && lpTimerFunc);
	return SetTimer(window->super, uId, Elapse, lpTimerFunc) ? TRUE : FALSE;
}
// Delete the timer
BOOL SUSAPI susWindowKillTimer(_In_ SUS_WINDOW window, _In_ UINT uId) {
	SUS_ASSERT(window);
	return KillTimer(window->super, uId);
}

// -------------------------------------------------

// Set your window data
SUS_OBJECT SUSAPI susWindowSetUserData(_In_ SUS_WINDOW window, _In_ SUS_OBJECT userData) {
	SUS_ASSERT(window);
	return window->userData = userData;
}
// Get your window information
SUS_OBJECT SUSAPI susWindowGetUserData(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window);
	return window->userData;
}

// Set a property for a window
BOOL SUSAPI susWindowSetProperty(_In_ SUS_WINDOW window, _In_ LPCSTR key, _In_ LONG_PTR value) {
	SUS_ASSERT(window && window->super);
	return SetPropA(window->super, key, (HANDLE)value);
}
// Get a window property
LONG_PTR SUSAPI susWindowGetProperty(_In_ SUS_WINDOW window, _In_ LPCSTR key) {
	SUS_ASSERT(window && window->super);
	return (LONG_PTR)GetPropA(window->super, key);
}

// -------------------------------------------------

// Install a mouse listener
VOID SUSAPI susWindowSetMouseListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_LISTENER mouseListener) {
	SUS_ASSERT(window);
	window->mouseListener = mouseListener;
}
// Install a keyboard listener
VOID SUSAPI susWindowSetKeyboardListener(_Inout_ SUS_WINDOW window, _In_ SUS_KEYBOARD_LISTENER keyboardListener) {
	SUS_ASSERT(window);
	window->keyboardListener = keyboardListener;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											 WINDOW BUILDERS										 //
/*									  Building and creating a window								 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//						Basic definition of constants and structures					//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

#define SUS_WCNAMEA "suswc"
#define SUS_WCNAMEW L"suswc"

// Standard window class
static const WNDCLASSEXA susDefWndClassA = {
	.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
	.lpszClassName = SUS_WCNAMEA,
	.cbSize = sizeof(WNDCLASSEXA)
};
// Standard window class
static const WNDCLASSEXW susDefWndClassW = {
	.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
	.lpszClassName = SUS_WCNAMEW,
	.cbSize = sizeof(WNDCLASSEXW)
};

// -------------------------------------------------

static const CREATESTRUCTA susDefWndStructA = {
	.lpszClass = SUS_WCNAMEA,
	.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	.x = CW_USEDEFAULT,
	.y = CW_USEDEFAULT,
	.cx = CW_USEDEFAULT,
	.cy = CW_USEDEFAULT
};
static const CREATESTRUCTW susDefWndStructW = {
	.lpszClass = SUS_WCNAMEW,
	.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	.x = CW_USEDEFAULT,
	.y = CW_USEDEFAULT,
	.cx = CW_USEDEFAULT,
	.cy = CW_USEDEFAULT
};

// -------------------------------------------------

static const CREATESTRUCTA susDefWidgetStructA = {
	.style = WS_VISIBLE | WS_CHILD,
	.x = CW_USEDEFAULT,
	.y = CW_USEDEFAULT,
	.cx = 100,
	.cy = 50,
	.lpszClass = "STATIC"
};
static const CREATESTRUCTW susDefWidgetStructW = {
	.style = WS_VISIBLE | WS_CHILD,
	.x = CW_USEDEFAULT,
	.y = CW_USEDEFAULT,
	.cx = 100,
	.cy = 50,
	.lpszClass = L"STATIC"
};

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//								Builders of a simple window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Basic initialization of the window
SUS_WINDOW_BUILDERA SUSAPI susWindowBuilderA(_In_opt_ LPCSTR lpTitle)
{
	SUS_PRINTDL("Setting up the window");
	SUS_ASSERT(lpTitle);
	SUS_WINDOW_BUILDERA window = { 0 };
	window.wcEx = susDefWndClassA;
	window.wStruct = susDefWndStructA;
	window.wcEx.lpfnWndProc = susWindowHandler;
	window.wStruct.hInstance = GetModuleHandleA(NULL);
	window.wStruct.lpszName = lpTitle ? lpTitle : "";
	sus_formattingA(window.classNameBuffer, "%s-%s%pA", SUS_WCNAMEA, lpTitle, &window);
	return window;
}
// Basic initialization of the window
SUS_WINDOW_BUILDERW SUSAPI susWindowBuilderW(_In_opt_ LPCWSTR lpTitle)
{
	SUS_PRINTDL("Setting up the window");
	SUS_ASSERT(lpTitle);
	SUS_WINDOW_BUILDERW window = { 0 };
	window.wcEx = susDefWndClassW;
	window.wStruct = susDefWndStructW;
	window.wcEx.lpfnWndProc = susWindowHandler;
	window.wStruct.hInstance = GetModuleHandleW(NULL);
	window.wStruct.lpszName = lpTitle ? lpTitle : L"";
	sus_formattingW(window.classNameBuffer, L"%s-%s%pW", SUS_WCNAMEW, lpTitle, &window);
	return window;
}

// -------------------------------------------------

// Build a window
SUS_WINDOW SUSAPI susBuildWindowA(_In_ SUS_WINDOW_BUILDERA builder, _In_opt_ SUS_WINDOW_HANDLER handler)
{
	SUS_PRINTDL("Creating a window");
	WNDCLASSA wcData;
	if (!GetClassInfoA(builder.wStruct.hInstance, builder.wcEx.lpszClassName, &wcData)) {
		if (!RegisterClassExA(&builder.wcEx)) {
			SUS_PRINTDE("Failed to register window class");
			SUS_PRINTDC(GetLastError());
			return NULL;
		}
	}
	SUS_WINDOW_STRUCT constructor = { .handler = handler ? handler : susDefWindowHandler, .userData = builder.wStruct.lpCreateParams };
	SUS_WINDOW window = sus_newmem(sizeof(SUS_WINDOW_STRUCT), &constructor);
	if (!window) return NULL;
	if (!CreateWindowExA(
		builder.wStruct.dwExStyle,
		builder.wStruct.lpszClass,
		builder.wStruct.lpszName,
		builder.wStruct.style,
		builder.wStruct.x,
		builder.wStruct.y,
		builder.wStruct.cx,
		builder.wStruct.cy,
		builder.wStruct.hwndParent,
		builder.wStruct.hMenu,
		builder.wStruct.hInstance,
		window
	)) {
		SUS_PRINTDE("Couldn't create a window");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The window has been successfully created");
	return window;
}
// Build a window
SUS_WINDOW SUSAPI susBuildWindowW(_In_ SUS_WINDOW_BUILDERW builder, _In_opt_ SUS_WINDOW_HANDLER handler)
{
	SUS_PRINTDL("Creating a window");
	WNDCLASSW wcData;
	if (!GetClassInfoW(builder.wStruct.hInstance, builder.wcEx.lpszClassName, &wcData)) {
		if (!RegisterClassExW(&builder.wcEx)) {
			SUS_PRINTDE("Failed to register window class");
			SUS_PRINTDC(GetLastError());
			return NULL;
		}
	}
	SUS_WINDOW_STRUCT constructor = { .handler = handler ? handler : susDefWindowHandler, .userData = builder.wStruct.lpCreateParams };
	SUS_WINDOW window = sus_newmem(sizeof(SUS_WINDOW_STRUCT), &constructor);
	if (!window) return NULL;
	if (!CreateWindowExW(
		builder.wStruct.dwExStyle,
		builder.wStruct.lpszClass,
		builder.wStruct.lpszName,
		builder.wStruct.style,
		builder.wStruct.x,
		builder.wStruct.y,
		builder.wStruct.cx,
		builder.wStruct.cy,
		builder.wStruct.hwndParent,
		builder.wStruct.hMenu,
		builder.wStruct.hInstance,
		window
	)) {
		SUS_PRINTDE("Couldn't create a window");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The window has been successfully created");
	return window;
}

// -------------------------------------------------

// Create a window
SUS_WINDOW SUSAPI susNewWindowA(_In_opt_ LPCSTR title, _In_ SIZE size, _In_opt_ SUS_WINDOW_HANDLER handler)
{
	SUS_WINDOW_BUILDERA builder = susWindowBuilderA(title);
	susWinBuilderSetSize(&builder, size);
	SUS_WINDOW wnd = susBuildWindowA(builder, handler);
	if (!wnd) return NULL;
	susWindowSetVisible(wnd, TRUE);
	return wnd;
}
// Create a window
SUS_WINDOW SUSAPI susNewWindowW(_In_opt_ LPCWSTR title, _In_ SIZE size, _In_opt_ SUS_WINDOW_HANDLER handler)
{
	SUS_WINDOW_BUILDERW builder = susWindowBuilderW(title);
	susWinBuilderSetSize((SUS_LPWINDOW_BUILDER)&builder, size);
	SUS_WINDOW wnd = susBuildWindowW(builder, handler);
	if (!wnd) return NULL;
	susWindowSetVisible(wnd, TRUE);
	return wnd;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											 WINDOW BUILDERS										 //
/*									  Building and creating a window								 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									  Widget Builder									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Create a Widget builder
SUS_WIDGET_BUILDERA SUSAPI susWidgetBuilderA(_In_opt_ LPCSTR lpClassName)
{
	SUS_PRINTDL("Setting up the widget");
	SUS_WIDGET_BUILDERA widget = { 0 };
	widget.wStruct = susDefWidgetStructA;
	widget.wStruct.hInstance = GetModuleHandleA(NULL);
	if (lpClassName) widget.wStruct.lpszClass = lpClassName;
	return widget;
}
// Create a Widget builder
SUS_WIDGET_BUILDERW SUSAPI susWidgetBuilderW(_In_opt_ LPCWSTR lpClassName)
{
	SUS_PRINTDL("Setting up the widget");
	SUS_WIDGET_BUILDERW widget = { 0 };
	widget.wStruct = susDefWidgetStructW;
	widget.wStruct.hInstance = GetModuleHandleW(NULL);
	if (lpClassName) widget.wStruct.lpszClass = lpClassName;
	return widget;
}

// -------------------------------------------------

// Build a widget
SUS_WINDOW SUSAPI susBuildWidgetA(_In_opt_ SUS_WINDOW parent, _In_ UINT uId, _In_opt_ SUS_WINDOW_HANDLER handler, _In_ SUS_WIDGET_BUILDERA builder)
{
	SUS_PRINTDL("Creating a widget");
	SUS_WINDOW_STRUCT constructor = { .handler = handler ? handler : susDefWindowHandler, .userData = parent ? parent->userData : NULL };
	SUS_WINDOW window = sus_newmem(sizeof(SUS_WINDOW_STRUCT), &constructor);
	if (!window) return NULL;
	if (!CreateWindowExA(
		builder.wStruct.dwExStyle,
		builder.wStruct.lpszClass,
		builder.wStruct.lpszName,
		builder.wStruct.style,
		builder.wStruct.x,
		builder.wStruct.y,
		builder.wStruct.cx,
		builder.wStruct.cy,
		(HWND)(parent ? parent->super : NULL),
		(HMENU)((LONG_PTR)uId),
		builder.wStruct.hInstance,
		window
	)) {
		SUS_PRINTDE("Couldn't create a widget");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("Widget %s has been successfully created!", builder.wStruct.lpszClass);
	return window;
}
// Build a window
SUS_WINDOW SUSAPI susBuildWidgetW(_In_opt_ SUS_WINDOW parent, _In_ UINT uId, _In_opt_ SUS_WINDOW_HANDLER handler, _In_ SUS_WIDGET_BUILDERW builder)
{
	SUS_PRINTDL("Creating a widget");
	SUS_WINDOW_STRUCT constructor = { .handler = handler ? handler : susDefWindowHandler, .userData = parent ? susWindowLoadData(parent->super) : NULL };
	SUS_WINDOW window = sus_newmem(sizeof(SUS_WINDOW_STRUCT), &constructor);
	if (!window) return NULL;
	if (!CreateWindowExW(
		builder.wStruct.dwExStyle,
		builder.wStruct.lpszClass,
		builder.wStruct.lpszName,
		builder.wStruct.style,
		builder.wStruct.x,
		builder.wStruct.y,
		builder.wStruct.cx,
		builder.wStruct.cy,
		(HWND)(parent ? parent->super : NULL),
		(HMENU)((LONG_PTR)uId),
		builder.wStruct.hInstance,
		window
	)) {
		SUS_PRINTDE("Couldn't create a widget");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_WPRINTDL("Widget %s has been successfully created!", builder.wStruct.lpszClass);
	return window;
}

// -------------------------------------------------

// Create a window
SUS_WINDOW SUSAPI susNewWidgetExA(_In_opt_ SUS_WINDOW parent, _In_opt_ LPCSTR title, _In_ SIZE size, _In_ UINT uId, _In_opt_ SUS_WINDOW_HANDLER handler, _In_opt_ LPCSTR lpClassName, _In_opt_ DWORD styles, _In_opt_ DWORD exStyles)
{
	return susBuildWidgetA(parent, uId, handler,
		susWgSetTitleA(title,
			susWgSetSize(size,
				susWgAddStyle(styles,
					susWgAddExStyle(exStyles,
						susWidgetBuilderA(lpClassName)
					)
				)
			)
		)
	);
}
// Create a window
SUS_WINDOW SUSAPI susNewWidgetExW(_In_opt_ SUS_WINDOW parent, _In_opt_ LPCWSTR title, _In_ SIZE size, _In_ UINT uId, _In_opt_ SUS_WINDOW_HANDLER handler, _In_opt_ LPCWSTR lpClassName, _In_opt_ DWORD styles, _In_opt_ DWORD exStyles)
{
	return susBuildWidgetW(parent, uId, handler,
		susWgSetTitleW(title,
			susWgSetSizeW(size,
				susWgAddStyleW(styles,
					susWgAddExStyleW(exStyles,
						susWidgetBuilderW(lpClassName)
					)
				)
			)
		)
	);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
