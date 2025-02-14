/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#ifndef ULTRALIGHT_CAPI_GPUDRIVER_H
#define ULTRALIGHT_CAPI_GPUDRIVER_H

#include <Ultralight/CAPI/CAPI_Defines.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * GPUDriver
 *****************************************************************************/

///
/// Render buffer description.
///
typedef struct {
  unsigned int texture_id; // The backing texture for this RenderBuffer
  unsigned int width;      // The width of the RenderBuffer texture
  unsigned int height;     // The height of the RenderBuffer texture
  bool has_stencil_buffer; // Currently unused, always false.
  bool has_depth_buffer;   // Currently unsued, always false.
} ULRenderBuffer;

///
/// @note  This pragma pack(push, 1) command is important! Vertex layouts should not be padded with
///        any bytes.
///
#pragma pack(push, 1)

///
/// Vertex layout for path vertices.
///
/// (this struct's members aligned on single-byte boundaries)
///
typedef struct {
  float pos[2];
  unsigned char color[4];
  float obj[2];
} ULVertex_2f_4ub_2f;

///
/// Vertex layout for quad vertices.
///
/// (this struct's members aligned on single-byte boundaries)
///
typedef struct {
  float pos[2];
  unsigned char color[4];
  float tex[2];
  float obj[2];
  float data0[4];
  float data1[4];
  float data2[4];
  float data3[4];
  float data4[4];
  float data5[4];
  float data6[4];
} ULVertex_2f_4ub_2f_2f_28f;

///
/// End single-byte alignment.
///
#pragma pack(pop)

///
/// Vertex formats.
///
typedef enum {
  kVertexBufferFormat_2f_4ub_2f,
  kVertexBufferFormat_2f_4ub_2f_2f_28f,
} ULVertexBufferFormat;

///
/// Vertex buffer data.
///
typedef struct {
  ULVertexBufferFormat format;
  unsigned int size;
  unsigned char* data;
} ULVertexBuffer;

///
/// Vertex index type.
///
typedef unsigned int ULIndexType;

///
/// Vertex index buffer data.
///
typedef struct {
  unsigned int size;
  unsigned char* data;
} ULIndexBuffer;

///
/// Shader types, used with ULGPUState::shader_type
///
/// Each of these correspond to a vertex/pixel shader pair. You can find stock shader code for these
/// in the `shaders` folder of the AppCore repo.
///
typedef enum {
  kShaderType_Fill,     // Shader program for quad geometry
  kShaderType_FillPath, // Shader program for path geometry
} ULShaderType;

///
/// Raw 4x4 matrix as an array of floats
///
typedef struct {
  float data[16];
} ULMatrix4x4;

///
/// 4-component float vector
///
typedef struct {
  float value[4];
} ULvec4;

///
/// GPU State description.
///
typedef struct {
  /// Viewport width in pixels
  unsigned int viewport_width;

  /// Viewport height in pixels
  unsigned int viewport_height;

  /// Transform matrix-- you should multiply this with the screen-space orthographic projection
  /// matrix then pass to the vertex shader.
  ULMatrix4x4 transform;

  /// Whether or not we should enable texturing for the current draw command.
  bool enable_texturing;

  /// Whether or not we should enable blending for the current draw command. If blending is
  /// disabled, any drawn pixels should overwrite existing. Mainly used so we can modify alpha
  /// values of the RenderBuffer during scissored clears.
  bool enable_blend;

  /// The vertex/pixel shader program pair to use for the current draw command. You should cast this
  /// to ShaderType to get the corresponding enum.
  unsigned char shader_type;

  /// The render buffer to use for the current draw command.
  unsigned int render_buffer_id;

  /// The texture id to bind to slot #1. (Will be 0 if none)
  unsigned int texture_1_id;

  /// The texture id to bind to slot #2. (Will be 0 if none)
  unsigned int texture_2_id;

  /// The texture id to bind to slot #3. (Will be 0 if none)
  unsigned int texture_3_id;

  /// The following four members are passed to the pixel shader via uniforms.
  float uniform_scalar[8];
  ULvec4 uniform_vector[8];
  unsigned char clip_size;
  ULMatrix4x4 clip[8];

  /// Whether or not scissor testing should be used for the current draw
  /// command.
  bool enable_scissor;

  /// The scissor rect to use for scissor testing (units in pixels)
  ULIntRect scissor_rect;
} ULGPUState;

///
/// Command types, used with ULCommand::command_type
///
typedef enum {
  kCommandType_ClearRenderBuffer,
  kCommandType_DrawGeometry,
} ULCommandType;

///
/// Command description.
///
typedef struct {
  unsigned char command_type; // The type of command to dispatch.
  ULGPUState gpu_state;       // GPU state parameters for current command.

  /// The following members are only used with kCommandType_DrawGeometry
  unsigned int geometry_id;    // The geometry ID to bind
  unsigned int indices_count;  // The number of indices
  unsigned int indices_offset; // The index to start from
} ULCommand;

