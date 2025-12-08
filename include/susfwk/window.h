// window.h
//
#ifndef _SUS_WINDOW_API_
#define _SUS_WINDOW_API_

// ================================================================================================= //
// ************************************************************************************************* //
//										   WORKING WITH WINDOWS										 //
/*									Window operation and configuration								 */
// ************************************************************************************************* //
// ================================================================================================= //

// -------------------------------------------------

// Window structure
typedef struct sus_window SUS_WINDOW_STRUCT, *SUS_WINDOW;

// -------------------------------------------------

//////////////////////////////////////////////////////////////////////////////////////////
//							Working with the Input window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The modifier
typedef enum sus_window_key_modifier {
	SUS_MODIFIER_KEY_NONE,				// Missing modifier
	SUS_MODIFIER_KEY_LSHIFT = 1 << 0,		// The modifier key is LEFT SHIFT
	SUS_MODIFIER_KEY_LCTRL = 1 << 1,		// The modifier key is LEFT CTRL
	SUS_MODIFIER_KEY_LALT = 1 << 2,		// The modifier key is LEFT ALT
	SUS_MODIFIER_KEY_RSHIFT = 1 << 6,	// The modifier key is RIGHT SHIFT
	SUS_MODIFIER_KEY_RCTRL = 1 << 7,		// The modifier key is RIGHT CTRL
	SUS_MODIFIER_KEY_RALT = 1 << 8,		// The modifier key is RIGHT ALT
	SUS_MODIFIER_KEY_WIN = 1 << 3,		// The modifier key is WIN
	SUS_MODIFIER_KEY_CAPSLOCK = 1 << 4,	// The modifier key is CAPSLOCK
	SUS_MODIFIER_KEY_NUMLOCK = 1 << 5,	// The modifier key is NUMLOCK
	SUS_MODIFIER_KEY_SHIFT = SUS_MODIFIER_KEY_LSHIFT | SUS_MODIFIER_KEY_RSHIFT,
	SUS_MODIFIER_KEY_CTRL = SUS_MODIFIER_KEY_LCTRL | SUS_MODIFIER_KEY_RCTRL,
	SUS_MODIFIER_KEY_ALT = SUS_MODIFIER_KEY_LALT | SUS_MODIFIER_KEY_RALT
} SUS_MODIFIER_KEY;
// Get global modifiers
DWORD SUSAPI susSystemGetModifier();

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The mouse button
typedef enum sus_mouse_button {
	SUS_MOUSE_BUTTON_NONE,		// Unknown button
	SUS_MOUSE_BUTTON_LEFT,		// Unknown button
	SUS_MOUSE_BUTTON_MIDDLE,	// Unknown button
	SUS_MOUSE_BUTTON_RIGHT,		// Unknown button
	SUS_MOUSE_BUTTON_X1,		// Unknown button
	SUS_MOUSE_BUTTON_X2,		// Unknown button
	SUS_MOUSE_BUTTON_COUNT		// Count of mouse buttons
} SUS_MOUSE_BUTTON;
// The structure of the mouse event description
typedef struct sus_mouse_event {
	POINT				position;	// 
	DWORD				modifier;	// SUS_MODIFIER_KEY
	SUS_MOUSE_BUTTON	button;		// 
	ULONGLONG			time;		// 
	UINT				clickCount;	// 
} SUS_MOUSE_EVENT, * SUS_LPMOUSE_EVENT;
// Mouse messages
typedef enum sus_mouse_message {
	SUS_MOUSE_MESSAGE_UNKNOWN,	// Unknown message
	SUS_MOUSE_MESSAGE_CLICK,	// Unknown message
	SUS_MOUSE_MESSAGE_PRESS,	// Unknown message
	SUS_MOUSE_MESSAGE_RELEASE,	// Unknown message
	SUS_MOUSE_MESSAGE_ENTER,	// Unknown message
	SUS_MOUSE_MESSAGE_EXIT,		// Unknown message
	SUS_MOUSE_MESSAGE_MOVE		// Unknown message
} SUS_MOUSE_MESSAGE;
// The mouse listener
typedef VOID(SUSAPI* SUS_MOUSE_LISTENER)(SUS_WINDOW window, SUS_MOUSE_MESSAGE msg, SUS_MOUSE_EVENT event);

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
} SUS_KEYBOARD_EVENT, *SUS_LPKEYBOARD_EVENT;
// Keyboard messages
typedef enum sus_keyboard_message {
	SUS_KEYBOARD_MESSAGE_UNKNOWN,	// Unknown message
	SUS_KEYBOARD_MESSAGE_PRESS,		// Unknown message
	SUS_KEYBOARD_MESSAGE_RELEASE,	// Unknown message
	SUS_KEYBOARD_MESSAGE_TYPE,		// Unknown message
} SUS_KEYBOARD_MESSAGE;
// The kerboard listener
typedef VOID(SUSAPI* SUS_KEYBOARD_LISTENER)(SUS_WINDOW window, SUS_KEYBOARD_MESSAGE msg, SUS_KEYBOARD_EVENT event);

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							The Layout system for the window							//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Layout type
typedef enum sus_layout_type {
	SUS_LAYOUT_TYPE_ABSOLUTE,	// Absolute placement
	SUS_LAYOUT_TYPE_BORDER,		// Placement by borders
	SUS_LAYOUT_TYPE_FLOW,		// Place them in the list one after the other
	SUS_LAYOUT_TYPE_PAGE,		// Place only one, the others are hidden
	SUS_LAYOUT_TYPE_GRID		// Place only one, the others are hidden
} SUS_LAYOUT_TYPE;
//
typedef enum sus_layout_alignment {
	SUS_LAYOUT_ALIGMENT_START,
	SUS_LAYOUT_ALIGMENT_CENTER,
	SUS_LAYOUT_ALIGMENT_END,
	SUS_LAYOUT_ALIGMENT_STRETCH
} SUS_LAYOUT_ALIGMENT;

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
	SUS_WNDMSG_ON_CREATE,	// Unknown message\param param = NULL
	SUS_WNDMSG_ON_DESTROY,	// Unknown message\param param = NULL
	SUS_WNDMSG_ON_CLOSE,	// Unknown message\param param = NULL
	SUS_WNDMSG_ON_RESIZE,	// Unknown message\param param = SIZE (New size)
	SUS_WNDMSG_ON_MOVE,		// Unknown message\param param = POINT (New position)
	SUS_WNDMSG_ON_FOCUS,	// Unknown message\param param = BOOL (Focused)
} SUS_WINDOW_MESSAGE;
// Basic command Handler
typedef LRESULT(SUSAPI* SUS_WINDOW_HANDLER)(SUS_WINDOW widget, SUS_WINDOW_MESSAGE msg, LPARAM param);

