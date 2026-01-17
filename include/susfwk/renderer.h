// renderer.h
//
#ifndef _SUS_RENDERER_
#define _SUS_RENDERER_

// Renderer - graphics output
// v1.1
/*
* Renderer is an api for working with high-level graphics,
* providing functions for working with shaders, cameras, meshes, textures, etc.
* Sample code at the end of the file
* 
* Use the window module for convenient operation.
* Using the window module, some of the work will be done automatically!
*/

// -----------------------------------------------

#include "tmath.h"
#include "time.h"
#include "uicommon.h"
#include "vector.h"
#include "hashtable.h"
#include "..\susgl\suswgl.h"

// -----------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4201)

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

// How do I work with the camera?
/*
* To start working with the camera, you need to:
* 1. Create a camera, 2d for orthographic projection, 3d for perspective projection
* 2. Set the camera as the current one
* 3. You can start working
* 
* Do not call the susRendererCameraSize and susRendererCameraUpdate functions directly!
* The camera is controlled automatically, so you don't need to remove it unnecessarily.
*/

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

// Set the camera position
VOID SUSAPI susRendererCameraSetPosition(_Inout_ SUS_CAMERA camera, _In_ SUS_VEC3 pos);
// Move the camera
SUS_INLINE VOID SUSAPI susRendererCameraMove(_Inout_ SUS_CAMERA camera, _In_ SUS_VEC3 move) { susRendererCameraSetPosition(camera, susVec3Sum(camera->position, move)); }
// Set the camera rotation
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

// What is a shader?
/*
* A shader in the susfwk context is a GPU program that you can load or use templates for
* 
* It is important to note that a shader
* is one of the key components of graphics, and in susfwk,
* you are required to use and understand it.
* 
* Many components require a shader,
* and some objects require
* a specific shader (or a general shader) when initialized
*/

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Basic 3D Shader Code - Vertex Shader
static const CHAR* SUSBuiltinShaderVS =
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
// Basic 3D Shader Code - Fragment Shader
static const CHAR* SUSBuiltinShaderFS =
"#version 330 core\n"
"in vec4 vColor;\n"
"in vec2 vTexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D uTexture;\n"
"void main() {\n"
"    vec4 texColor = texture(uTexture, vTexCoord);\n"
"    FragColor = vColor * texColor;\n"
"}\n";

// -----------------------------------------------

// 3D vertex shader for instancing
static const CHAR* SUSBuiltinInstancedShaderVS =
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

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Shader Module
typedef struct sus_renderer_shader_module { GLuint shader; } SUS_RENDERER_SHADER_MODULE, * SUS_LPRENDERER_SHADER_MODULE;
// Load a shader from a file
SUS_RENDERER_SHADER_MODULE SUSAPI susRendererLoadShaderModule(_In_ LPCSTR path);

// -----------------------------------------------

// Global Shader uniforms
typedef enum sus_renderer_base_uniforms {
	SUS_RENDERER_BASE_UNIFORM_MVP,		// sus_mvp -> mat4
	SUS_RENDERER_BASE_UNIFORM_TIME,		// sus_time -> float
	SUS_RENDERER_BASE_UNIFORM_COSTIME,	// sus_ctime -> float
	SUS_RENDERER_BASE_UNIFORM_SINTIME,	// sus_stime -> float
	SUS_RENDERER_BASE_UNIFORM_DELTA,	// sus_delta -> float
	SUS_RENDERER_BASE_UNIFORM_PROJVIEW,	// sus_projview -> mat4
	SUS_RENDERER_BASE_UNIFORM_TEXTURE,	// sus_texture -> sampler2D
	SUS_RENDERER_BASE_UNIFORM_COUNT
} SUS_RENDERER_BASE_UNIFORM;
// High-level shader structure
typedef struct sus_renderer_shader {
	GLuint				program;									// Shader program
	GLuint				uniforms[SUS_RENDERER_BASE_UNIFORM_COUNT];	// SUS_RENDERER_BASE_UNIFORMS
} SUS_RENDERER_SHADER_STRUCT, *SUS_RENDERER_SHADER;

// Load the shader as the current one
SUS_RENDERER_SHADER SUSAPI susRendererNewShader(_In_ UINT count, _In_ SUS_LPRENDERER_SHADER_MODULE modules);
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

