// window.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/math.h"
#include "include/susfwk/windowbase.h"
#include "include/susfwk/window.h"

// ================================================================================================= //
// ************************************************************************************************* //
//										   WORKING WITH WINDOWS										 //
/*									Window operation and configuration								 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

//
typedef struct sus_window_fullscreen {
	DWORD		oldStyles;
	DWORD		oldExStyles;
	SUS_BOUNDS	oldBounds;
	BOOL		enabled;
} SUS_WINDOW_FULLSCREEN;

// Window status
typedef struct sus_window_state {
	BOOL						mouseTracked;
} SUS_WINDOW_STATE;
// Window Configuration
typedef struct sus_window_config {
	SUS_WINDOW_CLOSE_OPERATION	closeOperation;
	SUS_WINDOW_FULLSCREEN		fullScreen;
} SUS_WINDOW_CONFIG;
// Window Handlers
typedef struct sus_window_handlers {
	SUS_WINDOW_HANDLER			handler;
	SUS_WINDOW_MOUSE_LISTENERS	mouseListener;
	SUS_KEYBOARD_LISTENER		keyboardListener;
	SUS_WINDOW_TIMER_LISTENER	timerListener;
} SUS_WINDOW_HANDLERS;
// Window structure
struct sus_window {
	HWND						_PARENT_;
	SUS_WINDOW_HANDLERS			handlers;
	SUS_WINDOW_STATE			state;
	SUS_WINDOW_CONFIG			config;
	SUS_OBJECT					userData;
};

// Allocate memory for the window
static SUS_WINDOW SUSAPI susAllocWindow(_In_ SUS_WINDOW_HANDLER handler, _In_opt_ SUS_OBJECT userData) {
	SUS_WINDOW_STRUCT window = { .handlers.handler = handler, .userData = userData };
	return sus_newmem(sizeof(SUS_WINDOW_STRUCT), &window);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Working with the Input window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Get global modifiers
DWORD SUSAPI susSystemGetModifier()
{
	DWORD modifier = 0;
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000) modifier |= SUS_MODIFIER_KEY_SHIFT;
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000) modifier |= SUS_MODIFIER_KEY_CTRL;
	if (GetAsyncKeyState(VK_MENU) & 0x8000) modifier |= SUS_MODIFIER_KEY_ALT;
	if (GetKeyState(VK_CAPITAL) & 0x0001) modifier |= SUS_MODIFIER_KEY_CAPSLOCK;
	if (GetKeyState(VK_NUMLOCK) & 0x0001) modifier |= SUS_MODIFIER_KEY_NUMLOCK;
	return modifier;
}

// -------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 28159)

// Checking whether a click is pressed
static BOOL SUSAPI susSystemMouseIsClick(_In_ SUS_MOUSE_BUTTON button, _In_ SUS_MOUSE_MESSAGE msg) {
	static struct {
		DWORD time; UINT count;
		SUS_MOUSE_BUTTON button;
	} lastPress = { 0 };
	switch (msg)
	{
	case SUS_MOUSE_MESSAGE_PRESS: {
		lastPress.time = GetTickCount();
		lastPress.button = button;
	} return FALSE;
	case SUS_MOUSE_MESSAGE_RELEASE: {
		return (button == lastPress.button) && (GetTickCount() - lastPress.time < GetDoubleClickTime() / 2);
	}
	default: return FALSE;
	}
}
// Get the number of clicks in a series
static UINT SUSAPI susSystemGetClickCount(_In_ SUS_POINT pos, _In_ SUS_MOUSE_BUTTON button) {
	static struct {
		DWORD time; SUS_POINT pos;
		UINT count; SUS_MOUSE_BUTTON button;
	} lastClick = { 0 };
	DWORD currTime = GetTickCount();
	if (!lastClick.time) {
		lastClick.time = currTime; lastClick.pos = pos;
		lastClick.button = button; lastClick.count = 0;
		return ++lastClick.count;
	}
	SIZE dbClickRound = (SIZE){ GetSystemMetrics(SM_CXDOUBLECLK), GetSystemMetrics(SM_CYDOUBLECLK) };
	if ((lastClick.button == button) && (currTime - lastClick.time < GetDoubleClickTime()) &&
		((sus_abs(lastClick.pos.x - pos.x) <= dbClickRound.cx) && (sus_abs(lastClick.pos.y - pos.y) <= dbClickRound.cy))) {
		return ++lastClick.count;
	}
	lastClick.time = 0;
	return susSystemGetClickCount(pos, button);
}

// -------------------------------------------------

// Call the mouse handler
static LRESULT SUSAPI susWindowTrackMouseEvent(_In_ SUS_WINDOW window, _In_ SUS_MOUSE_MESSAGE msg, _In_ SUS_MOUSE_BUTTON button, _In_ LPARAM lParam) {
	SUS_ASSERT(window && msg);
	if (!window->handlers.mouseListener.mouse) return 0;
	SUS_MOUSE_EVENT event = { .position = (SUS_POINT) { LOWORD(lParam), HIWORD(lParam) }, .specific = button, .time = GetTickCount64(), .modifier = susSystemGetModifier() };
	if (susSystemMouseIsClick(button, msg)) msg = SUS_MOUSE_MESSAGE_CLICK;
	if (msg == SUS_MOUSE_MESSAGE_CLICK) event.clickCount = susSystemGetClickCount(event.position, button);
	window->handlers.mouseListener.mouse(window, msg, event);
	return 0;
}
// Call the mouse handler
static LRESULT SUSAPI susWindowTrackMouseWheelEvent(_In_ SUS_WINDOW window, _In_ SUS_MOUSE_WHEEL wheel, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	SUS_ASSERT(window && wheel);
	if (!window->handlers.mouseListener.wheel) return 0;
	ScreenToClient(window->super, (LPPOINT)&lParam);
	SUS_MOUSE_EVENT event = { .position = (SUS_POINT) { LOWORD(lParam), HIWORD(lParam) }, .specific.wheel = wheel, .time = GetTickCount64(), .modifier = susSystemGetModifier(), .wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA };
	window->handlers.mouseListener.wheel(window, event);
	return 0;
}
// Call the mouse handler
static LRESULT SUSAPI susWindowTrackMouseMotionEvent(_In_ SUS_WINDOW window, _In_ SUS_MOUSE_MOTION_MESSAGE msg, _In_ LPARAM lParam) {
	SUS_ASSERT(window && msg);
	if (!window->handlers.mouseListener.motion) return 0;
	SUS_MOUSE_EVENT event = { .position = (SUS_POINT) { LOWORD(lParam), HIWORD(lParam) }, .time = GetTickCount64(), .modifier = susSystemGetModifier() };
	window->handlers.mouseListener.motion(window, msg, event);
	return 0;
}

#pragma warning(pop)

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Call the keyboard handler
static LRESULT SUSAPI susWindowTrackKeyboardEvent(_In_ SUS_WINDOW window, _In_ SUS_KEYBOARD_MESSAGE msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	SUS_ASSERT(window && msg);
	if (!window->handlers.keyboardListener) return 0;
	SUS_KEYBOARD_EVENT event = { .key.as_virtual = (DWORD)wParam, .repeatCount = LOWORD(lParam), .scanCode = (lParam >> 16) & 0xFF, .modifier = susSystemGetModifier() };
	window->handlers.keyboardListener(window, msg, event);
	return 0;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Window Event handler implementation							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Call the main window handler
#define susWindowCallHandler(window, msg, param) (LRESULT)((window)->handlers.handler(window, msg, (LPARAM)(param)))

// Send a message to the window
LRESULT SUSAPI susWindowSendMessage(_In_ SUS_WINDOW window, _In_ UINT msg, LPARAM param) {
	SUS_ASSERT(window && window->super && msg >= SUS_WNDMSG_USER);
	return SendMessageW(window->super, msg - SUS_WNDMSG_USER + WM_USER, 0, param);
}
// Send a message to the window later
BOOL SUSAPI susWindowPostMessage(_In_ SUS_WINDOW window, _In_ UINT msg, LPARAM param) {
	SUS_ASSERT(window && window->super && msg >= SUS_WNDMSG_USER);
	return PostMessageW(window->super, msg - SUS_WNDMSG_USER + WM_USER, 0, param);
}

// The main window handler
LRESULT WINAPI susWindowSystemHandler(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	SUS_WINDOW wnd = susWindowLoadData(hWnd);
	switch (uMsg)
	{
	// ---------------- Mouse handler ----------------
	case WM_LBUTTONDOWN: susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_LEFT, lParam); break;
	case WM_RBUTTONDOWN: susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_RIGHT, lParam); break;
	case WM_MBUTTONDOWN: susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_MIDDLE, lParam); break;
	case WM_LBUTTONUP: susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_LEFT, lParam); break;
	case WM_RBUTTONUP: susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_RIGHT, lParam); break;
	case WM_MBUTTONUP: susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_MIDDLE, lParam); break;
	case WM_MOUSEMOVE: {
		if (wnd->handlers.mouseListener.motion && !wnd->state.mouseTracked) {
			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), .dwFlags = TME_LEAVE | TME_HOVER, .hwndTrack = hWnd, .dwHoverTime = HOVER_DEFAULT };
			TrackMouseEvent(&tme);
			wnd->state.mouseTracked = TRUE;
			susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_ENTER, SUS_MOUSE_BUTTON_NONE, lParam);
		}
		else susWindowTrackMouseMotionEvent(wnd, SUS_MOUSE_MOTION_MESSAGE_MOVE, lParam);
	} break;
	case WM_MOUSELEAVE: wnd->state.mouseTracked = FALSE; susWindowTrackMouseEvent(wnd, SUS_MOUSE_MESSAGE_EXIT, SUS_MOUSE_BUTTON_NONE, lParam); break;
	case WM_MOUSEHOVER: susWindowTrackMouseMotionEvent(wnd, SUS_MOUSE_MOTION_MESSAGE_HOVER, lParam); break;
	case WM_MOUSEWHEEL: susWindowTrackMouseWheelEvent(wnd, SUS_MOUSE_WHEEL_VERTICAL, wParam, lParam); break;
	case WM_MOUSEHWHEEL: susWindowTrackMouseWheelEvent(wnd, SUS_MOUSE_WHEEL_HORIZONTAL, wParam, lParam); break;
	// ---------------- Keyboard handler ----------------
	case WM_KEYDOWN: susWindowTrackKeyboardEvent(wnd, SUS_KEYBOARD_MESSAGE_PRESS, wParam, lParam); break;
	case WM_KEYUP: susWindowTrackKeyboardEvent(wnd, SUS_KEYBOARD_MESSAGE_RELEASE, wParam, lParam); break;
	case WM_CHAR: susWindowTrackKeyboardEvent(wnd, SUS_KEYBOARD_MESSAGE_TYPE, wParam, lParam); break;
	// ---------------- Main handler ----------------
	case WM_SIZE: {
		for (HWND hChild = GetWindow(hWnd, GW_CHILD); hChild; hChild = GetNextWindow(hChild, GW_HWNDNEXT)) {
			// TODO: Layouy
		}
		switch (wParam)
		{
		case SIZE_MAXIMIZED: susWindowCallHandler(wnd, SUS_WNDMSG_MAXIMIZED, NULL); break;
		case SIZE_MINIMIZED: susWindowCallHandler(wnd, SUS_WNDMSG_MINIMIZED, NULL); break;
		default: susWindowCallHandler(wnd, SUS_WNDMSG_RESIZE, lParam); break;
		}
	} break;
	case WM_MOVE:		susWindowCallHandler(wnd, SUS_WNDMSG_MOVE, lParam); break;
	case WM_SETFOCUS:	susWindowCallHandler(wnd, SUS_WNDMSG_FOCUS, TRUE); break;
	case WM_KILLFOCUS:	susWindowCallHandler(wnd, SUS_WNDMSG_FOCUS, FALSE); break;
	case WM_CLOSE: {
		if (susWindowCallHandler(wnd, SUS_WNDMSG_CLOSE, NULL));
		else if (wnd->config.closeOperation == SUS_WINDOW_HIDE_ON_CLOSE) susWindowSetVisible(wnd, FALSE);
		else if (wnd->config.closeOperation == SUS_WINDOW_EXIT_ON_CLOSE || wnd->config.closeOperation == SUS_WINDOW_DELETE_ON_CLOSE) DestroyWindow(hWnd);
	}  break;
	case WM_DESTROY:	susWindowCallHandler(wnd, SUS_WNDMSG_DESTROY, NULL); break;
	case WM_CREATE:		susWindowCallHandler(wnd, SUS_WNDMSG_CREATE, lParam); break;
	// ---------------- Timer handler ----------------
	case WM_TIMER: if (wnd->handlers.timerListener) wnd->handlers.timerListener(wnd, (UINT)wParam); break;
	// ---------------------------------------------
	case WM_NCCREATE: {
		wnd = (SUS_WINDOW)susGetWindowParam(lParam);
		if (!wnd) return FALSE;
		susWindowWriteData(hWnd, (LONG_PTR)wnd);
		wnd->super = hWnd;
	} break;
	case WM_NCDESTROY: {
		if (wnd->config.closeOperation == SUS_WINDOW_EXIT_ON_CLOSE) PostQuitMessage(0);
		sus_free(wnd);
	} break;
	default: if (uMsg >= WM_USER) susWindowCallHandler(wnd, uMsg - WM_USER + SUS_WNDMSG_USER, lParam);
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
// Default window handler
static LRESULT SUSAPI susDefaultWindowHandler(SUS_WINDOW window, SUS_WINDOW_MESSAGE msg, LPARAM param) {
	UNREFERENCED_PARAMETER(window);
	UNREFERENCED_PARAMETER(msg);
	UNREFERENCED_PARAMETER(param);
	return 0;
}

// -------------------------------------------------

// The main Window cycle
INT SUSAPI susWindowMainLoop()
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
	window->config.closeOperation = closeOperation;
}

// -------------------------------------------------

// Set a timer
BOOL SUSAPI susWindowSetTimer(_In_ SUS_WINDOW window, _In_ UINT uId, _In_ UINT elapse) {
	SUS_ASSERT(window);
	return SetTimer(window->super, uId, elapse, NULL) ? TRUE : FALSE;
}
// Delete the timer
BOOL SUSAPI susWindowKillTimer(_In_ SUS_WINDOW window, _In_ UINT uId) {
	SUS_ASSERT(window);
	return KillTimer(window->super, uId);
}

// -------------------------------------------------

// Install a mouse listener
VOID SUSAPI susWindowSetMouseListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_LISTENER mouseListener) {
	SUS_ASSERT(window);
	window->handlers.mouseListener.mouse = mouseListener;
}
// Install a mouse motion listener
VOID SUSAPI susWindowSetMouseMotionListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_MOTION_LISTENER mouseMotionListener) {
	SUS_ASSERT(window);
	window->handlers.mouseListener.motion = mouseMotionListener;
}
// Install a mouse wheel listener
VOID SUSAPI susWindowSetMouseWheelListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_WHEEL_LISTENER mouseMotionListener) {
	SUS_ASSERT(window);
	window->handlers.mouseListener.wheel = mouseMotionListener;
}

// Install a keyboard listener
VOID SUSAPI susWindowSetKeyboardListener(_Inout_ SUS_WINDOW window, _In_ SUS_KEYBOARD_LISTENER keyboardListener) {
	SUS_ASSERT(window);
	window->handlers.keyboardListener = keyboardListener;
}
// Install a timer listener
VOID SUSAPI susWindowSetTimerListener(_Inout_ SUS_WINDOW window, _In_ SUS_WINDOW_TIMER_LISTENER timerListener) {
	SUS_ASSERT(window);
	window->handlers.timerListener = timerListener;
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
	susWindowSetVisible(window, FALSE);
	if (enabled && !window->config.fullScreen.enabled) {
		SUS_WINDOW_FULLSCREEN fs = { .oldExStyles = susWindowGetExStyle(window), .oldStyles = susWindowGetStyle(window), .oldBounds = susWindowGetBounds(window), enabled = TRUE };
		window->config.fullScreen = fs;
		susWindowSetStyle(window, WS_POPUP);
		susWindowSetExStyle(window, WS_EX_APPWINDOW);
		return SetWindowPos(window->super, HWND_TOPMOST, 0, 0, susGetScreenSizeX(), susGetScreenSizeY(), SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}
	if (!window->config.fullScreen.enabled) return FALSE;
	window->config.fullScreen.enabled = FALSE;
	susWindowSetStyle(window, window->config.fullScreen.oldStyles);
	susWindowSetExStyle(window, window->config.fullScreen.oldExStyles);
	return SetWindowPos(window->super, HWND_NOTOPMOST, (int)window->config.fullScreen.oldBounds.x, (int)window->config.fullScreen.oldBounds.y, (int)window->config.fullScreen.oldBounds.cx, (int)window->config.fullScreen.oldBounds.cy, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}
// Set the status - always on top
BOOL SUSAPI susWindowSetAlwaysOnTop(_In_ SUS_WINDOW window, _In_ BOOL enabled) {
	SUS_ASSERT(window && window->super);
	return SetWindowPos(window->super, enabled ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

// -------------------------------------------------

// Set the window position
BOOL SUSAPI susWindowSetPosition(_In_ SUS_WINDOW window, _In_ SUS_POINT position) {
	SUS_ASSERT(window && window->super);
	return SetWindowPos(window->super, NULL, (INT)position.x, (INT)position.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
// Set the window size
BOOL SUSAPI susWindowSetSize(_In_ SUS_WINDOW window, _In_ SUS_SIZE size) {
	SUS_ASSERT(window && window->super);
	return SetWindowPos(window->super, NULL, 0, 0, (INT)size.cx, (INT)size.cy, SWP_NOMOVE | SWP_NOZORDER);
}
// Set the position and size of the window
BOOL SUSAPI susWindowSetBounds(_In_ SUS_WINDOW window, _In_ SUS_BOUNDS bounds) {
	SUS_ASSERT(window && window->super);
	return SetWindowPos(window->super, NULL, (INT)bounds.x, (INT)bounds.y, (INT)bounds.cx, (INT)bounds.cy, SWP_NOZORDER);
}
// Get Window frames
SUS_BOUNDS SUSAPI susWindowGetBounds(_In_ SUS_WINDOW window) {
	RECT rect = { 0 };
	GetWindowRect(window->super, &rect);
	return (SUS_BOUNDS) { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
}
// Get the window position
SUS_POINT SUSAPI susWindowGetPosition(_In_ SUS_WINDOW window) {
	SUS_BOUNDS rect = susWindowGetBounds(window);
	return (SUS_POINT) { rect.x, rect.y };
}
// Get the window size
SUS_SIZE SUSAPI susWindowGetSize(_In_ SUS_WINDOW window) {
	SUS_BOUNDS rect = susWindowGetBounds(window);
	return (SUS_SIZE) { rect.cx, rect.cy };
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
BOOL SUSAPI susWindowSetTitle(_In_ SUS_WINDOW window, _In_ LPCWSTR title) {
	SUS_ASSERT(window && window->super);
	return SetWindowTextW(window->super, title);
}
// Get a dynamic window title
LPCWSTR SUSAPI susWindowGetTitle(_In_ SUS_WINDOW window) {
	static WCHAR titleBuffer[128] = { 0 };
	GetWindowTextW(window->super, titleBuffer, sizeof(titleBuffer) / sizeof(WCHAR) - 1);
	return titleBuffer;
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
BOOL SUSAPI susWindowSetProperty(_In_ SUS_WINDOW window, _In_ LPCWSTR key, _In_ LONG_PTR value) {
	SUS_ASSERT(window && window->super);
	return SetPropW(window->super, key, (HANDLE)value);
}
// Get a window property
LONG_PTR SUSAPI susWindowGetProperty(_In_ SUS_WINDOW window, _In_ LPCWSTR key) {
	SUS_ASSERT(window && window->super);
	return (LONG_PTR)GetPropW(window->super, key);
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
// Get the system window handle
HWND SUSAPI susWindowGetSuper(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window);
	return window->super;
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

#define SUS_WCNAMEW L"suswc"

// Standard window class
static const WNDCLASSEXW susDefWndClass = {
	.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
	.lpszClassName = SUS_WCNAMEW,
	.cbSize = sizeof(WNDCLASSEXW),
	.lpfnWndProc = susWindowSystemHandler
};

// -------------------------------------------------

static const CREATESTRUCTW susDefWndStruct = {
	.lpszClass = SUS_WCNAMEW,
	.lpszName = L"",
	.style = WS_OVERLAPPEDWINDOW,
	.x = CW_USEDEFAULT,
	.y = CW_USEDEFAULT,
	.cx = CW_USEDEFAULT,
	.cy = CW_USEDEFAULT
};

// -------------------------------------------------

static const CREATESTRUCTW susDefWidgetStruct = {
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
SUS_WINDOW_BUILDER SUSAPI susWindowBuilder(_In_opt_ LPCWSTR lpTitle)
{
	SUS_PRINTDL("Setting up the window");
	SUS_ASSERT(lpTitle);
	SUS_WINDOW_BUILDER builder = { 0 };
	builder.wcEx = susDefWndClass;
	builder.wcEx.lpszClassName = builder.classNameBuffer;
	builder.wcEx.hInstance = GetModuleHandleW(NULL);
	builder.wStruct = susDefWndStruct;
	builder.wStruct.hInstance = GetModuleHandleW(NULL);
	if (lpTitle) builder.wStruct.lpszName = lpTitle;
	builder.wStruct.lpszClass = builder.classNameBuffer;
	sus_formattingW(builder.classNameBuffer, L"%s-%s%p", SUS_WCNAMEW, lpTitle, &builder);
	return builder;
}

// Build a window
SUS_WINDOW SUSAPI susBuildWindow(_In_ SUS_WINDOW_BUILDER builder, _In_opt_ SUS_WINDOW_HANDLER handler)
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
	SUS_WINDOW window = susAllocWindow(handler ? handler : susDefaultWindowHandler, builder.wStruct.lpCreateParams);
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
		sus_free(window);
		return NULL;
	}
	UpdateWindow(window->super);
	SUS_PRINTDL("The window has been successfully created");
	return window;
}

// Create a window
SUS_WINDOW SUSAPI susNewWindow(_In_opt_ LPCWSTR title, _In_ SIZE size, _In_opt_ SUS_WINDOW_HANDLER handler)
{
	SUS_WINDOW_BUILDER builder = susWindowBuilder(title);
	susWinBuilderSetSize((SUS_LPWINDOW_BUILDER)&builder, size);
	SUS_WINDOW wnd = susBuildWindow(builder, handler);
	if (!wnd) return NULL;
	susWindowSetVisible(wnd, TRUE);
	return wnd;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
