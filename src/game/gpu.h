// migrated from FSNN, change C99 -> C++
#pragma once
//#include "macros.h"
#include "common.h"
#define GPU_MODULE_STATE_SIZE	128

//C_API_BEGIN()

struct ShaderStorageHeader;

typedef struct { unsigned int handle; } gpuhandle_t;

struct Shader {
	gpuhandle_t id;
};

enum VertexLayout {
	VL_Pos,
	VL_PosUV,
    VL_PosCol,
    VL_PosUVCol,
    VL_PosNormUV
};

enum TextureFormat {
    TF_R,
    TF_RGBA,

    TF_Count
};

enum TextureType {
    TT_2D,
    TT_Buffer,

    TT_Count
};

enum ByteFormat {
    BF_Uint8,
    BF_Uint16,
    BF_Half,
    BF_Float,

    BF_Count
};

enum DrawMode {
	DRW_Tris,
	DRW_Lines,
	DRW_LineStrip,
	DRW_Points
};

enum DepthMode {
    DEP_Less,
    DEP_LessOrEqual,
    DEP_Greater,
    DEP_GreaterOrEqual,
    DEP_NotEqual,
    DEP_Equal,
    DEP_Never,
    DEP_Always,

    DEP_Count
};

/* :THINK:
 * I want to remove per-mesh VAOs and consolidate the mesh
 * into a thing made up of lower-level pieces and also not point
 * to any 'client-side' memory that was used in the creation of the mesh.
 * This can help unify how buffer objects are used and created.
 *
 * An example of a direct translation into using buffers directly:
 *
 * struct Mesh {
 *  VertexLayout layout;
 *  GPUBuffer vertex_buf;
 *  GPUBuffer index_buf;
 * };
 *
 * Or a more 'data-oriented' sort of thing where the buffer is non-owned
 * and the mesh is simply an index into the vertex buffer.
 *
 * struct Mesh {
 *   VertexLayout layout;
 *   GPUBuffer *vertex_buf;
 *   GPUBuffer *index_buf;
 *   uint32_t base_index;
 *   uint32_t element_count;
 * }
 *
 * Of course, the layout could become part of the pipeline
 * (for better portability to newer APIs like Vulkan),
 * and still be part of the mesh for pure sanity-checking reasons.
 */
struct Mesh {
	float 	  		 *verts;
    size_t			  verts_count; // NOTE: This may be incorrect, it's used as both vert count and buffer size in different places!!
	uint16_t  		 *indices;
	size_t			  indices_count;
	enum VertexLayout layout;

	char impl[4 * sizeof(unsigned int)];
};

enum BufferType {
    BT_Uniform,
    BT_Texture,

    BT_Count
};

struct GPUBuffer {
    gpuhandle_t id;
    enum BufferType type;
    size_t size;
};

struct Texture {
    gpuhandle_t id;
    struct GPUBuffer *buffer;

    int width;
    int height;

    enum TextureFormat format;
    enum ByteFormat byte_format;
    enum TextureType type;
};

struct Framebuffer {
    gpuhandle_t id;
    struct Texture *col_buffer;
};

struct Pipeline {
    struct Shader shader;
    struct GPUBuffer *uniforms[1];
    struct Texture *textures[8];
    struct Framebuffer *framebuffer;
	enum DrawMode draw_mode;
    enum DepthMode depth_mode;
    bool wireframe;
    bool alpha_blending;
};

void gpu_loaded(void *mem, void *(*loader)(const char *));
void gpu_init(void);
void gpu_quit(void);
void gpu_viewport_set(float x, float y, float w, float h);

void gpu_clear(float r, float g, float b, float a);

void gpu_pipeline_set(struct Pipeline *pipeline);

void gpu_compile_shaders(struct Shader *shaders, const struct ShaderStorageHeader *src);

void gpu_buffer_add(struct GPUBuffer *buf, void *data);
void gpu_buffer_remove(struct GPUBuffer *buf);
void gpu_buffer_update(struct GPUBuffer *buf, void *data);

void gpu_texture_add(struct Texture *tex, void *data);
void gpu_texture_remove(struct Texture *tex);

void gpu_framebuffer_add(struct Framebuffer *buf);
void gpu_framebuffer_remove(struct Framebuffer *buf);

void gpu_mesh_add(struct Mesh *mesh);
void gpu_mesh_remove(struct Mesh *mesh);
void gpu_mesh_draw(struct Mesh *mesh);

//C_API_END()
