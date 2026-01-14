// renderer.h
//
#ifndef _SUS_RENDERER_
#define _SUS_RENDERER_

// v1.0

#include "tmath.h"
#include "time.h"
#include "uicommon.h"
#include "vector.h"
#include "hashtable.h"
#include "..\susgl\suswgl.h"

#pragma warning(push)
#pragma warning(disable: 4201)

// ================================================================================================= //
// ************************************************************************************************* //
//												 GPU RENDER											 //
/*								Working with graphics on a opengl api gpu							 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Basic set of functions      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The context of the render
typedef struct sus_renderer_direct {
	HGLRC	hGlrc;
	HDC		hdc;
} SUS_RENDERER_DIRECT;

// Basic graphics initialization
SUS_RENDERER_DIRECT SUSAPI susRendererDirectBasicSetup(_In_ HWND hWnd);
// A modern way to initialize graphics
SUS_RENDERER_DIRECT SUSAPI susRendererDirectModernSetup(_In_ HWND hWnd, _In_ WORD major, _In_ WORD minor);
// Clean up graphics resources
VOID SUSAPI susRendererDirectCleanup(_In_ HWND hWnd, _In_ SUS_RENDERER_DIRECT context);
// Output a log about the graph
VOID SUSAPI susRendererDirectPrint();

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Working with shaders      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Types of shaders
typedef enum sus_shader_type {
	SUS_SHADER_TYPE_UNKNOWN,
	SUS_SHADER_TYPE_VERTEX = GL_VERTEX_SHADER,
	SUS_SHADER_TYPE_FRAGMENT = GL_FRAGMENT_SHADER,
	SUS_SHADER_TYPE_GEOMETRY = GL_GEOMETRY_SHADER
} SUS_SHADER_TYPE, * SUS_LPSHADER_TYPE;

// -----------------------------------------------

// Create a shader
GLuint SUSAPI susRendererDirectLoadShader(_In_ SUS_SHADER_TYPE type, _In_ LPCSTR source);
// Get the shader log
LPSTR SUSAPI susRendererDirectShaderGetLog(_In_ GLuint shader);

// -----------------------------------------------

// Load shaders to GPU
GLuint SUSAPI susRendererDirectLinkProgram(_In_ UINT shaderCount, _In_ GLuint* shaders, _In_ BOOL DeleteShadersAfterUploading);
// Remove the program from the GPU
VOID SUSAPI susRendererDirectProgramCleanup(_In_ GLuint program);
// Get the program log
LPSTR SUSAPI susRendererDirectProgramGetLog(_In_ GLuint program);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									     Dynamic array on GPU    								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The default number of elements in the array
#define SUS_BASIC_GPU_VECTOR_COUNT 4
// A large number of elements in the GPU array
#define SUS_GPU_VECTOR_LARGE_COUNT 50
// Dynamic array on GPU
typedef struct sus_gpu_vector {
	sus_uint32_t	itemSize;	// The size of the type in the array
	sus_uint32_t	length;		// Length of the array
	sus_uint32_t	capacity;	// Maximum buffer capacity in element
	GLuint			vbo;		// Buffer on GPU
} SUS_GPU_VECTOR, *SUS_LPGPU_VECTOR;

// -----------------------------------------------

// Create a new array on the GPU
SUS_GPU_VECTOR SUSAPI susNewGpuVector(_In_ sus_uint_t itemSize);
// Destroy the array
VOID SUSAPI susGpuVectorDestroy(_In_ SUS_LPGPU_VECTOR vector);
// Output the array log
VOID SUSAPI susGpuVectorLog(SUS_LPGPU_VECTOR vector);

// -----------------------------------------------

// Clear the array
VOID SUSAPI susGpuVectorClean(_In_ SUS_LPGPU_VECTOR vector);
// Add multiple elements to an array
BOOL SUSAPI susGpuVectorInsertArr(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index, _In_opt_ SUS_OBJECT value, _In_ sus_uint_t count);
// Delete an element from an array
BOOL SUSAPI susGpuVectorEraseArr(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index, _In_ sus_uint_t count);
// Get an array of elements from a vector\param The returned value must be cleared using sus_free
SUS_OBJECT SUSAPI susGpuVectorRead(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index, _In_ sus_uint_t count);

// Swap elements
BOOL SUSAPI susGpuVectorSwap(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t a, _In_ sus_uint_t b);
// Swap places and delete
BOOL SUSAPI susGpuVectorSwapErase(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index);

// -----------------------------------------------

// Insert an element into an array
SUS_INLINE BOOL SUSAPI susGpuVectorInsert(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index, _In_opt_ SUS_OBJECT value) {
	SUS_ASSERT(vector && index <= vector->length);
	return susGpuVectorInsertArr(vector, index, value, 1);
}
// Delete an element from an array
SUS_INLINE BOOL SUSAPI susGpuVectorErase(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index) {
	SUS_ASSERT(vector && index < vector->length);
	return susGpuVectorEraseArr(vector, index, 1);
}
// Get an element from an array\param The returned value must be cleared using sus_free
SUS_INLINE SUS_OBJECT SUSAPI susGpuVectorGet(_In_ SUS_LPGPU_VECTOR vector, _In_ sus_uint_t index) {
	SUS_ASSERT(vector && index < vector->length);
	return susGpuVectorRead(vector, index, 1);
}

// -----------------------------------------------

// Push an element to the end of the array
SUS_INLINE BOOL SUSAPI susGpuVectorPush(_In_ SUS_LPGPU_VECTOR vector, _In_opt_ SUS_OBJECT value) {
	SUS_ASSERT(vector);
	return susGpuVectorInsert(vector, vector->length, value);
}
// Delete the last element of the array
SUS_INLINE VOID SUSAPI susGpuVectorPop(_In_ SUS_LPGPU_VECTOR vector) {
	SUS_ASSERT(vector && vector->length);
	susGpuVectorErase(vector, vector->length - 1);
}
// Insert an element at the beginning of the array
SUS_INLINE BOOL SUSAPI susGpuVectorUnshift(_In_ SUS_LPGPU_VECTOR vector, _In_opt_ SUS_OBJECT value) {
	SUS_ASSERT(vector);
	return susGpuVectorInsert(vector, 0, value);
}
// Delete the first element of the array
SUS_INLINE VOID SUSAPI susGpuVectorShift(_In_ SUS_LPGPU_VECTOR vector) {
	SUS_ASSERT(vector);
	susGpuVectorErase(vector, 0);
}

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// ================================================================================================= //
// ************************************************************************************************* //
//											 RENDERING API											 //
/*								High-level api for working with rendering							 */
// ************************************************************************************************* //
// ================================================================================================= //

