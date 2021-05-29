#include "platform.h"

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct Platform {
    bool running;
    SDL_Window *window;
    SDL_GLContext gl_context;
};

struct Module {
    void *handle;
    void *mem;
    size_t mem_size;
    ModuleApi api;
};

static void panic(const char *str)
{
	fprintf(stderr, "[CRITICAL ERROR]: %s\nAbout to crash...\n", str);
	fflush(stderr);
    assert(0);
}

static void sdl_assume_ptr(void *ptr)
{
	char *c = static_cast<char*>(ptr);
	if(!c) {
		panic(SDL_GetError());
	}
}

static void APIENTRY debug_gl_callback(GLenum source,
									   GLenum type,
									   GLuint id,
									   GLenum severity,
									   GLsizei length,
									   const GLchar *message,
									   const void *user_param)
{
    (void)source; (void)type; (void)id;
    (void)severity; (void)length; (void)user_param;

	/*
    FILE *outf = severity == GL_DEBUG_SEVERITY_LOW ? stdout : stderr;
    fprintf(outf, "%s\n", message);
	fflush(outf);
	*/
	if(severity == GL_DEBUG_SEVERITY_HIGH) {
		panic(message);
	}
	else {
		printf("[OpenGL] %s\n", message);
	}
}

static void debug_enable_gl_callback(void)
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debug_gl_callback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
}

static void module_load(Module *module)
{
#if WITH_HOTRELOAD
    if(module->handle) {
        SDL_UnloadObject(module->handle);
    }

    module->handle = SDL_LoadObject("./devgame_m.so");
    if(!module->handle) {
        panic(SDL_GetError());
    }

    module_get_api_func_t *module_get_api_p = (module_get_api_func_t *)SDL_LoadFunction(
        module->handle,
        STRINGIFY(MODULE_GET_API_NAME)
    );

    if(!module_get_api_p) {
        panic("Could not load module handshake function");
    }
#else
    module_get_api_func_t *module_get_api_p = MODULE_GET_API_NAME;
#endif
    module_get_api_p(&module->api);

    if(module->mem) {
        if(module->mem_size < module->api.mem_required) {
            panic("Needs more memory than allocated!");
        }
    }
    else {
        module->mem_size = module->api.mem_required;
        module->mem = calloc(module->mem_size, 1);
    }

    module->api.loaded(module->mem);
}

int main(int, char **)
{
    Platform plf = {};
    PlatformApi plf_api = {};
    Module module = {};

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        panic(SDL_GetError());
    }

    module_load(&module);

    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG); // debug output

    plf.window = SDL_CreateWindow("4MB Jam (SDL)",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  800, 600,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    sdl_assume_ptr(plf.window);

    plf.gl_context = SDL_GL_CreateContext(plf.window);
    sdl_assume_ptr(plf.gl_context);

    gladLoadGLLoader(SDL_GL_GetProcAddress);
	debug_enable_gl_callback();

    SDL_GL_SetSwapInterval(1);

    plf.running = true;
    while(plf.running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT:
                plf.running = false;
                break;
            }
        }

        glClearColor(128, 128, 128, 255);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(plf.window);

        fflush(stdout);
    }

    SDL_GL_DeleteContext(plf.gl_context);
    SDL_DestroyWindow(plf.window);

    SDL_Quit();
    return 0;
}
