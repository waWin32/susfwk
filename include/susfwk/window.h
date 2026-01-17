// window.h
//
#ifndef _SUS_WINDOW_
#define _SUS_WINDOW_

#include "graphics.h"
#include "renderer.h"
#pragma warning(push)
#pragma warning(disable: 4201)

// Window - Working with the UI
// v2.1
/*
* Window - api for working with windows and widgets
* It is closely related to graphical modules
* and provides a convenient interface for working with them
* 
* How to create a window?
* To create a window, you need to:
* 1. Create a frame
* 2. Configure it
* Then, decide which render type you want to use
* 1. Just GDI ->
* - Create a panel on the frame
* - Install widgets
* 2. Modern graphics ->
* - Install renderer on the frame
* - In main, create an uninterruptible loop - render
* - Do not install widgets!
* 
* Code examples at the end of the file.
* For more documentation, just study the code.
*/

// ================================================================================================= //
// ************************************************************************************************* //
//										THE CORE OF WINDOW API										 //
/*									Basic definitions of structures									 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									Window Structures									//
//////////////////////////////////////////////////////////////////////////////////////////

// How does the window system work?
/*
* The window system is based on the principle of inheritance
* SUS_WINDOW - general window structure, does not have specific functions
* SUS_WIDGET - extension of SUS_WINDOW and contains the GDi Graphics and Layout components
* SUS_FRAME is an extension of SUS_WINDOW that contains GPU graphics components and additional window functions
* 
* The inheritance system - 
* SUS_FRAME or SUS_WIDGET can be cast to the SUS_WINDOW type as pointers
* If you are sure that SUS_WINDOW is SUS_FRAME or SUS_WIDGET,
* you can simply cast the types.
*/

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Basic window structure
typedef struct sus_window SUS_WINDOW_STRUCT, * SUS_WINDOW;

// -------------------------------------------------

// Window frame
typedef struct sus_frame SUS_FRAME_STRUCT, * SUS_FRAME;

// -------------------------------------------------

