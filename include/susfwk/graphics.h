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

// Version OpenGL Default
#define SUS_GRAPHICS_DEFAULT_VERSION "3.3"

// -----------------------------------------------

// Basic graphics initialization
HGLRC SUSAPI susGraphicsBasicSetup(_In_ HWND hWnd);
// A modern way to initialize graphics
HGLRC SUSAPI susGraphicsModernSetup(_In_ HWND hWnd, _In_ WORD major, _In_ WORD minor);
// Get graphics information
VOID SUSAPI susGraphicsPrint();
// Initialize graphics
HGLRC SUSAPI susGraphicsSetup(_In_ HWND hWnd, _In_ WORD major, _In_ WORD minor);
// Clean up graphics resources
VOID SUSAPI susGraphicsCleanup(_In_ HWND hWnd, _In_ HGLRC hGlrc);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Working with shaders      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Types of shaders
typedef enum sus_graphics_shader_type {
	SUS_GRAPHICS_SHADER_TYPE_VERTEX = GL_VERTEX_SHADER,
	SUS_GRAPHICS_SHADER_TYPE_FRAGMENT = GL_FRAGMENT_SHADER,
	SUS_GRAPHICS_SHADER_TYPE_GEOMETRY = GL_GEOMETRY_SHADER,
	SUS_GRAPHICS_SHADER_TYPE_TESS_CONTROL = GL_TESS_CONTROL_SHADER,
	SUS_GRAPHICS_SHADER_TYPE_TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
	SUS_GRAPHICS_SHADER_TYPE_COMPUTE = GL_COMPUTE_SHADER
} SUS_GRAPHICS_SHADER_TYPE, *SUS_LPGRAPHICS_SHADER_TYPE;
// Shader
typedef GLuint SUS_GRAPHICS_SHADER, *SUS_LPGRAPHICS_SHADER;
// Shader Program
typedef GLuint SUS_GRAPHICS_PROGRAM, *SUS_LPGRAPHICS_PROGRAM;

// -----------------------------------------------

// Create a shader
SUS_GRAPHICS_SHADER SUSAPI susGraphicsLoadShader(_In_ SUS_GRAPHICS_SHADER_TYPE type, _In_ LPCSTR source);
// Download a shader from a file
SUS_GRAPHICS_SHADER SUSAPI susGraphicsLoadShaderFromFile(_In_ LPCSTR path);
// Unload the shader
VOID SUSAPI susGraphicsRemoveShader(_In_ SUS_GRAPHICS_SHADER shader);
// Get the shader log
LPSTR SUSAPI susGraphicsShaderGetLog(_In_ SUS_GRAPHICS_SHADER shader);

// -----------------------------------------------