// How to work with textures?
/*
* To load a texture, write - susRendererLoadTexture with the following parameters
* To load it into the GPU slot, call susRendererTextureBind
* To use the renderer, call susRendererSetTexture
* Quick Start - susprenderertextureactive
* 
* Textures are managed automatically, and you don't need to release them manually
*/

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The type of texture stretching
typedef enum sus_renderer_texture_wrap {
	SUS_TEXTURE_WRAP_REPEAT,	// Duplicate the texture
	SUS_TEXTURE_WRAP_MIRROR,	// Repeat with reflection
	SUS_TEXTURE_WRAP_CLAMP		// Stretch the texture
} SUS_TEXTURE_WRAP;
// Smoothing level
typedef enum sus_renderer_texture_smoothing {
	SUS_TEXTURE_SMOOTHING_NONE,
	SUS_TEXTURE_SMOOTHING_LOW,
	SUS_TEXTURE_SMOOTHING_MEDIUM,
	SUS_TEXTURE_SMOOTHING_HIGH,
} SUS_TEXTURE_SMOOTHING;
// Texture format
typedef struct sus_texture_format {
	SUS_TEXTURE_WRAP		wrapX, wrapY;	// The type of texture stretching
	SUS_TEXTURE_SMOOTHING	smoothing;		// Smoothing level
	BOOL					mipmap;			// Create mipmap for a texture in 3D
} SUS_TEXTURE_FORMAT;
// Texture Builder
typedef struct sus_texture_builder {
	UINT						channels;	// Number of channels per pixel
	SUS_SIZE					size;		// Image Size
	SUS_LPMEMORY				data;		// Image Data
	SUS_TEXTURE_FORMAT format;		// Image Format
} SUS_TEXTURE_BUILDER, * SUS_LPTEXTURE_BUILDER;
// Texture
typedef struct sus_texture {
	GLuint _PARENT_;	// Texture
	SUS_SIZE size;		// Texture size in pixels
} SUS_TEXTURE_STRUCT, *SUS_TEXTURE;

// Create a 2d texture
SUS_TEXTURE SUSAPI susRendererNewTexture(_In_ const SUS_LPTEXTURE_BUILDER builder);
// Destroy the texture
VOID SUSAPI susRendererTextureDestroy(_In_ SUS_TEXTURE texture);
// Load a texture from a file
SUS_TEXTURE SUSAPI susRendererLoadTexture(_In_ LPCWSTR path, _In_ SUS_TEXTURE_FORMAT format);
// Load an image from resources by type - TEXTURE
SUS_TEXTURE SUSAPI susRendererLoadTextureResource(_In_ LPCSTR resourceName, _In_ SUS_TEXTURE_FORMAT format);
// Load the texture into the GPU slot
VOID SUSAPI susRendererTextureBind(_In_ SUS_TEXTURE texture, _In_ sus_uint_t slot);
// Set the texture as the current one for the slot
VOID SUSAPI susRendererSetTexture(_In_ sus_uint_t slot);
// Load and activate the texture on slot 0
VOID SUSAPI susRendererTextureActive(_In_ SUS_TEXTURE texture);

// -----------------------------------------------

// Texture template - 2D Sprite
#define SUS_TEXTURE_FORMAT_SPRITE2D (SUS_TEXTURE_FORMAT) { .mipmap = FALSE, .smoothing = SUS_TEXTURE_SMOOTHING_NONE, .wrapX = SUS_TEXTURE_WRAP_CLAMP, .wrapY = SUS_TEXTURE_WRAP_CLAMP }
// Texture template - 2D Texture
#define SUS_TEXTURE_FORMAT_TEX2D (SUS_TEXTURE_FORMAT) { .mipmap = FALSE, .smoothing = SUS_TEXTURE_SMOOTHING_NONE, .wrapX = SUS_TEXTURE_WRAP_REPEAT, .wrapY = SUS_TEXTURE_WRAP_REPEAT }
// Texture Template - 3D Sprite
#define SUS_TEXTURE_FORMAT_SPRITE3D (SUS_TEXTURE_FORMAT) { .mipmap = TRUE, .smoothing = SUS_TEXTURE_SMOOTHING_LOW, .wrapX = SUS_TEXTURE_WRAP_CLAMP, .wrapY = SUS_TEXTURE_WRAP_CLAMP }
// Texture template - 3D Texture
#define SUS_TEXTURE_FORMAT_TEX3D (SUS_TEXTURE_FORMAT) { .mipmap = TRUE, .smoothing = SUS_TEXTURE_SMOOTHING_LOW, .wrapX = SUS_TEXTURE_WRAP_REPEAT, .wrapY = SUS_TEXTURE_WRAP_REPEAT }

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
__declspec(align(1)) typedef struct sus_vertex_format {
	SUS_VERTEX_TYPE		type : 16;
	SUS_VERTEX_ATTRIBUT attributes : 16;
} SUS_VERTEX_FORMAT;

