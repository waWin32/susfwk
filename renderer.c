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
	susErrorPushCritical(SUS_ERROR_SYSTEM_ERROR, SUS_ERROR_TYPE_RESOURCE);
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
	susErrorPushCritical(SUS_ERROR_SYSTEM_ERROR, SUS_ERROR_TYPE_RESOURCE);
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
		susErrorPushCritical(SUS_ERROR_SYNTAX_ERROR, SUS_ERROR_TYPE_SYSTEM);
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
		susErrorPushCritical(SUS_ERROR_SYSTEM_ERROR, SUS_ERROR_TYPE_RESOURCE);
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
	for (GLenum error = glGetError(); error != GL_NO_ERROR;) {
		SUS_PRINTDE("OpenGL error %d: Shader binding error", error);
		susErrorPushCritical(SUS_ERROR_SYSTEM_ERROR, SUS_ERROR_TYPE_SYSTEM);
		susRendererDirectProgramCleanup(program);
		return 0;
	}
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
		susErrorPushCritical(SUS_ERROR_OUT_OF_MEMORY, SUS_ERROR_TYPE_MEMORY);
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
		susErrorPushCritical(SUS_ERROR_OUT_OF_MEMORY, SUS_ERROR_TYPE_MEMORY);
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
		susErrorPushCritical(SUS_ERROR_WRITE_MEMORY, SUS_ERROR_TYPE_MEMORY);
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
		susErrorPushCritical(SUS_ERROR_WRITE_MEMORY, SUS_ERROR_TYPE_MEMORY);
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
		susErrorPushCritical(SUS_ERROR_WRITE_MEMORY, SUS_ERROR_TYPE_MEMORY);
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

// Global rendering context
static SUS_RENDERER SUSCurrentRenderer = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////
//							  The structure of working with the camera    						  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Create a general camera
SUS_CAMERA SUSAPI susRendererNewCamera(_In_ SUS_CAMERA_TYPE type) {
	SUS_PRINTDL("Creating a %s camera", type == SUS_CAMERA_TYPE_3D ? "3D" : "2D");
	SUS_ASSERT(SUSCurrentRenderer);
	SUS_CAMERA camera = sus_malloc(type == SUS_CAMERA_TYPE_3D ? sizeof(SUS_CAMERA3D_STRUCT) : sizeof(SUS_CAMERA2D_STRUCT));
	if (!camera) return NULL;
	camera->dirty = TRUE;
	camera->type = type;
	camera->view = camera->proj = camera->projview = susMat4Identity();
	susRendererCameraSetRotation(camera, (SUS_VEC3) { 0.0f, 0.0f, 0.0f });
	susRendererResourcePoolRegister(susRendererResourceManagerCurrent(&SUSCurrentRenderer->resources), SUS_RENDERER_RESOURCE_TYPE_CAMERA, camera);
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
	SUS_ASSERT(camera && SUSCurrentRenderer);
	susRendererResourcePoolRemove(susRendererResourceManagerCurrent(&SUSCurrentRenderer->resources), SUS_RENDERER_RESOURCE_TYPE_CAMERA, camera);
	sus_free(camera);
}
// Set the current camera
VOID SUSAPI susRendererSetCamera(_In_ SUS_CAMERA camera) {
	SUS_ASSERT(SUSCurrentRenderer);
	SUSCurrentRenderer->context.currentCamera = camera;
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
	if (sus_memcmp((LPBYTE)&camera->position, (LPBYTE)&pos, sizeof(pos))) return;
	camera->position = pos;
	camera->viewDirty = TRUE;
}
// Set the camera rotation
VOID SUSAPI susRendererCameraSetRotation(_Inout_ SUS_CAMERA camera, _In_ SUS_VEC3 rotation) {
	SUS_ASSERT(camera && camera->type);
	camera->rotation = rotation;
	if (camera->type == SUS_CAMERA_TYPE_3D) ((SUS_CAMERA3D)camera)->forward = susVec3ToForward(rotation);
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
		camera->view = susMat4Mult(susMat4Translate((SUS_VEC3) { -c2d->position.x, -c2d->position.y, -c2d->position.z }), susMat4RotateZ((sus_int16_t)-c2d->rotation.z));
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
SUS_RENDERER_SHADER_MODULE SUSAPI susRendererLoadShaderModule(_In_ LPCSTR path)
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
	return (SUS_RENDERER_SHADER_MODULE) { shader };
error:
	SUS_PRINTDL("Couldn't load shader from file");
	return (SUS_RENDERER_SHADER_MODULE) { 0 };
}

// -----------------------------------------------

// Load the shader as the current one
SUS_RENDERER_SHADER SUSAPI susRendererNewShader(_In_ UINT count, _In_ SUS_LPRENDERER_SHADER_MODULE modules)
{
	SUS_PRINTDL("Loading the shader");
	SUS_ASSERT(count && modules && SUSCurrentRenderer);
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
	// Table of basic uniforms
	static LPCSTR uniformNames[SUS_RENDERER_BASE_UNIFORM_COUNT] = { "sus_mvp", "sus_time", "sus_ctime", "sus_stime", "sus_delta", "sus_projview", "sus_texture" };
	for (UINT i = 0; i < SUS_RENDERER_BASE_UNIFORM_COUNT; i++) {
		shader->uniforms[i] = glGetUniformLocation(shader->program, uniformNames[i]);
	}
	susRendererResourcePoolRegister(susRendererResourceManagerCurrent(&(SUSCurrentRenderer->resources)), SUS_RENDERER_RESOURCE_TYPE_SHADER, shader);
	return shader;
}
// Destroy the shader
VOID SUSAPI susRendererShaderDestroy(_In_ SUS_RENDERER_SHADER shader)
{
	SUS_PRINTDL("Destroying the shader");
	SUS_ASSERT(shader && SUSCurrentRenderer);
	susRendererDirectProgramCleanup(shader->program);
	susRendererResourcePoolRemove(susRendererResourceManagerCurrent(&(SUSCurrentRenderer->resources)), SUS_RENDERER_RESOURCE_TYPE_SHADER, shader);
	sus_free(shader);
}
// Set the shader as the current one
VOID SUSAPI susRendererSetShader(_In_ SUS_RENDERER_SHADER shader) 
{ 
	SUS_ASSERT(SUSCurrentRenderer && shader);
	glUseProgram(shader->program);
	SUSCurrentRenderer->context.currentShader = shader;
}