////////////////////////////////////////////////////////////////////////////////////////////////////
//							  The structure of working with the camera    						  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Camera type
typedef enum SUS_CAMERA_type {
	SUS_CAMERA_TYPE_UNKNOWN,
	SUS_CAMERA_TYPE_2D,
	SUS_CAMERA_TYPE_3D
} SUS_CAMERA_TYPE;
#pragma warning(push)
#pragma warning(disable: 4201)

// Basic camera structure
typedef struct sus_camera {
	SUS_MAT4					view;			// The view matrix
	SUS_MAT4					proj;			// Projection matrix
	SUS_MAT4					projview;		// Cached view and projection matrix
	SUS_VEC3					position;		// The camera position
	UINT						type : 8;		// Camera type
	BOOL						dirty : 8;		// A redesign of the projview matrix is required
	BOOL						viewDirty : 8;	// The view matrix needs to be rebuilt
	sus_float_t					aspect;			// The ratio of the sides of the screen
} SUS_CAMERA_STRUCT, * SUS_CAMERA;
// 2d camera
typedef struct sus_camera2d {
	SUS_CAMERA_STRUCT;
	sus_float_t rotation;
	sus_float_t	zoom;
} SUS_CAMERA2D_STRUCT, * SUS_CAMERA2D;
// 3d camera
typedef struct sus_camera3d {
	SUS_CAMERA_STRUCT;
	SUS_VEC3 forward;
	SUS_LPVEC3 target;
	sus_float_t	nearp, farp;
	sus_uint_t fov;
} SUS_CAMERA3D_STRUCT, * SUS_CAMERA3D;

#pragma warning(pop)

// -----------------------------------------------

// Create a general camera
SUS_CAMERA SUSAPI susRendererNewCamera(_In_ SUS_CAMERA_TYPE type);
// Create a 2d camera
SUS_CAMERA2D SUSAPI susRendererNewCamera2D();
// Create a 3d camera
SUS_CAMERA3D SUSAPI susRendererNewCamera3D(_In_ sus_uint_t fov, _In_ sus_float_t nearp, _In_ sus_float_t farp);
// Destroy the camera
VOID SUSAPI susRendererCameraDestroy(_In_ SUS_CAMERA camera);
// Set the current camera
VOID SUSAPI susRendererSetCamera(_In_ SUS_CAMERA camera);