// Vertex format template - 2d vertices
#define SUS_VERTEX_FORMAT_2D (SUS_VERTEX_FORMAT) { .type = SUS_VERTEX_TYPE_2D, .attributes = SUS_VERTEX_ATTRIBUT_BASE }
// Vertex format template - 3d vertices
#define SUS_VERTEX_FORMAT_3D (SUS_VERTEX_FORMAT) { .type = SUS_VERTEX_TYPE_3D, .attributes = SUS_VERTEX_ATTRIBUT_FULL }

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
	SUS_MESH_GEOMETRY			geometry;		// Mesh geometry
	SUS_MESH_PRIMITIVE_TYPE		primitiveType;	// Type of primitive
	SUS_MESH_UPDATE_TYPE		updateType;		// The type of mesh update in drawing
	SUS_VERTEX_FORMAT			format;			// Mesh format
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

// The format of the instance variables
typedef enum sus_mesh_instance_attribute {
	SUS_MESH_INSTANCE_ATTRIBUTE_MATRIX		= 0,		// mat4 - default
	SUS_MESH_INSTANCE_ATTRIBUTE_COLOR		= 1 << 0,	// vec4
	SUS_MESH_INSTANCE_ATTRIBUTE_UVOFFSET	= 1 << 1	// vec2
} SUS_MESH_INSTANCE_ATTRIBUTE;
// A mesh instance
typedef struct sus_mesh_instance  SUS_MESH_INSTANCE_STRUCT, *SUS_MESH_INSTANCE;

// -----------------------------------------------

// Create an instance for meshes
SUS_MESH_INSTANCE SUSAPI susRendererNewInstance(_In_ SUS_MESH base, _In_ SUS_MESH_INSTANCE_ATTRIBUTE attributes);
// Delete a mesh instance
VOID SUSAPI susRendererInstanceDestroy(_In_ SUS_MESH_INSTANCE instance);
// Draw mesh instances
VOID SUSAPI susRendererDrawInstance(_In_ SUS_MESH_INSTANCE instance);

// -----------------------------------------------

// Add a new item to the instances\param parameters are passed in order
BOOL SUSAPIV susRendererInstanceAdd(_Inout_ SUS_MESH_INSTANCE instance, ...);
// Delete an object from an instance
BOOL SUSAPIV susRendererInstanceRemove(_Inout_ SUS_MESH_INSTANCE instance, _In_ UINT index);
// apply changes
BOOL SUSAPI susRendererInstanceFlush(_Inout_ SUS_MESH_INSTANCE instance);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//											Primitive meshes    								  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Build a 2d square
SUS_MESH susRendererPrimitivesBuild_Square(_In_ SUS_VEC2 scale);
// Build a 2d circle
SUS_MESH susRendererPrimitivesBuild_Circle(_In_ sus_uint_t segments, _In_ sus_float_t radius);
// Build a 3d cube
SUS_MESH susRendererPrimitivesBuild_Cube(_In_ SUS_VEC3 scale);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//									   Graphics Resource Manager      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

// What is a resource manager?
/*
* The Resource Manager is an important rendering module
* It serves as an automatic RAII for graphics resources
* It has a pool system that is necessary for professional work with resources
* 
* All resources you create are automatically added to the current pool in the resource manager.
* The manager manages the resources itself, so you don't need to release them manually.
* If you have created your own resource pool and no longer need it, simply clean it up using susRendererResourcePoolCleanup
*/

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Reserved keys
#define SUS_RENDERER_RESOURCE_RESERVED_KEYS 64
// Name of the main resource
#define SUS_RENDERER_RESOURCE_MANAGER_MAIN 0
// Type of resource
typedef enum sus_renderer_resource_type {
	SUS_RENDERER_RESOURCE_TYPE_SHADER,
	SUS_RENDERER_RESOURCE_TYPE_CAMERA,
	SUS_RENDERER_RESOURCE_TYPE_TEXTURE,
	SUS_RENDERER_RESOURCE_TYPE_MESH,
	SUS_RENDERER_RESOURCE_TYPE_INSTANCE,
	SUS_RENDERER_RESOURCE_TYPE_COUNT,
	SUS_RENDERER_RESOURCE_TYPE_UNKNOWN
} SUS_RENDERER_RESOURCE_TYPE;
// The resource's destructor template
typedef VOID(SUSAPI* SUS_RENDERER_RESOURCE_DESTRUCTOR)(_In_ SUS_OBJECT resource);
// Resource Destructor table
static const SUS_RENDERER_RESOURCE_DESTRUCTOR RendererResourceDestructorTable[SUS_RENDERER_RESOURCE_TYPE_COUNT] = { susRendererShaderDestroy, susRendererCameraDestroy, susRendererTextureDestroy, susRendererMeshDestroy, susRendererInstanceDestroy };
// Renderer Resource pool
typedef struct sus_renderer_resource_pool {
	UINT id; // Id of pool in manager
	SUS_HASHSET pool[SUS_RENDERER_RESOURCE_TYPE_COUNT];
} SUS_RENDERER_RESOURCE_POOL, * SUS_LPRENDERER_RESOURCE_POOL;
// Renderer Resource Manager
typedef struct sus_renderer_resource_manager {
	SUS_HASHMAP level;		// UINT -> SUS_RENDERER_RESOURCE_POOL
	UINT		current;	// Name of the current resource pool
} SUS_RENDERER_RESOURCE_MANAGER, * SUS_LPRENDERER_RESOURCE_MANAGER;

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Initialize the resource manager
VOID SUSAPI susRendererResourcePoolInit(_Out_ SUS_LPRENDERER_RESOURCE_POOL pool, _In_ UINT id);
// Destroy the renderer's resource manager
VOID SUSAPI susRendererResourcePoolCleanup(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ SUS_LPRENDERER_RESOURCE_POOL pool);