///
/// Command list, @see ULGPUDriverUpdateCommandList
typedef struct {
  unsigned int size;
  ULCommand* commands;
} ULCommandList;

///
/// The callback invoked when the GPUDriver will begin dispatching commands (such as CreateTexture
/// and UpdateCommandList) during the current call to ulRender().
///
typedef void (*ULGPUDriverBeginSynchronizeCallback)();

///
/// The callback invoked when the GPUDriver has finished dispatching commands during the current
/// call to ulRender().
///
typedef void (*ULGPUDriverEndSynchronizeCallback)();

///
/// The callback invoked when the GPUDriver wants to get the next available texture ID.
///
typedef unsigned int (*ULGPUDriverNextTextureIdCallback)();

///
/// The callback invoked when the GPUDriver wants to create a texture with a
/// certain ID and optional bitmap.
///
/// @note: If the Bitmap is empty (ulBitmapIsEmpty), then a RTT Texture should be created
///        instead. This will be used as a backing texture for a new RenderBuffer.
///
typedef void (*ULGPUDriverCreateTextureCallback)(unsigned int texture_id, ULBitmap bitmap);

///
/// The callback invoked when the GPUDriver wants to update an existing non-RTT texture with new
/// bitmap data.
///
typedef void (*ULGPUDriverUpdateTextureCallback)(unsigned int texture_id, ULBitmap bitmap);

///
/// The callback invoked when the GPUDriver wants to destroy a texture.
///
typedef void (*ULGPUDriverDestroyTextureCallback)(unsigned int texture_id);

///
/// The callback invoked when the GPUDriver wants to generate the next available render buffer ID.
///
typedef unsigned int (*ULGPUDriverNextRenderBufferIdCallback)();

///
/// The callback invoked when the GPUDriver wants to create a render buffer with certain ID and
/// buffer description.
///
typedef void (*ULGPUDriverCreateRenderBufferCallback)(unsigned int render_buffer_id,
                                                      ULRenderBuffer buffer);

///
/// The callback invoked when the GPUDriver wants to destroy a render buffer
///
typedef void (*ULGPUDriverDestroyRenderBufferCallback)(unsigned int render_buffer_id);

///
/// The callback invoked when the GPUDriver wants to generate the next available geometry ID.
///
typedef unsigned int (*ULGPUDriverNextGeometryIdCallback)();

///
/// The callback invoked when the GPUDriver wants to create geometry with certain ID and
/// vertex/index data.
///
typedef void (*ULGPUDriverCreateGeometryCallback)(unsigned int geometry_id, ULVertexBuffer vertices,
                                                  ULIndexBuffer indices);

///
/// The callback invoked when the GPUDriver wants to update existing geometry with new vertex/index
/// data.
///
typedef void (*ULGPUDriverUpdateGeometryCallback)(unsigned int geometry_id, ULVertexBuffer vertices,
                                                  ULIndexBuffer indices);

///
/// The callback invoked when the GPUDriver wants to destroy geometry.
///
typedef void (*ULGPUDriverDestroyGeometryCallback)(unsigned int geometry_id);

///
/// The callback invoked when the GPUDriver wants to update the command list (you should copy the
/// commands to your own structure).
///
typedef void (*ULGPUDriverUpdateCommandListCallback)(ULCommandList list);

typedef struct {
  ULGPUDriverBeginSynchronizeCallback begin_synchronize;
  ULGPUDriverEndSynchronizeCallback end_synchronize;
  ULGPUDriverNextTextureIdCallback next_texture_id;
  ULGPUDriverCreateTextureCallback create_texture;
  ULGPUDriverUpdateTextureCallback update_texture;
  ULGPUDriverDestroyTextureCallback destroy_texture;
  ULGPUDriverNextRenderBufferIdCallback next_render_buffer_id;
  ULGPUDriverCreateRenderBufferCallback create_render_buffer;
  ULGPUDriverDestroyRenderBufferCallback destroy_render_buffer;
  ULGPUDriverNextGeometryIdCallback next_geometry_id;
  ULGPUDriverCreateGeometryCallback create_geometry;
  ULGPUDriverUpdateGeometryCallback update_geometry;
  ULGPUDriverDestroyGeometryCallback destroy_geometry;
  ULGPUDriverUpdateCommandListCallback update_command_list;
} ULGPUDriver;

///
/// Sets up an orthographic projection matrix with a certain viewport width and height, multiplies
/// it by 'transform', and returns the result.
///
/// This should be used to calculate the model-view projection matrix for the vertex shaders using
/// the current ULGPUState.
///
/// The 'flip_y' can be optionally used to flip the Y coordinate-space. (Usually flip_y == true for
/// OpenGL)
///
ULExport ULMatrix4x4 ulApplyProjection(ULMatrix4x4 transform, float viewport_width,
                                       float viewport_height, bool flip_y);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ULTRALIGHT_CAPI_GPUDRIVER_H