// -----------------------------------------------
// 
// Set the camera position
VOID SUSAPI susRendererCameraSetPosition(_Inout_ SUS_CAMERA camera, _In_ SUS_VEC3 pos);
// Set the camera position
VOID SUSAPI susRendererCameraSetRotation(_Inout_ SUS_CAMERA camera, _In_ SUS_VEC3 rotation);
// Apply camera changes if camera is dirty
VOID SUSAPI susRendererCameraUpdate(_Inout_ SUS_CAMERA camera);
// Set a new size for the render
VOID SUSAPI susRendererCameraSize(_In_ SUS_CAMERA camera, _In_ sus_float_t aspect);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Set the view matrix for the camera
VOID SUSAPI susRendererCamera2DSetZoom(_Inout_ SUS_CAMERA2D camera, _In_ sus_float_t zoom);
// Set the view matrix for the camera
VOID SUSAPI susRendererCamera3DSetFov(_Inout_ SUS_CAMERA3D camera, _In_ sus_uint_t fov);
// Set the view matrix for the camera
VOID SUSAPI susRendererCamera3DSetNearPlane(_Inout_ SUS_CAMERA3D camera, _In_ sus_float_t nearp);
// Set the view matrix for the camera
VOID SUSAPI susRendererCamera3DSetFarPlane(_Inout_ SUS_CAMERA3D camera, _In_ sus_float_t farp);

// -----------------------------------------------

// Get a zoom camera
sus_float_t SUSAPI susRendererCamera2DGetZoom(_In_ SUS_CAMERA2D camera);
// Get the camera's fov
sus_uint_t SUSAPI susRendererCamera3DGetFov(_In_ SUS_CAMERA3D camera);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								   Creating and installing shaders    							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Vertex type
typedef enum sus_vertext_type {
	SUS_VERTEX_TYPE_2D = 2,
	SUS_VERTEX_TYPE_3D = 3,
} SUS_VERTEX_TYPE, * SUS_LPVERTEX_TYPE;
// Vertex Attributes
typedef enum sus_vertex_attribute {
	SUS_VERTEX_ATTRIBUT_NONE = 0,
	SUS_VERTEX_ATTRIBUT_POSITION = 1 << 0,
	SUS_VERTEX_ATTRIBUT_ALPHA_COLOR = 1 << 1,
	SUS_VERTEX_ATTRIBUT_COLOR = 1 << 2,
	SUS_VERTEX_ATTRIBUT_TEXTURE = 1 << 3,
	SUS_VERTEX_ATTRIBUT_NORMAL = 1 << 4,
	SUS_VERTEX_ATTRIBUT_COUNT = 4,
	SUS_VERTEX_ATTRIBUT_BASE = SUS_VERTEX_ATTRIBUT_POSITION | SUS_VERTEX_ATTRIBUT_COLOR | SUS_VERTEX_ATTRIBUT_TEXTURE,
	SUS_VERTEX_ATTRIBUT_FULL = SUS_VERTEX_ATTRIBUT_POSITION | SUS_VERTEX_ATTRIBUT_COLOR | SUS_VERTEX_ATTRIBUT_TEXTURE | SUS_VERTEX_ATTRIBUT_NORMAL
} SUS_VERTEX_ATTRIBUT;
// The format for mesh vertices
typedef struct sus_vertex_format {
	SUS_VERTEX_TYPE		type;
	SUS_VERTEX_ATTRIBUT attributes;
} SUS_VERTEX_FORMAT;

// Vertex format template - 2d vertices
#define SUS_VERTEX_FORMAT_2D (SUS_VERTEX_FORMAT) { .type = SUS_VERTEX_TYPE_2D, .attributes = SUS_VERTEX_ATTRIBUT_BASE }
// Vertex format template - 3d vertices
#define SUS_VERTEX_FORMAT_3D (SUS_VERTEX_FORMAT) { .type = SUS_VERTEX_TYPE_3D, .attributes = SUS_VERTEX_ATTRIBUT_FULL }

// -----------------------------------------------

// The format of the instance variables
typedef enum sus_mesh_instance_attribute {
	SUS_MESH_INSTANCE_ATTRIBUTE_MATRIX = 0,		// mat4 - default
	SUS_MESH_INSTANCE_ATTRIBUTE_COLOR = 1 << 0,	// vec4
	SUS_MESH_INSTANCE_ATTRIBUTE_UVOFFSET = 1 << 1	// vec2
} SUS_MESH_INSTANCE_ATTRIBUTE;

