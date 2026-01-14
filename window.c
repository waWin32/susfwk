// window.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/math.h"
#include "include/susfwk/tmath.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/graphics.h"
#include "include/susfwk/renderer.h"
#include "include/susfwk/window.h"

#pragma warning(push)
#pragma warning(disable: 28159)
#pragma warning(disable: 4201)

// ================================================================================================= //
// ************************************************************************************************* //
//										THE CORE OF WINDOW API										 //
/*									Basic definitions of structures									 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									Window Structures									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Window Handlers
typedef struct sus_window_listeners {
	SUS_WINDOW_LISTENER				mainListener;
	SUS_WINDOW_MOUSE_LISTENERS		mouseListener;
	SUS_KEYBOARD_LISTENER			keyboardListener;
	SUS_WINDOW_TIMER_LISTENER		timerListener;
} SUS_WINDOW_LISTENERS;
// type of window signature
typedef enum sus_window_signature_type {
	SUS_WINDOW_SIGNATURE_TYPE_UNKNOWN,
	SUS_WINDOW_SIGNATURE_TYPE_FRAME,
	SUS_WINDOW_SIGNATURE_TYPE_WIDGET
} SUS_WINDOW_SIGNATURE_TYPE;
// Basic window structure
struct sus_window {
	HWND						_PARENT_;		// System Window descriptor
	SUS_WINDOW_SIGNATURE_TYPE	type;			// Window type
	SUS_WINDOW_LISTENERS		listeners;		// Window Listeners
	SUS_USERDATA				userData;		// User data
};

// -------------------------------------------------

// Window settings before Full Screen
typedef struct sus_window_fullscreen {
	DWORD		oldStyles;
	DWORD		oldExStyles;
	SUS_BOUNDS	oldBounds;
	BOOL		enabled;
} SUS_WINDOW_FULLSCREEN;
// Window frame
struct sus_frame {
	SUS_WINDOW_STRUCT;
	SUS_WINDOW_FULLSCREEN		fullScreen;
	SUS_WINDOW_CLOSE_OPERATION	closeOperation;
	MINMAXINFO					minmaxinfo;
	SUS_RENDERER				graphics;
};

// -------------------------------------------------

// Widget struct
struct sus_widget {
	SUS_WINDOW_STRUCT;
	SUS_WINDOW_GRAPHICS_COMPONENT	graphics;
	SUS_LAYOUT						layout;
};

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//								Creating a system window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Register the window system class
static BOOL SUSAPI susRegisterWindowClass(_In_ WNDCLASSEXW wcEx) {
	SUS_WPRINTDL("Registering a window class - \"%s\"", wcEx.lpszClassName);
	WNDCLASSW wcData = { 0 };
	if (!GetClassInfoW(wcEx.hInstance, wcEx.lpszClassName, &wcData)) {
		if (!RegisterClassExW(&wcEx)) {
			SUS_PRINTDE("Failed to register window class");
			SUS_PRINTDC(GetLastError());
			return FALSE;
		}
	}
	SUS_PRINTDL("The window class has been successfully registered!");
	return TRUE;
}
// Create a system window
static HWND SUSAPI susBuildWindow(_In_ CREATESTRUCTW wStruct, _In_ SUS_WINDOW winParam) {
	SUS_PRINTDL("Creating a window");
	SUS_ASSERT(winParam);
	HWND hWnd = CreateWindowExW(
		wStruct.dwExStyle,
		wStruct.lpszClass,
		wStruct.lpszName,
		wStruct.style,
		wStruct.x,
		wStruct.y,
		wStruct.cx,
		wStruct.cy,
		wStruct.hwndParent,
		wStruct.hMenu,
		wStruct.hInstance,
		winParam
	);
	if (!hWnd) {
		SUS_PRINTDE("Couldn't create a window");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	SUS_PRINTDL("The window has been successfully created");
	return hWnd;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//								Useful validation functions								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Check if the window is valid
BOOL SUSAPI susWindowIsValid(_In_ SUS_WINDOW window) {
	return (window && window->super && window->type);
}
// Check if the window is a valid widget
BOOL SUSAPI susWindowIsWidget(_In_ SUS_WIDGET window) {
	return (window && window->super && window->type == SUS_WINDOW_SIGNATURE_TYPE_WIDGET);
}
// Check if the window is a valid frame
BOOL SUSAPI susWindowIsFrame(_In_ SUS_FRAME window) {
	return (window && window->super && window->type == SUS_WINDOW_SIGNATURE_TYPE_FRAME);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//									The main window handler								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

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
// Process current window messages, Process current window messages without blocking the flow
BOOL SUSAPI susWindowProcessing(_Out_ LPMSG msg)
{
	while (PeekMessageW(msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg->message == WM_QUIT) return FALSE;
		TranslateMessage(msg);
		DispatchMessageW(msg);
	}
	return TRUE;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											 WINDOW LISTENERS										 //
/*								Processing system operations with a window							 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//								Window message processing								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Call the window handler
#define susWindowTrackEvent(window, msg, param) (SUS_RESULT)((window)->listeners.mainListener((SUS_WINDOW)(window), msg, (SUS_PARAM)(param)))

// Send a message immediately
SUS_RESULT SUSAPI susWindowSendMessage(_In_ SUS_WINDOW window, _In_ UINT msg, SUS_PARAM param) {
	SUS_ASSERT(window && window->super && msg >= SUS_WINMSG_USER);
	return SendMessageW(window->super, msg - SUS_WINMSG_USER + WM_USER, 0, param);
}
// Send a message
BOOL SUSAPI susWindowPostMessage(_In_ SUS_WINDOW window, _In_ UINT msg, SUS_PARAM param) {
	SUS_ASSERT(window && window->super && msg >= SUS_WINMSG_USER);
	return PostMessageW(window->super, msg - SUS_WINMSG_USER + WM_USER, 0, param);
}
// Default window listener
static SUS_RESULT SUSAPI susWindowDefListener(SUS_WINDOW window, SUS_WINMSG uMsg, SUS_PARAM lParam) {
	UNREFERENCED_PARAMETER(window);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(lParam);
	return 0;
}

// -------------------------------------------------

// Call the window timer handler
#define susWindowTrackTimerEvent(window, uId) if ((window)->listeners.timerListener) (window)->listeners.timerListener(window, (UINT)uId); else susWindowTrackEvent(window, SUS_WINMSG_TIMER, uId)
// Timer Handler
typedef VOID(SUSAPI* SUS_WINDOW_TIMER_LISTENER)(_In_ SUS_WINDOW window, _In_ UINT uId);
// Install a timer handler for the window
VOID SUSAPI susWindowSetTimerListener(_Inout_ SUS_WINDOW window, _In_opt_ SUS_WINDOW_TIMER_LISTENER timerListener) {
	SUS_ASSERT(window);
	window->listeners.timerListener = timerListener;
}

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

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//									Getting Modifiers									//
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

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//								Working with the mouse									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

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
static VOID SUSAPI susWindowTrackMouseEvent(_In_ SUS_WINDOW window, _In_ SUS_MOUSE_MESSAGE msg, _In_ SUS_MOUSE_BUTTON button, _In_ LPARAM lParam) {
	SUS_ASSERT(window && msg);
	if (!window->listeners.mouseListener.mouse) return;
	SUS_MOUSE_EVENT event = { .position = (SUS_POINT) { LOWORD(lParam), HIWORD(lParam) }, .specific = button, .time = GetTickCount64(), .modifier = susSystemGetModifier() };
	if (susSystemMouseIsClick(button, msg)) msg = SUS_MOUSE_MESSAGE_CLICK;
	if (msg == SUS_MOUSE_MESSAGE_CLICK) event.clickCount = susSystemGetClickCount(event.position, button);
	window->listeners.mouseListener.mouse(window, msg, event);
}
// Call the mouse handler
static VOID SUSAPI susWindowTrackMouseWheelEvent(_In_ SUS_WINDOW window, _In_ SUS_MOUSE_WHEEL wheel, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	SUS_ASSERT(window && wheel);
	if (!window->listeners.mouseListener.wheel) return;
	POINT pos = (POINT){ LOWORD(lParam), HIWORD(lParam) };
	ScreenToClient(window->super, (LPPOINT)&pos);
	SUS_MOUSE_EVENT event = { .position = (SUS_POINT){ (SHORT)pos.x, (SHORT)pos.y }, .specific.wheel = wheel, .time = GetTickCount64(), .modifier = susSystemGetModifier(), .wheelDelta = (FLOAT)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA };
	window->listeners.mouseListener.wheel(window, event);
}
// Call the mouse handler
static VOID SUSAPI susWindowTrackMouseMotionEvent(_In_ SUS_WINDOW window, _In_ SUS_MOUSE_MOTION_MESSAGE msg, _In_ LPARAM lParam) {
	SUS_ASSERT(window && msg);
	if (!window->listeners.mouseListener.motion) return;
	static struct { HWND hWnd; } mouseTracked = { 0 };
	if (msg == SUS_MOUSE_MOTION_MESSAGE_EXIT) sus_zeromem((LPBYTE)&mouseTracked, sizeof(mouseTracked));
	else if (mouseTracked.hWnd != window->super) {
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), .dwFlags = TME_LEAVE | TME_HOVER, .hwndTrack = window->super, .dwHoverTime = HOVER_DEFAULT };
		TrackMouseEvent(&tme);
		mouseTracked.hWnd = window->super;
	}
	SUS_MOUSE_EVENT event = { .position = (SUS_POINT) { LOWORD(lParam), HIWORD(lParam) }, .time = GetTickCount64(), .modifier = susSystemGetModifier() };
	window->listeners.mouseListener.motion(window, msg, event);
}


// Install a mouse listener
VOID SUSAPI susWindowSetMouseListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_LISTENER mouseListener) {
	SUS_ASSERT(window);
	if (sus_wcscmpi(susWindowGetClassName(window), SUS_WIDGET_CLASSNAME_STATIC)) susWindowSetStyle(window, susWindowGetStyle(window) | SS_NOTIFY);
	window->listeners.mouseListener.mouse = mouseListener;
}
// Install a mouse motion listener
VOID SUSAPI susWindowSetMouseMotionListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_MOTION_LISTENER mouseMotionListener) {
	SUS_ASSERT(window);
	if (sus_wcscmpi(susWindowGetClassName(window), SUS_WIDGET_CLASSNAME_STATIC)) susWindowSetStyle(window, susWindowGetStyle(window) | SS_NOTIFY);
	window->listeners.mouseListener.motion = mouseMotionListener;
}
// Install a mouse wheel listener
VOID SUSAPI susWindowSetMouseWheelListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_WHEEL_LISTENER mouseWheelListener) {
	SUS_ASSERT(window);
	if (sus_wcscmpi(susWindowGetClassName(window), SUS_WIDGET_CLASSNAME_STATIC)) susWindowSetStyle(window, susWindowGetStyle(window) | SS_NOTIFY);
	window->listeners.mouseListener.wheel = mouseWheelListener;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Working with the keyboard									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Call the keyboard handler
static VOID SUSAPI susWindowTrackKeyboardEvent(_In_ SUS_WINDOW window, _In_ SUS_KEYBOARD_MESSAGE msg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
	SUS_ASSERT(window && msg);
	if (!window->listeners.keyboardListener) return;
	SUS_KEYBOARD_EVENT event = { .key.as_virtual = (DWORD)wParam, .repeatCount = LOWORD(lParam), .scanCode = (lParam >> 16) & 0xFF, .modifier = susSystemGetModifier() };
	window->listeners.keyboardListener(window, msg, event);
}

// Install a keyboard listener
VOID SUSAPI susWindowSetKeyboardListener(_Inout_ SUS_WINDOW window, _In_ SUS_KEYBOARD_LISTENER keyboardListener) {
	SUS_ASSERT(window);
	window->listeners.keyboardListener = keyboardListener;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											WINDOW FRAME BUILDER									 //
/*								 Building and configuring the window frame							 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									 Builder functions									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

#define SUS_WCNAMEW L"suswc"

// -------------------------------------------------

// Standard window class
static const WNDCLASSEXW susDefWndClass = {
	.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
	.lpszClassName = SUS_WCNAMEW,
	.cbSize = sizeof(WNDCLASSEXW),
	.lpfnWndProc = susFrameSystemHandler
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

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Create a Window builder
SUS_FRAME_BUILDER SUSAPI susFrameBuilder(_In_opt_ LPCWSTR title)
{
	SUS_PRINTDL("Setting up the frame");
	SUS_FRAME_BUILDER builder = { 0 };
	builder.wcEx = susDefWndClass;
	builder.wcEx.hInstance = GetModuleHandleW(NULL);
	builder.wcEx.lpszClassName = builder.classNameBuffer;
	builder.wcEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	builder.wStruct = susDefWndStruct;
	builder.wStruct.hInstance = builder.wcEx.hInstance;
	if (title) builder.wStruct.lpszName = title;
	builder.wStruct.lpszClass = builder.classNameBuffer;
	sus_formattingW(builder.classNameBuffer, L"%s-%s%p", SUS_WCNAMEW, title, &builder);
	return builder;
}
// Build a window
SUS_FRAME SUSAPI susBuildFrame(_In_ SUS_LPFRAME_BUILDER builder, _In_opt_ SUS_WINDOW_LISTENER handler)
{
	SUS_PRINTDL("Creating a frame");
	SUS_ASSERT(builder);
	if (!susRegisterWindowClass(builder->wcEx)) return NULL;
	SUS_FRAME_STRUCT frameStruct = { .listeners.mainListener = handler ? handler : susWindowDefListener, .userData = (SUS_USERDATA)builder->wStruct.lpCreateParams, .type = SUS_WINDOW_SIGNATURE_TYPE_FRAME };
	SUS_FRAME frame = sus_newmem(sizeof(SUS_FRAME_STRUCT), &frameStruct);
	if (!susBuildWindow(builder->wStruct, (SUS_WINDOW)frame)) { sus_free(frame); return NULL; }
	SUS_PRINTDL("The frame was created successfully!");
	return frame;
}
// Create a window
SUS_FRAME SUSAPI susNewFrame(_In_opt_ LPCWSTR title, _In_ SUS_SIZE size, _In_opt_ SUS_WINDOW_LISTENER handler)
{
	SUS_FRAME_BUILDER builder = susFrameBuilder(title);
	susFrameBuilderSetSize(&builder, size);
	return susBuildFrame(&builder, handler);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											WIDGET BUILDER											 //
/*								  Building and configuring the widget								 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									 Builder functions									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

static const CREATESTRUCTW susDefWidgetStruct = {
	.style = WS_VISIBLE | WS_CHILD,
	.x = 0,
	.y = 0,
	.cx = 100,
	.cy = 50,
	.lpszClass = L"STATIC"
};

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Create a Window builder
SUS_WIDGET_BUILDER SUSAPI susWidgetBuilder(_In_opt_ LPCWSTR className)
{
	SUS_PRINTDL("Setting up the widget");
	SUS_WIDGET_BUILDER builder = { 0 };
	builder.wStruct = susDefWidgetStruct;
	builder.wStruct.hInstance = GetModuleHandleW(NULL);
	static UINT id = SUS_WIDGET_RESERVED_IDS;
	builder.wStruct.hMenu = (HMENU)((ULONG_PTR)id++);
	if (className) builder.wStruct.lpszClass = className;
	return builder;
}
// Build a window
SUS_WIDGET SUSAPI susBuildWidget(_In_ SUS_LPWIDGET_BUILDER builder, _In_ SUS_WINDOW parent, _In_opt_ SUS_WINDOW_LISTENER handler)
{
	SUS_PRINTDL("Creating a widget");
	SUS_ASSERT(builder);
	builder->wStruct.hwndParent = parent->super;
	SUS_WIDGET_STRUCT frameStruct = { .listeners.mainListener = handler ? handler : susWindowDefListener, .graphics.backgroundComponent = susWindowDefBackgroundComponent, .userData = parent->userData, .type = SUS_WINDOW_SIGNATURE_TYPE_WIDGET, .graphics.context.clearColor = SUS_COLOR_WHITE };
	SUS_WIDGET widget = sus_newmem(sizeof(SUS_WIDGET_STRUCT), &frameStruct);
	widget->super = susBuildWindow(builder->wStruct, (SUS_WINDOW)widget);
	if (!widget->super) { sus_free(widget); return NULL; }
	susWindowWriteData(widget->super, (LONG_PTR)widget);
	susWindowTrackEvent(widget, SUS_WINMSG_CREATE, NULL);
	SetWindowSubclass(widget->super, susWidgetSystemHandler, (UINT_PTR)builder->wStruct.hMenu, (DWORD_PTR)widget);
	SUS_PRINTDL("The widget was created successfully!");
	return widget;
}
// Create a window
SUS_WIDGET SUSAPI susNewWidget(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR className, _In_opt_ LPCWSTR title, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ DWORD styles)
{
	SUS_WIDGET_BUILDER builder = susWidgetBuilder(className);
	if (title) susWidgetBuilderSetTitle(&builder, title);
	susWidgetBuilderAddStyle(&builder, styles);
	return susBuildWidget(&builder, parent, handler);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Create a button
SUS_WIDGET SUSAPI susNewButton(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR title, _In_opt_ SUS_WINDOW_LISTENER handler) {
	return susNewWidget(parent, SUS_WIDGET_CLASSNAME_BUTTON, title, handler, WS_TABSTOP);
}
// Create a checkbox button
SUS_WIDGET SUSAPI susNewCheckBox(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR title, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ BOOL selected) {
	SUS_WIDGET wg = susNewWidget(parent, SUS_WIDGET_CLASSNAME_BUTTON, title, handler, WS_TABSTOP | BS_AUTOCHECKBOX);
	if (wg && selected) SendMessageW(susWindowGetSuper((SUS_WINDOW)wg), BM_SETCHECK, BST_CHECKED, 0);
	return wg;
}
// Create a switch
SUS_WIDGET SUSAPI susNewRadioButton(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR title, _In_opt_ SUS_WINDOW_LISTENER handler) {
	return susNewWidget(parent, SUS_WIDGET_CLASSNAME_BUTTON, title, handler, WS_TABSTOP | BS_AUTORADIOBUTTON);
}

// -------------------------------------------------

// Create a label
SUS_WIDGET SUSAPI susNewLabel(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR text, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ SUS_WIDGET_TEXT_ALIGN align) {
	return susNewWidget(parent, SUS_WIDGET_CLASSNAME_STATIC, text, handler, 
		(align == SUS_WIDGET_TEXT_ALIGN_LEFT) ? SS_LEFT : (align == SUS_WIDGET_TEXT_ALIGN_CENTER) ? SS_CENTER : (align == SUS_WIDGET_TEXT_ALIGN_RIGHT) ? SS_RIGHT : 0);
}
// Create a panel
SUS_WIDGET SUSAPI susNewPanel(_In_ SUS_WINDOW parent, _In_opt_ SUS_WINDOW_LISTENER handler) {
	SUS_WIDGET panel = susNewLabel(parent, NULL, handler, SUS_WIDGET_TEXT_ALIGN_NONE);
	if (panel) susWidgetSetLayout(panel, (SUS_LAYOUT) { 0.0f, 0.0f, 1.0f, 1.0f });
	return panel;
}
// Create a text input field
SUS_WIDGET SUSAPI susNewTextField(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR text, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ SUS_WIDGET_TEXT_ALIGN align) {
	return susNewWidget(parent, SUS_WIDGET_CLASSNAME_EDIT, text, handler, WS_TABSTOP | ES_AUTOHSCROLL | WS_BORDER |
		((align == SUS_WIDGET_TEXT_ALIGN_LEFT) ? ES_LEFT : (align == SUS_WIDGET_TEXT_ALIGN_CENTER) ? ES_CENTER : (align == SUS_WIDGET_TEXT_ALIGN_RIGHT) ? ES_RIGHT : 0));
}
// Create a text entry area
SUS_WIDGET SUSAPI susNewTextArea(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR text, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ SUS_WIDGET_TEXT_ALIGN align) {
	return susNewWidget(parent, SUS_WIDGET_CLASSNAME_EDIT, text, handler, WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_BORDER |
		((align == SUS_WIDGET_TEXT_ALIGN_LEFT) ? ES_LEFT : (align == SUS_WIDGET_TEXT_ALIGN_CENTER) ? ES_CENTER : (align == SUS_WIDGET_TEXT_ALIGN_RIGHT) ? ES_RIGHT : 0));
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											THE BASE WINDOW											 //
/*									 Working with the base windowg									 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									Basic window treatment								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Listener Processing
BOOL SUSAPI susWindowListenerHandler(_In_ SUS_WINDOW window, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:	susWindowTrackMouseEvent(window, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_LEFT, lParam); return TRUE;
	case WM_RBUTTONDOWN:	susWindowTrackMouseEvent(window, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_RIGHT, lParam); return TRUE;
	case WM_MBUTTONDOWN:	susWindowTrackMouseEvent(window, SUS_MOUSE_MESSAGE_PRESS, SUS_MOUSE_BUTTON_MIDDLE, lParam); return TRUE;
	case WM_LBUTTONUP:		susWindowTrackMouseEvent(window, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_LEFT, lParam); return TRUE;
	case WM_RBUTTONUP:		susWindowTrackMouseEvent(window, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_RIGHT, lParam); return TRUE;
	case WM_MBUTTONUP:		susWindowTrackMouseEvent(window, SUS_MOUSE_MESSAGE_RELEASE, SUS_MOUSE_BUTTON_MIDDLE, lParam); return TRUE;
	case WM_MOUSEMOVE:		susWindowTrackMouseMotionEvent(window, SUS_MOUSE_MOTION_MESSAGE_MOVE, lParam); return TRUE;
	case WM_MOUSELEAVE:		susWindowTrackMouseMotionEvent(window, SUS_MOUSE_MOTION_MESSAGE_EXIT, lParam); return TRUE;
	case WM_MOUSEHOVER:		susWindowTrackMouseMotionEvent(window, SUS_MOUSE_MOTION_MESSAGE_HOVER, lParam); return TRUE;
	case WM_MOUSEWHEEL:		susWindowTrackMouseWheelEvent(window, SUS_MOUSE_WHEEL_VERTICAL, wParam, lParam); return TRUE;
	case WM_MOUSEHWHEEL:	susWindowTrackMouseWheelEvent(window, SUS_MOUSE_WHEEL_HORIZONTAL, wParam, lParam); return TRUE;
	case WM_KEYDOWN:		susWindowTrackKeyboardEvent(window, SUS_KEYBOARD_MESSAGE_PRESS, wParam, lParam); return TRUE;
	case WM_KEYUP:			susWindowTrackKeyboardEvent(window, SUS_KEYBOARD_MESSAGE_RELEASE, wParam, lParam); return TRUE;
	case WM_CHAR:			susWindowTrackKeyboardEvent(window, SUS_KEYBOARD_MESSAGE_CHAR, wParam, lParam); return TRUE;
	case WM_TIMER:			susWindowTrackTimerEvent(window, wParam); return TRUE;
	default: return FALSE;
	}
}

// -------------------------------------------------

// Process the background of the window children
static LRESULT SUSAPI susWindowHandleChildBackground(WPARAM wParam, LPARAM lParam) {
	SUS_WIDGET child = (SUS_WIDGET)susWindowLoadData((HWND)lParam);
	SUS_ASSERT(susWindowIsWidget(child));
	SUS_GRAPHICS_STRUCT graphics = { .context = &child->graphics.context, .super = (HDC)wParam };
	child->graphics.backgroundComponent(child, &graphics);
	SetBkMode((HDC)wParam, TRANSPARENT);
	return (LRESULT)GetStockObject(NULL_BRUSH);
}
// Basic window treatment
BOOL SUSAPI susWindowHandler(SUS_WINDOW window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE: {
		for (HWND hChild = GetWindow(window->super, GW_CHILD); hChild; hChild = GetNextWindow(hChild, GW_HWNDNEXT)) {
			SUS_WINDOW child = susWindowLoadData(hChild);
			if (child->type == SUS_WINDOW_SIGNATURE_TYPE_WIDGET)susWidgetUpdateLayout((SUS_WIDGET)child, (SUS_SIZE) { LOWORD(lParam), HIWORD(lParam) });
		}
		susWindowTrackEvent(window, SUS_WINMSG_RESIZE, lParam);
		switch (wParam)
		{
		case SIZE_MAXIMIZED:	susWindowTrackEvent(window, SUS_WINMSG_MAXIMIZED, NULL); break;
		case SIZE_MINIMIZED:	susWindowTrackEvent(window, SUS_WINMSG_MINIMIZED, NULL); break;			
		}
	} return FALSE;
	case WM_MOVE:			susWindowTrackEvent(window, SUS_WINMSG_MOVE, lParam); return TRUE;
	case WM_SETFOCUS:		susWindowTrackEvent(window, SUS_WINMSG_FOCUS, TRUE); return TRUE;
	case WM_KILLFOCUS:		susWindowTrackEvent(window, SUS_WINMSG_FOCUS, FALSE); return TRUE;
	case WM_CREATE:			susWindowTrackEvent(window, SUS_WINMSG_CREATE, NULL); return TRUE;
	case WM_DESTROY:		susWindowTrackEvent(window, SUS_WINMSG_DESTROY, NULL); return TRUE;
	default: return FALSE;
	}
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//						 Functions for working with the window							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set window visibility
BOOL SUSAPI susWindowSetVisible(_In_ SUS_WINDOW window, _In_ BOOL enabled) {
	SUS_ASSERT(window && window->super);
	return ShowWindow(window->super, enabled ? SW_SHOW : SW_HIDE);
}
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
// Get the client area of the window
SUS_SIZE SUSAPI susWindowGetContent(_In_ SUS_WINDOW window) {
	RECT rect = { 0 };
	GetClientRect(window->super, &rect);
	return (SUS_SIZE) { rect.right, rect.bottom };
}
// Set the title for the window
BOOL SUSAPI susWindowSetText(_In_ SUS_WINDOW window, _In_ LPCWSTR title) {
	SUS_ASSERT(window && window->super);
	return SetWindowTextW(window->super, title);
}
// Get a dynamic window title
LPWSTR SUSAPI susWindowGetText(_In_ SUS_WINDOW window) {
	UINT length = GetWindowTextLengthW(window->super);
	LPWSTR text = sus_calloc(length + 1, sizeof(WCHAR));
	if (!text) return NULL;
	GetWindowTextW(window->super, text, length);
	return text;
}
// Set transparency for the window (From 0 to 1)
BOOL SUSAPI susWindowSetTransparency(_In_ SUS_WINDOW window, _In_ sus_float_t transparency) {
	SUS_ASSERT(window && window->super);
	transparency = sus_clamp(transparency, 0.0f, 1.0f);
	DWORD exstyle = (DWORD)GetWindowLongPtr(window->super, GWL_EXSTYLE);
	if (transparency <= SUS_EPSILON) return SetWindowLongPtr(window->super, GWL_EXSTYLE, exstyle & ~WS_EX_LAYERED) ? TRUE : FALSE;
	if (!(exstyle & WS_EX_LAYERED)) SetWindowLongPtr(window->super, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);
	return SetLayeredWindowAttributes(window->super, 0, (BYTE)((1.0f - transparency) * 255.0f), LWA_ALPHA);
}

// -------------------------------------------------

// Set user window data
SUS_USERDATA SUSAPI susWindowSetData(_In_ SUS_WINDOW window, _In_ SUS_USERDATA userData) {
	SUS_ASSERT(window);
	return window->userData = userData;
}
// Get user window data
SUS_USERDATA SUSAPI susWindowGetData(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window);
	return window->userData;
}
// Set a property for a window
BOOL SUSAPI susWindowSetProperty(_In_ SUS_WINDOW window, _In_ LPCWSTR key, _In_ SUS_PARAM value) {
	SUS_ASSERT(window && window->super);
	return SetPropW(window->super, key, (HANDLE)value);
}
// Get a window property
SUS_PARAM SUSAPI susWindowGetProperty(_In_ SUS_WINDOW window, _In_ LPCWSTR key) {
	SUS_ASSERT(window && window->super);
	return (SUS_PARAM)GetPropW(window->super, key);
}
// Get a widget from a window
SUS_WIDGET SUSAPI susWindowGetWidget(_In_ SUS_WINDOW window, _In_ UINT id) {
	SUS_ASSERT(window);
	HWND hWidget = GetDlgItem(window->super, id);
	if (!hWidget) return NULL;
	return (SUS_WIDGET)susWindowLoadData(hWidget);
}
// Set your id for the window
VOID SUSAPI susWindowSetId(_In_ SUS_WINDOW window, _In_ UINT id) {
	SUS_ASSERT(window && window->super);
	SetWindowLongPtrW(window->super, GWL_ID, id);
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
// Get the window class name as a static string
LPCWSTR SUSAPI susWindowGetClassName(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window);
	static WCHAR className[64];
	if (!GetClassNameW(window->super, className, sizeof(className) / sizeof(WCHAR))) return NULL;
	return className;
}
// Get the parent's window
SUS_WINDOW SUSAPI susWindowGetParent(_In_ SUS_WINDOW window) {
	SUS_ASSERT(window && window->super);
	HWND hparent = GetParent(window->super);
	SUS_WINDOW parent = susWindowLoadData(hparent);
	SUS_ASSERT(parent);
	return parent;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//										  WORKING WITH FRAME										 //
/*								    Frame operation and processing									 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//										Window Handler									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// System Window Handler
LRESULT WINAPI susFrameSystemHandler(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	SUS_FRAME window = (SUS_FRAME)susWindowLoadData(hWnd);
	if (!susWindowListenerHandler((SUS_WINDOW)window, uMsg, wParam, lParam) && !susWindowHandler((SUS_WINDOW)window, uMsg, wParam, lParam)) {
		switch (uMsg)
		{
		case WM_CLOSE: {
			if (susWindowTrackEvent(window, SUS_WINMSG_CLOSE, 0)) break;
			if (window->closeOperation == SUS_WINDOW_HIDE_ON_CLOSE) susWindowSetVisible((SUS_WINDOW)window, FALSE);
			else if (window->closeOperation == SUS_WINDOW_EXIT_ON_CLOSE || window->closeOperation == SUS_WINDOW_DELETE_ON_CLOSE) DestroyWindow(hWnd);
		} break;
		case WM_CTLCOLORSTATIC: return susWindowHandleChildBackground(wParam, lParam);
		case WM_ERASEBKGND: return TRUE;
		case WM_GETMINMAXINFO: {
			if (!window) break;
			LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
			if (window->minmaxinfo.ptMaxTrackSize.x) {
				mmi->ptMaxTrackSize = window->minmaxinfo.ptMaxTrackSize;
				mmi->ptMaxSize = window->minmaxinfo.ptMaxSize;
			}
			if (window->minmaxinfo.ptMinTrackSize.x)
				mmi->ptMinTrackSize = window->minmaxinfo.ptMinTrackSize;
		} break;
		case WM_SIZE: {
			if (window->graphics) susRendererSize();
		} break;
		case WM_NCCREATE: {
			window = (SUS_FRAME)susGetWindowParam(lParam);
			susWindowWriteData(hWnd, (LONG_PTR)window);
			window->super = hWnd;
		} break;
		case WM_NCDESTROY: {
			susWindowSetRenderer(window, FALSE);
			if (window->closeOperation == SUS_WINDOW_EXIT_ON_CLOSE) PostQuitMessage(0);
			sus_free(window);
		} break;
		}
	}
	return uMsg >= WM_USER ? susWindowTrackEvent(window, uMsg - WM_USER + SUS_WINMSG_USER, lParam) : DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Functions for working with the window						//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set a standard action when closing a window
VOID SUSAPI susWindowSetCloseOperation(_Inout_ SUS_FRAME window, _In_ SUS_WINDOW_CLOSE_OPERATION closeOperation) {
	SUS_ASSERT(susWindowIsFrame(window));
	window->closeOperation = closeOperation;
}
// Set Fullscreen for the window
BOOL SUSAPI susWindowSetFullscreen(_In_ SUS_FRAME window, _In_ BOOL enabled) {
	SUS_ASSERT(susWindowIsFrame(window));
	susWindowSetVisible((SUS_WINDOW)window, FALSE);
	if (enabled && !window->fullScreen.enabled) {
		SUS_WINDOW_FULLSCREEN fs = { .oldExStyles = susWindowGetExStyle((SUS_WINDOW)window), .oldStyles = susWindowGetStyle((SUS_WINDOW)window), .oldBounds = susWindowGetBounds((SUS_WINDOW)window), enabled = TRUE };
		window->fullScreen = fs;
		susWindowSetStyle((SUS_WINDOW)window, WS_POPUP);
		susWindowSetExStyle((SUS_WINDOW)window, WS_EX_APPWINDOW);
		return SetWindowPos(window->super, HWND_TOPMOST, 0, 0, susGetScreenSizeX(), susGetScreenSizeY(), SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}
	if (!window->fullScreen.enabled) return FALSE;
	window->fullScreen.enabled = FALSE;
	susWindowSetStyle((SUS_WINDOW)window, window->fullScreen.oldStyles);
	susWindowSetExStyle((SUS_WINDOW)window, window->fullScreen.oldExStyles);
	return SetWindowPos(window->super, HWND_NOTOPMOST, (int)window->fullScreen.oldBounds.x, (int)window->fullScreen.oldBounds.y, (int)window->fullScreen.oldBounds.cx, (int)window->fullScreen.oldBounds.cy, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}
// Minimize the window
BOOL SUSAPI susWindowMinimize(_In_ SUS_FRAME window) {
	SUS_ASSERT(susWindowIsFrame(window));
	return ShowWindow(window->super, SW_MINIMIZE);
}
// Expand the window
BOOL SUSAPI susWindowMaximize(_In_ SUS_FRAME window) {
	SUS_ASSERT(susWindowIsFrame(window));
	return ShowWindow(window->super, SW_MAXIMIZE);
}
// Restore the window after maximizing/minimizing
BOOL SUSAPI susWindowRestore(_In_ SUS_FRAME window) {
	SUS_ASSERT(susWindowIsFrame(window));
	return ShowWindow(window->super, SW_RESTORE);
}
// Set the status - always on top
BOOL SUSAPI susWindowSetAlwaysOnTop(_In_ SUS_FRAME window, _In_ BOOL enabled) {
	SUS_ASSERT(susWindowIsFrame(window));
	return SetWindowPos(window->super, enabled ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}
// Set the cursor for the window
HCURSOR SUSAPI susWindowSetCursor(_In_ SUS_FRAME window, _In_ HCURSOR hCursor) {
	SUS_ASSERT(susWindowIsFrame(window) && hCursor);
	SetWindowLongPtrW(window->super, GCLP_HCURSOR, (LONG_PTR)hCursor);
	return SetCursor(hCursor);
}
// Set the icon for the window
HICON SUSAPI susWindowSetIcon(_In_ SUS_FRAME window, _In_ HICON hIcon) {
	SUS_ASSERT(susWindowIsFrame(window) && hIcon);
	return (HICON)SendMessageW(window->super, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
}
// Set the Menu for the window
BOOL SUSAPI susWindowSetMenuBar(_In_ SUS_FRAME window, _In_ HMENU hMenu) {
	SUS_ASSERT(susWindowIsFrame(window) && hMenu);
	return SetMenu(window->super, hMenu);
}
// Set the minimum size for the window
VOID SUSAPI susWindowSetMinimumSize(_In_ SUS_FRAME window, _In_ SUS_SIZE minSize) {
	SUS_ASSERT(susWindowIsFrame(window));
	window->minmaxinfo.ptMinTrackSize = (POINT){ minSize.cx, minSize.cy };
	SetWindowPos(window->super, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}
// Set the maximum size for the window
VOID SUSAPI susWindowSetMaximumSize(_In_ SUS_FRAME window, _In_ SUS_SIZE maxSize) {
	SUS_ASSERT(susWindowIsFrame(window));
	window->minmaxinfo.ptMaxSize = (POINT){ maxSize.cx, maxSize.cy };
	window->minmaxinfo.ptMaxTrackSize = (POINT){ maxSize.cx, maxSize.cy };
	SetWindowPos(window->super, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}
// Set a fixed size for the window
VOID SUSAPI susWindowSetFixedSize(_In_ SUS_FRAME window, _In_ SUS_SIZE size) {
	susWindowSetMinimumSize(window, size);
	susWindowSetMaximumSize(window, size);
}
// Install a renderer for an OpenGL-based window
BOOL SUSAPI susWindowSetRenderer(_In_ SUS_FRAME window, _In_ BOOL enable) {
	SUS_ASSERT(window);
	if (window->graphics) susRendererCleanup(window->graphics);
	if (enable) return (window->graphics = susRendererSetup(window->super)) != NULL;
	return TRUE;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//										  WORKING WITH WIDGET										 //
/*								    Widget operation and processing									 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									Widget layout system								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Update the widget layout after resizing the window
VOID SUSAPI susWidgetUpdateLayout(_In_ SUS_WIDGET widget, _In_ SUS_SIZE newScreenSize)
{
	SUS_ASSERT(widget && widget->super);
	if (sus_fabsf(widget->layout.x) > SUS_EPSILON && sus_fabsf(widget->layout.y) > SUS_EPSILON)
		susWindowSetPosition((SUS_WINDOW)widget,
			(SUS_POINT) { (sus_int_t)(widget->layout.x * (sus_float_t)newScreenSize.cx), (sus_int_t)(widget->layout.y * (sus_float_t)newScreenSize.cy) });
	if (sus_fabsf(widget->layout.cx) > SUS_EPSILON && sus_fabsf(widget->layout.cy) > SUS_EPSILON)
		susWindowSetSize((SUS_WINDOW)widget,
			(SUS_SIZE) { (sus_uint_t)(widget->layout.cx * (sus_float_t)newScreenSize.cx), (sus_uint_t)(widget->layout.cy * (sus_float_t)newScreenSize.cy) });
}

// Set a new layout for the widget
VOID SUSAPI susWidgetSetLayout(_In_ SUS_WIDGET widget, SUS_LAYOUT layout)
{
	SUS_ASSERT(widget && widget->super);
	widget->layout = layout;
	SetWindowPos(widget->super, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//										Window Handler									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// System Window Handler
LRESULT WINAPI susWidgetSystemHandler(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	SUS_WIDGET window = (SUS_WIDGET)dwRefData;
	if (!susWindowListenerHandler((SUS_WINDOW)window, uMsg, wParam, lParam) && !susWindowHandler((SUS_WINDOW)window, uMsg, wParam, lParam)) {
		switch (uMsg)
		{
		case WM_PAINT: {
			if (!window->graphics.paintComponent) break;
			SUS_GRAPHICS_STRUCT graphics = susGraphicsBegin(window->super, &window->graphics.context);
			window->graphics.paintComponent(window, &graphics);
			susGraphicsEnd(window->super, &graphics);
		} return 0;
		case WM_SIZE: {
			susGraphicsResize(window->super, &window->graphics.context);
		} break;
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORLISTBOX: 
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC: return susWindowHandleChildBackground(wParam, lParam);
		case WM_ERASEBKGND: return TRUE;
		case WM_NCDESTROY: {
			susWindowSetDoubleBuffer(window, FALSE);
			RemoveWindowSubclass(hWnd, susWidgetSystemHandler, uIdSubclass);
			sus_free(window);
		} break;
		}
	}
	return uMsg >= WM_USER ? susWindowTrackEvent(window, uMsg - WM_USER + SUS_WINMSG_USER, lParam) : DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Functions for working with the widget						//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Enable/disable rendering processing
VOID SUSAPI susWindowSetPaintComponent(_In_ SUS_WIDGET window, _In_ SUS_WINDOW_PAINT_COMPONENT paintListener) {
	SUS_ASSERT(susWindowIsWidget(window));
	if (sus_wcscmpi(susWindowGetClassName((SUS_WINDOW)window), SUS_WIDGET_CLASSNAME_STATIC)) susWindowSetStyle((SUS_WINDOW)window, susWindowGetStyle((SUS_WINDOW)window) | SS_NOTIFY);
	window->graphics.paintComponent = paintListener;
}
// Get the context of the window graphics
SUS_LPGRAPHICS_CONTEXT SUSAPI susWindowGetGraphics(_In_ SUS_WIDGET window) {
	SUS_ASSERT(susWindowIsWidget(window));
	return &window->graphics.context;
}
// Redraw the entire window
VOID SUSAPI susWindowRepaint(_In_ SUS_WIDGET window) {
	SUS_ASSERT(susWindowIsWidget(window));
	InvalidateRect(window->super, NULL, FALSE);
}
// immediately redraw the window
VOID SUSAPI susWindowRepaintImmediately(_In_ SUS_WIDGET window) {
	SUS_ASSERT(susWindowIsWidget(window));
	InvalidateRect(window->super, NULL, FALSE);
	UpdateWindow(window->super);
}

// -------------------------------------------------

// Install the background rendering component
VOID SUSAPI susWindowSetBackgroundComponent(_In_ SUS_WIDGET window, _In_ SUS_WINDOW_BACKGROUND_COMPONENT backgroundComponent) {
	SUS_ASSERT(susWindowIsWidget(window));
	window->graphics.backgroundComponent = backgroundComponent ? backgroundComponent : susWindowDefBackgroundComponent;
}

// Standard window drawing
VOID SUSAPI susWindowDefBackgroundComponent(_In_ SUS_WIDGET window, _In_ SUS_GRAPHICS gr) {
	UNREFERENCED_PARAMETER(window);
	susGraphicsClear(gr);
}
// The standard window drawing is an image
VOID SUSAPI susWindowDefBackgroundComponentImage(_In_ SUS_WIDGET window, _In_ SUS_GRAPHICS gr) {
	UNREFERENCED_PARAMETER(window);
	SUS_ASSERT(gr->context->userData);
	susGraphicsDrawImage(gr, (SUS_GRAPHICS_IMAGE)gr->context->userData, (SUS_BOUNDS) { .size = gr->context->screen });
}

// -------------------------------------------------

// Install a double buffering system
BOOL SUSAPI susWindowSetDoubleBuffer(_Inout_ SUS_WIDGET window, _In_ BOOL enabled) {
	SUS_ASSERT(susWindowIsWidget(window));
	if (window->graphics.context.backBuffer.hdcMem) susGraphicsRemoveDoubleBuffer(&window->graphics.context);
	if (enabled) return susGraphicsSetDoubleBuffer(window->super, &window->graphics.context);
	return TRUE;
}
// Set the window background color
VOID SUSAPI susWindowSetBackground(_In_ SUS_WIDGET window, _In_ SUS_COLOR color) {
	SUS_ASSERT(susWindowIsWidget(window));
	susGraphicsSetClear(&window->graphics.context, color);
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

#pragma warning(pop)