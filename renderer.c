// renderer.c
//
#include "coreframe.h"
#include "include/susfwk/core.h"
#include "include/susfwk/tmath.h"
#include "include/susfwk/time.h"
#include "include/susfwk/uicommon.h"
#include "include/susfwk/resapi.h"
#include "include/susgl/suswgl.h"
#include "include/susfwk/vector.h"
#include "include/susfwk/hashtable.h"
#include "include/susfwk/sus_image.h"
#include "include/susfwk/renderer.h"

#pragma warning(disable: 28159)

// ================================================================================================= //
// ************************************************************************************************* //
//												 GPU RENDER											 //
/*								Working with graphics on a opengl api gpu							 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Basic set of functions      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Adjust the pixel for graphics
static BOOL SUSAPI susRendererDirectBasicSetPixel(_In_ HDC hdc)
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
SUS_RENDERER_DIRECT SUSAPI susRendererDirectBasicSetup(_In_ HWND hWnd)
{
	SUS_PRINTDL("Basic graphics initialization");
	SUS_ASSERT(hWnd);
	SUS_RENDERER_DIRECT context = { 0 };
	context.hdc = GetDC(hWnd);
	if (!context.hdc) goto error;
	if (!susRendererDirectBasicSetPixel(context.hdc)) {
		ReleaseDC(hWnd, context.hdc);
		goto error;
	}
	context.hGlrc = wglCreateContext(context.hdc);
	if (!context.hGlrc) {
		ReleaseDC(hWnd, context.hdc);
		goto error;
	}
	wglMakeCurrent(context.hdc, context.hGlrc);
	SUS_PRINTDL("The basic graphics have been successfully initialized!");
	return context;
error:
	SUS_PRINTDE("Failed to initialize graphics");
	SUS_PRINTDC(GetLastError());
	return (SUS_RENDERER_DIRECT) { 0 };
}

// Pixel Setting for Graphics is Modern
static BOOL SUSAPI susRendererDirectSetPixelModern(_In_ HDC hdc)
{
	SUS_PRINTDL("Pixel installation - modern");
	SUS_ASSERT(wglChoosePixelFormatARB);
	CONST GLint pixelAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		0
	};
	int pixelFormat = 0;
	UINT numFormats = 0;
	if (!wglChoosePixelFormatARB(hdc, pixelAttribs, NULL, 1, &pixelFormat, &numFormats)) {
		SUS_PRINTDE("Couldn't select a modern pixel format");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	if (!DescribePixelFormat(hdc, pixelFormat, sizeof(pfd), &pfd)) {
		SUS_PRINTDE("Couldn't get the pixel format");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
		SUS_PRINTDE("Couldn't set the modern pixel format");
		SUS_PRINTDC(GetLastError());
		return FALSE;
	}
	SUS_PRINTDL("The pixels have been successfully modernly configured");
	return TRUE;
}
// Set up modern pixel attributes
static HGLRC SUSAPI susRendererDirectCreateModernContext(_In_ HDC hdc, _In_ WORD major, _In_ WORD minor)
{
	SUS_PRINTDL("Configuring modern pixel attributes");
	SUS_ASSERT(major);
	SUS_ASSERT(wglCreateContextAttribsARB);
	const int contextAttribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor,
		(major > 3 || (major == 3 && minor >= 2)) ? WGL_CONTEXT_PROFILE_MASK_ARB : 0,
		(major > 3 || (major == 3 && minor >= 2)) ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : 0,
#ifdef _DEBUG
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif // !_DEBUG
		0
	};
	HGLRC hGlrc = wglCreateContextAttribsARB(hdc, 0, contextAttribs);
	if (!hGlrc) {
		SUS_PRINTDE("Failed to initialize the graphics in a modern way, failed to create a context");
		SUS_PRINTDC(GetLastError());
		return NULL;
	}
	return hGlrc;
}
// A modern way to initialize graphics
SUS_RENDERER_DIRECT SUSAPI susRendererDirectModernSetup(_In_ HWND hWnd, _In_ WORD major, _In_ WORD minor)
{
	SUS_PRINTDL("Modern graphics initialization");
	SUS_ASSERT(hWnd);
	HWND hTmp = CreateWindowA("STATIC", "", WS_POPUP, 0, 0, 1, 1, NULL, NULL, NULL, NULL);
	if (!hTmp) goto error;
	SUS_RENDERER_DIRECT context = susRendererDirectBasicSetup(hTmp);
	if (!context.hGlrc) return context;
	INT res = gladLoadWGL(context.hdc);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(context.hGlrc);
	ReleaseDC(hTmp, context.hdc);
	DestroyWindow(hTmp);
	if (!res) return susRendererDirectBasicSetup(hWnd);
	context.hdc = GetDC(hWnd);
	if (!susRendererDirectSetPixelModern(context.hdc)) {
		ReleaseDC(hWnd, context.hdc);
		goto error;
	}
	context.hGlrc = susRendererDirectCreateModernContext(context.hdc, major, minor);
	if (!context.hGlrc) {
		ReleaseDC(hWnd, context.hdc);
		goto error;
	}
	if (!wglMakeCurrent(context.hdc, context.hGlrc)) {
		wglDeleteContext(context.hGlrc);
		ReleaseDC(hWnd, context.hdc);
		goto error;
	}
	if (!gladLoadGL()) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(context.hGlrc);
		ReleaseDC(hWnd, context.hdc);
		goto error;
	}
	SUS_PRINTDL("Modern graphics have been successfully initialized");
#ifdef _DEBUG
	susRendererDirectPrint();
#endif // !_DEBUG
	return context;
error:
	SUS_PRINTDE("Failed to initialize graphics");
	SUS_PRINTDC(GetLastError());
	return (SUS_RENDERER_DIRECT) { 0 };
}
// Clean up graphics resources
VOID SUSAPI susRendererDirectCleanup(_In_ HWND hWnd, _In_ SUS_RENDERER_DIRECT context)
{
	SUS_PRINTDL("Cleaning up OpenGL resources");
	SUS_ASSERT(context.hGlrc && hWnd);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(context.hGlrc);
	ReleaseDC(hWnd, context.hdc);
}
// Output a log about the graph
VOID SUSAPI susRendererDirectPrint()
{
	SUS_PRINTDL("Information about the renderer driver:");
	SUS_PRINTDL("Renderer vendor: %s", glGetString(GL_VENDOR));
	SUS_PRINTDL("Renderer renderer: %s", glGetString(GL_RENDERER));
	SUS_PRINTDL("Renderer version: %s", glGetString(GL_VERSION));
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
static BOOL SUSAPI susRendererDirectAssembleShader(_In_ GLuint shader, _In_ LPCSTR source)
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
		SUS_PRINTDE("OpenGL Log - '%s'", susRendererDirectShaderGetLog(shader));
		return FALSE;
	}
	SUS_PRINTDL("Shader code loaded successfully");
	return TRUE;
}
// Create a shader
GLuint SUSAPI susRendererDirectLoadShader(_In_ SUS_SHADER_TYPE type, _In_ LPCSTR source)
{
	SUS_PRINTDL("Creating a shader");
	SUS_ASSERT(source && lstrlenA(source));
	GLuint shader = glCreateShader(type);
	if (!shader) {
		SUS_PRINTDE("Couldn't create shader");
		SUS_PRINTDC(glGetError());
		return 0;
	}
	if (!susRendererDirectAssembleShader(shader, source)) {
		glDeleteShader(shader);
		SUS_PRINTDE("Couldn't create shader");
		SUS_PRINTDC(glGetError());
		return 0;
	}
	SUS_PRINTDL("Shader has been created successfully");
	return shader;
}
// Get the shader log
LPSTR SUSAPI susRendererDirectShaderGetLog(_In_ GLuint shader)
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
GLuint SUSAPI susRendererDirectLinkProgram(_In_ UINT shaderCount, _In_ GLuint* shaders, _In_ BOOL DeleteShadersAfterUploading)
{
	SUS_PRINTDL("Loading shaders on the GPU");
	SUS_ASSERT(shaders);
	GLuint program = glCreateProgram();
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
		SUS_PRINTDE("OpenGL Log - '%s'", susRendererDirectProgramGetLog(program));
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
// Remove the program from the GPU
VOID SUSAPI susRendererDirectProgramCleanup(_In_ GLuint program)
{
	SUS_PRINTDL("Removing a program from the GPU");
	GLuint current = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &((GLint)current));
	if (current == program) glUseProgram(0);
	glDeleteProgram(program);
}
// Get the program log
LPSTR SUSAPI susRendererDirectProgramGetLog(_In_ GLuint program)
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
//									     Dynamic array on GPU    								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Re-allocate memory to the GPU
static BOOL SUSAPI susGpuVectorRealloc(SUS_LPGPU_VECTOR vector)
{
	SUS_PRINTDL("Re-allocating memory to the GPU");
	SUS_ASSERT(vector && vector->capacity && vector->vbo);
	GLuint newVbo = 0;
	glGenBuffers(1, &newVbo);
	glBindBuffer(GL_ARRAY_BUFFER, newVbo);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vector->capacity * vector->itemSize, NULL, GL_STATIC_DRAW);
	GLsizeiptr copySize = (GLsizeiptr)min(vector->length, vector->capacity) * vector->itemSize;
	if (copySize) {
		glBindBuffer(GL_COPY_READ_BUFFER, vector->vbo);
		glBindBuffer(GL_COPY_WRITE_BUFFER, newVbo);
		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, copySize);
 		glBindBuffer(GL_COPY_READ_BUFFER, 0);
		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	}
	SUS_PRINTDL("New GPU Buffer capacity: %d items", vector->capacity);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	for (GLenum error = glGetError(); error != GL_NO_ERROR;) {
		SUS_PRINTDE("OpenGL error %d: Couldn't reallocate memory to GPU", error);
		return FALSE;
	}
	glDeleteBuffers(1, &vector->vbo);
	vector->vbo = newVbo;
	return TRUE;
}
// Make sure that adding this size is enough in the buffer
static BOOL SUSAPI susGpuVectorReserve(_In_ SUS_LPGPU_VECTOR vector, _In_ UINT requiredCount)
{
	SUS_ASSERT(vector && vector->vbo);
	if (vector->capacity >= vector->length + requiredCount) return TRUE;
	vector->capacity = max(vector->capacity * SUS_BUFFER_GROWTH_FACTOR, vector->length * SUS_BUFFER_GROWTH_FACTOR + requiredCount);
	return susGpuVectorRealloc(vector);
}
// Compress array data to reduce size
static VOID SUSAPI susGpuVectorCompress(_In_ SUS_LPGPU_VECTOR vector)
{
	SUS_ASSERT(vector && vector->vbo);
	if (vector->capacity > SUS_GPU_VECTOR_LARGE_COUNT && vector->length * SUS_BUFFER_GROWTH_FACTOR * SUS_BUFFER_GROWTH_FACTOR < vector->capacity) {
		vector->capacity = vector->length * SUS_BUFFER_GROWTH_FACTOR;
		susGpuVectorRealloc(vector);
	}
}

// -----------------------------------------------

// Create a new array on the GPU
SUS_GPU_VECTOR SUSAPI susNewGpuVector(_In_ sus_uint_t itemSize)
{
	SUS_PRINTDL("Creating an array on the GPU");
	SUS_ASSERT(itemSize);
	SUS_GPU_VECTOR vector = { .itemSize = itemSize, .capacity = SUS_BASIC_GPU_VECTOR_COUNT };
	glGenBuffers(1, &vector.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, (GLsizeiptr)vector.vbo);
	glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vector.capacity * itemSize, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	for (GLenum error = glGetError(); error != GL_NO_ERROR;) {
		SUS_PRINTDE("OpenGL error %d: Couldn't reallocate memory to GPU", error);
		if (vector.vbo) glDeleteBuffers(1, &vector.vbo);
		return (SUS_GPU_VECTOR) { 0 };
	}
	return vector;
}
// Destroy the array
VOID SUSAPI susGpuVectorDestroy(_In_ SUS_LPGPU_VECTOR vector)
{
	SUS_PRINTDL("Destroying the array on gpu");
	SUS_ASSERT(vector);
	glDeleteBuffers(1, &vector->vbo);
}

// -----------------------------------------------

// Output the array log
VOID SUSAPI susGpuVectorLog(SUS_LPGPU_VECTOR vector) {
	SUS_ASSERT(vector);
	SUS_PRINTDL("Output of the GPU vector log:");
	GLint bufferSize = 0;
	glBindBuffer(GL_ARRAY_BUFFER, vector->vbo);
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	SUS_PRINTDL("GPU memory consumption: %d byte", bufferSize);
	SUS_PRINTDL("Buffer capacity and usage ratio: %d/%d", vector->length, vector->capacity);
	SUS_PRINTDL("Buffer element size: %d byte", vector->itemSize);
}
// Clear the array
VOID SUSAPI susGpuVectorClean(_In_ SUS_LPGPU_VECTOR vector) {
	SUS_ASSERT(vector);
	vector->length = 0;
}
// Add multiple elements to an array
BOOL SUSAPI susGpuVectorInsertArr(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index, _In_opt_ SUS_OBJECT value, _In_ sus_uint_t count)
{
	//SUS_PRINTDL("Inserting an %d elements by index %d", count, index);
	SUS_ASSERT(vector && vector->vbo && count && index <= vector->length);
	if (!susGpuVectorReserve(vector, count)) {
		SUS_PRINTDE("Couldn't add an element to the array");
		return FALSE;
	}
	glBindBuffer(GL_ARRAY_BUFFER, vector->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)index * vector->itemSize, (GLintptr)count * vector->itemSize, value);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vector->length += count;
	for (GLenum error = glGetError(); error != GL_NO_ERROR;) {
		SUS_PRINTDE("OpenGL error %d: Error when adding an element to the array", error);
		return FALSE;
	}
	return TRUE;
}
// Delete an element from an array
BOOL SUSAPI susGpuVectorEraseArr(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index, _In_ sus_uint_t count)
{
	SUS_PRINTDL("Removing an %d elements by index %d", count, index);
	SUS_ASSERT(vector && vector->vbo && count && index + count <= vector->length);
	glBindBuffer(GL_ARRAY_BUFFER, vector->vbo);
	sus_lpubyte_t base = (sus_lpubyte_t)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	if (!base) return FALSE;
	sus_lpubyte_t mem = base + index * vector->itemSize;
	sus_size_t bytesToMove = (sus_size_t)(vector->length - index - count) * vector->itemSize;
	if (bytesToMove) sus_memmove(mem, mem + vector->itemSize * count, bytesToMove);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	vector->length -= count;
	susGpuVectorCompress(vector);
	for (GLenum error = glGetError(); error != GL_NO_ERROR;) {
		SUS_PRINTDE("OpenGL error %d: Error when deleting from the array", error);
		return FALSE;
	}
	return TRUE;
}
// Get an array of elements from a vector\param The returned value must be cleared using sus_free
SUS_OBJECT SUSAPI susGpuVectorRead(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index, _In_ sus_uint_t count)
{
	SUS_PRINTDL("Getting %d elements by index %d", count, index);
	SUS_ASSERT(vector && vector->vbo && count && index + count - 1 < vector->length);
	SUS_OBJECT value = sus_malloc((SIZE_T)count * vector->itemSize);
	if (!value) return NULL;
	glBindBuffer(GL_ARRAY_BUFFER, vector->vbo);
	glGetBufferSubData(GL_ARRAY_BUFFER, (GLintptr)index * vector->itemSize, (GLintptr)count * vector->itemSize, value);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return value;
}

// Swap elements
BOOL SUSAPI susGpuVectorSwap(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t a, _In_ sus_uint_t b)
{
	SUS_PRINTDL("Swapping elements from index %d to index %d", a, b);
	SUS_ASSERT(vector && vector->vbo && vector->itemSize && a < vector->length && b < vector->length);
	sus_ubyte_t buff[256] = { 0 }; sus_lpubyte_t tmp = buff;
	if (vector->itemSize > sizeof(buff)) { tmp = sus_malloc(vector->itemSize); if (!tmp) return FALSE; }
	glBindBuffer(GL_ARRAY_BUFFER, vector->vbo);
	sus_lpubyte_t base = (sus_lpubyte_t)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	if (!base) { SUS_PRINTDE("Buffer memory could not be opened"); if (vector->itemSize > sizeof(buff)) sus_free(tmp); return FALSE; }
	sus_lpubyte_t memA = base + a * vector->itemSize;
	sus_lpubyte_t memB = base + b * vector->itemSize;
	sus_memcpy(tmp, memA, vector->itemSize);
	sus_memcpy(memA, memB, vector->itemSize);
	sus_memcpy(memB, tmp, vector->itemSize);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (vector->itemSize > sizeof(buff)) sus_free(tmp);
	for (GLenum error = glGetError(); error != GL_NO_ERROR;) {
		SUS_PRINTDE("OpenGL error %d: Error when swapped the array", error);
		return FALSE;
	}
	return TRUE;
}
// Swap places and delete
BOOL SUSAPI susGpuVectorSwapErase(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index)
{
	SUS_PRINTDL("Replacing and deleting an element");
	SUS_ASSERT(vector && index < vector->length);
	if (!susGpuVectorSwap(vector, index, vector->length - 1)) return FALSE;
	return susGpuVectorErase(vector, vector->length - 1);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											 RENDERING API											 //
/*								High-level api for working with rendering							 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Global rendering context
static SUS_RENDERER_CONTEXT RendererContext = { 0 };

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//							  The structure of working with the camera    						  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Create a general camera
SUS_CAMERA SUSAPI susRendererNewCamera(_In_ SUS_CAMERA_TYPE type) {
	SUS_PRINTDL("Creating a %s camera", type == SUS_CAMERA_TYPE_3D ? "3D" : "2D");
	SUS_CAMERA camera = sus_malloc(type == SUS_CAMERA_TYPE_3D ? sizeof(SUS_CAMERA3D_STRUCT) : sizeof(SUS_CAMERA2D_STRUCT));
	if (!camera) return NULL;
	camera->dirty = TRUE;
	camera->type = type;
	camera->view = camera->proj = camera->projview = susMat4Identity();
	return camera;
}
// Create a 2d camera
SUS_CAMERA2D SUSAPI susRendererNewCamera2D() {
	SUS_CAMERA2D camera = (SUS_CAMERA2D)susRendererNewCamera(SUS_CAMERA_TYPE_2D);
	camera->zoom = 1.0f;
	return camera;
}
// Create a 3d camera
SUS_CAMERA3D SUSAPI susRendererNewCamera3D(_In_ sus_uint_t fov, _In_ sus_float_t nearp, _In_ sus_float_t farp) {
	SUS_CAMERA3D camera = (SUS_CAMERA3D)susRendererNewCamera(SUS_CAMERA_TYPE_3D);
	camera->fov = fov;
	camera->nearp = nearp;
	camera->farp = farp;
	return camera;
}
// Destroy the camera
VOID SUSAPI susRendererCameraDestroy(_In_ SUS_CAMERA camera) {
	SUS_PRINTDL("Destroying the camera");
	SUS_ASSERT(camera);
	sus_free(camera);
}
// Set the current camera
VOID SUSAPI susRendererSetCamera(_In_ SUS_CAMERA camera) {
	RendererContext.currentCamera = camera;
}

// -----------------------------------------------

// Set a new size for the render
VOID SUSAPI susRendererCameraSize(_In_ SUS_CAMERA camera, _In_ sus_float_t aspect) {
	SUS_ASSERT(camera);
	camera->aspect = aspect;
	switch (camera->type)
	{
	case SUS_CAMERA_TYPE_2D: {
		SUS_CAMERA2D c2d = (SUS_CAMERA2D)camera;
		sus_float_t hHeight = 1.0f / c2d->zoom;
		sus_float_t hWidth = hHeight * aspect;
		camera->proj = susMat4Ortho(-hWidth, hWidth, -hHeight, hHeight, -1.0f, 1.0f);
	} break;
	case SUS_CAMERA_TYPE_3D: {
		SUS_CAMERA3D c3d = (SUS_CAMERA3D)camera;
		camera->proj = susMat4Perspective(c3d->fov, aspect, c3d->nearp, c3d->farp);
	} break;
	}
	camera->dirty = TRUE;
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set the view matrix for the camera
VOID SUSAPI susRendererCamera2DSetZoom(_Inout_ SUS_CAMERA2D camera, _In_ sus_float_t zoom) {
	SUS_ASSERT(camera);
	camera->zoom = zoom;
	susRendererCameraSize((SUS_CAMERA)camera, camera->aspect);
}
// Set the view matrix for the camera
VOID SUSAPI susRendererCamera3DSetFov(_Inout_ SUS_CAMERA3D camera, _In_ sus_uint_t fov) {
	SUS_ASSERT(camera);
	camera->fov = fov;
	susRendererCameraSize((SUS_CAMERA)camera, camera->aspect);
}
// Set the view matrix for the camera
VOID SUSAPI susRendererCamera3DSetNearPlane(_Inout_ SUS_CAMERA3D camera, _In_ sus_float_t nearp) {
	SUS_ASSERT(camera);
	camera->nearp = nearp;
	susRendererCameraSize((SUS_CAMERA)camera, camera->aspect);
}
// Set the view matrix for the camera
VOID SUSAPI susRendererCamera3DSetFarPlane(_Inout_ SUS_CAMERA3D camera, _In_ sus_float_t farp) {
	SUS_ASSERT(camera);
	camera->farp = farp;
	susRendererCameraSize((SUS_CAMERA)camera, camera->aspect);
}

// -----------------------------------------------

// Get a zoom camera
sus_float_t SUSAPI susRendererCamera2DGetZoom(_In_ SUS_CAMERA2D camera) {
	SUS_ASSERT(camera);
	return camera->zoom;
}
// Get the camera's fov
sus_uint_t SUSAPI susRendererCamera3DGetFov(_In_ SUS_CAMERA3D camera) {
	SUS_ASSERT(camera);
	return camera->fov;
}

// -----------------------------------------------

// Set the camera position
VOID SUSAPI susRendererCameraSetPosition(_Inout_ SUS_CAMERA camera, _In_ SUS_VEC3 pos) {
	SUS_ASSERT(camera && camera->type);
	camera->position = pos;
	camera->viewDirty = TRUE;
}
// Set the camera position
VOID SUSAPI susRendererCameraSetRotation(_Inout_ SUS_CAMERA camera, _In_ SUS_VEC3 rotation) {
	SUS_ASSERT(camera && camera->type);
	if (camera->type == SUS_CAMERA_TYPE_3D) ((SUS_CAMERA3D)camera)->forward = susVec3ToForward(rotation);
	else ((SUS_CAMERA2D)camera)->rotation = rotation.z;
	camera->viewDirty = TRUE;
}
// Set the view matrix for the camera
static VOID SUSAPI susRendererCameraFlushView(_Inout_ SUS_CAMERA camera) {
	SUS_ASSERT(camera && camera->type);
	if (camera->type == SUS_CAMERA_TYPE_3D) {
		SUS_CAMERA3D c3d = (SUS_CAMERA3D)camera;
		SUS_VEC3 target = { 0 };
		if (c3d->target) target = *c3d->target;
		else target = susVec3Sum(c3d->position, c3d->forward);
		camera->view = susMat4LookAt(c3d->position, target, (SUS_VEC3) { 0.0f, 1.0f, 0.0f });
	}
	else {
		SUS_CAMERA2D c2d = (SUS_CAMERA2D)camera;
		camera->view = susMat4Mult(susMat4Translate((SUS_VEC3) { -c2d->position.x, -c2d->position.y, -c2d->position.z }), susMat4RotateZ((sus_int16_t)-c2d->rotation));
	}
	camera->viewDirty = FALSE;
	camera->dirty = TRUE;
}
// Apply camera changes if camera is dirty
VOID SUSAPI susRendererCameraUpdate(_Inout_ SUS_CAMERA camera) {
	SUS_ASSERT(camera);
	if (camera->viewDirty) susRendererCameraFlushView(camera);
	if (!camera->dirty) return;
	camera->projview = susMat4Mult(camera->proj, camera->view);
	camera->dirty = FALSE;
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								   Creating and installing shaders    							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Load a shader from a file
GLuint SUSAPI susRendererLoadShaderModule(_In_ LPCSTR path)
{
	SUS_PRINTDL("Loading a shader from a file");
	SUS_ASSERT(path);
	LPCSTR extension = sus_strrchr(path, '.');
	if (!extension || !*extension) goto error;
	static const struct {
		LPCSTR extensions[5];
		SUS_SHADER_TYPE type;
	} shaderTypes[] = {
		{ {".vert", ".vs", ".vsh", ".vshader", "glslv" },	SUS_SHADER_TYPE_VERTEX,},
		{ {".frag", ".fs", ".fsh", ".fshader", "glslf" },	SUS_SHADER_TYPE_FRAGMENT,  },
		{ {".geom", ".gs", ".gsh", ".gshader", "glslg" },	SUS_SHADER_TYPE_GEOMETRY,  }
	};
	SUS_SHADER_TYPE type = SUS_SHADER_TYPE_UNKNOWN;
	for (UINT i = 0; i < SUS_COUNT_OF(shaderTypes); i++) {
		for (UINT j = 0; j < 4; j++) if (shaderTypes[i].extensions[j] && sus_strcmpi(extension, shaderTypes[i].extensions[j])) { type = shaderTypes[i].type; break; }
	}
	if (!type) goto error;
	SUS_FILE file = sus_fopenA(path, GENERIC_READ);
	if (!file) goto error;
	SUS_DATAVIEW source = sus_fread(file);
	sus_fclose(file);
	if (!source.data) goto error;
	GLuint shader = susRendererDirectLoadShader(type, (LPCSTR)source.data);
	susDataDestroy(source);
	return shader;
error:
	SUS_PRINTDL("Couldn't load shader from file");
	return 0;
}

// -----------------------------------------------

// Load the shader as the current one
SUS_RENDERER_SHADER SUSAPI susRendererNewShader(_In_ UINT count, _In_ SUS_LPRENDERER_SHADER_MODULE modules, _In_ SUS_SHADER_FORMAT format)
{
	SUS_PRINTDL("Loading the shader");
	SUS_ASSERT(count && modules);
	SUS_RENDERER_SHADER shader = sus_malloc(sizeof(SUS_RENDERER_SHADER_STRUCT));
	if (!shader) return NULL;
	shader->program = susRendererDirectLinkProgram(count, (GLuint*)modules, TRUE);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		SUS_PRINTDE("A fatal error occurred while loading the shader");
		SUS_PRINTDC(error);
		sus_free(shader);
		return NULL;
	}
	shader->format = format;
	// Table of basic uniforms
	static LPCSTR uniformNames[SUS_RENDERER_BASE_UNIFORM_COUNT] = { "sus_mvp", "sus_time", "sus_ctime", "sus_stime", "sus_delta", "sus_projview" };
	for (UINT i = 0; i < SUS_RENDERER_BASE_UNIFORM_COUNT; i++) {
		shader->uniforms[i] = glGetUniformLocation(shader->program, uniformNames[i]);
		error = glGetError();
		if (error != GL_NO_ERROR) {
			SUS_PRINTDE("Error when loading uniforms from a shader variable");
			SUS_PRINTDC(error);
		}
	}
	return shader;
}
// Destroy the shader
VOID SUSAPI susRendererShaderDestroy(_In_ SUS_RENDERER_SHADER shader)
{
	SUS_PRINTDL("Destroying the shader");
	SUS_ASSERT(shader);
	susRendererDirectProgramCleanup(shader->program);
	sus_free(shader);
}
// Set the shader as the current one
VOID SUSAPI susRendererSetShader(_In_ SUS_RENDERER_SHADER shader) 
{ 
	SUS_ASSERT(shader); 
	glUseProgram(shader->program);
	RendererContext.currentShader = shader;
}

// -----------------------------------------------


// Transfer transformation data to graphics
VOID SUSAPI susRendererMatrixFlush(_In_ SUS_MAT4 model) {
	SUS_ASSERT(RendererContext.currentShader && RendererContext.currentCamera);
	SUS_MAT4 mvp = susMat4Mult(RendererContext.currentCamera->projview, model);
	glUniformMatrix4fv(RendererContext.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_MVP], 1, GL_FALSE, (GLfloat*)&mvp);
}
// Send frame data to GPU
VOID SUSAPI susRendererFrameFlush(sus_float_t time, sus_float_t delta) {
	SUS_ASSERT(RendererContext.currentCamera);
	glUniformMatrix4fv(RendererContext.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_PROJVIEW], 1, GL_FALSE, (GLfloat*)&RendererContext.currentCamera->projview);
	glUniform1f(RendererContext.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_TIME], time);
	glUniform1f(RendererContext.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_DELTA], delta);
	time *= 360.0f;
	glUniform1f(RendererContext.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_COSTIME], sus_cos((sus_int_t)time));
	glUniform1f(RendererContext.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_SINTIME], sus_sin((sus_int_t)time));
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Working with 2d textures     								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set the wrapper functions for the texture
static VOID SUSAPI susRendererTextureSetWrap(_In_ GLenum target, _In_ SUS_RENDERER_TEXTURE_WRAP wrapX, _In_ SUS_RENDERER_TEXTURE_WRAP wrapY) {
	glTexParameteri(target, GL_TEXTURE_WRAP_S,
		wrapX == SUS_RENDERER_TEXTURE_WRAP_REPEAT ? GL_REPEAT :
		wrapX == SUS_RENDERER_TEXTURE_WRAP_MIRROR ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE
	);
	glTexParameteri(target, GL_TEXTURE_WRAP_T,
		wrapY == SUS_RENDERER_TEXTURE_WRAP_REPEAT ? GL_REPEAT :
		wrapY == SUS_RENDERER_TEXTURE_WRAP_MIRROR ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE
	);
}
_Success_(return != FALSE)
// Get color formats for textures
static BOOL SUSAPI susRendererGetTextureFormat(_In_ UINT channels, _Out_ GLenum * internalFormat, _Out_ GLenum * format) {
	switch (channels)
	{
	case 3: *internalFormat = GL_RGB8; *format = GL_RGB; return TRUE;
	case 4: *internalFormat = GL_RGBA8; *format = GL_RGBA; return TRUE;
	case 1: *internalFormat = GL_R8; *format = GL_RED; return TRUE;
	case 2: *internalFormat = GL_RG8; *format = GL_RG; return TRUE;
	default:
		SUS_PRINTDE("Image upload error - Not a defined number of channels per pixel");
		return FALSE;
	}
}
// Set filters on the texture
static VOID SUSAPI susRendererTextureSetFilters(_In_ GLenum target, _In_ BOOL mipmap, _In_ SUS_RENDERER_TEXTURE_SMOOTHING smoothing) {
	if (mipmap) {
		glGenerateMipmap(target);
		GLint level = 0;
		switch (smoothing)
		{
		case SUS_RENDERER_TEXTURE_SMOOTHING_NONE: level = GL_NEAREST_MIPMAP_NEAREST; break;
		case SUS_RENDERER_TEXTURE_SMOOTHING_LOW: level = GL_LINEAR_MIPMAP_NEAREST; break;
		case SUS_RENDERER_TEXTURE_SMOOTHING_MEDIUM: level = GL_NEAREST_MIPMAP_LINEAR; break;
		case SUS_RENDERER_TEXTURE_SMOOTHING_HIGH: level = GL_LINEAR_MIPMAP_LINEAR; break;
		}
		SUS_ASSERT(level);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, level);
	}
	else glTexParameteri(target, GL_TEXTURE_MIN_FILTER, smoothing >= SUS_RENDERER_TEXTURE_SMOOTHING_MEDIUM ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, smoothing >= SUS_RENDERER_TEXTURE_SMOOTHING_MEDIUM ? GL_LINEAR : GL_NEAREST);
}
// Create a texture
SUS_RENDERER_TEXTURE SUSAPI susRendererNewTexture(_In_ const SUS_LPRENDERER_TEXTURE_BUILDER builder)
{
	SUS_PRINTDL("Creating a texture");
	SUS_ASSERT(TRUE);
	SUS_RENDERER_TEXTURE texture = sus_malloc(sizeof(SUS_RENDERER_TEXTURE_STRUCT));
	if (!texture) goto error;
	glGenTextures(1, &texture->super);
	glBindTexture(GL_TEXTURE_2D, texture->super);
	susRendererTextureSetWrap(GL_TEXTURE_2D, builder->format.wrapX, builder->format.wrapY);
	GLenum internalFormat = GL_RGB, format = GL_RGB;
	susRendererGetTextureFormat(builder->channels, &internalFormat, &format);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, builder->size.cx, builder->size.cy, 0, format, GL_UNSIGNED_BYTE, builder->data);
	susRendererTextureSetFilters(GL_TEXTURE_2D, builder->format.mipmap, builder->format.smoothing);
	glBindTexture(GL_TEXTURE_2D, 0);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		SUS_PRINTDE("OpenGL error: %d Couldn't apply wrappers", error);
		susRendererTextureDestroy(texture);
		goto error;
	}
	texture->size = builder->size;
	return texture;
error:
	SUS_PRINTDE("Couldn't create texture");
	SUS_PRINTDC(glGetError());
	return NULL;
}
// Destroy the texture
VOID SUSAPI susRendererTextureDestroy(_In_ SUS_RENDERER_TEXTURE texture)
{
	SUS_PRINTDL("2d Texture Destruction");
	SUS_ASSERT(texture && texture->super);
	glDeleteTextures(1, &texture->super);
	sus_free(texture);
}
// Load a texture from a file
SUS_RENDERER_TEXTURE SUSAPI susRendererLoadTexture(_In_ LPCWSTR path, _In_ SUS_RENDERER_TEXTURE_FORMAT format)
{
	SUS_RENDERER_TEXTURE_BUILDER builder = { .format = format };
	builder.data = susLoadImageW(path, TRUE, &builder.size, (sus_pint_t)&builder.channels);
	if (!builder.data) return NULL;
	SUS_RENDERER_TEXTURE texture = susRendererNewTexture(&builder);
	sus_free(builder.data);
	return texture;
}
// Load an image from resources by type - TEXTURE
SUS_RENDERER_TEXTURE SUSAPI susRendererLoadTextureResource(_In_ LPCSTR resourceName, _In_ SUS_RENDERER_TEXTURE_FORMAT format)
{
	SUS_RENDERER_TEXTURE_BUILDER builder = { .format = format };
	SUS_DATAVIEW imageData = { 0 };
	imageData.data = susLoadResourceA(resourceName, "TEXTURE", (DWORD*)&imageData.size, NULL);
	builder.data = susLoadImageFromMemory(imageData, TRUE, &builder.size, (sus_pint_t)&builder.channels);
	if (!builder.data) return NULL;
	return susRendererNewTexture(&builder);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								   Creating and rendering meshes    							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set the offset for the uv coordinates of the mesh
VOID SUSAPI susVertexUVTransform(_Inout_ SUS_DATAVIEW vertexes, _In_ SUS_VEC2 offset, _In_ SUS_VEC2 scale)
{
	SUS_ASSERT(vertexes.data && !(vertexes.size % sizeof(SUS_VERTEX)));
	if (scale.x == 1.0f && scale.y == 1.0f) {
		for (INT i = (INT)(vertexes.size / sizeof(SUS_VERTEX)) - 1; i >= 0; i--) {
			((SUS_LPVERTEX)vertexes.data)[i].uv = susVec2Sum(((SUS_LPVERTEX)vertexes.data)[i].uv, offset);
		}
		return;
	}
	for (INT i = (INT)(vertexes.size / sizeof(SUS_VERTEX)) - 1; i >= 0; i--) {
		((SUS_LPVERTEX)vertexes.data)[i].uv = susVec2Sum(susVec2Mult(((SUS_LPVERTEX)vertexes.data)[i].uv, scale), offset);
	}
}

// -----------------------------------------------

// Mesh structure
struct sus_mesh {
	GLuint		vao;	// Vertex Array Objext
	GLuint		vbo;	// Vertex Buffer Object
	GLuint		ibo;	// Index Buffer Object
	UINT		count;	// The number of indexes in the mesh
	GLuint		type;	// The type of feature of the mesh
	sus_int_t	refc;	// The link counter for this mesh
};

// -----------------------------------------------

// Set attributes for the mesh
static VOID SUSAPI susRendererMeshSetAttributes(_In_ SUS_VERTEX_FORMAT format)
{
	SUS_PRINTDL("Setting attributes for a mesh");
	if (format.attributes & SUS_VERTEX_ATTRIBUT_POSITION) {
		glVertexAttribPointer(0, format.type, GL_FLOAT, GL_FALSE, sizeof(SUS_VERTEX), (void*)0);
		glEnableVertexAttribArray(0);
	}
	if (format.attributes & SUS_VERTEX_ATTRIBUT_COLOR) {
		glVertexAttribPointer(1, format.attributes & SUS_VERTEX_ATTRIBUT_ALPHA_COLOR ? 4 : 3, GL_FLOAT, GL_TRUE, sizeof(SUS_VERTEX), (void*)SUS_OFFSET_OF(SUS_VERTEX, color));
		glEnableVertexAttribArray(1);
	}
	if (format.attributes & SUS_VERTEX_ATTRIBUT_TEXTURE) {
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SUS_VERTEX), (void*)SUS_OFFSET_OF(SUS_VERTEX, uv));
		glEnableVertexAttribArray(2);
	}
	if (format.attributes & SUS_VERTEX_ATTRIBUT_NORMAL) {
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, sizeof(SUS_VERTEX), (void*)SUS_OFFSET_OF(SUS_VERTEX, normal));
		glEnableVertexAttribArray(3);
	}
}
// Initialize the GPU mesh
static VOID SUSAPI susRendererMeshInit(_Inout_ SUS_MESH mesh, _In_ SUS_LPMESH_BUILDER builder)
{
	SUS_PRINTDL("Initializing the GPU mesh");
	SUS_ASSERT(mesh && builder->geometry.vertexes.size);
	mesh->type = builder->primitiveType;
	mesh->count = (UINT)(builder->geometry.indexes.size ? builder->geometry.indexes.size / sizeof(SUS_INDEX) : builder->geometry.vertexes.size / sizeof(SUS_VERTEX));
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, builder->geometry.vertexes.size, builder->geometry.vertexes.data, builder->updateType);
	susRendererMeshSetAttributes(RendererContext.currentShader->format.vFormat);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (builder->geometry.indexes.size) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, builder->geometry.indexes.size, builder->geometry.indexes.data, builder->updateType);
	}
	glBindVertexArray(0);
}
// Build a mesh
SUS_MESH SUSAPI susRendererBuildMesh(_In_ const SUS_LPMESH_BUILDER builder)
{
	SUS_ASSERT(builder->geometry.vertexes.size && RendererContext.currentShader);
	SUS_PRINTDL("Building the mesh");
	SUS_MESH mesh = sus_malloc(sizeof(SUS_MESH_STRUCT));
	if (!mesh) return NULL;
	mesh->refc = 0;
	glGenVertexArrays(1, &mesh->vao);
	glGenBuffers(1, &mesh->vbo);
	if (builder->geometry.indexes.size) glGenBuffers(1, &mesh->ibo);
	susRendererMeshInit(mesh, builder);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		susRendererMeshDestroy(mesh);
		SUS_PRINTDE("OpenGL error loading the mesh: %d", error);
		return NULL;
	}
	SUS_PRINTDL("The mesh has been successfully loaded into the GPU");
	return mesh;
}
// Clear the cache from the graphics card
VOID SUSAPI susRendererMeshDestroy(_Inout_ SUS_MESH mesh)
{
	SUS_PRINTDL("Freeing up resources from the GPU");
	SUS_ASSERT(mesh);
	if (mesh->refc > 1) { mesh->refc--; return; }
	SUS_ASSERT(mesh->refc >= 0);
	if (mesh->ibo) glDeleteBuffers(1, &mesh->ibo);
	if (mesh->vbo) glDeleteBuffers(1, &mesh->vbo);
	if (mesh->vao) glDeleteVertexArrays(1, &mesh->vao);
	sus_free(mesh);
}
// Set new vertices
VOID SUSAPI susRendererMeshSetVertices(_In_ SUS_MESH mesh, _In_ SUS_DATAVIEW vertexes)
{
	SUS_ASSERT(mesh && mesh->vao && mesh->vbo && vertexes.data && vertexes.size);
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertexes.size, vertexes.data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	if (!mesh->ibo) mesh->count = (UINT)(vertexes.size / sizeof(SUS_VERTEX));
}
// Set new vertex indexes
VOID SUSAPI susRendererMeshSetIndexes(_In_ SUS_MESH mesh, _In_ SUS_DATAVIEW indexes)
{
	SUS_ASSERT(mesh && mesh->vao && mesh->ibo && indexes.data && indexes.size);
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexes.size, indexes.data);
	glBindVertexArray(0);
	mesh->count = (UINT)(indexes.size / sizeof(SUS_INDEX));
}
// Draw mesh
VOID SUSAPI susRendererDrawMesh(_In_ SUS_MESH mesh, _In_ SUS_MAT4 model)
{
	SUS_ASSERT(mesh && mesh->vao && mesh->vbo);
	susRendererMatrixFlush(model);
	glBindVertexArray(mesh->vao);
	if (mesh->ibo) glDrawElements(mesh->type, mesh->count, GL_UNSIGNED_SHORT, NULL);
	else glDrawArrays(mesh->type, 0, mesh->count);
	glBindVertexArray(0);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								Creating and rendering mesh instances    						  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Calculate the size of the instance
static sus_size_t SUSAPI susCalculateInstanceStride(_In_ SUS_MESH_INSTANCE_ATTRIBUTE attributes) {
	sus_size_t stride = sizeof(SUS_MAT4); // SUS_MESH_INSTANCE_ATTRIBUTE_MATRIX is default
	if (attributes & SUS_MESH_INSTANCE_ATTRIBUTE_COLOR) stride += sizeof(SUS_VEC4);
	if (attributes & SUS_MESH_INSTANCE_ATTRIBUTE_UVOFFSET) stride += sizeof(SUS_VEC2);
	stride = SUS_ALIGN(stride, 16);
	return stride;
}
// Set attributes for a mesh instance
static VOID SUSAPI susRendererMeshInstanceSetAttributes(SUS_MESH_INSTANCE_ATTRIBUTE attributes) {
	sus_size_t offset = 0, stride = susCalculateInstanceStride(attributes);
	UINT location = SUS_VERTEX_ATTRIBUT_COUNT;
	// Set model matrix
	for (int i = 0; i < 4; i++) {
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)offset);
		glEnableVertexAttribArray(location);
		glVertexAttribDivisor(location++, 1);
		offset += sizeof(SUS_VEC4);
	}
	// Set color attribute
	if (attributes & SUS_MESH_INSTANCE_ATTRIBUTE_COLOR) {
		glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)offset);
		glEnableVertexAttribArray(location);
		glVertexAttribDivisor(location++, 1);
		offset += sizeof(SUS_VEC4);
	}
	// Set uv offset attribute
	if (attributes & SUS_MESH_INSTANCE_ATTRIBUTE_UVOFFSET) {
		glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride, (void*)offset);
		glEnableVertexAttribArray(location);
		glVertexAttribDivisor(location++, 1);
		offset += sizeof(SUS_VEC2);
	}
}
// Initialize arrays in the mesh instance
static VOID SUSAPI susRendererMeshInstanceInit(_In_ SUS_MESH_INSTANCE instance, _In_ SUS_MESH_INSTANCE_ATTRIBUTE attributes, _In_ SUS_VERTEX_FORMAT format) {
	glBindVertexArray(instance->vao);
	glBindBuffer(GL_ARRAY_BUFFER, instance->sample->vbo);
	susRendererMeshSetAttributes(format);
	glBindBuffer(GL_ARRAY_BUFFER, instance->instanceData.vbo);
	susRendererMeshInstanceSetAttributes(attributes);
	if (instance->sample->ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->sample->ibo);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
// Create an instance for meshes
SUS_MESH_INSTANCE SUSAPI susRendererNewMeshInstance(_In_ SUS_MESH base)
{
	SUS_PRINTDL("Creating a Mesh instance");
	SUS_ASSERT(base && base->vbo && RendererContext.currentShader);
	SUS_MESH_INSTANCE instance = sus_malloc(sizeof(SUS_MESH_INSTANCE_STRUCT));
	if (!instance) return NULL;
	instance->sample = base;
	base->refc++;
	glGenVertexArrays(1, &instance->vao);
	instance->instanceData = susNewGpuVector((DWORD)susCalculateInstanceStride(RendererContext.currentShader->format.iFormat));
	susRendererMeshInstanceInit(instance, RendererContext.currentShader->format.iFormat, RendererContext.currentShader->format.vFormat);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		SUS_PRINTDE("OpenGL error loading the mesh: %d", error);
		susRendererMeshInstanceDestroy(instance);
		return NULL;
	}
	return instance;
}
// Delete a mesh instance
VOID SUSAPI susRendererMeshInstanceDestroy(_In_ SUS_MESH_INSTANCE instance)
{
	SUS_PRINTDL("Deleting a Mesh instance");
	SUS_ASSERT(instance && instance->vao);
	susGpuVectorDestroy(&instance->instanceData);
	glDeleteVertexArrays(1, &instance->vao);
	susRendererMeshDestroy(instance->sample);
	sus_free(instance);
}
// Draw mesh instances
VOID SUSAPI susRendererDrawMeshInstance(_In_ SUS_MESH_INSTANCE instance)
{
	SUS_ASSERT(instance && instance->vao);
	glBindVertexArray(instance->vao);
	if (instance->sample->ibo) {
		glDrawElementsInstanced(
			instance->sample->type,
			(GLsizei)instance->sample->count,
			GL_UNSIGNED_SHORT,
			NULL,
			(GLsizei)instance->instanceData.length
		);
	}
	else {
		glDrawArraysInstanced(
			instance->sample->type,
			(GLsizei)0,
			(GLsizei)instance->sample->count,
			(GLsizei)instance->instanceData.length
		);
	}
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		SUS_PRINTDE("OpenGL error %d: Error in rendering a mesh instance", error);
	}
	glBindVertexArray(0);
}

// -----------------------------------------------

// Extract data from attributes\param output is dynamic data
static LPBYTE SUSAPI susExtractInstanceAttributeData(sus_va_list list)
{
	SUS_ASSERT(list);
	LPBYTE buffer = (LPBYTE)sus_malloc(susCalculateInstanceStride(RendererContext.currentShader->format.iFormat));
	if (!buffer) return NULL;
	SUS_LPMAT4 model = sus_va_arg(list, SUS_LPMAT4);
	SUS_ASSERT(model);
	sus_memcpy(buffer, (LPBYTE)model, sizeof(*model));
	LPBYTE curr = buffer;
	curr += sizeof(*model);
	if (RendererContext.currentShader->format.iFormat & SUS_MESH_INSTANCE_ATTRIBUTE_COLOR) {
		SUS_LPVEC4 color = sus_va_arg(list, SUS_LPVEC4);
		SUS_ASSERT(color);
		sus_memcpy(curr, (LPBYTE)color, sizeof(*color));
		curr += sizeof(*color);
	}
	if (RendererContext.currentShader->format.iFormat & SUS_MESH_INSTANCE_ATTRIBUTE_UVOFFSET) {
		SUS_LPVEC2 uvoffset = sus_va_arg(list, SUS_LPVEC2);
		SUS_ASSERT(uvoffset);
		sus_memcpy(curr, (LPBYTE)uvoffset, sizeof(*uvoffset));
		curr += SUS_ALIGN(sizeof(*uvoffset), 16);
	}
	return buffer;
}
// Add a new item to the instances\param parameters are passed in order
BOOL SUSAPIV susRendererMeshInstanceAdd(_Inout_ SUS_MESH_INSTANCE instance, ...)
{
	SUS_PRINTDL("Adding a new object to mesh instances");
	SUS_ASSERT(instance && instance->vao);
	sus_va_list list;
	sus_va_start(list, instance);
	LPBYTE data = susExtractInstanceAttributeData(list);
	sus_va_end(list);
	if (!data) goto error;
	if (!susGpuVectorPush(&instance->instanceData, data)) { sus_free(data); goto error; }
	sus_free(data);
	return TRUE;
error:
	SUS_PRINTDE("Couldn't add object to instances");
	return FALSE;
}
// Delete an object from an instance
BOOL SUSAPIV susRendererMeshInstanceRemove(_Inout_ SUS_MESH_INSTANCE instance, _In_ UINT index)
{
	SUS_PRINTDL("Removing an item from instances");
	SUS_ASSERT(instance && instance->vao);
	if (!susGpuVectorSwapErase(&instance->instanceData, index)) goto error;
	return TRUE;
error:
	SUS_PRINTDE("Failed to delete an item from instances");
	return FALSE;
}
// apply changes
BOOL SUSAPI susRendererMeshInstanceFlush(_Inout_ SUS_MESH_INSTANCE instance)
{
	SUS_PRINTDL("Applying changes after REALLOC in the bag");
	SUS_ASSERT(instance && instance->vao);
	GLuint oldVao = instance->vao;
	glGenVertexArrays(1, &instance->vao);
	susRendererMeshInstanceInit(instance, RendererContext.currentShader->format.iFormat, RendererContext.currentShader->format.vFormat);
	for (GLenum error = glGetError(); error != GL_NO_ERROR;) {
		SUS_PRINTDE("OpenGL error %d: Failed to apply changes to mesh instances", error);
		if (instance->vao) glDeleteVertexArrays(1, &instance->vao);
		instance->vao = oldVao;
		return FALSE;
	}
	glDeleteVertexArrays(1, &oldVao);
	return TRUE;
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//											Primitive meshes    								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

//// Build a 2d square
//static SUS_MESH susRendererPrimitivesBuild_Square() {
//	static const SUS_VERTEX vertexes[] = {
//		{.pos = { -0.5f, -0.5f }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 0, 0 } },
//		{.pos = { 0.5f, -0.5f }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 1, 0 } },
//		{.pos = { 0.5f, 0.5f }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 1, 1 } },
//		{.pos = { -0.5f, 0.5f }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 0, 1 } },
//	};
//	static const SUS_INDEX indexes[] = { 0, 1, 2, 3, 0, 2 };
//	SUS_MESH_BUILDER builder = { 0 };
//	builder.geometry.vertexes = (SUS_DATAVIEW){ .data = (LPBYTE)vertexes, .size = sizeof(vertexes) };
//	builder.geometry.indexes = (SUS_DATAVIEW){ .data = (LPBYTE)indexes, .size = sizeof(indexes) };
//	builder.primitiveType = SUS_MESH_PRIMITIVE_TYPE_TRIANGLES;
//	builder.updateType = SUS_MESH_UPDATE_TYPE_STATIC;
//	return susRendererBuildMesh(&builder);
//}
//// Build a 2d circle
//static SUS_MESH susRendererPrimitivesBuild_Circle(sus_uint_t segments) {
//	segments = sus_min(segments, 360); // Because the angles are integers
//	SUS_LPVERTEX vertexes = sus_calloc(segments + 2, sizeof(SUS_VERTEX));
//	if (!vertexes) return NULL;
//	vertexes->pos = (SUS_VEC3){ 0.0f, 0.0f };
//	vertexes->color = (SUS_VEC4){ 1.0f, 1.0f, 1.0f };
//	vertexes->uv = (SUS_VEC2){ 0.5f, 0.5f };
//	for (sus_uint_t i = 0; i <= segments; i++) {
//		sus_int_t angle = sus_round((sus_float_t)i / (sus_float_t)segments * 360.0f);
//		SUS_VEC3 pos = { sus_cos(angle), sus_sin(angle) };
//		vertexes[i + 1].pos = pos;
//		vertexes[i + 1].color = (SUS_VEC4){ 1.0f, 1.0f, 1.0f };
//		vertexes[i + 1].uv = (SUS_VEC2){ 0.5f + 0.5f * pos.x, 0.5f - 0.5f * pos.y };
//	}
//	SUS_MESH_BUILDER builder = { 0 };
//	builder.geometry.vertexes = (SUS_DATAVIEW){ .data = (LPBYTE)vertexes, .size = (segments + 2) * sizeof(SUS_VERTEX) };
//	builder.primitiveType = SUS_MESH_PRIMITIVE_TYPE_TRIANGLES_FAN;
//	builder.updateType = SUS_MESH_UPDATE_TYPE_STATIC;
//	SUS_MESH mesh = susRendererBuildMesh(&builder);
//	sus_free(vertexes);
//	return mesh;
//}
//// Build a 3d cube
//static SUS_MESH susRendererPrimitivesBuild_Cube() {
//	static const SUS_VERTEX vertexes[] = {
//		{.pos = { -0.5f, -0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {0, 0, 1} },
//		{.pos = { 0.5f, -0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {0, 0, 1} },
//		{.pos = { 0.5f, 0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {0, 0, 1} },
//		{.pos = { -0.5f, 0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {0, 0, 1} },
//
//		{.pos = { 0.5f, -0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {0, 0, -1} },
//		{.pos = { -0.5f, -0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {0, 0, -1} },
//		{.pos = { -0.5f, 0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {0, 0, -1} },
//		{.pos = { 0.5f, 0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {0, 0, -1} },
//
//		{.pos = { -0.5f, 0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {0, 1, 0} },
//		{.pos = { 0.5f, 0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {0, 1, 0} },
//		{.pos = { 0.5f, 0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {0, 1, 0} },
//		{.pos = { -0.5f, 0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {0, 1, 0} },
//
//		{.pos = { -0.5f, -0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {0, -1, 0} },
//		{.pos = { 0.5f, -0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {0, -1, 0} },
//		{.pos = { 0.5f, -0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {0, -1, 0} },
//		{.pos = { -0.5f, -0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {0, -1, 0} },
//
//		{.pos = { 0.5f, -0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {1, 0, 0} },
//		{.pos = { 0.5f, -0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {1, 0, 0} },
//		{.pos = { 0.5f, 0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {1, 0, 0} },
//		{.pos = { 0.5f, 0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {1, 0, 0} },
//
//		{.pos = { -0.5f, -0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {-1, 0, 0} },
//		{.pos = { -0.5f, -0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {-1, 0, 0} },
//		{.pos = { -0.5f, 0.5f, 0.5f }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {-1, 0, 0} },
//		{.pos = { -0.5f, 0.5f, -0.5f }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {-1, 0, 0} },
//	};
//	static const SUS_INDEX indexes[] = {
//		0, 1, 2, 0, 2, 3,
//		4, 5, 6, 4, 6, 7,
//		8, 9, 10, 8, 10, 11,
//		12, 13, 14, 12, 14, 15,
//		16, 17, 18, 16, 18, 19,
//		20, 21, 22, 20, 22, 23
//	};
//	SUS_MESH_BUILDER builder = { 0 };
//	builder.geometry.vertexes = (SUS_DATAVIEW){ .data = (LPBYTE)vertexes, .size = sizeof(vertexes) };
//	builder.geometry.indexes = (SUS_DATAVIEW){ .data = (LPBYTE)indexes, .size = sizeof(indexes) };
//	builder.primitiveType = SUS_MESH_PRIMITIVE_TYPE_TRIANGLES;
//	builder.updateType = SUS_MESH_UPDATE_TYPE_STATIC;
//	return susRendererBuildMesh(&builder);
//}

////////////////////////////////////////////////////////////////////////////////////////////////////
//								    High-level work with graphics      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Initialize the graphics for the window
SUS_RENDERER SUSAPI susRendererSetup(_In_ HWND hWnd)
{
	SUS_PRINTDL("Initializing the renderer for the window");
	SUS_ASSERT(hWnd);
	SUS_RENDERER renderer = sus_malloc(sizeof(SUS_RENDERER_STRUCT));
	if (!renderer) goto error;
	renderer->super = susRendererDirectModernSetup(hWnd, 3, 3);
	if (!renderer->super.hGlrc) { sus_free(renderer); goto error; }
	renderer->hWnd = hWnd;
	susRendererSetCurrent(renderer);
	sus_timer_start(&renderer->frameTimer);
	return renderer;
error:
	SUS_PRINTDE("Failed to initialize rendering on the window");
	return NULL;
}
// Set the renderer as the current one
VOID SUSAPI susRendererSetCurrent(_In_opt_ SUS_RENDERER renderer) {
	SUS_PRINTDL("Setting the context as the current one");
	if (renderer) wglMakeCurrent(renderer->super.hdc, renderer->super.hGlrc);
	else wglMakeCurrent(NULL, NULL);
	RendererContext.currentRenderer = renderer;
}
// Remove the graphical context
VOID SUSAPI susRendererCleanup(_In_ SUS_RENDERER renderer) {
	SUS_PRINTDL("Clearing the graphical context");
	SUS_ASSERT(renderer);
	susRendererDirectCleanup(renderer->hWnd, renderer->super);
	if (RendererContext.currentRenderer == renderer) susRendererSetCurrent(0);
	sus_timer_stop(&renderer->frameTimer);
	sus_free(renderer);
}
// Update the context output area
VOID SUSAPI susRendererSize() {
	SUS_ASSERT(RendererContext.currentRenderer && RendererContext.currentCamera);
	RECT rect = { 0 };
	GetClientRect(RendererContext.currentRenderer->hWnd, &rect);
	sus_float_t aspect = (sus_float_t)rect.right / (sus_float_t)rect.bottom;
	susRendererCameraSize(RendererContext.currentCamera, aspect);
	glViewport(rect.left, rect.top, rect.right, rect.bottom);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Start rendering the frame
VOID SUSAPI susRendererBeginFrame() {
	SUS_ASSERT(RendererContext.currentShader && RendererContext.currentCamera && RendererContext.currentRenderer);
	SUS_RENDERER renderer = RendererContext.currentRenderer;
	susRendererCameraUpdate(RendererContext.currentCamera);
	susRendererFrameFlush(sus_timef(), renderer->frameTimer.state.delta);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
// Finish painting the frame
VOID SUSAPI susRendererEndFrame() {
	SUS_ASSERT(RendererContext.currentShader && RendererContext.currentCamera && RendererContext.currentRenderer);
	SUS_RENDERER renderer = RendererContext.currentRenderer;
	if (!SwapBuffers(renderer->super.hdc)) {
		SUS_PRINTDE("Failed to apply buffer transfer for the frame");
		SUS_PRINTDC(GetLastError());
		return;
	}
	sus_timer_update(&renderer->frameTimer);
	for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
		SUS_PRINTDE("OpenGL error: %d", error);
	}
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