// -----------------------------------------------

// Shader format
typedef struct sus_shader_format {
	SUS_VERTEX_FORMAT			vFormat;	// The vertex format
	SUS_MESH_INSTANCE_ATTRIBUTE iFormat;	// The format of the instances
} SUS_SHADER_FORMAT;

// Shader format - 2D shader
#define SUS_SHADER_FORMAT_2D (SUS_SHADER_FORMAT) { .vFormat = SUS_VERTEX_FORMAT_2D, .iFormat = SUS_MESH_INSTANCE_ATTRIBUTE_MATRIX }
// Shader format - 3D shader
#define SUS_SHADER_FORMAT_3D (SUS_SHADER_FORMAT) { .vFormat = SUS_VERTEX_FORMAT_3D, .iFormat = SUS_MESH_INSTANCE_ATTRIBUTE_MATRIX }

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Basic 2D Shader Code - Vertex Shader
static const CHAR* SUSBuiltinShader2D_VS =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec4 aColor;\n"
"layout(location = 2) in vec2 aTexCoord;\n"
"out vec4 vColor;\n"
"out vec2 vTexCoord;\n"
"uniform mat4 sus_mvp;\n"
"void main() {\n"
"    gl_Position = sus_mvp * vec4(aPos, 0.0, 1.0);\n"
"    vColor = aColor;\n"
"    vTexCoord = aTexCoord;\n"
"}\n";
// Basic 2D Shader Code - Fragment Shader
static const CHAR* SUSBuiltinShader2D_FS =
"#version 330 core\n"
"in vec4 vColor;\n"
"in vec2 vTexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D uTexture;\n"
"void main() {\n"
"    vec4 texColor = texture(uTexture, vTexCoord);\n"
"    FragColor = vColor * texColor;\n"
"}\n";
// Format for a regular 2D shader
#define g_BuiltinShader2D_FORMAT (SUS_SHADER_FORMAT) { .vFormat = (SUS_VERTEX_FORMAT){ .type = SUS_VERTEX_TYPE_2D, .attributes = SUS_VERTEX_ATTRIBUT_BASE } }

// -----------------------------------------------

// Basic 2D shader code - Vertex shader without textures
static const CHAR* SUSBuiltinShader2DColor_VS =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec4 aColor;\n"
"out vec4 vColor;\n"
"uniform mat4 sus_mvp;\n"
"void main() {\n"
"    gl_Position = sus_mvp * vec4(aPos, 0.0, 1.0);\n"
"    vColor = aColor;\n"
"}\n";
// Basic 2D shader code - Fragment shader without textures
static const CHAR* SUSBuiltinShader2DColor_FS =
"#version 330 core\n"
"in vec4 vColor;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = vColor;\n"
"}\n";
// Format for a 2D shader without textures
#define g_BuiltinShader2DColor_FORMAT (SUS_SHADER_FORMAT) { .vFormat = (SUS_VERTEX_FORMAT){ .type = SUS_VERTEX_TYPE_2D, .attributes = SUS_VERTEX_ATTRIBUT_BASE & ~SUS_VERTEX_ATTRIBUT_TEXTURE } }

// -----------------------------------------------

// Basic 3D shader code - vertex shader
static const CHAR* SUSBuiltinShader3D_VS =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec4 aColor;\n"
"layout(location = 2) in vec2 aTexCoord;\n"
"out vec4 vColor;\n"
"out vec2 vTexCoord;\n"
"uniform mat4 sus_mvp;\n"
"void main() {\n"
"    gl_Position = sus_mvp * vec4(aPos, 1.0);\n"
"    vColor = aColor;\n"
"    vTexCoord = aTexCoord;\n"
"}\n";
// Basic 3D shader code - fragment shader
static const CHAR* SUSBuiltinShader3D_FS =
"#version 330 core\n"
"in vec4 vColor;\n"
"in vec2 vTexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D uTexture;\n"
"void main() {\n"
"    vec4 texColor = texture(uTexture, vTexCoord);\n"
"    FragColor = vColor * texColor;\n"
"}\n";
// The format for the 3d shader
#define g_BuiltinShader3D_FORMAT (SUS_SHADER_FORMAT) { .vFormat = (SUS_VERTEX_FORMAT){ .type = SUS_VERTEX_TYPE_3D, .attributes = SUS_VERTEX_ATTRIBUT_BASE } }

// -----------------------------------------------