// Widget struct
typedef struct sus_widget SUS_WIDGET_STRUCT, * SUS_WIDGET;

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//				 Low-level functions for working with the system window					//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Get userData parameters by lParam
SUS_INLINE SUS_OBJECT susGetWindowParam(LPARAM lParam) {
	CONST LPCREATESTRUCT pCreate = (CONST LPCREATESTRUCT)lParam;
	return pCreate ? pCreate->lpCreateParams : NULL;
}
// Save data to a window
SUS_INLINE SUS_OBJECT susWindowWriteData(HWND hWnd, LONG_PTR dwNewLong) {
	SetWindowLongPtr(hWnd, GWLP_USERDATA, dwNewLong);
	return (SUS_OBJECT)dwNewLong;
}
// Get a window from a window procedure
SUS_INLINE SUS_OBJECT susWindowLoadData(HWND hWnd) {
	return (SUS_OBJECT)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

// -------------------------------------------------

// Get the location of an object located in the center of the area
SUS_INLINE SUS_POINT SUSAPI susGetCenterPos(_In_ SIZE objSize, _In_ SIZE areaSize) {
	return (SUS_POINT) { (areaSize.cx - objSize.cx) / 2, (areaSize.cy - objSize.cy) / 2 };
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
SUS_INLINE SUS_SIZE SUSAPI susGetScreenSize() {
	return (SUS_SIZE) { susGetScreenSizeX(), susGetScreenSizeY() };
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//									The main window handler								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The main Window cycle
INT SUSAPI susWindowMainLoop();
// Process current window messages, Process current window messages without blocking the flow
BOOL SUSAPI susWindowProcessing(_Out_ LPMSG msg);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//								Useful validation functions								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Check if the window is valid
BOOL SUSAPI susWindowIsValid(_In_ SUS_WINDOW window);
// Check if the window is a valid widget
BOOL SUSAPI susWindowIsWidget(_In_ SUS_WIDGET window);
// Check if the window is a valid frame
BOOL SUSAPI susWindowIsFrame(_In_ SUS_FRAME window);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											 WINDOW LISTENERS										 //
/*								Processing system operations with a window							 */
// ************************************************************************************************* //
// ================================================================================================= //

// How does the listener system work?
/*
* Listeners are a way to receive information from the system
* Listeners cannot replace the default functionality.
* Listeners != handler
* All listeners are flexible and can be installed or not installed (optimally).
* Listeners can be installed on any SUS_WINDOW structure descendant.
*/

//////////////////////////////////////////////////////////////////////////////////////////
//								Window message processing								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Window Messages
typedef enum sus_winmsg {
	SUS_WINMSG_UNKNOWN,		// Unknown message
	SUS_WINMSG_CREATE,		// The window has been created\param param: NULL
	SUS_WINMSG_DESTROY,		// The window is destroyed\param param: NULL
	SUS_WINMSG_FOCUS,		// The window has gained/lost focus\param param: BOOL(enabled)
	SUS_WINMSG_MOVE,		// The window was moved\param param: SUS_POINT
	SUS_WINMSG_RESIZE,		// The window has changed size\param param: SUS_SIZE
	SUS_WINMSG_CLOSE,		// The window received a closing message\param param: NULL
	SUS_WINMSG_MINIMIZED,	// The window was rolled down\param param: NULL
	SUS_WINMSG_TIMER,		// The window call timer\param param: id
	SUS_WINMSG_MAXIMIZED,	// The window has been expanded to full screen\param param: NULL
	SUS_WINMSG_USER			// The User messages
} SUS_WINMSG;

// Window Listener
typedef SUS_RESULT(SUSAPI* SUS_WINDOW_LISTENER)(SUS_WINDOW window, SUS_WINMSG msg, SUS_PARAM param);

// Send a message immediately
SUS_RESULT SUSAPI susWindowSendMessage(_In_ SUS_WINDOW window, _In_ UINT msg, SUS_PARAM param);
// Send a message
BOOL SUSAPI susWindowPostMessage(_In_ SUS_WINDOW window, _In_ UINT msg, SUS_PARAM param);

// -------------------------------------------------

// Timer Handler
typedef VOID(SUSAPI* SUS_WINDOW_TIMER_LISTENER)(_In_ SUS_WINDOW window, _In_ UINT uId);
// Install a timer handler for the window
VOID SUSAPI susWindowSetTimerListener(_Inout_ SUS_WINDOW window, _In_opt_ SUS_WINDOW_TIMER_LISTENER timerListener);

// Set a timer
BOOL SUSAPI susWindowSetTimer(_In_ SUS_WINDOW window, _In_ UINT id, _In_ UINT elapse);
// Delete the timer
BOOL SUSAPI susWindowKillTimer(_In_ SUS_WINDOW window, _In_ UINT id);

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//									Getting Modifiers									//
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

//////////////////////////////////////////////////////////////////////////////////////////
//								Working with the mouse									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

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
		FLOAT		wheelDelta;	// Delta of the wheel\param EVENT: MouseWheel
	};
} SUS_MOUSE_EVENT, * SUS_LPMOUSE_EVENT;
// Mouse messages
typedef enum sus_mouse_message {
	SUS_MOUSE_MESSAGE_UNKNOWN,	// Unknown message
	SUS_MOUSE_MESSAGE_CLICK,	// Mouse click
	SUS_MOUSE_MESSAGE_PRESS,	// The mouse button is clamped
	SUS_MOUSE_MESSAGE_RELEASE	// The mouse button is released
} SUS_MOUSE_MESSAGE;
// Mouse motion messages
typedef enum sus_mouse_motion_message {
	SUS_MOUSE_MOTION_MESSAGE_UNKNOWN,	// Unknown message
	SUS_MOUSE_MOTION_MESSAGE_MOVE,		// The mouse has moved
	SUS_MOUSE_MOTION_MESSAGE_HOVER,		// The mouse hovered over the window
	SUS_MOUSE_MOTION_MESSAGE_ENTER,		// The mouse got into the window
	SUS_MOUSE_MOTION_MESSAGE_EXIT		// The mouse went out the window
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

// Install a mouse listener
VOID SUSAPI susWindowSetMouseListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_LISTENER mouseListener);
// Install a mouse motion listener
VOID SUSAPI susWindowSetMouseMotionListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_MOTION_LISTENER mouseMotionListener);
// Install a mouse wheel listener
VOID SUSAPI susWindowSetMouseWheelListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_WHEEL_LISTENER mouseWheelListener);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Working with the keyboard									//
//////////////////////////////////////////////////////////////////////////////////////////

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
	SUS_KEYBOARD_MESSAGE_CHAR		// Character input
} SUS_KEYBOARD_MESSAGE;
// The kerboard listener
typedef VOID(SUSAPI* SUS_KEYBOARD_LISTENER)(SUS_WINDOW window, SUS_KEYBOARD_MESSAGE msg, SUS_KEYBOARD_EVENT event);