// -------------------------------------------------

// The main window handler
LRESULT WINAPI susWindowHandler(
	_In_ HWND hWnd,
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

// -------------------------------------------------

// The main Window cycle
INT SUSAPI susWindowMainLoopA();
// The main Window cycle
INT SUSAPI susWindowMainLoopW();

#ifdef UNICODE
#define susWindowMainLoop	susWindowMainLoopW
#else // ELSE UNICODE
#define susWindowMainLoop	susWindowMainLoopA
#endif // !UNICODE

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//								 Working with the window								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set an action when closing
typedef enum sus_window_close_operation {
	SUS_WINDOW_EXIT_ON_CLOSE,		// End the program when closing
	SUS_WINDOW_DO_NOTHING_ON_CLOSE,	// Do nothing when closing
	SUS_WINDOW_HIDE_ON_CLOSE,		// Hide the window when closing
	SUS_WINDOW_DELETE_ON_CLOSE		// Free up window resources when closing
} SUS_WINDOW_CLOSE_OPERATION;

// Set a standard action when closing a window
VOID SUSAPI susWindowSetCloseOperation(_Inout_ SUS_WINDOW window, _In_ SUS_WINDOW_CLOSE_OPERATION closeOperation);

// -------------------------------------------------

// Set extended window styles
BOOL SUSAPI susWindowSetExStyle(_Inout_ SUS_WINDOW window, _In_ DWORD styles);
// Get extended window styles
DWORD SUSAPI susWindowGetExStyle(_In_ SUS_WINDOW window);
// Set window styles
BOOL SUSAPI susWindowSetStyle(_Inout_ SUS_WINDOW window, _In_ DWORD styles);
// Get window styles
DWORD SUSAPI susWindowGetStyle(_In_ SUS_WINDOW window);

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
BOOL SUSAPI susWindowSetPosition(_In_ SUS_WINDOW window, _In_ POINT position);
// Set the window size
BOOL SUSAPI susWindowSetSize(_In_ SUS_WINDOW window, _In_ SIZE size);
// Set the position and size of the window
BOOL SUSAPI susWindowSetBounds(_In_ SUS_WINDOW window, _In_ RECT bounds);
// Get Window frames
RECT SUSAPI susWindowGetBounds(_In_ SUS_WINDOW window);
// Get the window position
POINT SUSAPI susWindowGetPosition(_In_ SUS_WINDOW window);
// Get the window size
SIZE SUSAPI susWindowGetSize(_In_ SUS_WINDOW window);

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
BOOL SUSAPI susWindowSetTitleA(_In_ SUS_WINDOW window, _In_ LPCSTR title);
// Set the title for the window
BOOL SUSAPI susWindowSetTitleW(_In_ SUS_WINDOW window, _In_ LPCWSTR title);
// Get a static window title
LPCSTR SUSAPI susWindowGetTitleA(_In_ SUS_WINDOW window);
// Get a static window title
LPCWSTR SUSAPI susWindowGetTitleW(_In_ SUS_WINDOW window);

#ifdef UNICODE
#define susWindowSetTitle	susWindowSetTitleW
#define susWindowGetTitle	susWindowGetTitleW
#else
#define susWindowSetTitle	susWindowSetTitleA
#define susWindowGetTitle	susWindowGetTitleA
#endif // !UNICODE

// -------------------------------------------------

// Set a timer
BOOL SUSAPI susWindowSetTimer(_In_ SUS_WINDOW window, _In_ UINT uId, _In_ UINT Elapse, _In_ TIMERPROC lpTimerFunc);
// Delete the timer
BOOL SUSAPI susWindowKillTimer(_In_ SUS_WINDOW window, _In_ UINT uId);

// -------------------------------------------------

// Set your window data
SUS_OBJECT SUSAPI susWindowSetUserData(_In_ SUS_WINDOW window, _In_ SUS_OBJECT userData);
// Get your window information
SUS_OBJECT SUSAPI susWindowGetUserData(_In_ SUS_WINDOW window);

// Set a property for a window
BOOL SUSAPI susWindowSetProperty(_In_ SUS_WINDOW window, _In_ LPCSTR key, _In_ LONG_PTR value);
// Get a window property
LONG_PTR SUSAPI susWindowGetProperty(_In_ SUS_WINDOW window, _In_ LPCSTR key);

// -------------------------------------------------

// Install a mouse listener
VOID SUSAPI susWindowSetMouseListener(_Inout_ SUS_WINDOW window, _In_ SUS_MOUSE_LISTENER mouseListener);
// Install a keyboard listener
VOID SUSAPI susWindowSetKeyboardListener(_Inout_ SUS_WINDOW window, _In_ SUS_KEYBOARD_LISTENER keyboardListener);

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
typedef struct sus_window_builderA {
	WNDCLASSEXA		wcEx;					// Window Class structure
	CREATESTRUCTA	wStruct;				// Window creation structure
	CHAR			classNameBuffer[64];	// Buffer for the class name
} SUS_WINDOW_BUILDERA, *SUS_LPWINDOW_BUILDERA;
// The Window Builder
typedef struct sus_window_builderW {
	WNDCLASSEXW			wcEx;					// Window Class structure
	CREATESTRUCTW		wStruct;				// Window creation structure
	WCHAR				classNameBuffer[64];	// Buffer for the class name
} SUS_WINDOW_BUILDERW, *SUS_LPWINDOW_BUILDERW;

#ifdef UNICODE
#define SUS_WINDOW_BUILDER		SUS_WINDOW_BUILDERW
#define SUS_LPWINDOW_BUILDER	SUS_LPWINDOW_BUILDERW
#else // ELSE UNICODE
#define SUS_WINDOW_BUILDER		SUS_WINDOW_BUILDERA
#define SUS_LPWINDOW_BUILDER	SUS_LPWINDOW_BUILDERA
#endif // !UNICODE

// -------------------------------------------------

// Create a Window builder
SUS_WINDOW_BUILDERA SUSAPI susWindowBuilderA(_In_opt_ LPCSTR lpTitle);
// Create a Window builder
SUS_WINDOW_BUILDERW SUSAPI susWindowBuilderW(_In_opt_ LPCWSTR lpTitle);

#ifdef UNICODE
#define susWindowBuilder	susWindowBuilderW
#else // ELSE UNICODE
#define susWindowBuilder	susWindowBuilderA
#endif // !UNICODE

// -------------------------------------------------

// Build a window
SUS_WINDOW SUSAPI susBuildWindowA(_In_ SUS_WINDOW_BUILDERA builder, _In_opt_ SUS_WINDOW_HANDLER handler);
// Build a window
SUS_WINDOW SUSAPI susBuildWindowW(_In_ SUS_WINDOW_BUILDERW builder, _In_opt_ SUS_WINDOW_HANDLER handler);

#ifdef UNICODE
#define susBuildWindow		susBuildWindowW
#else // ELSE UNICODE
#define susBuildWindow		susBuildWindowA
#endif // !UNICODE

// -------------------------------------------------

// Create a window
SUS_WINDOW SUSAPI susNewWindowA(_In_opt_ LPCSTR title, _In_ SIZE size, _In_opt_ SUS_WINDOW_HANDLER handler);
// Create a window
SUS_WINDOW SUSAPI susNewWindowW(_In_opt_ LPCWSTR title, _In_ SIZE size, _In_opt_ SUS_WINDOW_HANDLER handler);

#ifdef UNICODE
#define susNewWindow		susNewWindowW
#else // ELSE UNICODE
#define susNewWindow		susNewWindowA
#endif // !UNICODE

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
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetTitleA(_Inout_ SUS_LPWINDOW_BUILDERA builder, _In_ LPCSTR title) {
	SUS_ASSERT(builder);
	builder->wStruct.lpszName = title;
}
// Set the title for the window
SUS_FORCEINLINE VOID SUSAPI susWinBuilderSetTitleW(_Inout_ SUS_LPWINDOW_BUILDERW builder, _In_ LPCWSTR title) {
	SUS_ASSERT(builder);
	builder->wStruct.lpszName = title;
}

// -------------------------------------------------

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

// ================================================================================================= //
// ************************************************************************************************* //
//											 WIDGET BUILDERS										 //
/*									  Building and creating a widgets								 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									  Widget Builder									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// The Widget Builder
typedef struct sus_widget_builderA {
	CREATESTRUCTA	wStruct;		// Widget creation structure
} SUS_WIDGET_BUILDERA, *SUS_LPWIDGET_BUILDERA;
// The Widget Builder
typedef struct sus_widget_builderW {
	CREATESTRUCTW		wStruct;	// Widget creation structure
} SUS_WIDGET_BUILDERW, *SUS_LPWIDGET_BUILDERW;

#ifdef UNICODE
#define SUS_WIDGET_BUILDER		SUS_WINDOW_BUILDERW
#define SUS_LPWIDGET_BUILDERW	SUS_LPWINDOW_BUILDERW
#else // ELSE UNICODE
#define SUS_WIDGET_BUILDER		SUS_WIDGET_BUILDERA
#define SUS_LPWIDGET_BUILDER	SUS_LPWIDGET_BUILDERA
#endif // !UNICODE

// -------------------------------------------------

// Create a Widget builder
SUS_WIDGET_BUILDERA SUSAPI susWidgetBuilderA(_In_opt_ LPCSTR lpClassName);
// Create a Widget builder
SUS_WIDGET_BUILDERW SUSAPI susWidgetBuilderW(_In_opt_ LPCWSTR lpClassName);

#ifdef UNICODE
#define susWidgetBuilder	susWidgetBuilderW
#else // ELSE UNICODE
#define susWidgetBuilder	susWidgetBuilderA
#endif // !UNICODE

// -------------------------------------------------

// Build a widget
SUS_WINDOW SUSAPI susBuildWidgetA(_In_opt_ SUS_WINDOW parent, _In_ UINT uId, _In_opt_ SUS_WINDOW_HANDLER handler, _In_ SUS_WIDGET_BUILDERA builder);
// Build a widget
SUS_WINDOW SUSAPI susBuildWidgetW(_In_opt_ SUS_WINDOW parent, _In_ UINT uId, _In_opt_ SUS_WINDOW_HANDLER handler, _In_ SUS_WIDGET_BUILDERW builder);

#ifdef UNICODE
#define susBuildWidget		susBuildWidgetW
#else // ELSE UNICODE
#define susBuildWidget		susBuildWidgetA
#endif // !UNICODE

// -------------------------------------------------

// Create a window
SUS_WINDOW SUSAPI susNewWidgetExA(
	_In_opt_ SUS_WINDOW parent, _In_opt_ LPCSTR title, _In_ SIZE size,
	_In_ UINT uId, _In_opt_ SUS_WINDOW_HANDLER handler, _In_opt_ LPCSTR lpClassName,
	_In_opt_ DWORD styles, _In_opt_ DWORD exStyles
);
// Create a window
SUS_WINDOW SUSAPI susNewWidgetExW(
	_In_opt_ SUS_WINDOW parent, _In_opt_ LPCWSTR title, _In_ SIZE size,
	_In_ UINT uId, _In_opt_ SUS_WINDOW_HANDLER handler, _In_opt_ LPCWSTR lpClassName,
	_In_opt_ DWORD styles, _In_opt_ DWORD exStyles
);

#ifdef UNICODE
#define susNewWidgetEx		susNewWidgetExW
#else // ELSE UNICODE
#define susNewWidgetEx		susNewWidgetExA
#endif // !UNICODE

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//							  Working with a widget builder								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

// Set the title for the widget
SUS_FORCEINLINE SUS_WIDGET_BUILDERA SUSAPI susWgSetTitleA(_In_opt_ LPCSTR title, _In_ SUS_WIDGET_BUILDERA builder) {
	builder.wStruct.lpszName = title ? title : "";
	return builder;
}
// Set the title for the widget
SUS_FORCEINLINE SUS_WIDGET_BUILDERW SUSAPI susWgSetTitleW(_In_opt_ LPCWSTR title, _In_ SUS_WIDGET_BUILDERW builder) {
	builder.wStruct.lpszName = title ? title : L"";
	return builder;
}

// -------------------------------------------------

// Set the widget position
SUS_FORCEINLINE SUS_WIDGET_BUILDERA SUSAPI susWgSetPosition(_In_ POINT position, _In_ SUS_WIDGET_BUILDERA builder) {
	builder.wStruct.x = position.x;
	builder.wStruct.y = position.y;
	return builder;
}
// Set the widget position
SUS_FORCEINLINE SUS_WIDGET_BUILDERW SUSAPI susWgSetPositionW(_In_ POINT position, _In_ SUS_WIDGET_BUILDERW builder) {
	builder.wStruct.x = position.x;
	builder.wStruct.y = position.y;
	return builder;
}
// Set the widget size
SUS_FORCEINLINE SUS_WIDGET_BUILDERA SUSAPI susWgSetSize(_In_ SIZE size, _In_ SUS_WIDGET_BUILDERA builder) {
	if (size.cx) builder.wStruct.cx = size.cx;
	if (size.cy) builder.wStruct.cy = size.cy;
	return builder;
}
// Set the widget size
SUS_FORCEINLINE SUS_WIDGET_BUILDERW SUSAPI susWgSetSizeW(_In_ SIZE size, _In_ SUS_WIDGET_BUILDERW builder) {
	if (size.cx) builder.wStruct.cx = size.cx;
	if (size.cy) builder.wStruct.cy = size.cy;
	return builder;
}
// Set the position and size of the widget
SUS_FORCEINLINE SUS_WIDGET_BUILDERA SUSAPI susWgSetBounds(_In_ RECT rect, _In_ SUS_WIDGET_BUILDERA builder) {
	return susWgSetPosition((POINT) { rect.left, rect.top }, susWgSetSize((SIZE) { rect.right, rect.bottom }, builder));
}
// Set the position and size of the widget
SUS_FORCEINLINE SUS_WIDGET_BUILDERW SUSAPI susWgSetBoundsW(_In_ RECT rect, _In_ SUS_WIDGET_BUILDERW builder) {
	return susWgSetPositionW((POINT) { rect.left, rect.top }, susWgSetSizeW((SIZE) { rect.right, rect.bottom }, builder));
}

// -------------------------------------------------

// Set the extended style for the widget
SUS_FORCEINLINE SUS_WIDGET_BUILDERA SUSAPI susWgAddExStyle(_In_ DWORD exstyle, _In_ SUS_WIDGET_BUILDERA builder) {
	builder.wStruct.dwExStyle |= exstyle;
	return builder;
}
// Set the extended style for the widget
SUS_FORCEINLINE SUS_WIDGET_BUILDERW SUSAPI susWgAddExStyleW(_In_ DWORD exstyle, _In_ SUS_WIDGET_BUILDERW builder) {
	builder.wStruct.dwExStyle |= exstyle;
	return builder;
}
// Set the style for the widget
SUS_FORCEINLINE SUS_WIDGET_BUILDERA SUSAPI susWgAddStyle(_In_ DWORD style, _In_ SUS_WIDGET_BUILDERA builder) {
	builder.wStruct.style |= style;
	return builder;
}
// Set the style for the widget
SUS_FORCEINLINE SUS_WIDGET_BUILDERW SUSAPI susWgAddStyleW(_In_ DWORD style, _In_ SUS_WIDGET_BUILDERW builder) {
	builder.wStruct.style |= style;
	return builder;
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//										  ADDITIONAL FUNCTIONS										 //
/*									   Additional useful functions									 */
// ************************************************************************************************* //
// ================================================================================================= //

//////////////////////////////////////////////////////////////////////////////////////////
//									Simple screen functions								//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

#endif /* !_SUS_WINDOW_API_ */