// 2D vertex shader for instancing
static const CHAR* g_BuiltinShader2D_Instanced_VS =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"
"layout(location = 1) in vec4 aColor;\n"
"layout(location = 2) in vec2 aTexCoord;\n"
"layout(location = 4) in mat4 aInstanceMatrix;\n"
"layout(location = 8) in vec4 aInstanceColor;\n"
"out vec4 vColor;\n"
"out vec2 vTexCoord;\n"
"uniform mat4 sus_projview;\n"
"void main() {\n"
"    vec4 worldPos = aInstanceMatrix * vec4(aPos, 0.0, 1.0);\n"
"    gl_Position = sus_projview * worldPos;\n"
"    vColor = aColor * aInstanceColor;\n"
"    vTexCoord = aTexCoord;\n"
"}\n";
// Format for 2d shader instances
#define g_BuiltinShader2D_Instanced_FORMAT (SUS_SHADER_FORMAT) { .vFormat = (SUS_VERTEX_FORMAT){ .type = SUS_VERTEX_TYPE_2D, .attributes = SUS_VERTEX_ATTRIBUT_BASE }, .iFormat = SUS_MESH_INSTANCE_ATTRIBUTE_MATRIX | SUS_MESH_INSTANCE_ATTRIBUTE_COLOR }
// 3D vertex shader for instancing
static const CHAR* g_BuiltinShader3D_Instanced_VS =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec4 aColor;\n"
"layout(location = 2) in vec2 aTexCoord;\n"
"layout(location = 4) in mat4 aInstanceMatrix;\n"
"out vec4 vColor;\n"
"out vec2 vTexCoord;\n"
"uniform mat4 sus_projview;\n"
"void main() {\n"
"    vec4 worldPos = aInstanceMatrix * vec4(aPos, 1.0);\n"
"    gl_Position = sus_projview * worldPos;\n"
"    vColor = aColor;\n"
"    vTexCoord = aTexCoord;\n"
"}\n";
// Format for 2d shader instances
#define g_BuiltinShader3D_Instanced_FORMAT (SUS_SHADER_FORMAT) { .vFormat = (SUS_VERTEX_FORMAT){ .type = SUS_VERTEX_TYPE_3D, .attributes = SUS_VERTEX_ATTRIBUT_BASE }, .iFormat = SUS_MESH_INSTANCE_ATTRIBUTE_MATRIX }

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Shader Module
typedef struct sus_renderer_shader_module { GLuint shader; } SUS_RENDERER_SHADER_MODULE, * SUS_LPRENDERER_SHADER_MODULE;
// Load a shader from a file
GLuint SUSAPI susRendererLoadShaderModule(_In_ LPCSTR path);

// -----------------------------------------------

// Global Shader uniforms
typedef enum sus_renderer_base_uniforms {
	SUS_RENDERER_BASE_UNIFORM_MVP,		// sus_mvp -> mat4
	SUS_RENDERER_BASE_UNIFORM_TIME,		// sus_time -> float
	SUS_RENDERER_BASE_UNIFORM_COSTIME,	// sus_ctime -> float
	SUS_RENDERER_BASE_UNIFORM_SINTIME,	// sus_stime -> float
	SUS_RENDERER_BASE_UNIFORM_DELTA,	// sus_delta -> float
	SUS_RENDERER_BASE_UNIFORM_PROJVIEW,	// sus_projview -> mat4
	SUS_RENDERER_BASE_UNIFORM_COUNT
} SUS_RENDERER_BASE_UNIFORM;
// High-level shader structure
typedef struct sus_renderer_shader {
	GLuint				program;									// Shader program
	SUS_SHADER_FORMAT	format;										// Shader meta data
	GLuint				uniforms[SUS_RENDERER_BASE_UNIFORM_COUNT];	// SUS_RENDERER_BASE_UNIFORMS
} SUS_RENDERER_SHADER_STRUCT, *SUS_RENDERER_SHADER;

// Load the shader as the current one
SUS_RENDERER_SHADER SUSAPI susRendererNewShader(_In_ UINT count, _In_ SUS_LPRENDERER_SHADER_MODULE modules, _In_ SUS_SHADER_FORMAT format);
// Destroy the shader
VOID SUSAPI susRendererShaderDestroy(_In_ SUS_RENDERER_SHADER shader);
// Set the shader as the current one
VOID SUSAPI susRendererSetShader(_In_ SUS_RENDERER_SHADER shader);

// -----------------------------------------------

