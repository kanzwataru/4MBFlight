// migrated from FSNN, change C99 -> C++
#include "gpu.h"
#include "shaders/shader_toc.h" // THINK: Should the gpu module really be dependant on hard-coded stuff?
#include <glad/glad.h>

// TODO: Make proper logging and asserts
#include <assert.h>
#include <stdio.h>
#include <stdlib.h> // for exit

struct GPUMesh {
	bool added;
	GLuint vao;
	GLuint vbo;
	GLuint ebo;
};

struct GPUContext {
	struct Pipeline pipeline;
};

static_assert(sizeof(struct GPUMesh) <= STATIC_SIZEOF(struct Mesh, impl), "");
static_assert(sizeof(struct GPUContext) <= GPU_MODULE_STATE_SIZE, "");

static struct GPUContext *gpu;

static inline gpuhandle_t handle_make(GLuint gl_handle) {
	// NOTE: This assumes a valid OpenGL handle!
    return (gpuhandle_t){ gl_handle + 1 };
}

static inline gpuhandle_t handle_make_empty() {
    return (gpuhandle_t){0};
}

static inline bool handle_valid(gpuhandle_t handle) {
    return handle.handle;
}

static inline GLuint handle_get(gpuhandle_t handle) {
    assert(handle_valid(handle));
    return handle.handle - 1;
}

/* Shader compilation
 * TODO: Use glslang or something to validate shaders during compile-time
*/
static void check_shader_compilation(GLuint shader, const char *shader_src)
{
	GLint status;
	char  msg_buf[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE) {
        glGetShaderInfoLog(shader, sizeof(msg_buf), NULL, msg_buf);
        fprintf(stderr, "*** SHADER COMPILATION FAILED ***\n, %s %s\n\n", shader_src, msg_buf);
		exit(1);
	}
}

static void check_program_compilation(GLuint program)
{
	GLint status;
	char  msg_buf[512];

	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status != GL_TRUE) {
        glGetProgramInfoLog(program, sizeof(msg_buf), NULL, msg_buf);
        fprintf(stderr, "*** PROGRAM COMPILATION FAILED ***\n, %s\n\n", msg_buf);
		exit(1);
	}
}

static void compile_shader(struct Shader *shader, const char *stages[SS_Total])
{
	const GLenum stage_table[SS_Total] = {
		[SS_Fragment] = GL_FRAGMENT_SHADER,
		[SS_Vertex] = GL_VERTEX_SHADER
	};

	GLuint shader_handles[SS_Total];
	int shader_count = 0;

	for(int i = 0; i < SS_Total; ++i) {
		const char *src = stages[i];
		if(!src)
			continue;

        //printf("------\n%s\n", src);

		GLuint id = glCreateShader(stage_table[i]);
		glShaderSource(id, 1, &stages[i], NULL);
		glCompileShader(id);
        check_shader_compilation(id, src);

		shader_handles[shader_count++] = id;
	}

	const GLuint program = glCreateProgram();
	for(int i = 0; i < shader_count; ++i) {
		glAttachShader(program, shader_handles[i]);
	}

	glLinkProgram(program);
    check_program_compilation(program);

	for(int i = 0; i < shader_count; ++i) {
		glDeleteShader(shader_handles[i]);
	}

	shader->id = handle_make(program);

    /* --- set universal uniforms ---
     *
     * Ah, if only we could use OpenGL 4.3 and have
     * layout binding numbers in the shaders!
    */
    GLuint loc;
	glUseProgram(program);

    // uniform buffers
    // TODO: Support multiple uniform buffer
    loc = glGetUniformBlockIndex(program, "UniformBuffer");
	if(loc != GL_INVALID_INDEX) {
		glUniformBlockBinding(program, loc, 0);
	}

    // textures
    char sampler_name[128];
    const size_t texture_count = STATIC_COUNTOF(struct Pipeline, textures);
    for(size_t i = 0; i < texture_count; ++i) {
        snprintf(sampler_name, countof(sampler_name), "texture%zu", i);

        loc = glGetUniformLocation(program, sampler_name);
        if(loc != GL_INVALID_INDEX) {
            glUniform1i(loc, i);
        }
    }
}

void gpu_compile_shaders(struct Shader *shaders, const struct ShaderStorageHeader *src)
{
	// NOTE: Using shader_toc.h but ignoring hard-coded stuff, for future-proofness
	for(uint32_t i = 0; i < src->shader_entry_count; ++i) {
		const struct ShaderEntry *entry = &src->toc[i];

		const char *stages[SS_Total] = {};
		for(int j = 0; j < SS_Total; ++j) {
			if(entry->stages[j].size == 0)
				continue;

			stages[j] = (const char *)src + entry->stages[j].offset;
		}

		compile_shader(&shaders[i], stages);
	}
}