// -----------------------------------------------

// Get a resource
BOOL SUSAPI susRendererResourcePoolContains(_In_ SUS_LPRENDERER_RESOURCE_POOL pool, _In_ SUS_RENDERER_RESOURCE_TYPE type, _In_ SUS_OBJECT resource);
// Register a resource
VOID SUSAPI susRendererResourcePoolRegister(_Inout_ SUS_LPRENDERER_RESOURCE_POOL pool, _In_ SUS_RENDERER_RESOURCE_TYPE type, _In_ SUS_OBJECT resource);
// Delete a resource
VOID SUSAPI susRendererResourcePoolRemove(_In_ SUS_LPRENDERER_RESOURCE_POOL pool, _In_ SUS_RENDERER_RESOURCE_TYPE type, _In_ SUS_OBJECT resource);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// Create a Resource manager
VOID SUSAPI susRendererResourceManagerInit(_Out_ SUS_LPRENDERER_RESOURCE_MANAGER manager);
// Destroy the resource manager and its resources
VOID SUSAPI susRendererResourceManagerCleanup(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager);

// -----------------------------------------------

// Set the resource pool as current
VOID SUSAPI susRendererResourceManagerSetCurrent(_Inout_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ UINT pool);
// Set the resource pool as current
SUS_LPRENDERER_RESOURCE_POOL SUSAPI susRendererResourceManagerCurrent(_Inout_ SUS_LPRENDERER_RESOURCE_MANAGER manager);
// Get a pool of resources
SUS_LPRENDERER_RESOURCE_POOL SUSAPI susRendererResourceManagerGet(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ UINT pool);

// -----------------------------------------------

// Add a new pool to the resource manager
SUS_LPRENDERER_RESOURCE_POOL SUSAPI susRendererResourceManagerAddPool(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ UINT pool);
// Delete a pool from the resource manager
VOID SUSAPI susRendererResourceManagerRemovePool(_In_ SUS_LPRENDERER_RESOURCE_MANAGER manager, _In_ UINT pool);

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//								    High-level work with graphics      							  //
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

// The context of the renderer
typedef struct sus_renderer_context {
	SUS_RENDERER_SHADER		currentShader;
	SUS_CAMERA				currentCamera;
} SUS_RENDERER_CONTEXT;
// The structure of the renderer
typedef struct sus_renderer {
	SUS_RENDERER_DIRECT				_PARENT_;
	HWND							hWnd;
	SUS_TIMER						frameTimer;
	SUS_RENDERER_RESOURCE_MANAGER	resources;
	SUS_RENDERER_CONTEXT			context;
} SUS_RENDERER_STRUCT, * SUS_RENDERER;

// -----------------------------------------------

// Initialize the graphics for the window and set current
SUS_RENDERER SUSAPI susRendererSetup(_In_ HWND hWnd);
// Set optimization for 3D graphics
VOID SUSAPI susRendererSet3D();
// Set the background color
VOID SUSAPI susRendererSetBackground(SUS_VEC3 color);
// Set the renderer as the current one
VOID SUSAPI susRendererSetCurrent(_In_opt_ SUS_RENDERER renderer);
// Remove the graphical context
VOID SUSAPI susRendererCleanup(_In_ SUS_RENDERER renderer);
// Update the context output area
VOID SUSAPI susRendererSize();

// -----------------------------------------------

// Get the current renderer
SUS_RENDERER SUSAPI susGetRenderer();
// Get the current camera
SUS_CAMERA SUSAPI susRendererGetCamera(_In_ SUS_RENDERER renderer);
// Get the current shader
SUS_RENDERER_SHADER SUSAPI susRendererGetShader(_In_ SUS_RENDERER renderer);

// -----------------------------------------------

// Start rendering the frame
VOID SUSAPI susRendererBeginFrame();
// Finish painting the frame
VOID SUSAPI susRendererEndFrame();

// -----------------------------------------------

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
//											  Sample code      									  //
////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------

/* There will be no sample code. -_-
* 
* 
* 
*/

// -----------------------------------------------

#pragma warning(pop)

#endif /* !_SUS_RENDERER_ */