// Transfer transformation data to graphics
VOID SUSAPI susRendererMatrixFlush(_In_ SUS_MAT4 model);
// Send frame data to GPU
VOID SUSAPI susRendererFrameFlush(sus_float_t time, sus_float_t delta);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									  Working with 2d textures     								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The type of texture stretching
typedef enum sus_renderer_texture_wrap {
	SUS_RENDERER_TEXTURE_WRAP_REPEAT,	// Duplicate the texture
	SUS_RENDERER_TEXTURE_WRAP_MIRROR,	// Repeat with reflection
	SUS_RENDERER_TEXTURE_WRAP_CLAMP		// Stretch the texture
} SUS_RENDERER_TEXTURE_WRAP;
// Smoothing level
typedef enum sus_renderer_texture_smoothing {
	SUS_RENDERER_TEXTURE_SMOOTHING_NONE,
	SUS_RENDERER_TEXTURE_SMOOTHING_LOW,
	SUS_RENDERER_TEXTURE_SMOOTHING_MEDIUM,
	SUS_RENDERER_TEXTURE_SMOOTHING_HIGH,
} SUS_RENDERER_TEXTURE_SMOOTHING;
// Texture format
typedef struct sus_renderer_texture_format {
	SUS_RENDERER_TEXTURE_WRAP		wrapX, wrapY;	// The type of texture stretching
	SUS_RENDERER_TEXTURE_SMOOTHING	smoothing;		// Smoothing level
	BOOL							mipmap;			// Create mipmap for a texture in 3D
} SUS_RENDERER_TEXTURE_FORMAT;
// Texture Builder
typedef struct sus_renderer_texture_builder {
	UINT						channels;	// Number of channels per pixel
	SUS_SIZE					size;		// Image Size
	SUS_LPMEMORY				data;		// Image Data
	SUS_RENDERER_TEXTURE_FORMAT format;		// Image Format
} SUS_RENDERER_TEXTURE_BUILDER, * SUS_LPRENDERER_TEXTURE_BUILDER;
// Texture
typedef struct sus_renderer_texture {
	GLuint _PARENT_;	// Texture
	SUS_SIZE size;		// Texture size in pixels
} SUS_RENDERER_TEXTURE_STRUCT, * SUS_RENDERER_TEXTURE;

// Create a 2d texture
SUS_RENDERER_TEXTURE SUSAPI susRendererNewTexture(_In_ const SUS_LPRENDERER_TEXTURE_BUILDER builder);
// Destroy the texture
VOID SUSAPI susRendererTextureDestroy(_In_ SUS_RENDERER_TEXTURE texture);
// Load a texture from a file
SUS_RENDERER_TEXTURE SUSAPI susRendererLoadTexture(_In_ LPCWSTR path, _In_ SUS_RENDERER_TEXTURE_FORMAT format);
// Load an image from resources by type - TEXTURE
SUS_RENDERER_TEXTURE SUSAPI susRendererLoadTextureResource(_In_ LPCSTR resourceName, _In_ SUS_RENDERER_TEXTURE_FORMAT format);

// -----------------------------------------------

// Texture template - 2D Sprite
#define SUS_RENDERER_TEXTURE_FORMAT_SPRITE2D (SUS_RENDERER_TEXTURE_FORMAT) { .mipmap = FALSE, .smoothing = SUS_RENDERER_TEXTURE_SMOOTHING_NONE, .wrapX = SUS_RENDERER_TEXTURE_WRAP_CLAMP, .wrapY = SUS_RENDERER_TEXTURE_WRAP_CLAMP }
// Texture template - 2D Texture
#define SUS_RENDERER_TEXTURE_FORMAT_TEX2D (SUS_RENDERER_TEXTURE_FORMAT) { .mipmap = FALSE, .smoothing = SUS_RENDERER_TEXTURE_SMOOTHING_NONE, .wrapX = SUS_RENDERER_TEXTURE_WRAP_REPEAT, .wrapY = SUS_RENDERER_TEXTURE_WRAP_REPEAT }
// Texture Template - 3D Sprite
#define SUS_RENDERER_TEXTURE_FORMAT_SPRITE3D (SUS_RENDERER_TEXTURE_FORMAT) { .mipmap = TRUE, .smoothing = SUS_RENDERER_TEXTURE_SMOOTHING_LOW, .wrapX = SUS_RENDERER_TEXTURE_WRAP_CLAMP, .wrapY = SUS_RENDERER_TEXTURE_WRAP_CLAMP }
// Texture template - 3D Texture
#define SUS_RENDERER_TEXTURE_FORMAT_TEX3D (SUS_RENDERER_TEXTURE_FORMAT) { .mipmap = TRUE, .smoothing = SUS_RENDERER_TEXTURE_SMOOTHING_LOW, .wrapX = SUS_RENDERER_TEXTURE_WRAP_REPEAT, .wrapY = SUS_RENDERER_TEXTURE_WRAP_REPEAT }

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								   Creating and rendering meshes    							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Vertex
typedef struct sus_vertex {
	SUS_VEC3 pos;
	SUS_VEC4 color;
	SUS_VEC2 uv;
	SUS_VEC3 normal;
} SUS_VERTEX, * SUS_LPVERTEX;
// Index in the array of vertices
typedef sus_uint16_t SUS_INDEX, * SUS_LPINDEX;