// Install a keyboard listener
VOID SUSAPI susWindowSetKeyboardListener(_Inout_ SUS_WINDOW window, _In_ SUS_KEYBOARD_LISTENER keyboardListener);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											WINDOW FRAME BUILDER									 //
/*								 Building and configuring the window frame							 */
// ************************************************************************************************* //
// ================================================================================================= //

// How do I create a window?
/*
* There are two ways to create a window in susfwk -
* 1. Simple - through the susNewX function
* 2. Detailed - use window builders
* In most cases, it is better to always use the simple option.
*/

//////////////////////////////////////////////////////////////////////////////////////////
//									 Builder functions									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The frame Builder
typedef struct sus_frame_builder {
	WNDCLASSEXW			wcEx;					// Window Class structure
	CREATESTRUCTW		wStruct;				// Window creation structure
	WCHAR				classNameBuffer[64];	// Buffer for the class name
} SUS_FRAME_BUILDER, *SUS_LPFRAME_BUILDER;

// -------------------------------------------------

// Create a Window builder
SUS_FRAME_BUILDER SUSAPI susFrameBuilder(_In_opt_ LPCWSTR title);

// Build a window
SUS_FRAME SUSAPI susBuildFrame(_In_ SUS_LPFRAME_BUILDER builder, _In_opt_ SUS_WINDOW_LISTENER handler);

// Create a window
SUS_FRAME SUSAPI susNewFrame(_In_opt_ LPCWSTR title, _In_ SUS_SIZE size, _In_opt_ SUS_WINDOW_LISTENER handler);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//						  Working with a frame window builder							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set the window position
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetPosition(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ SUS_POINT position) {
	SUS_ASSERT(builder);
	builder->wStruct.x = position.x;
	builder->wStruct.y = position.y;
}
// Set the window size
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetSize(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ SUS_SIZE size) {
	SUS_ASSERT(builder);
	builder->wStruct.cx = size.cx;
	builder->wStruct.cy = size.cy;
}
// Set the position and size of the window
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetBounds(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ SUS_BOUNDS bounds) {
	SUS_ASSERT(builder);
	susFrameBuilderSetPosition(builder, (SUS_POINT) { bounds.x, bounds.y });
	susFrameBuilderSetSize(builder, (SUS_SIZE) { bounds.cx, bounds.cy });
}
// Set the title for the window
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetTitle(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ LPCWSTR title) {
	SUS_ASSERT(builder);
	builder->wStruct.lpszName = title;
}

// -------------------------------------------------

// Set the parameters for the window
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetParam(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ SUS_OBJECT lpCreateParams) {
	SUS_ASSERT(builder);
	builder->wStruct.lpCreateParams = lpCreateParams;
}
// Set the menu for the window
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetMenu(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ HMENU hMenu) {
	SUS_ASSERT(builder && hMenu);
	builder->wStruct.hMenu = hMenu;
}
// Set the icon for the window
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetIcon(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ HICON hIcon) {
	SUS_ASSERT(builder && hIcon);
	builder->wcEx.hIcon = hIcon;
}
// Set the cursor for the window
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetCursor(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ HCURSOR hCursor) {
	SUS_ASSERT(builder && hCursor);
	builder->wcEx.hCursor = hCursor;
}
// Install with the ability to change the size
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderSetResizable(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ BOOL enabled) {
	SUS_ASSERT(builder);
	builder->wStruct.style = enabled ? builder->wStruct.style | (WS_MAXIMIZEBOX | WS_THICKFRAME) : builder->wStruct.style & ~(WS_MAXIMIZEBOX | WS_THICKFRAME);
}