// -----------------------------------------------


// Transfer transformation data to graphics
VOID SUSAPI susRendererMatrixFlush(_In_ SUS_MAT4 model) {
	SUS_ASSERT(SUSCurrentRenderer && SUSCurrentRenderer->context.currentShader && SUSCurrentRenderer->context.currentCamera);
	SUS_MAT4 mvp = susMat4Mult(SUSCurrentRenderer->context.currentCamera->projview, model);
	glUniformMatrix4fv(SUSCurrentRenderer->context.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_MVP], 1, GL_FALSE, (GLfloat*)&mvp);
}
// Send frame data to GPU
VOID SUSAPI susRendererFrameFlush(sus_float_t time, sus_float_t delta) {
	SUS_ASSERT(SUSCurrentRenderer && SUSCurrentRenderer->context.currentCamera);
	glUniformMatrix4fv(SUSCurrentRenderer->context.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_PROJVIEW], 1, GL_FALSE, (GLfloat*)&SUSCurrentRenderer->context.currentCamera->projview);
	glUniform1f(SUSCurrentRenderer->context.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_TIME], time);
	glUniform1f(SUSCurrentRenderer->context.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_DELTA], delta);
	time *= 360.0f;
	glUniform1f(SUSCurrentRenderer->context.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_COSTIME], sus_cos((sus_int_t)time));
	glUniform1f(SUSCurrentRenderer->context.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_SINTIME], sus_sin((sus_int_t)time));
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Working with 2d textures     								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set the wrapper functions for the texture
static VOID SUSAPI susRendererTextureSetWrap(_In_ GLenum target, _In_ SUS_TEXTURE_WRAP wrapX, _In_ SUS_TEXTURE_WRAP wrapY) {
	glTexParameteri(target, GL_TEXTURE_WRAP_S,
		wrapX == SUS_TEXTURE_WRAP_REPEAT ? GL_REPEAT :
		wrapX == SUS_TEXTURE_WRAP_MIRROR ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE
	);
	glTexParameteri(target, GL_TEXTURE_WRAP_T,
		wrapY == SUS_TEXTURE_WRAP_REPEAT ? GL_REPEAT :
		wrapY == SUS_TEXTURE_WRAP_MIRROR ? GL_MIRRORED_REPEAT : GL_CLAMP_TO_EDGE
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
static VOID SUSAPI susRendererTextureSetFilters(_In_ GLenum target, _In_ BOOL mipmap, _In_ SUS_TEXTURE_SMOOTHING smoothing) {
	if (mipmap) {
		glGenerateMipmap(target);
		GLint level = 0;
		switch (smoothing)
		{
		case SUS_TEXTURE_SMOOTHING_NONE: level = GL_NEAREST_MIPMAP_NEAREST; break;
		case SUS_TEXTURE_SMOOTHING_LOW: level = GL_LINEAR_MIPMAP_NEAREST; break;
		case SUS_TEXTURE_SMOOTHING_MEDIUM: level = GL_NEAREST_MIPMAP_LINEAR; break;
		case SUS_TEXTURE_SMOOTHING_HIGH: level = GL_LINEAR_MIPMAP_LINEAR; break;
		}
		SUS_ASSERT(level);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, level);
	}
	else glTexParameteri(target, GL_TEXTURE_MIN_FILTER, smoothing >= SUS_TEXTURE_SMOOTHING_MEDIUM ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, smoothing >= SUS_TEXTURE_SMOOTHING_MEDIUM ? GL_LINEAR : GL_NEAREST);
}
// Create a texture
SUS_TEXTURE SUSAPI susRendererNewTexture(_In_ const SUS_LPTEXTURE_BUILDER builder)
{
	SUS_PRINTDL("Creating a texture");
	SUS_ASSERT(builder && SUSCurrentRenderer);
	SUS_TEXTURE texture = sus_malloc(sizeof(SUS_TEXTURE_STRUCT));
	if (!texture) goto error;
	glGenTextures(1, &texture->super);
	glBindTexture(GL_TEXTURE_2D, texture->super);
	susRendererTextureSetWrap(GL_TEXTURE_2D, builder->format.wrapX, builder->format.wrapY);
	GLenum internalFormat = GL_RGB, format = GL_RGB;
	susRendererGetTextureFormat(builder->channels, &internalFormat, &format);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
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
	susRendererResourcePoolRegister(susRendererResourceManagerCurrent(&SUSCurrentRenderer->resources), SUS_RENDERER_RESOURCE_TYPE_TEXTURE, texture);
	return texture;
error:
	SUS_PRINTDE("Couldn't create texture");
	SUS_PRINTDC(glGetError());
	return NULL;
}
// Destroy the texture
VOID SUSAPI susRendererTextureDestroy(_In_ SUS_TEXTURE texture)
{
	SUS_PRINTDL("2d Texture Destruction");
	SUS_ASSERT(texture && texture->super);
	glDeleteTextures(1, &texture->super);
	susRendererResourcePoolRemove(susRendererResourceManagerCurrent(&SUSCurrentRenderer->resources), SUS_RENDERER_RESOURCE_TYPE_TEXTURE, texture);
	sus_free(texture);
}
// Load a texture from a file
SUS_TEXTURE SUSAPI susRendererLoadTexture(_In_ LPCWSTR path, _In_ SUS_TEXTURE_FORMAT format)
{
	SUS_TEXTURE_BUILDER builder = { .format = format };
	builder.data = susLoadImageW(path, TRUE, &builder.size, (sus_pint_t)&builder.channels);
	if (!builder.data) return NULL;
	SUS_TEXTURE texture = susRendererNewTexture(&builder);
	sus_free(builder.data);
	return texture;
}
// Load an image from resources by type - TEXTURE
SUS_TEXTURE SUSAPI susRendererLoadTextureResource(_In_ LPCSTR resourceName, _In_ SUS_TEXTURE_FORMAT format)
{
	SUS_TEXTURE_BUILDER builder = { .format = format };
	SUS_DATAVIEW imageData = { 0 };
	imageData.data = susLoadResourceA(resourceName, "TEXTURE", (DWORD*)&imageData.size, NULL);
	builder.data = susLoadImageFromMemory(imageData, TRUE, &builder.size, (sus_pint_t)&builder.channels);
	if (!builder.data) return NULL;
	SUS_TEXTURE texture = susRendererNewTexture(&builder);
	sus_free(builder.data);
	return texture;
}
// Load the texture into the GPU slot
VOID SUSAPI susRendererTextureBind(_In_ SUS_TEXTURE texture, _In_ sus_uint_t slot) {
	SUS_ASSERT(texture && slot <= 64);
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture->super);
}
// Set the texture as the current one for the slot
VOID SUSAPI susRendererSetTexture(_In_ sus_uint_t slot) {
	SUS_ASSERT(SUSCurrentRenderer && SUSCurrentRenderer->context.currentShader && slot <= 64);
	glUniform1i(SUSCurrentRenderer->context.currentShader->uniforms[SUS_RENDERER_BASE_UNIFORM_TEXTURE], slot);
}
// Load and activate the texture on slot 0
VOID SUSAPI susRendererTextureActive(_In_ SUS_TEXTURE texture) {
	SUS_ASSERT(texture);
	susRendererTextureBind(texture, 0);
	susRendererSetTexture(0);
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
		for (sus_int_t i = (sus_int_t)(vertexes.size / sizeof(SUS_VERTEX)) - 1; i >= 0; i--) {
			((SUS_LPVERTEX)vertexes.data)[i].uv = susVec2Sum(((SUS_LPVERTEX)vertexes.data)[i].uv, offset);
		}
		return;
	}
	for (sus_int_t i = (sus_int_t)(vertexes.size / sizeof(SUS_VERTEX)) - 1; i >= 0; i--) {
		((SUS_LPVERTEX)vertexes.data)[i].uv = susVec2Sum(susVec2Mult(((SUS_LPVERTEX)vertexes.data)[i].uv, scale), offset);
	}
}