// Set the offset for the uv coordinates of the mesh
VOID SUSAPI susVertexUVTransform(_Inout_ SUS_DATAVIEW vertexes, _In_ SUS_VEC2 offset, _In_ SUS_VEC2 scale);

// -----------------------------------------------

// The type of the mesh primitive
typedef enum sus_mesh_primitive_type {
	SUS_MESH_PRIMITIVE_TYPE_TRIANGLES = GL_TRIANGLES,
	SUS_MESH_PRIMITIVE_TYPE_TRIANGLES_STRIP = GL_TRIANGLE_STRIP,
	SUS_MESH_PRIMITIVE_TYPE_TRIANGLES_FAN = GL_TRIANGLE_FAN,
	SUS_MESH_PRIMITIVE_TYPE_LINES = GL_LINES,
	SUS_MESH_PRIMITIVE_TYPE_LINES_STRIP = GL_LINE_STRIP,
	SUS_MESH_PRIMITIVE_TYPE_POINTS = GL_POINTS
} SUS_MESH_PRIMITIVE_TYPE, * SUS_LPMESH_PRIMITIVE_TYPE;
// Mesh type by refresh rate
typedef enum sus_mesh_update_type {
	SUS_MESH_UPDATE_TYPE_STATIC = GL_STATIC_DRAW,
	SUS_MESH_UPDATE_TYPE_DYNAMIC = GL_DYNAMIC_DRAW,
	SUS_MESH_UPDATE_TYPE_STREAM = GL_STREAM_DRAW
} SUS_MESH_UPDATE_TYPE, * SUS_LPMESH_UPDATE_TYPE;
// mesh geometry
typedef struct sus_mesh_geometry {
	SUS_DATAVIEW			vertexes;		// An array of vertices
	SUS_DATAVIEW			indexes;		// Array of indexes
} SUS_MESH_GEOMETRY;
// The Mesh Builder
typedef struct sus_mesh_builder {
	SUS_MESH_GEOMETRY		geometry;		// Mesh geometry
	SUS_MESH_PRIMITIVE_TYPE	primitiveType;	// Type of primitive
	SUS_MESH_UPDATE_TYPE	updateType;		// The type of mesh update in drawing
} SUS_MESH_BUILDER, * SUS_LPMESH_BUILDER;
// Mesh structure
typedef struct sus_mesh SUS_MESH_STRUCT, * SUS_MESH;

// -----------------------------------------------

// Build a mesh
SUS_MESH SUSAPI susRendererBuildMesh(_In_ const SUS_LPMESH_BUILDER builder);
// Clear the cache from the gpu
VOID SUSAPI susRendererMeshDestroy(_Inout_ SUS_MESH mesh);
// Set new vertices
VOID SUSAPI susRendererMeshSetVertices(_In_ SUS_MESH mesh, _In_ SUS_DATAVIEW vertexes);
// Set new vertex indexes
VOID SUSAPI susRendererMeshSetIndexes(_In_ SUS_MESH mesh, _In_ SUS_DATAVIEW indexes);
// Draw mesh
VOID SUSAPI susRendererDrawMesh(_In_ SUS_MESH mesh, _In_ SUS_MAT4 model);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								Creating and rendering mesh instances    						  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// A mesh instance
typedef struct sus_mesh_instance  SUS_MESH_INSTANCE_STRUCT, *SUS_MESH_INSTANCE;

// -----------------------------------------------

// Create an instance for meshes
SUS_MESH_INSTANCE SUSAPI susRendererNewMeshInstance(_In_ SUS_MESH base);
// Delete a mesh instance
VOID SUSAPI susRendererMeshInstanceDestroy(_In_ SUS_MESH_INSTANCE instance);
// Draw mesh instances
VOID SUSAPI susRendererDrawMeshInstance(_In_ SUS_MESH_INSTANCE instance);

// -----------------------------------------------

