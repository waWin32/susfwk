// window.h
//
#ifndef _SUS_WINDOW_
#define _SUS_WINDOW_

#include "windowbase.h"

// ================================================================================================= //
// ************************************************************************************************* //
//										   WORKING WITH WINDOWS										 //
/*									Window operation and configuration								 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Window structure
typedef struct sus_window SUS_WINDOW_STRUCT, *SUS_WINDOW;

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Window Event handler implementation							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Basic window messages
typedef enum sus_window_message {
	SUS_WNDMSG_UNKNOWN,		// Unknown message
	SUS_WNDMSG_CREATE,		// The window has been created\param param = NULL
	SUS_WNDMSG_DESTROY,		// The window is remote\param param = NULL
	SUS_WNDMSG_CLOSE,		// They're trying to close the window\param param = NULL
	SUS_WNDMSG_RESIZE,		// The window has changed size\param param = SUS_SIZE (New size)
	SUS_WNDMSG_MINIMIZED,	// The window is minimized\param param = NULL
	SUS_WNDMSG_MAXIMIZED,	// The window is maximized\param param = NULL
	SUS_WNDMSG_MOVE,		// The window has been moved\param param = SUS_POINT (New position)
	SUS_WNDMSG_FOCUS,		// The window has gained/lost focus\param param = BOOL (Focused)
	SUS_WNDMSG_USER			// The user message\param param = userParam
} SUS_WINDOW_MESSAGE;
// Basic command Handler
typedef LRESULT(SUSAPI* SUS_WINDOW_HANDLER)(SUS_WINDOW window, SUS_WINDOW_MESSAGE msg, LPARAM param);

// Send a message to the window
LRESULT SUSAPI susWindowSendMessage(_In_ SUS_WINDOW window, _In_ UINT msg, LPARAM param);
// Send a message to the window later
BOOL SUSAPI susWindowPostMessage(_In_ SUS_WINDOW window, _In_ UINT msg, LPARAM param);

// -------------------------------------------------

// The main window handler
LRESULT WINAPI susWindowSystemHandler(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);
// The main Window cycle
INT SUSAPI susWindowMainLoop();

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Working with the Input window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The modifier
typedef enum sus_window_key_modifier {
	SUS_MODIFIER_KEY_NONE,				// Missing modifier
	SUS_MODIFIER_KEY_SHIFT = 1 << 0,	// The modifier key is SHIFT
	SUS_MODIFIER_KEY_CTRL = 1 << 1,		// The modifier key is CTRL
	SUS_MODIFIER_KEY_ALT = 1 << 2,		// The modifier key is ALT
	SUS_MODIFIER_KEY_CAPSLOCK = 1 << 4,	// The modifier key is CAPSLOCK
	SUS_MODIFIER_KEY_NUMLOCK = 1 << 5	// The modifier key is NUMLOCK
} SUS_MODIFIER_KEY;
// Get global modifiers
DWORD SUSAPI susSystemGetModifier();

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)

// The mouse button
typedef enum sus_mouse_button {
	SUS_MOUSE_BUTTON_NONE,		// Unknown button
	SUS_MOUSE_BUTTON_LEFT,		// Left mouse button
	SUS_MOUSE_BUTTON_MIDDLE,	// Middle mouse button
	SUS_MOUSE_BUTTON_RIGHT,		// Right mouse button
	SUS_MOUSE_BUTTON_X1,		// X1 mouse button
	SUS_MOUSE_BUTTON_X2,		// X2 mouse button
	SUS_MOUSE_BUTTON_COUNT		// Count of mouse buttons
} SUS_MOUSE_BUTTON;
// The mouse wheel
typedef enum sus_mouse_wheel {
	SUS_MOUSE_WHEEL_NONE,		// Unknown wheel
	SUS_MOUSE_WHEEL_HORIZONTAL,	// Horizontal mouse scrolling
	SUS_MOUSE_WHEEL_VERTICAL	// Vertical mouse scrolling
} SUS_MOUSE_WHEEL;
// A specific operation
typedef union sus_mouse_specific {
	SUS_MOUSE_BUTTON	button;		// The mouse button
	SUS_MOUSE_WHEEL		wheel;		// Mouse wheel action
} SUS_MOUSE_SPECIFIC;
// The structure of the mouse event description
typedef struct sus_mouse_event {
	SUS_POINT			position;	// Mouse position
	DWORD				modifier;	// SUS_MODIFIER_KEY
	ULONGLONG			time;		// Click Time
	SUS_MOUSE_SPECIFIC	specific;	// A specific operation is a click or a wheel
	union {
		UINT		clickCount;	// Number of mouse clicks\param EVENT: MouseClick
		INT			wheelDelta;	// Delta of the wheel\param EVENT: MouseWheel
	};
} SUS_MOUSE_EVENT, *SUS_LPMOUSE_EVENT;
// Mouse messages
typedef enum sus_mouse_message {
	SUS_MOUSE_MESSAGE_UNKNOWN,	// Unknown message
	SUS_MOUSE_MESSAGE_CLICK,	// Mouse click
	SUS_MOUSE_MESSAGE_PRESS,	// The mouse button is clamped
	SUS_MOUSE_MESSAGE_RELEASE,	// The mouse button is released
	SUS_MOUSE_MESSAGE_ENTER,	// The mouse got into the window
	SUS_MOUSE_MESSAGE_EXIT		// The mouse went out the window
} SUS_MOUSE_MESSAGE;
// Mouse motion messages
typedef enum sus_mouse_motion_message {
	SUS_MOUSE_MOTION_MESSAGE_UNKNOWN,	// Unknown message
	SUS_MOUSE_MOTION_MESSAGE_MOVE,		// The mouse has moved
	SUS_MOUSE_MOTION_MESSAGE_HOVER		// The mouse hovered over the window
} SUS_MOUSE_MOTION_MESSAGE;
// The mouse listener
typedef VOID(SUSAPI* SUS_MOUSE_LISTENER)(SUS_WINDOW window, SUS_MOUSE_MESSAGE msg, SUS_MOUSE_EVENT event);
// The mouse motion listener
typedef VOID(SUSAPI* SUS_MOUSE_MOTION_LISTENER)(SUS_WINDOW window, SUS_MOUSE_MOTION_MESSAGE msg, SUS_MOUSE_EVENT event);
// The mouse wheel listener
typedef VOID(SUSAPI* SUS_MOUSE_WHEEL_LISTENER)(SUS_WINDOW window, SUS_MOUSE_EVENT event);
// All mouse handlers
typedef struct sus_window_mouse_listners {
	SUS_MOUSE_LISTENER			mouse;	// Basic mouse handling - clicks, taps, etc.
	SUS_MOUSE_MOTION_LISTENER	motion; // Processing mouse movement
	SUS_MOUSE_WHEEL_LISTENER	wheel;	// Mouse wheel processing
} SUS_WINDOW_MOUSE_LISTENERS;

#pragma warning(pop)

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The structure of the keyboard event description
typedef struct sus_keyboard_event {
	DWORD	modifier;		// SUS_MODIFIER_KEY
	union {
		WCHAR	as_char;	// Symbol
		DWORD	as_virtual;	// The virtual key
	} key;					// The pressed symbol
	DWORD	scanCode;		// The physical code of the symbol
	DWORD	repeatCount;	// Number of repetitions (Key hold)
} SUS_KEYBOARD_EVENT, * SUS_LPKEYBOARD_EVENT;
// Keyboard messages
typedef enum sus_keyboard_message {
	SUS_KEYBOARD_MESSAGE_UNKNOWN,	// Unknown message from the keyboard
	SUS_KEYBOARD_MESSAGE_PRESS,		// The key is pressed
	SUS_KEYBOARD_MESSAGE_RELEASE,	// The key is released
	SUS_KEYBOARD_MESSAGE_TYPE,		// Character input
} SUS_KEYBOARD_MESSAGE;
// The kerboard listener
typedef VOID(SUSAPI* SUS_KEYBOARD_LISTENER)(SUS_WINDOW window, SUS_KEYBOARD_MESSAGE msg, SUS_KEYBOARD_EVENT event);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//								 Working with the window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set an action when closing
typedef enum sus_window_close_operation {
	SUS_WINDOW_DELETE_ON_CLOSE,		// Free up window resources when closing
	SUS_WINDOW_EXIT_ON_CLOSE,		// End the program when closing
	SUS_WINDOW_DO_NOTHING_ON_CLOSE,	// Do nothing when closing
	SUS_WINDOW_HIDE_ON_CLOSE		// Hide the window when closing
} SUS_WINDOW_CLOSE_OPERATION;
// Set a standard action when closing a window
VOID SUSAPI susWindowSetCloseOperation(_Inout_ SUS_WINDOW window, _In_ SUS_WINDOW_CLOSE_OPERATION closeOperation);

// -------------------------------------------------

// Timer Handler
typedef VOID(SUSAPI* SUS_WINDOW_TIMER_LISTENER)(_In_ SUS_WINDOW window, _In_ UINT uId);
// Set a timer
BOOL SUSAPI susWindowSetTimer(_In_ SUS_WINDOW window, _In_ UINT uId, _In_ UINT elapse);
// Delete the timer
BOOL SUSAPI susWindowKillTimer(_In_ SUS_WINDOW window, _In_ UINT uId);

// -------------------------------------------------

// Install a mouse listener
VOID SUSAPI susWindowSetMouseListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_LISTENER mouseListener);
// Install a mouse motion listener
VOID SUSAPI susWindowSetMouseMotionListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_MOTION_LISTENER mouseMotionListener);
// Install a mouse wheel listener
VOID SUSAPI susWindowSetMouseWheelListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_WHEEL_LISTENER mouseMotionListener);

// Install a keyboard listener
VOID SUSAPI susWindowSetKeyboardListener(_Inout_ SUS_WINDOW window, _In_ SUS_KEYBOARD_LISTENER keyboardListener);
// Install a timer listener
VOID SUSAPI susWindowSetTimerListener(_Inout_ SUS_WINDOW window, _In_ SUS_WINDOW_TIMER_LISTENER timerListener);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set window visibility
BOOL SUSAPI susWindowSetVisible(_In_ SUS_WINDOW window, _In_ BOOL enabled);
// Minimize the window
BOOL SUSAPI susWindowMinimize(_In_ SUS_WINDOW window);
// Expand the window
BOOL SUSAPI susWindowMaximize(_In_ SUS_WINDOW window);
// Restore the window after maximizing/minimizing
BOOL SUSAPI susWindowRestore(_In_ SUS_WINDOW window);
// Set Fullscreen for the window
BOOL SUSAPI susWindowFullscreen(_In_ SUS_WINDOW window, _In_ BOOL enabled);
// Set the status - always on top
BOOL SUSAPI susWindowSetAlwaysOnTop(_In_ SUS_WINDOW window, _In_ BOOL enabled);

// -------------------------------------------------

// Set the window position
BOOL SUSAPI susWindowSetPosition(_In_ SUS_WINDOW window, _In_ SUS_POINT position);
// Set the window size
BOOL SUSAPI susWindowSetSize(_In_ SUS_WINDOW window, _In_ SUS_SIZE size);
// Set the position and size of the window
BOOL SUSAPI susWindowSetBounds(_In_ SUS_WINDOW window, _In_ SUS_BOUNDS bounds);
// Get Window frames
SUS_BOUNDS SUSAPI susWindowGetBounds(_In_ SUS_WINDOW window);
// Get the window position
SUS_POINT SUSAPI susWindowGetPosition(_In_ SUS_WINDOW window);
// Get the window size
SUS_SIZE SUSAPI susWindowGetSize(_In_ SUS_WINDOW window);

// -------------------------------------------------

// Set the cursor for the window
HCURSOR SUSAPI susWindowSetCursor(_In_ SUS_WINDOW window, _In_ HCURSOR hCursor);
// Set the icon for the window
HCURSOR SUSAPI susWindowSetIcon(_In_ SUS_WINDOW window, _In_ HICON hIcon);
// Set the Menu for the window
BOOL SUSAPI susWindowSetMenuBar(_In_ SUS_WINDOW window, _In_ HMENU hMenu);
// Set transparency for the window (From 0 to 1)
BOOL SUSAPI susWindowSetTransparency(_In_ SUS_WINDOW window, _In_ sus_float transparency);
// Set the title for the window
BOOL SUSAPI susWindowSetTitle(_In_ SUS_WINDOW window, _In_ LPCWSTR title);
// Get a static window title
LPCWSTR SUSAPI susWindowGetTitle(_In_ SUS_WINDOW window);

// -------------------------------------------------

// Set your window data
SUS_OBJECT SUSAPI susWindowSetUserData(_In_ SUS_WINDOW window, _In_ SUS_OBJECT userData);
// Get your window information
SUS_OBJECT SUSAPI susWindowGetUserData(_In_ SUS_WINDOW window);

// Set a property for a window
BOOL SUSAPI susWindowSetProperty(_In_ SUS_WINDOW window, _In_ LPCWSTR key, _In_ LONG_PTR value);
// Get a window property
LONG_PTR SUSAPI susWindowGetProperty(_In_ SUS_WINDOW window, _In_ LPCWSTR key);

// -------------------------------------------------

// Set extended window styles
BOOL SUSAPI susWindowSetExStyle(_Inout_ SUS_WINDOW window, _In_ DWORD styles);
// Get extended window styles
DWORD SUSAPI susWindowGetExStyle(_In_ SUS_WINDOW window);
// Set window styles
BOOL SUSAPI susWindowSetStyle(_Inout_ SUS_WINDOW window, _In_ DWORD styles);
// Get window styles
DWORD SUSAPI susWindowGetStyle(_In_ SUS_WINDOW window);
// Get the system window handle
HWND SUSAPI susWindowGetSuper(_In_ SUS_WINDOW window);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											 WINDOW BUILDERS										 //
/*									  Building and creating a window								 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//								Builders of a simple window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The Window Builder
typedef struct sus_window_builder {
	WNDCLASSEXW			wcEx;					// Window Class structure
	CREATESTRUCTW		wStruct;				// Window creation structure
	WCHAR				classNameBuffer[64];	// Buffer for the class name
} SUS_WINDOW_BUILDER, *SUS_LPWINDOW_BUILDER;

// -------------------------------------------------

// Create a Window builder
SUS_WINDOW_BUILDER SUSAPI susWindowBuilder(_In_opt_ LPCWSTR lpTitle);

// Build a window
SUS_WINDOW SUSAPI susBuildWindow(_In_ SUS_WINDOW_BUILDER builder, _In_opt_ SUS_WINDOW_HANDLER handler);

// Create a window
SUS_WINDOW SUSAPI susNewWindow(_In_opt_ LPCWSTR title, _In_ SIZE size, _In_opt_ SUS_WINDOW_HANDLER handler);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//						  Working with a simple window builder							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set the window position
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetPosition(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ POINT position) {
	SUS_ASSERT(builder);
	builder->wStruct.x = position.x;
	builder->wStruct.y = position.y;
}
// Set the window size
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetSize(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ SIZE size) {
	SUS_ASSERT(builder);
	builder->wStruct.cx = size.cx;
	builder->wStruct.cy = size.cy;
}
// Set the position and size of the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetBounds(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ RECT rect) {
	SUS_ASSERT(builder);
	susWinBuilderSetPosition(builder, (POINT) { rect.left, rect.top });
	susWinBuilderSetSize(builder, (SIZE) { rect.right, rect.bottom });
}

// -------------------------------------------------

// Set the title for the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetTitle(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ LPCWSTR title) {
	SUS_ASSERT(builder);
	builder->wStruct.lpszName = title;
}
// Set the parameters for the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetParams(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ SUS_OBJECT lpCreateParams) {
	SUS_ASSERT(builder);
	builder->wStruct.lpCreateParams = lpCreateParams;
}
// Set the menu for the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetMenu(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ HMENU hMenu) {
	SUS_ASSERT(builder && hMenu);
	builder->wStruct.hMenu = hMenu;
}
// Set the icon for the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetIcon(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ HICON hIcon) {
	SUS_ASSERT(builder && hIcon);
	builder->wcEx.hIcon = hIcon;
}
// Set the cursor for the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetCursor(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ HCURSOR hCursor) {
	SUS_ASSERT(builder && hCursor);
	builder->wcEx.hCursor = hCursor;
}

// -------------------------------------------------

// Set the extended style for the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetExStyle(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ DWORD exstyle) {
	SUS_ASSERT(builder);
	builder->wStruct.dwExStyle = exstyle;
}
// Set the style for the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetStyle(_Inout_ SUS_LPWINDOW_BUILDER builder, _In_ DWORD style) {
	SUS_ASSERT(builder);
	builder->wStruct.style = style;
}
// Get the extended style from the window
SUS_FORCEINLINE DWORD SUSAPI susWinBuilderGetExStyle(_Inout_ SUS_LPWINDOW_BUILDER builder) {
	SUS_ASSERT(builder);
	return builder->wStruct.dwExStyle;
}
// Get the style from the window
SUS_FORCEINLINE DWORD SUSAPI susWinBuilderGetStyle(_Inout_ SUS_LPWINDOW_BUILDER builder) {
	SUS_ASSERT(builder);
	return builder->wStruct.style;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

#endif /* !_SUS_WINDOW_ */