void gpu_loaded(void *memory, void *(*loader)(const char *))
{
	gpu = (GPUContext *)memory;
	gladLoadGLLoader(loader);
}

void gpu_init(void)
{
    glLineWidth(2.0f);
    glPointSize(4.0f);
}

void gpu_quit(void)
{
    // unused
}

void gpu_viewport_set(float x, float y, float w, float h)
{
    glViewport(x, y, w, h);
}

void gpu_clear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

static const GLenum texture_binding_table[TT_Count] = {
    /*[TT_2D] =*/ GL_TEXTURE_2D,
    /*[TT_Buffer] =*/ GL_TEXTURE_BUFFER
};

static const GLenum depth_func_table[DEP_Count] = {
    /*[DEP_Less] =*/ GL_LESS,
    /*[DEP_LessOrEqual] =*/ GL_LEQUAL,
    /*[DEP_Greater] =*/ GL_GREATER,
    /*[DEP_GreaterOrEqual] =*/ GL_GEQUAL,
    /*[DEP_NotEqual] =*/ GL_NOTEQUAL,
    /*[DEP_Equal] =*/ GL_EQUAL,
    /*[DEP_Never] =*/ GL_NEVER,
    /*[DEP_Always] =*/ GL_ALWAYS
};

void gpu_pipeline_set(struct Pipeline *pipeline)
{
	// TODO PERF: only apply changes to pipeline
	gpu->pipeline = *pipeline;

	assert(handle_valid(pipeline->shader.id));
	glUseProgram(handle_get(pipeline->shader.id));

    if(pipeline->uniforms[0]) {
        assert(pipeline->uniforms[0]->type == BT_Uniform);
        assert(handle_valid(pipeline->uniforms[0]->id));

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, handle_get(pipeline->uniforms[0]->id), 0, pipeline->uniforms[0]->size);
    }

    if(pipeline->wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    for(size_t i = 0; i < countof(pipeline->textures); ++i) {
        struct Texture *tex = pipeline->textures[i];

        if(tex) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(texture_binding_table[tex->type], handle_get(tex->id));
        }
    }

    if(pipeline->alpha_blending) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else {
        glDisable(GL_BLEND);
    }

    assert(pipeline->depth_mode < countof(depth_func_table));
    glDepthFunc(depth_func_table[pipeline->depth_mode]);

    if(pipeline->framebuffer) {
        assert(handle_valid(pipeline->framebuffer->id));
        glBindFramebuffer(GL_FRAMEBUFFER, handle_get(pipeline->framebuffer->id));
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

static const GLenum buffer_type_table[BT_Count] = {
    /*[BT_Uniform] =*/ GL_UNIFORM_BUFFER,
    /*[BT_Texture] =*/ GL_TEXTURE_BUFFER
};

void gpu_buffer_add(struct GPUBuffer *buf, void *data)
{
    assert(!handle_valid(buf->id));

    GLuint id;
    glGenBuffers(1, &id);
    buf->id = handle_make(id);

    const GLenum binding = buffer_type_table[buf->type];
    glBindBuffer(binding, id);

    // NOTE: if data pointer is NULL, it just allocates an uninitialized buffer so it's OK
    glBufferData(binding, buf->size, data, GL_STATIC_DRAW);
}

void gpu_buffer_remove(struct GPUBuffer *buf)
{
    assert(handle_valid(buf->id));
    GLuint id = handle_get(buf->id);

    glDeleteBuffers(1, &id);
}

void gpu_buffer_update(struct GPUBuffer *buf, void *data)
{
    assert(handle_valid(buf->id));
    assert(data);
    GLuint id = handle_get(buf->id);

    const GLenum binding = buffer_type_table[buf->type];
    glBindBuffer(binding, id);

    /* NOTE PERF:
     * This potentially re-allocates memory,
     * it would apparently be better to use glBufferSubData */
    // NOTE PERF: Currently assuming GL_STATIC_DRAW, but maybe should expose this hint better?
    glBufferData(binding, buf->size, data, GL_STATIC_DRAW);
}

void gpu_mesh_add(struct Mesh *mesh)
{
	struct GPUMesh *impl = (struct GPUMesh *)mesh->impl;
	assert(!impl->added);
	assert(mesh->verts_count > 0);

	glGenVertexArrays(1, &impl->vao);
    glBindVertexArray(impl->vao);

    glGenBuffers(1, &impl->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, impl->vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh->verts_count * sizeof(float), mesh->verts, GL_STATIC_DRAW);

    if(mesh->indices_count > 0) {
        glGenBuffers(1, &impl->ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, impl->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(uint16_t), mesh->indices, GL_STATIC_DRAW);
    }

	switch(mesh->layout) {
	case VL_Pos:
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		break;
    case VL_PosUV: glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        break;
    case VL_PosCol:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        break;
    case VL_PosUVCol:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        break;
    default:
        assert(0);
	}

	glBindVertexArray(0);
	impl->added = true;
}

void gpu_texture_add(struct Texture *tex, void *data)
{
    assert(!handle_valid(tex->id));
    assert(tex->width > 0);

    // Untested (except for BF_Uint8)
    static const GLenum byte_format_table[BF_Count] = {
        /*[BF_Uint8]  =*/ GL_UNSIGNED_BYTE,
        /*[BF_Uint16] =*/ GL_UNSIGNED_SHORT,
        /*[BF_Half]   =*/ GL_HALF_FLOAT,
        /*[BF_Float]  =*/ GL_FLOAT
    };

    static const GLenum internal_format_table[TF_Count][BF_Count] = {
        /*[TF_R] = */{
            [BF_Uint8] = GL_R8,
            [BF_Uint16] = GL_R16UI,     // untested
            [BF_Half] = GL_R16F,        // untested
            [BF_Float] = GL_R32F        // untested
        },
        /*[TF_RGBA] = */{
            [BF_Uint8] = GL_RGBA8,
            [BF_Uint16] = GL_RGBA16UI,  // untested
            [BF_Half] = GL_RGBA16F,     // untested
            [BF_Float] = GL_RGBA32F
        }
    };

    static const GLenum format_table[TF_Count] = {
        /*[TF_R] = */GL_RED,
        /*[TF_RGBA] = */GL_RGBA
    };

    GLuint id;
    glGenTextures(1, &id);

    if(tex->type == TT_2D) {
        assert(!tex->buffer);
        //assert(data); // making a blank texture should have a better API I think
        assert(tex->height > 0);

        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format_table[tex->format][tex->byte_format],
                     tex->width, tex->height, 0,
                     format_table[tex->format], byte_format_table[tex->byte_format], data);

        // TODO: Support different texture filtering modes?
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // TODO: Support different texture clamping modes?
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else {
        assert(tex->buffer);
        assert(!data);
        assert(handle_valid(tex->buffer->id));

        // TODO: Test and support different buffer texture format types
        assert(tex->format == TF_RGBA);
        assert(tex->byte_format == BF_Float);

        glBindTexture(GL_TEXTURE_BUFFER, id);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, handle_get(tex->buffer->id));
    }

    tex->id = handle_make(id);
}