// Add a new item to the instances\param parameters are passed in order
BOOL SUSAPIV susRendererMeshInstanceAdd(_Inout_ SUS_MESH_INSTANCE instance, ...);
// Delete an object from an instance
BOOL SUSAPIV susRendererMeshInstanceRemove(_Inout_ SUS_MESH_INSTANCE instance, _In_ UINT index);
// apply changes
BOOL SUSAPI susRendererMeshInstanceFlush(_Inout_ SUS_MESH_INSTANCE instance);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   Graphics Resource Manager      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Reserved keys
#define SUS_RENDERER_RESOURCE_MANAGER_RESERVED_KEYS 64
// Type of resource
typedef enum sus_renderer_resource_type {
	SUS_RENDERER_RESOURCE_TYPE_SHADER,
	SUS_RENDERER_RESOURCE_TYPE_TEXTURE,
	SUS_RENDERER_RESOURCE_TYPE_MESH,
	SUS_RENDERER_RESOURCE_TYPE_INSTANCE,
	SUS_RENDERER_RESOURCE_TYPE_COUNT,
	SUS_RENDERER_RESOURCE_TYPE_UNKNOWN
} SUS_RENDERER_RESOURCE_TYPE;
// The descriptor of the graphic resource
typedef union sus_renderer_resource {
	struct {
		SUS_RENDERER_RESOURCE_TYPE	type;
		sus_uint32_t				name;
	};
	sus_uint64_t key;
} SUS_RENDERER_RESOURCE;
// The resource's destructor template
typedef VOID(SUSAPI* SUS_RENDERER_RESOURCE_DESTRUCTOR)(_In_ SUS_OBJECT resource);
// Resource Destructor table
static const SUS_RENDERER_RESOURCE_DESTRUCTOR RendererResourceDestructorTable[SUS_RENDERER_RESOURCE_TYPE_COUNT] = { susRendererShaderDestroy, susRendererTextureDestroy, susRendererMeshDestroy, susRendererMeshInstanceDestroy };
// Renderer Resource Manager
typedef struct sus_renderer_resource_manager {
	SUS_HASHMAP	res[SUS_RENDERER_RESOURCE_TYPE_COUNT];
} SUS_RENDERER_RESOURCE_MANAGER, * SUS_LPRENDERER_RESOURCE_MANAGER;

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Initialize the resource manager
VOID SUSAPI susRendererResourceManagerInit(_Out_ SUS_LPRENDERER_RESOURCE_MANAGER manager);
// Destroy the renderer's resource manager
VOID SUSAPI susRendererResourceManagerCleanup(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager);

// -----------------------------------------------

// Register a resource
VOID SUSAPI susRendererResourceRegister(_Inout_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ SUS_RENDERER_RESOURCE key, _In_ SUS_OBJECT resource);
// Get a resource
SUS_OBJECT SUSAPI susRendererResourceGet(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ SUS_RENDERER_RESOURCE key);
// Delete a resource
VOID SUSAPI susRendererResourceRemove(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ SUS_RENDERER_RESOURCE key);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								    High-level work with graphics      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The structure of the renderer
typedef struct sus_renderer {
	SUS_RENDERER_DIRECT				_PARENT_;
	HWND							hWnd;
	SUS_TIMER						frameTimer;
	SUS_RENDERER_RESOURCE_MANAGER	resources;
} SUS_RENDERER_STRUCT, * SUS_RENDERER;

// -----------------------------------------------

// Initialize the graphics for the window and set current
SUS_RENDERER SUSAPI susRendererSetup(_In_ HWND hWnd);
// Set the renderer as the current one
VOID SUSAPI susRendererSetCurrent(_In_opt_ SUS_RENDERER renderer);
// Remove the graphical context
VOID SUSAPI susRendererCleanup(_In_ SUS_RENDERER renderer);
// Update the context output area
VOID SUSAPI susRendererSize();

// -----------------------------------------------

// Start rendering the frame
VOID SUSAPI susRendererBeginFrame();
// Finish painting the frame
VOID SUSAPI susRendererEndFrame();

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////












////////////////////////////////////////////////////////////////////////////////////////////////////
//										  Graphics context      								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// A single renderer context
typedef struct sus_renderer_context {
	SUS_RENDERER			currentRenderer;
	SUS_RENDERER_SHADER		currentShader;
	SUS_CAMERA				currentCamera;
} SUS_RENDERER_CONTEXT, *SUS_LPRENDERER_CONTEXT;

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

#pragma warning(pop)

#endif /* !_SUS_RENDERER_ */
