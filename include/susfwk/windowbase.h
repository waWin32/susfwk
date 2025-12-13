// windowbase.h
//
#ifndef _SUS_WINDOW_BASE_
#define _SUS_WINDOW_BASE_

//////////////////////////////////////////////////////////////////////////////////////////
//									Defining structures									//
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)

// Position
typedef struct sus_point {
	sus_int x, y;
} SUS_POINT, *SUS_LPPOINT;
// Size
typedef struct sus_size {
	sus_int cx, cy;
} SUS_SIZE, *SUS_LPSIZE;
// Frames - position and size
typedef union sus_bounds {
	struct {
		SUS_POINT	p;
		SUS_SIZE	s;
	};
	struct {
		SUS_POINT;
		SUS_SIZE;
	};
} SUS_BOUNDS, *SUS_LPBOUNDS;

#pragma warning(pop)

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//									 System functions									//
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
	return (SIZE) { susGetScreenSizeX(), susGetScreenSizeY() };
}

// -------------------------------------------------

////////////////////////////////////////////////////////////////////////////////


#endif // !_SUS_WINDOW_BASE_