// untested
void gpu_texture_remove(struct Texture *tex)
{
    assert(handle_valid(tex->id));

    GLuint id = handle_get(tex->id);
    glDeleteTextures(1, &id);

    tex->id = handle_make_empty();
}

void gpu_framebuffer_add(struct Framebuffer *buf)
{
    assert(!handle_valid(buf->id));
    assert(buf->col_buffer);
    assert(handle_valid(buf->col_buffer->id));

    GLuint id;
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, handle_get(buf->col_buffer->id), 0);

    buf->id = handle_make(id);
}

void gpu_framebuffer_remove(struct Framebuffer *buf)
{
    assert(handle_valid(buf->id));

    GLuint id = handle_get(buf->id);
    glDeleteFramebuffers(1, &id);

    buf->id = handle_make_empty();
}

void gpu_mesh_remove(struct Mesh *mesh)
{
	struct GPUMesh *impl = (struct GPUMesh *)mesh->impl;
	assert(impl->added);

	glBindVertexArray(0);
	glDeleteBuffers(1, &impl->vbo);
	glDeleteBuffers(1, &impl->ebo);
	glDeleteVertexArrays(1, &impl->vao);

	struct GPUMesh empty_impl = {};
	*impl = empty_impl;
}

void gpu_mesh_draw(struct Mesh *mesh)
{
	struct GPUMesh *impl = (struct GPUMesh *)mesh->impl;
	struct Pipeline *pipeline = &gpu->pipeline;
	assert(impl->added);

	static const GLenum gl_draw_mode[] = {
        /*[DRW_Tris] = */GL_TRIANGLES,
        /*[DRW_Lines] = */GL_LINES,
        /*[DRW_LineStrip] = */GL_LINE_STRIP,
        /*[DRW_Points] = */GL_POINTS
	};

	assert(pipeline->draw_mode < countof(gl_draw_mode));

	glBindVertexArray(impl->vao);
    if(mesh->indices_count > 0) {
        glDrawElements(gl_draw_mode[pipeline->draw_mode], (GLsizei)mesh->indices_count, GL_UNSIGNED_SHORT, 0);
    }
    else {
        glDrawArrays(gl_draw_mode[pipeline->draw_mode], 0, mesh->verts_count);
    }
}