// -----------------------------------------------

// Mesh structure
struct sus_mesh {
	GLuint				vao;		// Vertex Array Objext
	GLuint				vbo;		// Vertex Buffer Object
	GLuint				ibo;		// Index Buffer Object
	USHORT				icount;		// The number of indexes in the mesh
	USHORT				vcount;		// The number of vertexes in the mesh
	GLuint				type;		// The type of the mesh primitive
	SUS_VERTEX_ATTRIBUT	attributes;	// Mesh Attributes
};

// -----------------------------------------------

// Table of vertex attribute elements
typedef struct sus_vertex_attribute_entry {
	sus_size_t	size;		// Attribute size in bytes
	sus_uint_t	ecount;		// The number of simple elements in an attribute
	sus_size_t	offset;		// Confusion regarding structure
	sus_uint_t	type;		// The simple type is OpenGL
	BOOL		normalized;	// Normalize
} SUS_VERTEX_ATTRIBUTE_ENTRY;
// The attribute table of mesh vertices
static const SUS_VERTEX_ATTRIBUTE_ENTRY MeshVertexAttributeTable[] = {
	{ sizeof(SUS_VEC3), 3, SUS_OFFSET_OF(SUS_VERTEX, pos), GL_FLOAT, FALSE },
	{ sizeof(SUS_VEC4), 4, SUS_OFFSET_OF(SUS_VERTEX, color), GL_FLOAT, TRUE },
	{ sizeof(SUS_VEC2), 2, SUS_OFFSET_OF(SUS_VERTEX, uv), GL_FLOAT, FALSE },
	{ sizeof(SUS_VEC3), 3, SUS_OFFSET_OF(SUS_VERTEX, normal), GL_FLOAT, FALSE },
};
// Get the step size from the vertex format
static sus_size_t SUSAPI susVertexFormatGetStride(_In_ SUS_VERTEX_ATTRIBUT attributes) {
	sus_size_t stride = 0;
	for (sus_uint_t type = 0; type < SUS_VERTEX_ATTRIBUT_COUNT; type++)
		if (attributes & (1 << type)) stride += MeshVertexAttributeTable[type].size;
	return stride;
}
// Convert the vertices by applying the format to them
static SUS_DATAVIEW SUSAPI susGeometryConvertFormat(_In_ SUS_LPVERTEX vertexes, _In_ sus_uint_t count, _In_ SUS_VERTEX_ATTRIBUT attributes) {
	SUS_ASSERT(vertexes && count);
	sus_size_t stride = susVertexFormatGetStride(attributes);
	SUS_DATAVIEW fvertexes = susNewData(stride * count);
	if (!fvertexes.data) return fvertexes;
	if (attributes == SUS_VERTEX_ATTRIBUT_FULL) {
		sus_memcpy(fvertexes.data, (LPBYTE)vertexes, fvertexes.size); return fvertexes;
	}
	sus_lpubyte_t curr = (sus_lpubyte_t)fvertexes.data;
	for (sus_uint_t i = 0; i < count; i++, curr += stride) {
		for (sus_uint_t type = 0, offset = 0; type < SUS_VERTEX_ATTRIBUT_COUNT; type++) {
			if (attributes & (1 << type)) {
				sus_memcpy((LPBYTE)(curr + offset), ((LPBYTE)&vertexes[i] + MeshVertexAttributeTable[type].offset), MeshVertexAttributeTable[type].size);
				offset += (sus_uint_t)MeshVertexAttributeTable[type].size;
			}
		}
	}
	return fvertexes;
}
// Set attributes for the mesh
static VOID SUSAPI susRendererMeshSetAttributes(_In_ SUS_VERTEX_ATTRIBUT attributes)
{
	SUS_PRINTDL("Setting attributes for a mesh");
	sus_size_t stride = susVertexFormatGetStride(attributes);
	for (sus_uint_t type = 0, offset = 0; type < SUS_VERTEX_ATTRIBUT_COUNT; type++) {
		if (attributes & (1 << type)) {
			glVertexAttribPointer(type, MeshVertexAttributeTable[type].ecount, MeshVertexAttributeTable[type].type, (GLboolean)MeshVertexAttributeTable[type].normalized, (GLsizei)stride, (void*)((sus_size_t)offset));
			glEnableVertexAttribArray(type);
			offset += (sus_uint_t)MeshVertexAttributeTable[type].size;
		}
	}
}
// Initialize the GPU mesh
static VOID SUSAPI susRendererMeshInit(_Inout_ SUS_MESH mesh, _In_ SUS_LPMESH_BUILDER builder)
{
	SUS_PRINTDL("Initializing the GPU mesh");
	SUS_ASSERT(SUSCurrentRenderer && mesh && builder->geometry.vertexes.size);
	mesh->type = builder->primitiveType;
	mesh->vcount = (USHORT)((!builder->geometry.isInterleaved) ? (builder->geometry.vertexes.size / sizeof(SUS_VERTEX)) : (builder->geometry.vertexes.size / susVertexFormatGetStride(builder->attributes)));
	mesh->icount = (USHORT)(builder->geometry.indexes.size / sizeof(SUS_INDEX));
	SUS_DATAVIEW vertexes = builder->geometry.vertexes;
	if (!builder->geometry.isInterleaved) {
		vertexes = susGeometryConvertFormat((SUS_LPVERTEX)builder->geometry.vertexes.data, mesh->vcount, builder->attributes);
	}
	if (!vertexes.data) return;
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexes.size, vertexes.data, builder->updateType);
	if (!builder->geometry.isInterleaved) susDataDestroy(vertexes);
	susRendererMeshSetAttributes(builder->attributes);
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
	SUS_ASSERT(SUSCurrentRenderer && builder->geometry.vertexes.size && SUSCurrentRenderer->context.currentShader);
	SUS_PRINTDL("Building the mesh");
	SUS_MESH mesh = sus_malloc(sizeof(SUS_MESH_STRUCT));
	if (!mesh) return NULL;
	mesh->attributes = builder->attributes;
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
	susRendererResourcePoolRegister(susRendererResourceManagerCurrent(&SUSCurrentRenderer->resources), SUS_RENDERER_RESOURCE_TYPE_MESH, mesh);
	SUS_PRINTDL("The mesh has been successfully loaded into the GPU");
	return mesh;
}
// Clear the cache from the graphics card
VOID SUSAPI susRendererMeshDestroy(_Inout_ SUS_MESH mesh)
{
	SUS_PRINTDL("Freeing up resources from the GPU");
	SUS_ASSERT(mesh);
	if (mesh->ibo) glDeleteBuffers(1, &mesh->ibo);
	if (mesh->vbo) glDeleteBuffers(1, &mesh->vbo);
	if (mesh->vao) glDeleteVertexArrays(1, &mesh->vao);
	susRendererResourcePoolRemove(susRendererResourceManagerCurrent(&SUSCurrentRenderer->resources), SUS_RENDERER_RESOURCE_TYPE_MESH, mesh);
	sus_free(mesh);
}
// Set new vertices
VOID SUSAPI susRendererMeshSetVertices(_In_ SUS_MESH mesh, _In_ sus_uint_t start, _In_ sus_uint_t count, _In_ SUS_LPVERTEX vertexes)
{
	SUS_ASSERT(mesh && mesh->vao && mesh->vbo && vertexes);
	SUS_DATAVIEW fvertexes = susGeometryConvertFormat(vertexes, count, mesh->attributes);
	if (!fvertexes.data) return;
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, start * susVertexFormatGetStride(mesh->attributes), fvertexes.size, fvertexes.data);
	susDataDestroy(fvertexes);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
