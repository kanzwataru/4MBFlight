#include "platform.h"

#include <SDL2/SDL.h>
#include <cstdio>
#include <cassert>

struct Platform {
    bool           running;
    SDL_Window    *window;
    SDL_GLContext *gl_context;
};

static void panic(const char *str)
{
    fprintf(stderr, "[CRITICAL ERROR]: %s\nAbout to crash...\n", str);
    fflush(stderr);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", str, NULL);
    //exit(1);
    assert(0);
}

static void sdl_assume_ptr(void *ptr)
{
    char *c = static_cast<char *>(ptr);
    if(!c) {
        panic(SDL_GetError());
    }
}

#if WITH_HOT_RELOAD
static void platform_recompile(Platform *plf)
{
    
}
#endif

int main(int argc, char **argv)
{
    Platform platform = {};
#if WITH_HOT_RELOAD
    ModuleAPI module = {};
#endif

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        panic(SDL_GetError());
    }
    
    SDL_GL_LoadLibrary(NULL);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG); // debug output

    platform.window = SDL_CreateWindow("4MB Jam",
									   SDL_WINDOWPOS_CENTERED,
									   SDL_WINDOWPOS_CENTERED,
                                       800, 600,
									   SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	sdl_assume_ptr(platform.window);

    platform.running = true;
    while(platform.running) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT:
                platform.running = false;
                break;
#if WITH_HOT_RELOAD
            case SDL_KEYDOWN:
                if(event.key.keysym.scancode == SDL_SCANCODE_F5) {
                    platform_recompile(&platform);
                }
                break;
#endif
            }
        }
    }
    
    SDL_GL_DeleteContext(platform.gl_context);
	SDL_DestroyWindow(platform.window);

	SDL_Quit();
	return 0;
}