// Load shaders to GPU
SUS_GRAPHICS_PROGRAM SUSAPI susGraphicsLoadProgram(_In_ UINT shaderCount, _In_ SUS_LPGRAPHICS_SHADER shaders, _In_ BOOL DeleteShadersAfterUploading);
// Install the program as the current one
VOID SUSAPI susGraphicsUseShader(_In_ SUS_GRAPHICS_PROGRAM program);
// Remove the program from the GPU
VOID SUSAPI susGraphicsRemoveProgram(_In_ SUS_GRAPHICS_PROGRAM program);
// Get the program log
LPSTR SUSAPI susGraphicsProgramGetLog(_In_ SUS_GRAPHICS_PROGRAM program);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//										 Meshes and models      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Vertex struct
typedef struct sus_vertex {
	SUS_VEC3	position;	// Vertex position
	SUS_VEC3	coord;		// vertex coordinate
	SUS_COLOR	color;		// Vertex Color
} SUS_VERTEX, *SUS_LPVERTICES;
// Index in the array of vertices
typedef sus_uint SUS_INDEX, *SUS_LPINDEXES;

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The type of the mesh primitive
typedef enum sus_graphics_primitive_type {
	SUS_GRAPHICS_PRIMITIVE_TYPE_TRIANGLES = GL_TRIANGLES,
	SUS_GRAPHICS_PRIMITIVE_TYPE_TRIANGLES_STRIP = GL_TRIANGLE_STRIP,
	SUS_GRAPHICS_PRIMITIVE_TYPE_TRIANGLES_FAN = GL_TRIANGLE_FAN,
	SUS_GRAPHICS_PRIMITIVE_TYPE_LINES = GL_LINES,
	SUS_GRAPHICS_PRIMITIVE_TYPE_LINES_STRIP = GL_LINE_STRIP,
	SUS_GRAPHICS_PRIMITIVE_TYPE_POINTS = GL_POINTS
} SUS_GRAPHICS_PRIMITIVE_TYPE, * SUS_LPGRAPHICS_PRIMITIVE_TYPE;
// Mesh type by refresh rate
typedef enum sus_graphics_mesh_update_type {
	SUS_GRAPHICS_MESH_TYPE_STATIC = GL_STATIC_DRAW,
	SUS_GRAPHICS_MESH_TYPE_DYNAMIC = GL_DYNAMIC_DRAW,
	SUS_GRAPHICS_MESH_TYPE_STREAM = GL_STREAM_DRAW
} SUS_GRAPHICS_MESH_UPDATE_TYPE, *SUS_LPGRAPHICS_MESH_UPDATE_TYPE;
// The Mesh Builder
typedef struct sus_graphics_mesh_builder {
	SUS_VECTOR						vertices;	// SUS_VERTEX
	SUS_VECTOR						indexes;	// SUS_INDEX
	SUS_GRAPHICS_PRIMITIVE_TYPE		type;		// Type of primitive
	SUS_GRAPHICS_MESH_UPDATE_TYPE	updateType;	// The type of mesh update in drawing
} SUS_GRAPHICS_MESH_BUILDER, * SUS_LPGRAPHICS_MESH_BUILDER;
// Mesh structure
typedef struct sus_graphics_mesh {
	GLuint	vao;	// Vertex Array Objext
	GLuint	vbo;	// Vertex Buffer Object
	GLuint	ibo;	// Index Buffer Object
	UINT	count;	// The number of indexes in the mesh
	GLenum	type;	// Type of primitive
} SUS_GRAPHICS_MESH, * SUS_LPGRAPHICS_MESH;

// -----------------------------------------------

// Create a Mesh builder
SUS_GRAPHICS_MESH_BUILDER SUSAPI susGraphicsMeshBuilder(_In_ SUS_GRAPHICS_PRIMITIVE_TYPE type, _In_ SUS_GRAPHICS_MESH_UPDATE_TYPE updateType);
// Add vertices to the mesh
BOOL SUSAPI susGraphicsMeshBuilderAddVertices(_Inout_ SUS_LPGRAPHICS_MESH_BUILDER builder, _In_ SUS_LPVERTICES vertices, _In_ UINT count);
// Add indexes to the mesh
BOOL SUSAPI susGraphicsMeshBuilderAddIndexes(_Inout_ SUS_LPGRAPHICS_MESH_BUILDER builder, _In_ SUS_LPINDEXES indexes, _In_ UINT count);

// -----------------------------------------------

// Build a mesh
SUS_GRAPHICS_MESH SUSAPI susGraphicsBuildMesh(_In_ SUS_GRAPHICS_MESH_BUILDER builder);
// Draw mesh
VOID SUSAPI susGraphicsDrawMesh(_In_ SUS_LPGRAPHICS_MESH mesh);
// Clear the cache from the gpu
VOID SUSAPI susGraphicsMeshDestroy(_Inout_ SUS_LPGRAPHICS_MESH mesh);
// Set new vertices
VOID SUSAPI susGraphicsMeshSetVertices(_In_ SUS_LPGRAPHICS_MESH mesh, _In_ SUS_LPVERTICES vertices, _In_ UINT count);
// Set new vertex indexes
VOID SUSAPI susGraphicsMeshSetIndexes(_In_ SUS_LPGRAPHICS_MESH mesh, _In_ SUS_LPINDEXES indexes, _In_ UINT count);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

#pragma warning(pop)

#endif /* !_SUS_GRAPHICS_ */