// Set new vertex indexes
VOID SUSAPI susRendererMeshSetIndexes(_In_ SUS_MESH mesh, _In_ SUS_LPINDEX indexes)
{
	SUS_ASSERT(mesh && mesh->vao && mesh->ibo && indexes);
	glBindVertexArray(mesh->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mesh->icount * sizeof(SUS_INDEX), indexes);
	glBindVertexArray(0);
}
// Draw mesh
VOID SUSAPI susRendererDrawMesh(_In_ SUS_MESH mesh, _In_ SUS_MAT4 model)
{
	SUS_ASSERT(mesh && mesh->vao && mesh->vbo);
	susRendererMatrixFlush(model);
	glBindVertexArray(mesh->vao);
	if (mesh->ibo) glDrawElements(mesh->type, mesh->icount, GL_UNSIGNED_SHORT, NULL);
	else glDrawArrays(mesh->type, 0, mesh->vcount);
	glBindVertexArray(0);
}

// -----------------------------------------------

// Get the size of the batch data
static VOID SUSAPI susRendererBatchGetSize(_In_ sus_uint_t count, _In_ SUS_LPBATCH_ENTRY entry, _Out_ sus_psize_t vsize, _Out_ sus_psize_t isize) {
	*vsize = *isize = 0;
	sus_size_t stride = susVertexFormatGetStride(entry->mesh->attributes);
	for (sus_uint_t i = 0; i < count; i++) {
		*vsize += entry[i].mesh->vcount * stride;
		*isize += entry[i].mesh->icount * sizeof(SUS_INDEX);
	}
}
// Fill the package with data
static VOID SUSAPI susRendererBatchFill(_In_ sus_uint_t count, _In_ SUS_LPBATCH_ENTRY entry, _Out_ sus_lpbyte_t vertexes, _Out_ sus_lpbyte_t indexes) {
	SUS_ASSERT(count && vertexes && indexes);
	sus_size_t stride = susVertexFormatGetStride(entry->mesh->attributes);
	SUS_INDEX ioffset = 0;
	for (sus_uint_t i = 0; i < count; i++) {
		SUS_MESH mesh = entry[i].mesh;
		sus_size_t mvsize = mesh->vcount * stride, misize = mesh->icount * sizeof(SUS_INDEX);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
		glGetBufferSubData(GL_ARRAY_BUFFER, (GLintptr)0, (GLintptr)mvsize, vertexes);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
		glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, (GLintptr)0, (GLintptr)misize, indexes);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		for (sus_uint_t j = 0; j < mesh->vcount; j++, vertexes += stride) {
			*((SUS_VEC3*)vertexes) = susVec4ToVec3(susMat4MultVec4(entry[i].model, susVec3ToVec4(*((SUS_VEC3*)vertexes), 1.0f)));
			SUS_VEC3 pos = *((SUS_VEC3*)vertexes);
			sus_printf("%d, %d, %d\n", (int)(pos.x * 100.0f), (int)(pos.y * 100.0f), (int)(pos.z * 100.0f));
		}
		for (sus_uint_t j = 0; j < mesh->icount; j++, indexes += sizeof(SUS_INDEX)) {
			*((SUS_INDEX*)indexes) += ioffset;
		}
		ioffset += mesh->vcount;
	}
}
// Glue all the meshes into one
SUS_MESH SUSAPI susRendererMeshBatch(_In_ sus_uint_t count, _In_ SUS_LPBATCH_ENTRY entry)
{
	SUS_PRINTDL("Group meshes into one");
	SUS_ASSERT(count && entry);
	sus_size_t vsize, isize; susRendererBatchGetSize(count, entry, &vsize, &isize);
	sus_lpbyte_t vertexes = sus_malloc(vsize);
	if (!vertexes) return NULL;
	sus_lpbyte_t indexes = sus_malloc(isize);
	if (!indexes) { sus_free(vertexes); return NULL; }
	susRendererBatchFill(count, entry, vertexes, indexes);
	SUS_MESH_BUILDER builder = {
		.attributes = entry->mesh->attributes,
		.geometry = {
			.vertexes = (SUS_DATAVIEW) {.data = (LPBYTE)vertexes, .size = vsize },
			.indexes = (SUS_DATAVIEW) {.data = (LPBYTE)indexes, .size = isize },
			.isInterleaved = TRUE
		},
		.primitiveType = entry->mesh->type,
		.updateType = SUS_MESH_UPDATE_TYPE_STATIC
	};
	SUS_MESH mesh = susRendererBuildMesh(&builder);
	sus_free(indexes);
	sus_free(vertexes);
	return mesh;
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								Creating and rendering mesh instances    						  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// A mesh instance
struct sus_mesh_instance {
	SUS_MESH					sample;			// Instance Template
	SUS_GPU_VECTOR				instanceData;	// Instance Data\param SUS_MESH_INSTANCE_FORMAT_MODEL ? SUS_MAT4 + SUS_MESH_INSTANCE_FORMAT_COLOR ? VEC4 + SUS_MESH_INSTANCE_FORMAT_UVOFFSET ? SUS_VEC2
	GLuint						vao;			// Array of instances
	SUS_MESH_INSTANCE_ATTRIBUTE attributes;		// Attributes for instances
};

// Table of attributes for instancing vertices
static const SUS_VERTEX_ATTRIBUTE_ENTRY InstanceVertexAttributeTable[] = {
	{ sizeof(SUS_VEC4), 4, 0, GL_FLOAT, FALSE },
	{ sizeof(SUS_VEC4), 4, 16, GL_FLOAT, FALSE },
	{ sizeof(SUS_VEC4), 4, 32, GL_FLOAT, FALSE },
	{ sizeof(SUS_VEC4), 4, 48, GL_FLOAT, FALSE },
	{ sizeof(SUS_VEC4), 4, 64, GL_FLOAT, FALSE },
	{ sizeof(SUS_VEC2), 2, 80, GL_FLOAT, FALSE },
};

// -----------------------------------------------

// Calculate the size of the instance
static sus_size_t SUSAPI susCalculateInstanceStride(_In_ SUS_MESH_INSTANCE_ATTRIBUTE attributes) {
	sus_size_t stride = 0;
	for (sus_uint_t type = 0; type < SUS_MESH_INSTANCE_ATTRIBUTE_COUNT;  type++)
		if (attributes & (1 << type)) stride += InstanceVertexAttributeTable[type].size;
	return stride;
}
// Set attributes for a mesh instance
static VOID SUSAPI susRendererInstanceSetAttributes(SUS_MESH_INSTANCE_ATTRIBUTE attributes) {
	sus_size_t offset = 0, stride = susCalculateInstanceStride(attributes);
	for (sus_uint_t type = 0; type < SUS_MESH_INSTANCE_ATTRIBUTE_COUNT; type++) {
		if (attributes & (1 << type)) {
			glVertexAttribPointer(type + SUS_VERTEX_ATTRIBUT_COUNT, InstanceVertexAttributeTable[type].ecount, InstanceVertexAttributeTable[type].type, (GLboolean)InstanceVertexAttributeTable[type].normalized, (GLsizei)stride, (void*)offset);
			glEnableVertexAttribArray(type + SUS_VERTEX_ATTRIBUT_COUNT);
			glVertexAttribDivisor(type + SUS_VERTEX_ATTRIBUT_COUNT, 1);
			offset += InstanceVertexAttributeTable[type].size;
		}
	}
}
// Initialize arrays in the mesh instance
static VOID SUSAPI susRendererInstanceInit(_In_ SUS_MESH_INSTANCE instance, _In_ SUS_MESH_INSTANCE_ATTRIBUTE iattributes, _In_ SUS_VERTEX_ATTRIBUT vattributes) {
	glBindVertexArray(instance->vao);
	glBindBuffer(GL_ARRAY_BUFFER, instance->sample->vbo);
	susRendererMeshSetAttributes(vattributes);
	glBindBuffer(GL_ARRAY_BUFFER, instance->instanceData.vbo);
	susRendererInstanceSetAttributes(iattributes);
	if (instance->sample->ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->sample->ibo);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
// Create an instance for meshes
SUS_MESH_INSTANCE SUSAPI susRendererNewInstance(_In_ SUS_MESH base, _In_ SUS_MESH_INSTANCE_ATTRIBUTE attributes)
{
	SUS_PRINTDL("Creating a Mesh instance");
	SUS_ASSERT(SUSCurrentRenderer && base && base->vbo && SUSCurrentRenderer->context.currentShader);
	SUS_MESH_INSTANCE instance = sus_malloc(sizeof(SUS_MESH_INSTANCE_STRUCT));
	if (!instance) return NULL;
	instance->sample = base;
	instance->attributes = attributes;
	glGenVertexArrays(1, &instance->vao);
	instance->instanceData = susNewGpuVector((DWORD)susCalculateInstanceStride(attributes));
	susRendererInstanceInit(instance, attributes, base->attributes);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		SUS_PRINTDE("OpenGL error loading the mesh: %d", error);
		susRendererInstanceDestroy(instance);
		return NULL;
	}
	susRendererResourcePoolRegister(susRendererResourceManagerCurrent(&SUSCurrentRenderer->resources), SUS_RENDERER_RESOURCE_TYPE_INSTANCE, instance);
	return instance;
}
// Delete a mesh instance
VOID SUSAPI susRendererInstanceDestroy(_In_ SUS_MESH_INSTANCE instance)
{
	SUS_PRINTDL("Deleting a Mesh instance");
	SUS_ASSERT(instance && instance->vao);
	susGpuVectorDestroy(&instance->instanceData);
	glDeleteVertexArrays(1, &instance->vao);
	susRendererResourcePoolRemove(susRendererResourceManagerCurrent(&SUSCurrentRenderer->resources), SUS_RENDERER_RESOURCE_TYPE_INSTANCE, instance);
	sus_free(instance);
}
// Draw mesh instances
VOID SUSAPI susRendererDrawInstance(_In_ SUS_MESH_INSTANCE instance)
{
	SUS_ASSERT(instance && instance->vao);
	glBindVertexArray(instance->vao);
	if (instance->sample->ibo) {
		glDrawElementsInstanced(
			instance->sample->type,
			(GLsizei)instance->sample->icount,
			GL_UNSIGNED_SHORT,
			NULL,
			(GLsizei)instance->instanceData.length
		);
	}
	else {
		glDrawArraysInstanced(
			instance->sample->type,
			(GLsizei)0,
			(GLsizei)instance->sample->vcount,
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
static LPBYTE SUSAPI susExtractInstanceAttributeData(_In_ SUS_MESH_INSTANCE instance, _In_ sus_va_list list)
{
	SUS_ASSERT(instance && list);
	LPBYTE buffer = (LPBYTE)sus_malloc(susCalculateInstanceStride(instance->attributes));
	if (!buffer) return NULL;
	LPBYTE curr = buffer;
	if (instance->attributes & SUS_MESH_INSTANCE_ATTRIBUTE_MATRIX) {
		SUS_LPMAT4 model = sus_va_arg(list, SUS_LPMAT4);
		SUS_ASSERT(model);
		sus_memcpy(buffer, (LPBYTE)model, sizeof(*model));
		curr += sizeof(*model);
	}
	if (instance->attributes & SUS_MESH_INSTANCE_ATTRIBUTE_COLOR) {
		SUS_LPVEC4 color = sus_va_arg(list, SUS_LPVEC4);
		SUS_ASSERT(color);
		sus_memcpy(curr, (LPBYTE)color, sizeof(*color));
		curr += sizeof(*color);
	}
	if (instance->attributes & SUS_MESH_INSTANCE_ATTRIBUTE_UVOFFSET) {
		SUS_LPVEC2 uvoffset = sus_va_arg(list, SUS_LPVEC2);
		SUS_ASSERT(uvoffset);
		sus_memcpy(curr, (LPBYTE)uvoffset, sizeof(*uvoffset));
		curr += SUS_ALIGN(sizeof(*uvoffset), 16);
	}
	return buffer;
}
// Add a new item to the instances\param parameters are passed in order
BOOL SUSAPIV susRendererInstanceAdd(_Inout_ SUS_MESH_INSTANCE instance, ...)
{
	SUS_PRINTDL("Adding a new object to mesh instances");
	SUS_ASSERT(instance && instance->vao);
	sus_va_list list;
	sus_va_start(list, instance);
	LPBYTE data = susExtractInstanceAttributeData(instance, list);
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
BOOL SUSAPIV susRendererInstanceRemove(_Inout_ SUS_MESH_INSTANCE instance, _In_ UINT index)
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
BOOL SUSAPI susRendererInstanceFlush(_Inout_ SUS_MESH_INSTANCE instance)
{
	SUS_PRINTDL("Applying changes after REALLOC in the bag");
	SUS_ASSERT(instance && instance->vao);
	GLuint oldVao = instance->vao;
	glGenVertexArrays(1, &instance->vao);
	susRendererInstanceInit(instance, instance->attributes, instance->sample->attributes);
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

// Build a 2d square
SUS_MESH susRendererPrimitivesBuild_Square(_In_ SUS_VEC2 scale) {
	scale = susVec2Scale(scale, 0.5f);
	const SUS_VERTEX vertexes[] = {
		{.pos = { -scale.x , -scale.y }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 0, 0 } },
		{.pos = { scale.x, -scale.y }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 1, 0 } },
		{.pos = { scale.x, scale.y }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 1, 1 } },
		{.pos = { -scale.x, scale.y }, .color = { 1.0f, 1.0f, 1.0f }, .uv = { 0, 1 } },
	};
	static const SUS_INDEX indexes[] = { 0, 1, 2, 3, 0, 2 };
	SUS_MESH_BUILDER builder = { .attributes = SUS_VERTEX_ATTRIBUT_BASE };
	builder.geometry.vertexes = (SUS_DATAVIEW){ .data = (LPBYTE)vertexes, .size = sizeof(vertexes) };
	builder.geometry.indexes = (SUS_DATAVIEW){ .data = (LPBYTE)indexes, .size = sizeof(indexes) };
	builder.primitiveType = SUS_MESH_PRIMITIVE_TYPE_TRIANGLES;
	builder.updateType = SUS_MESH_UPDATE_TYPE_STATIC;
	return susRendererBuildMesh(&builder);
}
// Build a 2d circle
SUS_MESH susRendererPrimitivesBuild_Circle(_In_ sus_uint_t segments, _In_ sus_float_t radius) {
	segments = sus_min(segments, 360);
	SUS_LPVERTEX vertexes = sus_calloc(segments + 2, sizeof(SUS_VERTEX));
	if (!vertexes) return NULL;
	vertexes->pos = (SUS_VEC3){ 0.0f, 0.0f };
	vertexes->color = (SUS_VEC4){ 1.0f, 1.0f, 1.0f };
	vertexes->uv = (SUS_VEC2){ 0.5f, 0.5f };
	for (sus_uint_t i = 0; i <= segments; i++) {
		sus_int_t angle = sus_round((sus_float_t)i / (sus_float_t)segments * 360.0f);
		SUS_VEC3 pos = { sus_cos(angle) * radius, sus_sin(angle) * radius };
		vertexes[i + 1].pos = pos;
		vertexes[i + 1].color = (SUS_VEC4){ 1.0f, 1.0f, 1.0f };
		vertexes[i + 1].uv = (SUS_VEC2){ 0.5f + 0.5f * pos.x, 0.5f - 0.5f * pos.y };
	}
	SUS_MESH_BUILDER builder = { .attributes = SUS_VERTEX_ATTRIBUT_BASE };
	builder.geometry.vertexes = (SUS_DATAVIEW){ .data = (LPBYTE)vertexes, .size = (segments + 2) * sizeof(SUS_VERTEX) };
	builder.primitiveType = SUS_MESH_PRIMITIVE_TYPE_TRIANGLES_FAN;
	builder.updateType = SUS_MESH_UPDATE_TYPE_STATIC;
	SUS_MESH mesh = susRendererBuildMesh(&builder);
	sus_free(vertexes);
	return mesh;
}
// Build a 3d cube
SUS_MESH susRendererPrimitivesBuild_Cube(_In_ SUS_VEC3 scale) {
	scale = susVec3Scale(scale, 0.5f);
	const SUS_VERTEX vertexes[] = {
		{.pos = { -scale.x, -scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {0, 0, 1} },
		{.pos = { scale.x, -scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {0, 0, 1} },
		{.pos = { scale.x, scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {0, 0, 1} },
		{.pos = { -scale.x, scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {0, 0, 1} },

		{.pos = { scale.x, -scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {0, 0, -1} },
		{.pos = { -scale.x, -scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {0, 0, -1} },
		{.pos = { -scale.x, scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {0, 0, -1} },
		{.pos = { scale.x, scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {0, 0, -1} },

		{.pos = { -scale.x, scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {0, 1, 0} },
		{.pos = { scale.x, scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {0, 1, 0} },
		{.pos = { scale.x, scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {0, 1, 0} },
		{.pos = { -scale.x, scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {0, 1, 0} },

		{.pos = { -scale.x, -scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {0, -1, 0} },
		{.pos = { scale.x, -scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {0, -1, 0} },
		{.pos = { scale.x, -scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {0, -1, 0} },
		{.pos = { -scale.x, -scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {0, -1, 0} },

		{.pos = { scale.x, -scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {1, 0, 0} },
		{.pos = { scale.x, -scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {1, 0, 0} },
		{.pos = { scale.x, scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {1, 0, 0} },
		{.pos = { scale.x, scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {1, 0, 0} },

		{.pos = { -scale.x, -scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {0, 0}, .normal = {-1, 0, 0} },
		{.pos = { -scale.x, -scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {1, 0}, .normal = {-1, 0, 0} },
		{.pos = { -scale.x, scale.y, scale.z }, .color = { 1, 1, 1 }, .uv = {1, 1}, .normal = {-1, 0, 0} },
		{.pos = { -scale.x, scale.y, -scale.z }, .color = { 1, 1, 1 }, .uv = {0, 1}, .normal = {-1, 0, 0} },
	};
	static const SUS_INDEX indexes[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};
	SUS_MESH_BUILDER builder = { .attributes = SUS_VERTEX_ATTRIBUT_BASE };
	builder.geometry.vertexes = (SUS_DATAVIEW){ .data = (LPBYTE)vertexes, .size = sizeof(vertexes) };
	builder.geometry.indexes = (SUS_DATAVIEW){ .data = (LPBYTE)indexes, .size = sizeof(indexes) };
	builder.primitiveType = SUS_MESH_PRIMITIVE_TYPE_TRIANGLES;
	builder.updateType = SUS_MESH_UPDATE_TYPE_STATIC;
	return susRendererBuildMesh(&builder);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   Graphics Resource Manager      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Initialize the resource pool
VOID SUSAPI susRendererResourcePoolInit(_Out_ SUS_LPRENDERER_RESOURCE_POOL pool, _In_ UINT id) {
	SUS_PRINTDL("Initializing the Resource pool");
	SUS_ASSERT(pool);
	pool->id = id;
	for (UINT i = 0; i < SUS_RENDERER_RESOURCE_TYPE_COUNT; i++) {
		pool->pool[i] = susNewSet(SUS_OBJECT);
	}
}
// Destroy the renderer's resource pool
VOID SUSAPI susRendererResourcePoolCleanup(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ SUS_LPRENDERER_RESOURCE_POOL pool) {
	SUS_PRINTDL("Cleaning up all resources");
	SUS_ASSERT(pool);
	UINT old = manager->current;
	susRendererResourceManagerSetCurrent(manager, pool->id);
	for (UINT i = 0; i < SUS_RENDERER_RESOURCE_TYPE_COUNT; i++) {
		susMapForeach(pool->pool[i], j) {
			SUS_OBJECT resource = *(SUS_OBJECT*)susMapIterKey(j);
			RendererResourceDestructorTable[i](resource);
		}
		susSetDestroy(pool->pool[i]);
	}
	susRendererResourceManagerSetCurrent(manager, old);
}

// -----------------------------------------------

// Get a resource
BOOL SUSAPI susRendererResourcePoolContains(_In_ SUS_LPRENDERER_RESOURCE_POOL pool, _In_ SUS_RENDERER_RESOURCE_TYPE type, _In_ SUS_OBJECT resource) {
	SUS_PRINTDL("Getting a resource");
	SUS_ASSERT(SUSCurrentRenderer && pool && (UINT)type < (UINT)SUS_RENDERER_RESOURCE_TYPE_COUNT);
	return susSetContains(pool->pool[type], &resource);
}
// Register a resource
VOID SUSAPI susRendererResourcePoolRegister(_Inout_ SUS_LPRENDERER_RESOURCE_POOL pool, _In_ SUS_RENDERER_RESOURCE_TYPE type, _In_ SUS_OBJECT resource) {
	SUS_PRINTDL("Registering a resource");
	SUS_ASSERT(SUSCurrentRenderer && pool && (UINT)type < (UINT)SUS_RENDERER_RESOURCE_TYPE_COUNT && !susRendererResourcePoolContains(pool, type, resource));
	susSetAdd(&pool->pool[type], &resource);
}
// Delete a resource
VOID SUSAPI susRendererResourcePoolRemove(_In_ SUS_LPRENDERER_RESOURCE_POOL pool, _In_ SUS_RENDERER_RESOURCE_TYPE type, _In_ SUS_OBJECT resource) {
	SUS_PRINTDL("Deleting a resource");
	SUS_ASSERT(SUSCurrentRenderer && pool && (UINT)type < (UINT)SUS_RENDERER_RESOURCE_TYPE_COUNT && susRendererResourcePoolContains(pool, type, resource));
	susSetRemove(&pool->pool[type], &resource);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Create a Resource manager
VOID SUSAPI susRendererResourceManagerInit(_Out_ SUS_LPRENDERER_RESOURCE_MANAGER manager) {
	SUS_PRINTDL("Creating a Resource manager");
	SUS_ASSERT(manager);
	manager->current = SUS_RENDERER_RESOURCE_MANAGER_MAIN;
	manager->level = susNewMap(UINT, SUS_RENDERER_RESOURCE_POOL);
	susRendererResourceManagerAddPool(manager, SUS_RENDERER_RESOURCE_MANAGER_MAIN);
	susRendererResourceManagerSetCurrent(manager, SUS_RENDERER_RESOURCE_MANAGER_MAIN);
}
// Destroy the resource manager and its resources
VOID SUSAPI susRendererResourceManagerCleanup(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager) {
	SUS_PRINTDL("Destroying the resource manager and its resources");
	SUS_ASSERT(manager);
	susMapForeach(manager->level, i) {
		SUS_LPRENDERER_RESOURCE_POOL pool = (SUS_LPRENDERER_RESOURCE_POOL)susMapIterValue(i);
		susRendererResourcePoolCleanup(manager, pool);
	}
	susMapDestroy(manager->level);
}

// -----------------------------------------------

// Set the resource pool as current
VOID SUSAPI susRendererResourceManagerSetCurrent(_Inout_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ UINT pool) {
	SUS_PRINTDL("Setting up a resource pool as the current one");
	SUS_ASSERT(manager);
	manager->current = pool;
}
// Set the resource pool as current
SUS_LPRENDERER_RESOURCE_POOL SUSAPI susRendererResourceManagerCurrent(_Inout_ SUS_LPRENDERER_RESOURCE_MANAGER manager) {
	SUS_ASSERT(manager);
	return susRendererResourceManagerGet(manager, manager->current);
}
// Get a pool of resources
SUS_LPRENDERER_RESOURCE_POOL SUSAPI susRendererResourceManagerGet(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ UINT pool) {
	SUS_ASSERT(manager);
	return (SUS_LPRENDERER_RESOURCE_POOL)susMapGet(manager->level, &pool);
}

// -----------------------------------------------

// Add a new pool to the resource manager
SUS_LPRENDERER_RESOURCE_POOL SUSAPI susRendererResourceManagerAddPool(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ UINT pool) {
	SUS_PRINTDL("Adding a resource pool with id %d", pool);
	SUS_ASSERT(manager && !susRendererResourceManagerGet(manager, pool));
	SUS_LPRENDERER_RESOURCE_POOL resource = (SUS_LPRENDERER_RESOURCE_POOL)susMapAdd(&manager->level, &pool, NULL);
	if (!resource) return NULL;
	susRendererResourcePoolInit(resource, pool);
	return resource;
}
// Delete a pool from the resource manager
VOID SUSAPI susRendererResourceManagerRemovePool(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ UINT pool) {
	SUS_PRINTDL("Destroy the graphics resource pool with id %d", pool);
	SUS_ASSERT(manager && susRendererResourceManagerGet(manager, pool));
	SUS_LPRENDERER_RESOURCE_POOL resource = susRendererResourceManagerGet(manager, pool);
	susRendererResourcePoolCleanup(manager, resource);
	susMapRemove(&manager->level, &pool);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

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
	sus_timer_start(&renderer->frameTimer);
	susRendererResourceManagerInit(&renderer->resources);
	return renderer;
error:
	SUS_PRINTDE("Failed to initialize rendering on the window");
	return NULL;
}
// Set optimization for 3D graphics
VOID SUSAPI susRendererSet3D() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}
// Set the background color
VOID SUSAPI susRendererSetBackground(SUS_VEC3 color) { glClearColor(color.r, color.g, color.b, 0.0f); }
// Set the renderer as the current one
VOID SUSAPI susRendererSetCurrent(_In_opt_ SUS_RENDERER renderer) {
	SUS_PRINTDL("Setting the context as the current one");
	if (renderer) wglMakeCurrent(renderer->super.hdc, renderer->super.hGlrc);
	else wglMakeCurrent(NULL, NULL);
	SUSCurrentRenderer = renderer;
}
// Remove the graphical context
VOID SUSAPI susRendererCleanup(_In_ SUS_RENDERER renderer) {
	SUS_PRINTDL("Clearing the graphical context");
	SUS_ASSERT(renderer);
	susRendererResourceManagerCleanup(&renderer->resources);
	susRendererDirectCleanup(renderer->hWnd, renderer->super);
	if (SUSCurrentRenderer == renderer) susRendererSetCurrent(0);
	sus_timer_stop(&renderer->frameTimer);
	sus_free(renderer);
}
// Update the context output area
VOID SUSAPI susRendererSize() {
	SUS_ASSERT(SUSCurrentRenderer && SUSCurrentRenderer->context.currentCamera);
	RECT rect = { 0 };
	GetClientRect(SUSCurrentRenderer->hWnd, &rect);
	sus_float_t aspect = (sus_float_t)rect.right / (sus_float_t)rect.bottom;
	susRendererCameraSize(SUSCurrentRenderer->context.currentCamera, aspect);
	glViewport(rect.left, rect.top, rect.right, rect.bottom);
}

// -----------------------------------------------

// Get the current renderer
SUS_RENDERER SUSAPI susGetRenderer() {
	return SUSCurrentRenderer;
}
// Get the current camera
SUS_CAMERA SUSAPI susRendererGetCamera() {
	SUS_ASSERT(SUSCurrentRenderer);
	return SUSCurrentRenderer->context.currentCamera;
}
// Get the current shader
SUS_RENDERER_SHADER SUSAPI susRendererGetShader() {
	SUS_ASSERT(SUSCurrentRenderer);
	return SUSCurrentRenderer->context.currentShader;
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Start rendering the frame
VOID SUSAPI susRendererBeginFrame() {
	SUS_ASSERT(SUSCurrentRenderer && SUSCurrentRenderer->context.currentShader && SUSCurrentRenderer->context.currentCamera);
	susRendererCameraUpdate(SUSCurrentRenderer->context.currentCamera);
	susRendererFrameFlush(sus_timef(), SUSCurrentRenderer->frameTimer.state.delta);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
// Finish painting the frame
VOID SUSAPI susRendererEndFrame() {
	SUS_ASSERT(SUSCurrentRenderer && SUSCurrentRenderer->context.currentShader && SUSCurrentRenderer->context.currentCamera);
	if (!SwapBuffers(SUSCurrentRenderer->super.hdc)) {
		SUS_PRINTDE("Failed to apply buffer transfer for the frame");
		SUS_PRINTDC(GetLastError());
		return;
	}
	sus_timer_update(&SUSCurrentRenderer->frameTimer);
	for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
		SUS_PRINTDE("OpenGL error: %d", error);
		susErrorPush(SUS_ERROR_API_ERROR, SUS_ERROR_TYPE_SYSTEM);
	}
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////
