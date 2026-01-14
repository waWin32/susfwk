// graphics.h
//
#ifndef _SUS_GRAPHICS_
#define _SUS_GRAPHICS_

// v1.5

#include "uicommon.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   Graph context structures      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Graphics status
typedef struct sus_graphics_state {
	HPEN	hPen;
	HBRUSH	hBrush;
	HFONT	hFont;
} SUS_GRAPHICS_STATE;
// Back buffer
typedef struct sus_graphics_back_buffer {
	HBITMAP hbmMem;		// The bitmap
	HBITMAP hbmOld;		// The old bitmap
	HDC		hdcMem;		// Graphics output device
} SUS_GRAPHICS_BACK_BUFFER;
// Graphics context
typedef struct sus_graphics_context {
	SUS_COLOR					clearColor;	// Window background fill color
	SUS_SIZE					screen;		// Cached size of the window's client area
	SUS_GRAPHICS_BACK_BUFFER	backBuffer;	// The context for double buffering
	SUS_USERDATA				userData;	// Additional data for rendering
} SUS_GRAPHICS_CONTEXT, *SUS_LPGRAPHICS_CONTEXT;
// The structure of the graphics render
typedef struct sus_graphics {
	HDC						_PARENT_;		// Current output device
	HDC						hdcScreen;		// Current output device
	PAINTSTRUCT				paintStruct;	// Drawing structure
	SUS_LPGRAPHICS_CONTEXT	context;		// Graphics context
	SUS_GRAPHICS_STATE		state;			// Rendering status
} SUS_GRAPHICS_STRUCT, *SUS_GRAPHICS;

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//											Basic GDI Functions      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set double buffering
BOOL SUSAPI susGraphicsSetDoubleBuffer(_In_ HWND hWnd, _Inout_ SUS_LPGRAPHICS_CONTEXT context);
// Remove double buffering
VOID SUSAPI susGraphicsRemoveDoubleBuffer(_Inout_ SUS_LPGRAPHICS_CONTEXT context);
// Update the graphics after resizing
VOID SUSAPI susGraphicsResize(_In_ HWND hWnd, _In_opt_ SUS_LPGRAPHICS_CONTEXT context);
// Start rendering the frame on GDI
SUS_GRAPHICS_STRUCT	SUSAPI susGraphicsBegin(_In_ HWND hWnd, _In_ SUS_LPGRAPHICS_CONTEXT context);
// Finish rendering the frame
BOOL SUSAPI susGraphicsEnd(_In_ HWND hWnd, _In_ SUS_GRAPHICS graphics);
// Clean the screen
VOID SUSAPI susGraphicsClear(_In_ SUS_GRAPHICS graphics);
// Set the screen cleaning color
SUS_INLINE VOID SUSAPI susGraphicsSetClear(_Inout_ SUS_LPGRAPHICS_CONTEXT context, _In_ SUS_COLOR clearColor) { context->clearColor = clearColor;  }
// Set data for graphics
SUS_INLINE VOID SUSAPI susGraphicsSetUserData(_Inout_ SUS_LPGRAPHICS_CONTEXT context, _In_ SUS_USERDATA userData) { context->userData = userData; }
// Get user data about the graph
SUS_INLINE SUS_USERDATA SUSAPI susGraphicsGetUserData(_In_ SUS_LPGRAPHICS_CONTEXT context) { return context->userData; }

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								  Functions for drawing primitives      						  //
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
VOID SUSAPI susGraphicsSetNullStroke(_In_ SUS_GRAPHICS graphics);
// Remove the drawing fill
VOID SUSAPI susGraphicsSetNullFill(_In_ SUS_GRAPHICS graphics);

// -----------------------------------------------

