// uicommon.h
//
#ifndef _SUS_UICOMMON_
#define _SUS_UICOMMON_

#include "core.h"

#pragma warning(push)
#pragma warning(disable: 4201)

// -----------------------------------------------

// A dot is a pixel on the screen
typedef struct sus_point {
	sus_int_t x, y;
} SUS_POINT, * SUS_LPPOINT;
// The size of the rectangle in pixels on the screen
typedef struct sus_size {
	sus_uint_t cx, cy;
} SUS_SIZE, * SUS_LPSIZE;
// Rectangle on the screen
typedef union sus_bounds {
	struct {
		SUS_POINT;
		SUS_SIZE;
	};
	struct {
		SUS_POINT pos;
		SUS_SIZE size;
	};
} SUS_BOUNDS, * SUS_LPBOUNDS;
// Rectangle on the screen
typedef union sus_rect {
	struct {
		sus_int_t left, top;
		sus_int_t right, bottom;
	};
	struct {
		SUS_POINT pos1;
		SUS_POINT pos2;
	};
} SUS_RECT, * SUS_LPRECT;
// RGBA Color
typedef union sus_color {
	sus_uint32_t ref;
	struct { sus_ubyte_t r, g, b, a; };
} SUS_COLOR, * SUS_LPCOLOR;
// 64 bit color RGBA
typedef struct sus_color64 {
	COLOR16 r, g, b, a;
} SUS_COLOR64, *SUS_LPCOLOR64;

// -----------------------------------------------

// Convert system rect to sus rect
SUS_INLINE SUS_RECT SUSAPI susRectToSusRect(_In_ const RECT rect) {
	return (SUS_RECT) { rect.left, rect.top, rect.right, rect.bottom };
}
// Convert rect to bounds parameters
SUS_INLINE SUS_BOUNDS SUSAPI susRectToBounds(_In_ const SUS_RECT rect) {
	return (SUS_BOUNDS) { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
}
// Convert bounds to rect parameters
SUS_INLINE SUS_RECT SUSAPI susBoundsToRect(_In_ const SUS_BOUNDS bounds) {
	return (SUS_RECT) { bounds.x, bounds.y, bounds.cx + bounds.x, bounds.cy + bounds.y };
}

// -----------------------------------------------

#define SUS_RGBA(red, green, blue, alpha) (SUS_COLOR) { .r = red, .g = green, .b = blue, .a = 255 - alpha }
#define SUS_RGB(red, green, blue) SUS_RGBA(red, green, blue, 255)

#define SUS_COLOR_RED		SUS_RGB(255, 0, 0)
#define SUS_COLOR_GREEN		SUS_RGB(0, 255, 0)
#define SUS_COLOR_BLUE		SUS_RGB(0, 0, 255)
#define SUS_COLOR_YELLOW	SUS_RGB(255, 255, 0)
#define SUS_COLOR_CYAN		SUS_RGB(0, 255, 255)
#define SUS_COLOR_GRAY		SUS_RGB(125, 125, 125)
#define SUS_COLOR_PINK		SUS_RGB(248, 24, 148)
#define SUS_COLOR_MAGENTA	SUS_RGB(255, 0, 255)
#define SUS_COLOR_BLACK		SUS_RGB(0, 0, 0)
#define SUS_COLOR_WHITE		SUS_RGB(255, 255, 255)

// -----------------------------------------------

#pragma warning(pop)

#endif // !_SUS_UICOMMON_
