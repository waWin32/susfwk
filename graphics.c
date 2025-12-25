// graphics.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/math.h"
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
