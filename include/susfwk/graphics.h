// graphics.h
//
#ifndef _SUS_GRAPHICS_
#define _SUS_GRAPHICS_

#pragma warning(push)
#pragma warning(disable: 4201)

////////////////////////////////////////////////////////////////////////////////////////////////////
//									Basic definitions of structures      						  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// A dot is a pixel on the screen
typedef struct sus_point {
	sus_int x, y;
} SUS_POINT, * SUS_LPPOINT;
// The size of the rectangle in pixels on the screen
typedef struct sus_size {
	sus_uint cx, cy;
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
		sus_int left, top;
		sus_int right, bottom;
	};
	struct {
		SUS_POINT pos1;
		SUS_POINT pos2;
	};
} SUS_RECT, *SUS_LPRECT;
// RGBA Color
typedef union sus_color {
	COLORREF ref;
	struct { BYTE r, g, b, a; };
} SUS_COLOR;

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

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

#define SUS_RGB(r, g, b) (SUS_COLOR){ RGB(r, g, b) }
#define SUS_RGBA(r, g, b, alpha) (SUS_COLOR){ .ref = RGB(r, g, b), .a = alpha }

#define SUS_COLOR_RED		SUS_RGB(255, 0, 0)
#define SUS_COLOR_GREEN		SUS_RGB(0, 255, 0)
#define SUS_COLOR_BLUE		SUS_RGB(0, 0, 255)
#define SUS_COLOR_YELLOW	SUS_RGB(255, 255, 0)
#define SUS_COLOR_CYAN		SUS_RGB(0, 255, 255)
#define SUS_COLOR_MAGENTA	SUS_RGB(255, 0, 255)
#define SUS_COLOR_BLACK		SUS_RGB(0, 0, 0)
#define SUS_COLOR_WHITE		SUS_RGB(255, 255, 255)

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//												 GDI RENDER											 //
/*								Working with graphics on a simple gdi system						 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////////////////////////
//									Basic definitions of structures      						  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Graphics status
typedef struct sus_graphics_state {
	HPEN	hPen;
	HBRUSH	hBrush;
	HFONT	hFont;
} SUS_GRAPHICS_STATE;
// Double Buffer Context
typedef struct sus_graphics_double_buffer {
	HBITMAP hbmMem;	// The bitmap
	HBITMAP hbmOld;	// The old bitmap
	HDC		hdcMem;	// Graphics output device
	RECT	oldRect;// Old output window sizes
} SUS_GRAPHICS_DOUBLE_BUFFER, *SUS_LPGRAPHICS_DOUBLE_BUFFER;
// The structure of the graphics render
typedef struct sus_graphics {
	HDC								_PARENT_;		// Current output device
	HDC								hdcScreen;		// Current output device
	RECT							scSize;			// Window screen size
	SUS_LPGRAPHICS_DOUBLE_BUFFER	doubleBuffer;	// Double Buffer Data
	PAINTSTRUCT						paintStruct;	// Drawing structure
	SUS_GRAPHICS_STATE				state;			// Rendering status
} SUS_GRAPHICS_STRUCT, *SUS_GRAPHICS;

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//											Basic GDI Functions      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Start rendering the frame on GDI
SUS_GRAPHICS_STRUCT	SUSAPI susGraphicsGDIBegin(_In_ HWND hWnd, _Inout_opt_ SUS_LPGRAPHICS_DOUBLE_BUFFER doubleBuffer);
// Finish rendering the frame
BOOL SUSAPI susGraphicsGDIEnd(_In_ SUS_GRAPHICS graphics, _In_ HWND hWnd);
// Clean the screen
VOID SUSAPI susGraphicsClear(_In_ SUS_GRAPHICS graphics, _In_ SUS_COLOR color);
// Delete Double Buffer resources
SUS_GRAPHICS_DOUBLE_BUFFER SUSAPI susGraphicsGDIDoubleBufferSetup(_In_ HWND hWnd);
// Delete Double Buffer resources
VOID SUSAPI susGraphicsGDIDoubleBufferCleanup(_In_ SUS_LPGRAPHICS_DOUBLE_BUFFER doubleBuffer);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//										Basic drawing functions      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set the outline color for drawing
VOID SUSAPI susGraphicsSetStroke(_In_ SUS_GRAPHICS graphics, SUS_COLOR color, UINT width);
// Set the fill color for drawing
VOID SUSAPI susGraphicsSetFill(_In_ SUS_GRAPHICS graphics, SUS_COLOR color);
// Set the color for drawing
VOID SUSAPI susGraphicsSetColor(_In_ SUS_GRAPHICS graphics, SUS_COLOR color);
// Remove the drawing outline
VOID SUSAPI susGraphicsRemoveStroke(_In_ SUS_GRAPHICS graphics);
// Remove the drawing fill
VOID SUSAPI susGraphicsRemoveFill(_In_ SUS_GRAPHICS graphics);

// -----------------------------------------------

// Styles for fonts
typedef enum sus_graphics_font {
	SUS_GRAPHICS_FONT_PLAIN		= 0,
	SUS_GRAPHICS_FONT_BOLD		= 1 << 0,
	SUS_GRAPHICS_FONT_ITALIC	= 1 << 1,
	SUS_GRAPHICS_FONT_STRIKEOUT = 1 << 2,
	SUS_GRAPHICS_FONT_UNDERLINE = 1 << 3
} SUS_GRAPHICS_FONT;
// Create a font
HFONT SUSAPI susGraphicsNewFont(_In_ LPCWSTR font, SUS_GRAPHICS_FONT style, UINT size);
// Set the font as the current one
VOID SUSAPI susGraphicsSetFont(_Inout_ SUS_GRAPHICS graphics, _In_ HFONT hFont);
// Delete the font
VOID SUSAPI susGraphicsFontDestroy(_In_ HFONT hFont);
// Install a new font
VOID SUSAPI susGraphicsSetNewFont(_Inout_ SUS_GRAPHICS graphics, _In_ LPCWSTR font, SUS_GRAPHICS_FONT style, UINT size);

// -----------------------------------------------

// Draw a line
VOID SUSAPI susGraphicsDrawLine(_In_ SUS_GRAPHICS graphics, _In_ SUS_POINT p1, _In_ SUS_POINT p2);
// Fill in the rectangle
VOID SUSAPI susGraphicsFillRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect);
// Draw a square
VOID SUSAPI susGraphicsDrawRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect);
// Fill in the oval
VOID SUSAPI susGraphicsFillOval(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect);
// Draw an oval
VOID SUSAPI susGraphicsDrawOval(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect);
// Get the text size in pixels
SUS_SIZE SUSAPI susGraphicsGetTextDimensions(_In_ SUS_GRAPHICS graphics, _In_ LPCWSTR text);
// Draw the text
VOID SUSAPI susGraphicsDrawText(_In_ SUS_GRAPHICS graphics, _In_ SUS_POINT pos, _In_ LPCWSTR text);
// Draw a polygon
VOID SUSAPI susGraphicsDrawPolygon(_In_ SUS_GRAPHICS graphics, _In_ SUS_LPPOINT points, _In_ UINT count);
// Fill in the polygon
VOID SUSAPI susGraphicsFillPolygon(_In_ SUS_GRAPHICS graphics, _In_ SUS_LPPOINT points, _In_ UINT count);
// Draw a rounded square
VOID SUSAPI susGraphicsDrawRoundRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect, _In_ SUS_SIZE radius);
// Fill in the rounded square
VOID SUSAPI susGraphicsFillRoundRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect, _In_ SUS_SIZE radius);

// -----------------------------------------------

// Image Structure
typedef struct sus_image {
	HBITMAP		_PARENT_;	// The system parent
	HBITMAP		hMemOld;	// 
	HDC			hdcMem;		// Output descriptor cache
	SUS_SIZE	size;		// Image Size
	BOOL		alpha;		// Is there an Alpha channel
	INT			bpp;		// Number of bytes per pixel
} SUS_IMAGE, *SUS_LPIMAGE;

// Load an image from a file
SUS_IMAGE SUSAPI susGraphicsLoadImage(_In_ LPCWSTR fileName);
// Create a new image
SUS_IMAGE SUSAPI susGraphicsNewImage(_In_ SUS_SIZE size, _In_opt_ SUS_COLOR fillColor);
// Copy the image
SUS_IMAGE SUSAPI susGraphicsImageCopyEx(_In_ SUS_IMAGE image, _In_ SUS_BOUNDS bounds);
// Copy the image
SUS_INLINE SUS_IMAGE SUSAPI susGraphicsImageCopy(_In_ SUS_IMAGE image) { return susGraphicsImageCopyEx(image, (SUS_BOUNDS) { .size = image.size }); }
// Destroy the image
VOID SUSAPI susGraphicsImageDestroy(_In_ SUS_IMAGE image);
// Draw an image
VOID SUSAPI susGraphicsDrawImageEx(_In_ SUS_GRAPHICS graphics, _In_ SUS_IMAGE image, _In_ SUS_BOUNDS bounds, _In_ BYTE alpha);
// Draw an image
SUS_INLINE VOID SUSAPI susGraphicsDrawImage(_In_ SUS_GRAPHICS graphics, _In_ SUS_IMAGE image, _In_ SUS_BOUNDS bounds) { susGraphicsDrawImageEx(graphics, image, bounds, 255); }

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

#pragma warning(pop)

#endif /* !_SUS_GRAPHICS_ */