// Styles for fonts
typedef enum sus_graphics_font {
	SUS_GRAPHICS_FONT_PLAIN = 0,
	SUS_GRAPHICS_FONT_BOLD = 1 << 0,
	SUS_GRAPHICS_FONT_ITALIC = 1 << 1,
	SUS_GRAPHICS_FONT_STRIKEOUT = 1 << 2,
	SUS_GRAPHICS_FONT_UNDERLINE = 1 << 3
} SUS_GRAPHICS_GDI_FONT;
// Create a font
HFONT SUSAPI susGraphicsNewFont(_In_ LPCWSTR font, SUS_GRAPHICS_GDI_FONT style, UINT size);
// Set the font as the current one
VOID SUSAPI susGraphicsSetFont(_Inout_ SUS_GRAPHICS graphics, _In_ HFONT hFont);
// Delete the font
VOID SUSAPI susGraphicsFontDestroy(_In_ HFONT hFont);
// Install a new font
VOID SUSAPI susGraphicsSetNewFont(_Inout_ SUS_GRAPHICS graphics, _In_ LPCWSTR font, SUS_GRAPHICS_GDI_FONT style, UINT size);

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
typedef struct sus_graphics_image {
	HBITMAP		_PARENT_;	// The system parent
	HBITMAP		hMemOld;	// 
	HDC			hdcMem;		// Output descriptor cache
	SUS_SIZE	size;		// Image Size
	DWORD		bpp;		// Number of bits per pixel
} SUS_GRAPHICS_IMAGE_STRUCT, *SUS_GRAPHICS_IMAGE;

// Load an image from a file
SUS_GRAPHICS_IMAGE SUSAPI susGraphicsLoadImage(_In_ LPCWSTR fileName);
// Create a new image
SUS_GRAPHICS_IMAGE SUSAPI susGraphicsNewImage(_In_ SUS_SIZE size, _In_opt_ SUS_COLOR fillColor);
// Copy the image
SUS_GRAPHICS_IMAGE SUSAPI susGraphicsImageCopyEx(_In_ SUS_GRAPHICS_IMAGE image, _In_ SUS_BOUNDS bounds);
// Copy the image
SUS_INLINE SUS_GRAPHICS_IMAGE SUSAPI susGraphicsImageCopy(_In_ SUS_GRAPHICS_IMAGE image) { return susGraphicsImageCopyEx(image, (SUS_BOUNDS) { .size = image->size }); }
// Destroy the image
VOID SUSAPI susGraphicsImageDestroy(_In_ SUS_GRAPHICS_IMAGE image);
// Draw an image
VOID SUSAPI susGraphicsDrawImageEx(_In_ SUS_GRAPHICS graphics, _In_ SUS_GRAPHICS_IMAGE image, _In_ SUS_BOUNDS bounds, _In_ BYTE alpha);
// Draw an image
SUS_INLINE VOID SUSAPI susGraphicsDrawImage(_In_ SUS_GRAPHICS graphics, _In_ SUS_GRAPHICS_IMAGE image, _In_ SUS_BOUNDS bounds) { susGraphicsDrawImageEx(graphics, image, bounds, 255); }
// View image pixels
SUS_LPCOLOR SUSAPI susGraphicsImageViewPixels(_In_ SUS_GRAPHICS_IMAGE image);

// -----------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)

// Vertex for the gradient
typedef union sus_graphics_vertex {
	struct {
		SUS_POINT pos;
		SUS_COLOR64 color;
	};
	struct {
		SUS_POINT;
		SUS_COLOR64;
	};
} SUS_GRAPHICS_VERTEX, *SUS_LPGRAPHICS_VERTEX;

#pragma warning(pop)

// Draw a gradient triangle
VOID SUSAPI susGraphicsGradient(_In_ SUS_GRAPHICS graphics, _In_ SUS_LPGRAPHICS_VERTEX vertexes, _In_ UINT vertexCount, _In_ PUINT indexes, _In_ UINT indexCount);
// Draw a gradient square
VOID SUSAPI susGraphicsGradientRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect, _In_ SUS_COLOR64 colors[4]);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

#endif /* !_SUS_GRAPHICS_ */