// -------------------------------------------------

// Add the extended styles for the window
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderAddExStyle(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ DWORD exstyle) {
	SUS_ASSERT(builder);
	builder->wStruct.dwExStyle |= exstyle;
}
// Add the styles for the window
SUS_FORCEINLINE VOID SUSAPI susFrameBuilderAddStyle(_Inout_ SUS_LPFRAME_BUILDER builder, _In_ DWORD style) {
	SUS_ASSERT(builder);
	builder->wStruct.style |= style;
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

// The Widget Builder
typedef struct sus_widget_builder {
	CREATESTRUCTW wStruct; // Window creation structure
} SUS_WIDGET_BUILDER, * SUS_LPWIDGET_BUILDER;

// -------------------------------------------------

// Create a Window builder
SUS_WIDGET_BUILDER SUSAPI susWidgetBuilder(_In_opt_ LPCWSTR className);

// Build a window
SUS_WIDGET SUSAPI susBuildWidget(_In_ SUS_LPWIDGET_BUILDER builder, _In_ SUS_WINDOW parent, _In_opt_ SUS_WINDOW_LISTENER handler);

// Create a window
SUS_WIDGET SUSAPI susNewWidget(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR className, _In_opt_ LPCWSTR title, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ DWORD styles);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

#define SUS_WIDGET_RESERVED_IDS		0xff
#define SUS_WIDGET_CLASSNAME_BUTTON L"BUTTON"
#define SUS_WIDGET_CLASSNAME_STATIC L"STATIC"
#define SUS_WIDGET_CLASSNAME_EDIT	L"EDIT"

// -------------------------------------------------

// Create a button
SUS_WIDGET SUSAPI susNewButton(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR title, _In_opt_ SUS_WINDOW_LISTENER handler);
// Create a checkbox button
SUS_WIDGET SUSAPI susNewCheckBox(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR title, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ BOOL selected);
// Create a switch
SUS_WIDGET SUSAPI susNewRadioButton(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR title, _In_opt_ SUS_WINDOW_LISTENER handler);

// -------------------------------------------------

// The type of alignment for the widget text
typedef enum sus_widget_text_align {
	SUS_WIDGET_TEXT_ALIGN_NONE,
	SUS_WIDGET_TEXT_ALIGN_LEFT,
	SUS_WIDGET_TEXT_ALIGN_CENTER,
	SUS_WIDGET_TEXT_ALIGN_RIGHT
} SUS_WIDGET_TEXT_ALIGN;

// Create a label
SUS_WIDGET SUSAPI susNewLabel(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR text, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ SUS_WIDGET_TEXT_ALIGN align);
// Create a panel
SUS_WIDGET SUSAPI susNewPanel(_In_ SUS_WINDOW parent, _In_opt_ SUS_WINDOW_LISTENER handler);
// Create a text input field
SUS_WIDGET SUSAPI susNewTextField(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR text, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ SUS_WIDGET_TEXT_ALIGN align);
// Create a text entry area
SUS_WIDGET SUSAPI susNewTextArea(_In_ SUS_WINDOW parent, _In_opt_ LPCWSTR text, _In_opt_ SUS_WINDOW_LISTENER handler, _In_opt_ SUS_WIDGET_TEXT_ALIGN align);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//						  Working with a widgets window builder							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set the window position
SUS_FORCEINLINE VOID SUSAPI susWidgetBuilderSetPosition(_Inout_ SUS_LPWIDGET_BUILDER builder, _In_ SUS_POINT position) {
	SUS_ASSERT(builder);
	builder->wStruct.x = position.x;
	builder->wStruct.y = position.y;
}
// Set the window size
SUS_FORCEINLINE VOID SUSAPI susWidgetBuilderSetSize(_Inout_ SUS_LPWIDGET_BUILDER builder, _In_ SUS_SIZE size) {
	SUS_ASSERT(builder);
	builder->wStruct.cx = size.cx;
	builder->wStruct.cy = size.cy;
}
// Set the position and size of the window
SUS_FORCEINLINE VOID SUSAPI susWidgetBuilderSetBounds(_Inout_ SUS_LPWIDGET_BUILDER builder, _In_ SUS_BOUNDS bounds) {
	SUS_ASSERT(builder);
	susWidgetBuilderSetPosition(builder, (SUS_POINT) { bounds.x, bounds.y });
	susWidgetBuilderSetSize(builder, (SUS_SIZE) { bounds.cx, bounds.cy });
}
// Set the title for the window
SUS_FORCEINLINE VOID SUSAPI susWidgetBuilderSetTitle(_Inout_ SUS_LPWIDGET_BUILDER builder, _In_ LPCWSTR title) {
	SUS_ASSERT(builder);
	builder->wStruct.lpszName = title;
}
// Set the parameters for the window
SUS_FORCEINLINE VOID SUSAPI susWidgetBuilderSetParams(_Inout_ SUS_LPWIDGET_BUILDER builder, _In_ SUS_OBJECT lpCreateParams) {
	SUS_ASSERT(builder);
	builder->wStruct.lpCreateParams = lpCreateParams;
}

// -------------------------------------------------

// Add the extended styles for the window
SUS_FORCEINLINE VOID SUSAPI susWidgetBuilderAddExStyle(_Inout_ SUS_LPWIDGET_BUILDER builder, _In_ DWORD exstyle) {
	SUS_ASSERT(builder);
	builder->wStruct.dwExStyle |= exstyle;
}
// Add the styles for the window
SUS_FORCEINLINE VOID SUSAPI susWidgetBuilderAddStyle(_Inout_ SUS_LPWIDGET_BUILDER builder, _In_ DWORD style) {
	SUS_ASSERT(builder);
	builder->wStruct.style |= style;
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
BOOL SUSAPI susWindowListenerHandler(_In_ SUS_WINDOW window, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

// -------------------------------------------------

// Basic window treatment
BOOL SUSAPI susWindowHandler(SUS_WINDOW window, UINT uMsg, WPARAM wParam, LPARAM lParam);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//						 Functions for working with the window							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set window visibility
BOOL SUSAPI susWindowSetVisible(_In_ SUS_WINDOW window, _In_ BOOL enabled);
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
// Get the client area of the window
SUS_SIZE SUSAPI susWindowGetContent(_In_ SUS_WINDOW window);
// Set the title for the window
BOOL SUSAPI susWindowSetText(_In_ SUS_WINDOW window, _In_ LPCWSTR title);
// Get a dynamic window title
LPWSTR SUSAPI susWindowGetText(_In_ SUS_WINDOW window);
// Set transparency for the window (From 0 to 1)
BOOL SUSAPI susWindowSetTransparency(_In_ SUS_WINDOW window, _In_ sus_float_t transparency);

// -------------------------------------------------

// Set user window data
SUS_USERDATA SUSAPI susWindowSetData(_In_ SUS_WINDOW window, _In_ SUS_USERDATA userData);
// Get user window data
SUS_USERDATA SUSAPI susWindowGetData(_In_ SUS_WINDOW window);
// Set a property for a window
BOOL SUSAPI susWindowSetProperty(_In_ SUS_WINDOW window, _In_ LPCWSTR key, _In_ SUS_PARAM value);
// Get a window property
SUS_PARAM SUSAPI susWindowGetProperty(_In_ SUS_WINDOW window, _In_ LPCWSTR key);
// Get a widget from a window
SUS_WIDGET SUSAPI susWindowGetWidget(_In_ SUS_WINDOW window, _In_ UINT id);
// Set your id for the window
VOID SUSAPI susWindowSetId(_In_ SUS_WINDOW window, _In_ UINT id);

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
// Get the window class name as a static string
LPCWSTR SUSAPI susWindowGetClassName(_In_ SUS_WINDOW window);
// Get the parent's window
SUS_WINDOW SUSAPI susWindowGetParent(_In_ SUS_WINDOW window);
// Set the window background color
VOID SUSAPI susWindowSetBackground(_In_ SUS_WINDOW window, _In_ SUS_COLOR color);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//										  WORKING WITH FRAME										 //
/*								    Frame operation and processing									 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// System Window Handler
LRESULT WINAPI susFrameSystemHandler(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Functions for working with the window						//
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
VOID SUSAPI susWindowSetCloseOperation(_Inout_ SUS_FRAME window, _In_ SUS_WINDOW_CLOSE_OPERATION closeOperation);

// -------------------------------------------------

// Set Fullscreen for the window
BOOL SUSAPI susWindowSetFullscreen(_In_ SUS_FRAME window, _In_ BOOL enabled);
// Minimize the window
BOOL SUSAPI susWindowMinimize(_In_ SUS_FRAME window);
// Expand the window
BOOL SUSAPI susWindowMaximize(_In_ SUS_FRAME window);
// Restore the window after maximizing/minimizing
BOOL SUSAPI susWindowRestore(_In_ SUS_FRAME window);
// Set the status - always on top
BOOL SUSAPI susWindowSetAlwaysOnTop(_In_ SUS_FRAME window, _In_ BOOL enabled);
// Set the cursor for the window
HCURSOR SUSAPI susWindowSetCursor(_In_ SUS_FRAME window, _In_ HCURSOR hCursor);
// Set the icon for the window
HICON SUSAPI susWindowSetIcon(_In_ SUS_FRAME window, _In_ HICON hIcon);
// Set the Menu for the window
BOOL SUSAPI susWindowSetMenuBar(_In_ SUS_FRAME window, _In_ HMENU hMenu);
// Set the minimum size for the window
VOID SUSAPI susWindowSetMinimumSize(_In_ SUS_FRAME window, _In_ SUS_SIZE minSize);
// Set the maximum size for the window
VOID SUSAPI susWindowSetMaximumSize(_In_ SUS_FRAME window, _In_ SUS_SIZE maxSize);
// Set a fixed size for the window
VOID SUSAPI susWindowSetFixedSize(_In_ SUS_FRAME window, _In_ SUS_SIZE size);
// Install a renderer for an OpenGL-based window
SUS_RENDERER SUSAPI susWindowSetRenderer(_In_ SUS_FRAME window, _In_ BOOL enable);
// Get a window renderer
SUS_RENDERER SUSAPI susWindowGetRenderer(_In_ SUS_FRAME window);

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

// Layout data cache
typedef union sus_layout_cache {
	struct {
		sus_float_t x, y;		// Relative window position, ranging from 0 to 1.
		sus_float_t cx, cy;	// Relative window size, ranging from 0 to 1.
	};
	sus_float_t arr[4]; // Value 0 - ignoring the parameter
} SUS_LAYOUT;

// -------------------------------------------------

// Update the widget layout after resizing the window
VOID SUSAPI susWidgetUpdateLayout(_In_ SUS_WIDGET widget, _In_ SUS_SIZE newScreenSize);

// Set a new layout for the widget
VOID SUSAPI susWidgetSetLayout(_In_ SUS_WIDGET widget, SUS_LAYOUT layout);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//										Window Handler									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// System Window Handler
LRESULT WINAPI susWidgetSystemHandler(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam,
	UINT_PTR uIdSubclass,
	DWORD_PTR dwRefData
);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							Functions for working with the widget						//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Paint Listener
typedef VOID(SUSAPI* SUS_WINDOW_PAINT_COMPONENT)(_In_ SUS_WIDGET window, _In_ SUS_GRAPHICS gr);
// Enable/disable rendering processing
VOID SUSAPI susWindowSetPaintComponent(_In_ SUS_WIDGET window, _In_ SUS_WINDOW_PAINT_COMPONENT paintComponent);
// Get the context of the window graphics
SUS_LPGRAPHICS_CONTEXT SUSAPI susWindowGetGraphics(_In_ SUS_WIDGET window);
// Redraw the entire window
VOID SUSAPI susWindowRepaint(_In_ SUS_WIDGET window);
// immediately redraw the window
VOID SUSAPI susWindowRepaintImmediately(_In_ SUS_WIDGET window);

// -------------------------------------------------

// Window background rendering component
typedef VOID(SUSAPI* SUS_WINDOW_BACKGROUND_COMPONENT)(_In_ SUS_WIDGET window, _In_ SUS_GRAPHICS gr);
// Install the background rendering component
VOID SUSAPI susWindowSetBackgroundComponent(_In_ SUS_WIDGET window, _In_ SUS_WINDOW_BACKGROUND_COMPONENT backgroundComponent);
// Standard window drawing
VOID SUSAPI susWindowDefBackgroundComponent(_In_ SUS_WIDGET window, _In_ SUS_GRAPHICS gr);
// The standard window drawing is an image
VOID SUSAPI susWindowDefBackgroundComponentImage(_In_ SUS_WIDGET window, _In_ SUS_GRAPHICS gr);

// -------------------------------------------------

// Install a double buffering system
BOOL SUSAPI susWindowSetDoubleBuffer(_Inout_ SUS_WIDGET window, _In_ BOOL enabled);

// -------------------------------------------------

// A component for working with graphics
typedef struct sus_window_graphics_component {
	SUS_GRAPHICS_CONTEXT			context;
	SUS_WINDOW_PAINT_COMPONENT		paintComponent;
	SUS_WINDOW_BACKGROUND_COMPONENT backgroundComponent;
} SUS_WINDOW_GRAPHICS_COMPONENT;

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
//											Code examples											//
//////////////////////////////////////////////////////////////////////////////////////////////////////

/* An example of a simple windowed application
// Main frame handler
SUS_RESULT SUSAPI FrameHandler(SUS_FRAME frame, SUS_WINMSG msg, SUS_PARAM param);
// Main panel handler
SUS_RESULT SUSAPI PanelHandler(SUS_WIDGET panel, SUS_WINMSG msg, SUS_PARAM param);

int main() {
	SUS_FRAME frame = susNewFrame(L"Application", (SUS_SIZE) { 800, 600 }, FrameHandler);
	if (!frame) sus_error(1);
	sus_exit(susWindowMainLoop());
}

// Window Data
typedef struct main_window {
	struct {
		CHAR dummy;
		// ...
	} data; // Data
	struct {
		CHAR dummy;
		// ...
	} wgs; // Widgets
	struct {
		CHAR dummy;
		// ...
	} res; // Resources
} MAIN_WINDOW_STRUCT, *MAIN_WINDOW;

// Main frame handler
SUS_RESULT SUSAPI FrameHandler(SUS_FRAME frame, SUS_WINMSG msg, SUS_PARAM param)
{
	switch (msg)
	{
	case SUS_WINMSG_CREATE: {
		susWindowSetData(frame, sus_malloc(sizeof(MAIN_WINDOW_STRUCT)));
		SUS_ASSERT(susWindowGetData(frame));
		susWindowSetCloseOperation(frame, SUS_WINDOW_EXIT_ON_CLOSE);
		SUS_WIDGET panel = susNewPanel(frame, PanelHandler);
		susWindowSetDoubleBuffer(panel, TRUE);
		susWindowSetBackground(panel, SUS_COLOR_MAGENTA);
		susWindowSetVisible(frame, TRUE);
	} return 0;
	case SUS_WINMSG_DESTROY: {
		sus_free(susWindowGetData(frame));
	} return 0;
	default: return 0;
	}
}
// Main panel handler
SUS_RESULT SUSAPI PanelHandler(SUS_WIDGET panel, SUS_WINMSG msg, SUS_PARAM param)
{
	MAIN_WINDOW window = susWindowGetData(panel);
	switch (msg)
	{
	case SUS_WINMSG_CREATE: {
		// Widget Placement
	} return 0;
	}
}
*/

#pragma warning(pop)

#endif /* !_SUS_WINDOW_ */