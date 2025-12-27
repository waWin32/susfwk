// graphics.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/math.h"
#include "include/susfwk/tmath.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/graphics.h"

// ================================================================================================= //
// ************************************************************************************************* //
//												 GDI RENDER											 //
/*								Working with graphics on a simple gdi system						 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////////////////////////
//											Basic GDI Functions      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Start rendering the frame on GDI
SUS_GRAPHICS_STRUCT	SUSAPI susGraphicsGDIBegin(_In_ HWND hWnd, _Inout_opt_ SUS_LPGRAPHICS_DOUBLE_BUFFER doubleBuffer)
{
	SUS_ASSERT(hWnd);
	SUS_GRAPHICS_STRUCT graphics = { .doubleBuffer = doubleBuffer };
	graphics.hdcScreen = BeginPaint(hWnd, &graphics.paintStruct);
	if (!graphics.hdcScreen) {
		SUS_PRINTDE("Couldn't start rendering the frame");
		SUS_PRINTDC(GetLastError());
		return (SUS_GRAPHICS_STRUCT) { 0 };
	}
	GetClientRect(hWnd, &graphics.scSize);
	if (doubleBuffer && doubleBuffer->hdcMem) {
		if (!sus_memcmp((LPBYTE)&doubleBuffer->oldRect, (LPBYTE)&graphics.scSize, sizeof(RECT))) {
			SelectObject(doubleBuffer->hdcMem, doubleBuffer->hbmOld);
			DeleteObject(doubleBuffer->hbmMem);
			doubleBuffer->hbmMem = CreateCompatibleBitmap(graphics.hdcScreen, graphics.scSize.right, graphics.scSize.bottom);
			doubleBuffer->hbmOld = SelectObject(doubleBuffer->hdcMem, doubleBuffer->hbmMem);
		}
		graphics.super = doubleBuffer->hdcMem;
	}
	else graphics.super = graphics.hdcScreen;
	return graphics;
}
// Finish rendering the frame
BOOL SUSAPI susGraphicsGDIEnd(_In_ SUS_GRAPHICS graphics, _In_ HWND hWnd)
{
	SUS_ASSERT(graphics && graphics->super && hWnd);
	if (graphics->doubleBuffer && graphics->doubleBuffer->hdcMem) {
		BitBlt(graphics->hdcScreen, graphics->scSize.left, graphics->scSize.top, graphics->scSize.right, graphics->scSize.bottom, graphics->super, 0, 0, SRCCOPY);
		graphics->doubleBuffer->oldRect = graphics->scSize;
	}
	if (graphics->state.hPen) DeleteObject(graphics->state.hPen);
	if (graphics->state.hBrush) DeleteObject(graphics->state.hBrush);
	if (graphics->state.hFont) DeleteObject(graphics->state.hFont);
	if (!EndPaint(hWnd, &graphics->paintStruct)) {
		SUS_PRINTDE("Couldn't finish drawing the frame");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	return TRUE;
}
// Clean the screen
VOID SUSAPI susGraphicsClear(_In_ SUS_GRAPHICS graphics, _In_ SUS_COLOR color)
{
	SUS_ASSERT(graphics && graphics->super);
	susGraphicsSetColor(graphics, color);
	susGraphicsFillRect(graphics, susRectToSusRect(graphics->paintStruct.rcPaint));
}
// Delete Double Buffer resources
SUS_GRAPHICS_DOUBLE_BUFFER SUSAPI susGraphicsGDIDoubleBufferSetup(_In_ HWND hWnd)
{
	SUS_ASSERT(hWnd);
	SUS_GRAPHICS_DOUBLE_BUFFER doubleBuffer = { 0 };
	doubleBuffer.hdcMem = CreateCompatibleDC(NULL);
	if (!doubleBuffer.hdcMem) {
		SUS_PRINTDE("Failed to create a double buffer");
		SUS_PRINTDC(GetLastError());
		return doubleBuffer;
	}
	HDC hdc = GetDC(hWnd);
	GetClientRect(hWnd, &doubleBuffer.oldRect);
	doubleBuffer.hbmMem = CreateCompatibleBitmap(hdc, doubleBuffer.oldRect.right, doubleBuffer.oldRect.bottom);
	doubleBuffer.hbmOld = SelectObject(doubleBuffer.hdcMem, doubleBuffer.hbmMem);
	ReleaseDC(hWnd, hdc);
	return doubleBuffer;
}
// Delete Double Buffer resources
VOID SUSAPI susGraphicsGDIDoubleBufferCleanup(_In_ SUS_LPGRAPHICS_DOUBLE_BUFFER doubleBuffer)
{
	SUS_ASSERT(doubleBuffer);
	if (!doubleBuffer->hdcMem) return;
	SelectObject(doubleBuffer->hdcMem, doubleBuffer->hbmOld);
	DeleteObject(doubleBuffer->hbmMem);
	DeleteDC(doubleBuffer->hdcMem);
}

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

//static SUS_GRAPHICS_CACHE GraphicsCache = { 0 };

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//										Basic drawing functions      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set the outline color for drawing
VOID SUSAPI susGraphicsSetStroke(_In_ SUS_GRAPHICS graphics, SUS_COLOR color, UINT width)
{
	SUS_ASSERT(graphics && graphics->super);
	if (graphics->state.hPen && graphics->state.hPen != GetStockObject(BLACK_PEN)) {
		DeleteObject(graphics->state.hPen);
	}
	graphics->state.hPen = CreatePen(PS_SOLID, width, color.ref);
	SelectObject(graphics->super, graphics->state.hPen);
}
// Set the fill color for drawing
VOID SUSAPI susGraphicsSetFill(_In_ SUS_GRAPHICS graphics, SUS_COLOR color)
{
	SUS_ASSERT(graphics && graphics->super);
	if (graphics->state.hBrush && graphics->state.hBrush != GetStockObject(BLACK_BRUSH)) {
		DeleteObject(graphics->state.hBrush);
	}
	graphics->state.hBrush = CreateSolidBrush(color.ref);
	SelectObject(graphics->super, graphics->state.hBrush);
}
// Set the color for drawing
VOID SUSAPI susGraphicsSetColor(_In_ SUS_GRAPHICS graphics, SUS_COLOR color)
{
	SUS_ASSERT(graphics && graphics->super);
	susGraphicsSetStroke(graphics, color, 1);
	susGraphicsSetFill(graphics, color);
	SetTextColor(graphics->super, color.ref);
}
// Remove the drawing outline
VOID SUSAPI susGraphicsRemoveStroke(_In_ SUS_GRAPHICS graphics) {
	SUS_ASSERT(graphics && graphics->super);
	SelectObject(graphics->super, GetStockObject(NULL_PEN));
}
// Remove the drawing fill
VOID SUSAPI susGraphicsRemoveFill(_In_ SUS_GRAPHICS graphics) {
	SUS_ASSERT(graphics && graphics->super);
	SelectObject(graphics->super, GetStockObject(NULL_BRUSH));
}

// -----------------------------------------------

// Create a font
HFONT SUSAPI susGraphicsNewFont(_In_ LPCWSTR font, SUS_GRAPHICS_FONT style, UINT size) {
	if (!font) font = L"Arial";
	LOGFONTW lf = { 0 };
	lf.lfHeight			= -(LONG)size;
	lf.lfWeight			= (style & SUS_GRAPHICS_FONT_BOLD) ? FW_BOLD : FW_NORMAL;
	lf.lfItalic			= (style & SUS_GRAPHICS_FONT_ITALIC) ? TRUE : FALSE;
	lf.lfUnderline		= (style & SUS_GRAPHICS_FONT_UNDERLINE) ? TRUE : FALSE;
	lf.lfStrikeOut		= (style & SUS_GRAPHICS_FONT_STRIKEOUT) ? TRUE : FALSE;
	lf.lfCharSet		= DEFAULT_CHARSET;
	lf.lfQuality		= CLEARTYPE_QUALITY;
	lf.lfOutPrecision	= OUT_TT_PRECIS;
	lf.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	DWORD fontCharCount = sus_wcslen(font) + 1;
	sus_memcpy((LPBYTE)lf.lfFaceName, (LPBYTE)font, sus_min(fontCharCount, LF_FACESIZE) * sizeof(WCHAR));
	HFONT hFont = CreateFontIndirectW(&lf);
	if (!hFont) {
		SUS_PRINTDE("Couldn't create font");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	return hFont;
}
// Set the font as the current one
VOID SUSAPI susGraphicsSetFont(_Inout_ SUS_GRAPHICS graphics, _In_ HFONT hFont) {
	SUS_ASSERT(graphics && graphics->super);
	if (graphics->state.hFont) { susGraphicsFontDestroy(graphics->state.hFont); graphics->state.hFont = NULL; }
	SelectObject(graphics->super, hFont);
}
// Delete the font
VOID SUSAPI susGraphicsFontDestroy(_In_ HFONT hFont) {
	SUS_ASSERT(hFont);
	DeleteObject(hFont);
}
// Install a new font
VOID SUSAPI susGraphicsSetNewFont(_Inout_ SUS_GRAPHICS graphics, _In_ LPCWSTR font, SUS_GRAPHICS_FONT style, UINT size) {
	graphics->state.hFont = susGraphicsNewFont(font, style, size);
	susGraphicsSetFont(graphics, graphics->state.hFont);
}

// -----------------------------------------------

// Draw a line
VOID SUSAPI susGraphicsDrawLine(_In_ SUS_GRAPHICS graphics, _In_ SUS_POINT p1, _In_ SUS_POINT p2)
{
	SUS_ASSERT(graphics && graphics->super);
	MoveToEx(graphics->super, p1.x, p1.y, NULL);
	LineTo(graphics->super, p2.x, p2.y);
}
// Fill in the rectangle
VOID SUSAPI susGraphicsFillRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect)
{
	SUS_ASSERT(graphics && graphics->super);
	Rectangle(graphics->super, rect.left, rect.top, rect.right, rect.bottom);
}
// Draw a square
VOID SUSAPI susGraphicsDrawRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect)
{
	SUS_ASSERT(graphics && graphics->super);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(graphics->super, GetStockObject(NULL_BRUSH));
	susGraphicsFillRect(graphics, rect);
	SelectObject(graphics->super, hOldBrush);
}
// Fill in the oval
VOID SUSAPI susGraphicsFillOval(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect)
{
	SUS_ASSERT(graphics && graphics->super);
	Ellipse(graphics->super, rect.left, rect.top, rect.right, rect.bottom);
}
// Draw an oval
VOID SUSAPI susGraphicsDrawOval(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect)
{
	SUS_ASSERT(graphics && graphics->super);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(graphics->super, GetStockObject(NULL_BRUSH));
	susGraphicsFillOval(graphics, rect);
	SelectObject(graphics->super, hOldBrush);
}
// Get the text size in pixels
SUS_SIZE SUSAPI susGraphicsGetTextDimensions(_In_ SUS_GRAPHICS graphics, _In_ LPCWSTR text)
{
	SUS_ASSERT(graphics && graphics->super);
	SIZE size = { 0 };
	GetTextExtentPoint32W(graphics->super, text, sus_wcslen(text), &size);
	return (SUS_SIZE) { size.cx, size.cy };
}
// Draw the text
VOID SUSAPI susGraphicsDrawText(_In_ SUS_GRAPHICS graphics, _In_ SUS_POINT pos, _In_ LPCWSTR text)
{
	SUS_ASSERT(graphics && graphics->super);
	TextOutW(graphics->super, pos.x, pos.y, text ? text : L"null", sus_wcslen(text ? text : L"null"));
}
// Draw a polygon
VOID SUSAPI susGraphicsDrawPolygon(_In_ SUS_GRAPHICS graphics, _In_ SUS_LPPOINT points, _In_ UINT count)
{
	SUS_ASSERT(graphics && graphics->super && count >= 3);
	HBRUSH hBrushOld = SelectObject(graphics->super, GetStockObject(NULL_BRUSH));
	Polygon(graphics->super, (POINT*)points, count);
	SelectObject(graphics->super, hBrushOld);
}
// Fill in the polygon
VOID SUSAPI susGraphicsFillPolygon(_In_ SUS_GRAPHICS graphics, _In_ SUS_LPPOINT points, _In_ UINT count)
{
	SUS_ASSERT(graphics && graphics->super && count >= 3);
	Polygon(graphics->super, (POINT*)points, count);
}
// Draw a rounded square
VOID SUSAPI susGraphicsDrawRoundRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect, _In_ SUS_SIZE radius)
{
	SUS_ASSERT(graphics && graphics->super);
	HBRUSH hBrushOld = SelectObject(graphics->super, GetStockObject(NULL_BRUSH));
	RoundRect(graphics->super, rect.left, rect.top, rect.right, rect.bottom, radius.cx, radius.cy);
	SelectObject(graphics->super, hBrushOld);
}
// Fill in the rounded square
VOID SUSAPI susGraphicsFillRoundRect(_In_ SUS_GRAPHICS graphics, _In_ SUS_RECT rect, _In_ SUS_SIZE radius)
{
	SUS_ASSERT(graphics && graphics->super);
	RoundRect(graphics->super, rect.left, rect.top, rect.right, rect.bottom, radius.cx, radius.cy);
}

// -----------------------------------------------

// Create a 32-bit bit map
static HBITMAP SUSAPI susGraphicsImageBuilder(_In_ SUS_SIZE size, VOID** bits) {
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = size.cx;
	bmi.bmiHeader.biHeight = -(LONG)size.cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	HDC hdc = GetDC(NULL);
	HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, bits, NULL, 0);
	ReleaseDC(NULL, hdc);
	return hBitmap;
}
// Create an image from a bitmap
static SUS_IMAGE SUSAPI susGraphicsBuildImage(_In_ HBITMAP hBitMap) {
	if (!hBitMap) return (SUS_IMAGE) { 0 };
	SUS_IMAGE image = { hBitMap };
	image.hdcMem = CreateCompatibleDC(NULL);
	if (!image.hdcMem) {
		SUS_PRINTDE("Couldn't load cache");
		SUS_PRINTDC(GetLastError());
		return (SUS_IMAGE) { 0 };
	}
	image.hMemOld = SelectObject(image.hdcMem, image.super);
	BITMAP bm = { 0 };
	if (GetObject(image.super, sizeof(BITMAP), &bm)) {
		image.size.cx = bm.bmWidth;
		image.size.cy = bm.bmHeight;
		image.bpp = bm.bmBitsPixel;
		image.alpha = (bm.bmBitsPixel == 32);
	}
	return image;
}
// Load an image from a file
SUS_IMAGE SUSAPI susGraphicsLoadImage(_In_ LPCWSTR fileName)
{
	SUS_ASSERT(fileName);
	HBITMAP hBitmap = LoadImageW(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	SUS_IMAGE image = susGraphicsBuildImage(hBitmap);
	if (!image.super) {
		if (hBitmap) DeleteObject(hBitmap);
		goto error;
	}
	return image;
error:
	SUS_PRINTDE("Couldn't load image");
	SUS_PRINTDC(GetLastError());
	return (SUS_IMAGE) { 0 };
}
// Create a new image
SUS_IMAGE SUSAPI susGraphicsNewImage(_In_ SUS_SIZE size, _In_opt_ SUS_COLOR fillColor)
{
	SUS_ASSERT(size.cx && size.cy);
	HBITMAP hBitmap = susGraphicsImageBuilder(size, NULL);
	SUS_IMAGE image = susGraphicsBuildImage(hBitmap);
	if (!image.super) {
		if (hBitmap) DeleteObject(hBitmap);
		SUS_PRINTDE("Couldn't create image");
		SUS_PRINTDC(GetLastError());
		return image;
	}
	if (fillColor.ref != 0) {
		HBRUSH hBrush = CreateSolidBrush(fillColor.ref);
		RECT rect = { 0, 0, size.cx, size.cy };
		FillRect(image.hdcMem, &rect, hBrush);
		DeleteObject(hBrush);
	}
	return image;
}
// Copy the image
SUS_IMAGE SUSAPI susGraphicsImageCopyEx(_In_ SUS_IMAGE image, _In_ SUS_BOUNDS bounds)
{
	SUS_ASSERT(image.super);
	HDC hdc = GetDC(NULL);
	SUS_IMAGE copy = { .super = CreateCompatibleBitmap(hdc, image.size.cx, image.size.cy), .hdcMem = CreateCompatibleDC(hdc), .alpha = image.alpha, .bpp = image.bpp, .size = image.size };
	if (!copy.super || !copy.hdcMem) {
		if (copy.super) DeleteObject(copy.super);
		else if (copy.hdcMem) DeleteDC(copy.hdcMem);
		ReleaseDC(NULL, hdc);
		goto error;
	}
	copy.hMemOld = SelectObject(copy.hdcMem, copy.super);
	StretchBlt(copy.hdcMem, 0, 0, copy.size.cx, copy.size.cy, image.hdcMem, bounds.x, bounds.y, bounds.cx, bounds.cy, SRCCOPY);
	ReleaseDC(NULL, hdc);
	return copy;
error:
	SUS_PRINTDE("Couldn't copy image");
	SUS_PRINTDC(GetLastError());
	return (SUS_IMAGE) { 0 };
}
// Destroy the image
VOID SUSAPI susGraphicsImageDestroy(_In_ SUS_IMAGE image)
{
	SUS_ASSERT(image.super);
	SelectObject(image.hdcMem, image.hMemOld);
	DeleteObject(image.super);
	DeleteDC(image.hdcMem);
}
// Draw an image
VOID SUSAPI susGraphicsDrawImageEx(_In_ SUS_GRAPHICS graphics, _In_ SUS_IMAGE image, _In_ SUS_BOUNDS bounds, _In_ BYTE alpha)
{
	SUS_ASSERT(graphics && image.super);
	if (alpha == 255) goto fallback;
	else {
		static BOOL (WINAPI* lpAlphaBlend)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
		if (!lpAlphaBlend) {
			HMODULE msimg32 = LoadLibraryW(L"msimg32.dll");
			if (!msimg32) goto fallback;
			lpAlphaBlend = (BOOL(WINAPI*)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION))GetProcAddress(msimg32, "AlphaBlend");
			if (!lpAlphaBlend) goto fallback;
		}
		BLENDFUNCTION blend = { .BlendOp = AC_SRC_OVER, .BlendFlags = 0, .AlphaFormat = (image.alpha ? AC_SRC_ALPHA : 0), .SourceConstantAlpha = alpha };
		lpAlphaBlend(graphics->super, bounds.x, bounds.y, bounds.cx, bounds.cy, image.hdcMem, 0, 0, image.size.cx, image.size.cy, blend);
	}
	return;
fallback:
	StretchBlt(graphics->super, bounds.x, bounds.y, bounds.cx, bounds.cy, image.hdcMem, 0, 0, image.size.cx, image.size.cy, SRCCOPY);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//												 GPU RENDER											 //
/*								Working with graphics on OpenGL gpu system							 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Basic set of functions      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Adjust the pixel for graphics
static BOOL SUSAPI susGraphicsSetPixel(_In_ HDC hdc)
{
	SUS_PRINTDL("Configuring Graphic Pixels");
	SUS_ASSERT(hdc);
	PIXELFORMATDESCRIPTOR pfd = {
		.nSize = sizeof(PIXELFORMATDESCRIPTOR),
		.nVersion = 1,
		.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL,
		.iPixelType = PFD_TYPE_RGBA,
		.cColorBits = 32,
		.iLayerType = PFD_MAIN_PLANE,
		.cAlphaBits = 8,
		.cDepthBits = 24,
		.cStencilBits = 8
	};
	int format = ChoosePixelFormat(hdc, &pfd);
	if (!SetPixelFormat(hdc, format, &pfd)) {
		SUS_PRINTDE("Couldn't adjust pixels");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("Pixels have been successfully configured");
	return TRUE;
}

// Basic graphics initialization
HGLRC SUSAPI susGraphicsBasicSetup(_In_ HWND hWnd)
{
	SUS_PRINTDL("Basic graphics initialization");
	SUS_ASSERT(hWnd);
	HDC hdc = GetDC(hWnd);
	if (!hdc) goto error;
	if (!susGraphicsSetPixel(hdc)) {
		ReleaseDC(hWnd, hdc);
		goto error;
	}
	HGLRC hGlrc = wglCreateContext(hdc);
	if (!hGlrc) {
		ReleaseDC(hWnd, hdc);
		goto error;
	}
	wglMakeCurrent(hdc, hGlrc);
	ReleaseDC(hWnd, hdc);
	SUS_PRINTDL("The basic graphics have been successfully initialized!");
	return hGlrc;
error:
	SUS_PRINTDE("Failed to initialize graphics");
	SUS_PRINTDC(GetLastError());
	return NULL;
}
// Pixel Setting for Graphics is Modern
static BOOL SUSAPI susGraphicsSetPixelModern(_In_ HDC hdc, _In_ PFNWGLCHOOSEPIXELFORMATARBPROC _wglChoosePixelFormatARB)
{
	SUS_PRINTDL("Pixel installation - modern");
	SUS_ASSERT(_wglChoosePixelFormatARB);
	CONST GLint pixelAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, 1,
		WGL_SAMPLES_ARB, 4,
		0
	};
	int pixelFormat = 0;
	UINT numFormats = 0;
	if (!_wglChoosePixelFormatARB(hdc, pixelAttribs, NULL, 1, &pixelFormat, &numFormats)) {
		return FALSE;
	}
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	DescribePixelFormat(hdc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
		SUS_PRINTDE("Failed to initialize pixels in a modern way");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The pixels have been successfully modernly configured");
	return TRUE;
}
// Set up modern pixel attributes
static HGLRC SUSAPI susGraphicsSetPixelAttributes(_In_ HDC hdc, _In_ WORD major, _In_ WORD minor, _In_ PFNWGLCREATECONTEXTATTRIBSARBPROC _wglCreateContextAttribsARB)
{
	SUS_PRINTDL("Configuring modern pixel attributes");
	SUS_ASSERT(_wglCreateContextAttribsARB);
	const int contextAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef _DEBUG
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif // !_DEBUG
		0
	};
	HGLRC hGlrc = _wglCreateContextAttribsARB(hdc, 0, contextAttribs);
	if (!hGlrc) {
		SUS_PRINTDE("Failed to initialize the graphics in a modern way, failed to create a context");
		SUS_PRINTDC(glGetError());
		return NULL;
	}
	return hGlrc;
}
// A modern way to initialize graphics
HGLRC SUSAPI susGraphicsModernSetup(_In_ HWND hWnd, _In_ WORD major, _In_ WORD minor)
{
	SUS_PRINTDL("Modern graphics initialization");
	SUS_ASSERT(hWnd);
	HGLRC hGlrc = susGraphicsBasicSetup(hWnd);
	if (!hGlrc) return NULL;
	PFNWGLCHOOSEPIXELFORMATARBPROC _wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	PFNWGLCREATECONTEXTATTRIBSARBPROC _wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!(_wglChoosePixelFormatARB && _wglCreateContextAttribsARB)) {
		SUS_PRINTDW("Failed to initialize the graphics in a modern way, and no functions are available");
		return hGlrc;
	}
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hGlrc);
	HDC hdc = GetDC(hWnd);
	if (!susGraphicsSetPixelModern(hdc, _wglChoosePixelFormatARB)) {
		ReleaseDC(hWnd, hdc);
		goto error;
	}
	hGlrc = susGraphicsSetPixelAttributes(hdc, major, minor, _wglCreateContextAttribsARB);
	if (!hGlrc) {
		ReleaseDC(hWnd, hdc);
		goto error;
	}
	if (!wglMakeCurrent(hdc, hGlrc)) {
		wglDeleteContext(hGlrc);
		ReleaseDC(hWnd, hdc);
		goto error;
	}
	if (major >= 3) glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hGlrc);
		ReleaseDC(hWnd, hdc);
		goto error;
	}
	SUS_PRINTDL("Modern graphics have been successfully initialized");
#ifdef _DEBUG
	susGraphicsPrint();
#endif // !_DEBUG
	return hGlrc;
error:
	SUS_PRINTDE("Failed to initialize graphics");
	SUS_PRINTDC(glGetError());
	return NULL;
}
// Initialize graphics
HGLRC SUSAPI susGraphicsSetup(_In_ HWND hWnd, _In_ WORD major, _In_ WORD minor)
{
	SUS_PRINTDL("Setting up graphics version %s", version);
	SUS_ASSERT(hWnd && major && minor);
	return major >= 3 ? susGraphicsModernSetup(hWnd, major, minor) : susGraphicsBasicSetup(hWnd);
}
// Clean up graphics resources
VOID SUSAPI susGraphicsCleanup(_In_ HWND hWnd, _In_ HGLRC hGlrc)
{
	SUS_PRINTDL("Cleaning up OpenGL resources");
	SUS_ASSERT(hWnd && hGlrc);
	HDC hdc = GetDC(hWnd);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hGlrc);
	ReleaseDC(hWnd, hdc);
}
// Get graphics information
VOID SUSAPI susGraphicsPrint()
{
	SUS_PRINTDL("Information about the graphics driver:");
	SUS_PRINTDL("Graphics vendor: %s", glGetString(GL_VENDOR));
	SUS_PRINTDL("Graphics renderer: %s", glGetString(GL_RENDERER));
	SUS_PRINTDL("Graphics version: %s", glGetString(GL_VERSION));
	SUS_PRINTDL("GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Working with shaders      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Upload the code to the shader
static BOOL SUSAPI susGraphicsLoadShaderCode(_In_ SUS_GRAPHICS_SHADER shader, _In_ LPCSTR source)
{
	SUS_PRINTDL("Loading the code into the shader");
	SUS_ASSERT(source);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		SUS_PRINTDE("Couldn't load shader code");
		SUS_PRINTDC(glGetError());
#ifdef _DEBUG
		SUS_PRINTDE("OpenGL Log - '%s'", susGraphicsShaderGetLog(shader));
#endif // !_DEBUG
		return FALSE;
	}
	SUS_PRINTDL("Shader code loaded successfully");
	return TRUE;
}
// Create a shader
SUS_GRAPHICS_SHADER SUSAPI susGraphicsLoadShader(_In_ SUS_GRAPHICS_SHADER_TYPE type, _In_ LPCSTR source)
{
	SUS_PRINTDL("Creating a shader");
	SUS_ASSERT(source && lstrlenA(source));
	SUS_GRAPHICS_SHADER shader = glCreateShader(type);
	if (!shader) {
		SUS_PRINTDE("Couldn't create shader");
		SUS_PRINTDC(glGetError());
		return 0;
	}
	if (!susGraphicsLoadShaderCode(shader, source)) {
		glDeleteShader(shader);
		SUS_PRINTDE("Couldn't create shader");
		SUS_PRINTDC(glGetError());
		return 0;
	}
	SUS_PRINTDL("Shader has been created successfully");
	return shader;
}
// Download a shader from a file
SUS_GRAPHICS_SHADER SUSAPI susGraphicsLoadShaderFromFile(_In_ LPCSTR path)
{
	SUS_PRINTDL("Loading a shader from a file");
	SUS_ASSERT(path);
	LPCSTR extension = sus_strrchr(path, '.');
	if (!extension || !*extension) goto error;
	static const struct {
		LPCSTR extensions[4];
		SUS_GRAPHICS_SHADER_TYPE type;
	} shaderTypes[] = {
		{ {".vert", ".vs", ".vsh", ".vshader"},	SUS_GRAPHICS_SHADER_TYPE_VERTEX,  },
		{ {".frag", ".fs", ".fsh", ".fshader"},	SUS_GRAPHICS_SHADER_TYPE_FRAGMENT,  },
		{ {".geom", ".gs", ".gsh", ".gshader"},	SUS_GRAPHICS_SHADER_TYPE_GEOMETRY,  },
		{ {".tesc", ".tsc", ".tc"},				SUS_GRAPHICS_SHADER_TYPE_TESS_CONTROL, },
		{ {".tese", ".tse", ".te"},				SUS_GRAPHICS_SHADER_TYPE_TESS_EVALUATION, },
		{ {".comp", ".cs", ".csh", ".cshader"},	SUS_GRAPHICS_SHADER_TYPE_COMPUTE, }
	};
	SUS_GRAPHICS_SHADER_TYPE type = -1;
	for (UINT i = 0; i < SUS_COUNT_OF(shaderTypes); i++) {
		for (UINT j = 0; j < 4; j++) if (shaderTypes[i].extensions[j] && lstrcmpA(extension, shaderTypes[i].extensions[j]) == 0) { type = shaderTypes[i].type; break; }
	}
	if (type == -1) goto error;
	SUS_FILE file = sus_fopenA(path, GENERIC_READ);
	if (!file) goto error;
	SUS_DATAVIEW source = sus_fread(file);
	sus_fclose(file);
	if (!source.data) goto error;
	SUS_GRAPHICS_SHADER shader = susGraphicsLoadShader(type, (LPCSTR)source.data);
	susDataDestroy(source);
	return shader;
error:
	SUS_PRINTDL("Couldn't load shader from file");
	return 0;
}
// Unload the shader
VOID SUSAPI susGraphicsRemoveShader(_In_ SUS_GRAPHICS_SHADER shader)
{
	SUS_PRINTDL("Unloading the shader");
	SUS_ASSERT(shader);
	glDeleteShader(shader);
}
// Get the shader log
LPSTR SUSAPI susGraphicsShaderGetLog(_In_ SUS_GRAPHICS_SHADER shader)
{
	SUS_PRINTDL("Getting the shader log");
	SUS_ASSERT(shader);
	GLsizei length = 0;
	static GLchar infoLog[1024] = { 0 };
	glGetShaderInfoLog(shader, sizeof(infoLog), &length, infoLog);
	return infoLog;
}

// -----------------------------------------------

// Load shaders to GPU
SUS_GRAPHICS_PROGRAM SUSAPI susGraphicsLoadProgram(_In_ UINT shaderCount, _In_ SUS_LPGRAPHICS_SHADER shaders, _In_ BOOL DeleteShadersAfterUploading)
{
	SUS_PRINTDL("Loading shaders on the GPU");
	SUS_ASSERT(shaders);
	SUS_GRAPHICS_PROGRAM program = glCreateProgram();
	for (UINT i = 0; i < shaderCount; i++) {
		SUS_ASSERT(shaders[i]);
		glAttachShader(program, shaders[i]);
	}
	glLinkProgram(program);
	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		SUS_PRINTDE("Couldn't install shaders on GPU");
		SUS_PRINTDC(glGetError());
#ifdef _DEBUG
		SUS_PRINTDE("OpenGL Log - '%s'", susGraphicsProgramGetLog(program));
#endif // !_DEBUG
		for (UINT i = 0; i < shaderCount; i++) glDetachShader(program, shaders[i]);
		glDeleteProgram(program);
		return 0;
	}
#ifdef _DEBUG
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	if (!success) {
		CHAR validateLog[1024];
		GLsizei len = 0;
		glGetProgramInfoLog(program, sizeof(validateLog), &len, validateLog);
		if (len > 0) SUS_PRINTDW("Program validation warning:\n\"%s\"", validateLog);
	}
#endif // !_DEBUG
	for (UINT i = 0; i < shaderCount; i++) glDetachShader(program, shaders[i]);
	if (DeleteShadersAfterUploading) for (UINT i = 0; i < shaderCount; i++) glDeleteShader(shaders[i]);
	SUS_PRINTDL("Successful loading of shaders on the GPU");
	return program;
}
// Install the program as the current one
VOID SUSAPI susGraphicsUseShader(_In_ SUS_GRAPHICS_PROGRAM program)
{
	SUS_PRINTDL("Installing program %d as the current one", program);
	glUseProgram(program);
}

// Remove the program from the GPU
VOID SUSAPI susGraphicsRemoveProgram(_In_ SUS_GRAPHICS_PROGRAM program)
{
	SUS_PRINTDL("Removing a program from the GPU");
	SUS_GRAPHICS_PROGRAM current = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &((GLint)current));
	if (current == program) glUseProgram(0);
	glDeleteProgram(program);
}
// Get the program log
LPSTR SUSAPI susGraphicsProgramGetLog(_In_ SUS_GRAPHICS_PROGRAM program)
{
	SUS_PRINTDL("Getting the program log");
	SUS_ASSERT(program);
	GLsizei length = 0;
	static GLchar infoLog[1024] = { 0 };
	glGetProgramInfoLog(program, sizeof(infoLog), &length, infoLog);
	return infoLog;
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Meshes and models      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Create a Mesh builder
SUS_GRAPHICS_MESH_BUILDER SUSAPI susGraphicsMeshBuilder(_In_ SUS_GRAPHICS_PRIMITIVE_TYPE type, _In_ SUS_GRAPHICS_MESH_UPDATE_TYPE updateType)
{
	SUS_PRINTDL("Configuring the Mesh Builder");
	return (SUS_GRAPHICS_MESH_BUILDER) {
		.indexes = NULL,
		.vertices = NULL,
		.type = type,
		.updateType = updateType
	};
}
// Add vertices to the mesh
BOOL SUSAPI susGraphicsMeshBuilderAddVertices(_Inout_ SUS_LPGRAPHICS_MESH_BUILDER builder, _In_ SUS_LPVERTICES vertices, _In_ UINT count)
{
	SUS_ASSERT(builder && vertices && count);
	if (!builder->vertices) builder->vertices = susNewVector(SUS_VERTEX);
	return susVectorAppend(&builder->vertices, vertices, count) ? TRUE : FALSE;
}
// Add indexes to the mesh
BOOL SUSAPI susGraphicsMeshBuilderAddIndexes(_Inout_ SUS_LPGRAPHICS_MESH_BUILDER builder, _In_ SUS_LPINDEXES indexes, _In_ UINT count)
{
	SUS_ASSERT(builder && indexes && count);
	if (!builder->indexes) builder->indexes = susNewVector(SUS_INDEX);
	return susVectorAppend(&builder->indexes, indexes, count) ? TRUE : FALSE;
}

// -----------------------------------------------

// Initialize the GPU mesh
static SUS_GRAPHICS_MESH SUSAPI susGraphicsMeshSetup(_In_ SUS_VECTOR vertices, _In_ SUS_VECTOR indexes, _In_ SUS_GRAPHICS_PRIMITIVE_TYPE primitiveType, _In_ SUS_GRAPHICS_MESH_UPDATE_TYPE updateType)
{
	SUS_PRINTDL("Initializing the GPU mesh");
	SUS_ASSERT(vertices && vertices->length);
	SUS_GRAPHICS_MESH mesh = { .type = primitiveType, .count = indexes ? indexes->length : vertices->length };
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices->size, vertices->data, updateType);
	if (indexes) {
		glGenBuffers(1, &mesh.ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes->size, indexes->data, updateType);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	return mesh;
}
// Set attributes for the mesh
static VOID SUSAPI susGraphicsMeshAttributesSetup(_Inout_ SUS_LPGRAPHICS_MESH mesh)
{
	SUS_PRINTDL("Setting attributes for a mesh");
	SUS_ASSERT(mesh);
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SUS_VERTEX), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SUS_VERTEX), (void*)SUS_OFFSET_OF(SUS_VERTEX, coord));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(SUS_VERTEX), (void*)SUS_OFFSET_OF(SUS_VERTEX, color));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
// Build a mesh
SUS_GRAPHICS_MESH SUSAPI susGraphicsBuildMesh(_In_ SUS_GRAPHICS_MESH_BUILDER builder)
{
	SUS_ASSERT(builder.vertices);
	SUS_GRAPHICS_MESH mesh = susGraphicsMeshSetup(builder.vertices, builder.indexes, builder.type, builder.updateType);
	susGraphicsMeshAttributesSetup(&mesh);
	susVectorDestroy(builder.vertices);
	if (builder.indexes) susVectorDestroy(builder.indexes);
	if (glGetError() != GL_NO_ERROR) {
		susGraphicsMeshDestroy(&mesh);
		SUS_PRINTDE("OpenGL error loading the mesh: %d", glGetError());
		return (SUS_GRAPHICS_MESH) { 0 };
	}
	SUS_PRINTDL("The mesh has been successfully loaded into the GPU");
	return mesh;
}
// Set new vertices
VOID SUSAPI susGraphicsMeshSetVertices(_In_ SUS_LPGRAPHICS_MESH mesh, _In_ SUS_LPVERTICES vertices, _In_ UINT count)
{
	SUS_ASSERT(mesh && mesh->vbo && vertices && count);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(SUS_VERTEX), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (!mesh->ibo) mesh->count = count;
}
// Set new vertex indexes
VOID SUSAPI susGraphicsMeshSetIndexes(_In_ SUS_LPGRAPHICS_MESH mesh, _In_ SUS_LPINDEXES indexes, _In_ UINT count)
{
	SUS_ASSERT(mesh && mesh->ibo && indexes && count);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * sizeof(SUS_INDEX), indexes);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	mesh->count = count;
}
// Draw mesh
VOID SUSAPI susGraphicsDrawMesh(_In_ SUS_LPGRAPHICS_MESH mesh)
{
	SUS_ASSERT(mesh && mesh->vao && mesh->vbo);
	glBindVertexArray(mesh->vao);
	if (mesh->ibo) glDrawElements(mesh->type, mesh->count, GL_UNSIGNED_INT, (void*)0);
	else glDrawArrays(mesh->type, 0, mesh->count);
	glBindVertexArray(0);
}
// Clear the cache from the graphics card
VOID SUSAPI susGraphicsMeshDestroy(_Inout_ SUS_LPGRAPHICS_MESH mesh)
{
	SUS_PRINTDL("Freeing up resources from the GPU");
	SUS_ASSERT(mesh);
	if (mesh->ibo) glDeleteBuffers(1, &mesh->ibo);
	if (mesh->vbo) glDeleteBuffers(1, &mesh->vbo);
	if (mesh->vao) glDeleteVertexArrays(1, &mesh->vao);
	mesh->vao = mesh->vbo = mesh->ibo = 0;
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////
