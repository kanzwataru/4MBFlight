#include "platform.h"
#include "game/mathlib.h"

#include <SDL2/SDL.h>
#include <glad/glad.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#if WITH_DEV
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#endif

struct Platform {
    bool running;
    SDL_Window *window;
    SDL_GLContext gl_context;

    int hotreload_count;
};

struct Module {
    void *handle;
    void *mem;
    size_t mem_size;
    ModuleApi api;
};

#define WINDOW_TITLE            "4MB Jam (SDL)"
#define WINDOW_TITLE_COMPILING  "4MB Jam (SDL) [Compiling...]"
#define WINDOW_TITLE_HOTLOADING "4MB Jam (SDL) [Hot-Reloading...]"
#define WINDOW_TITLE_FAILED     "4MB Jam (SDL) [Compile Error!]"

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

static void assets_load(PlatformApi *api)
{
    free(api->assets);

    // TODO: Actual assets, for now assets is just the shaders
    FILE *fp = fopen("shaders.bin", "rb");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    api->assets = (uint8_t *)malloc(size);
    api->assets_size = size;

    fread(api->assets, 1, size, fp);
    fclose(fp);
}

static void module_load(Module *module, PlatformApi *platform_api)
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

    assets_load(platform_api);
    module->api.loaded(module->mem, platform_api);
}

#if WITH_HOTRELOAD
static void module_unload(Module *module)
{
    if(module->handle) {
        SDL_UnloadObject(module->handle);
    }
    module->handle = nullptr;
}

static void recompile(Platform *plf, Module *module, PlatformApi *platform_api)
{
    char title[4096];

    snprintf(title, sizeof(title), "[%d] " WINDOW_TITLE_COMPILING, plf->hotreload_count);
    SDL_SetWindowTitle(plf->window, title);

    module_unload(module);
    int ret = system("sh -c \"cd ../ && ./hotreload.sh\"");
    if(ret == 0) {
        ++plf->hotreload_count;
    }

    snprintf(title, sizeof(title), "[%d] " WINDOW_TITLE_HOTLOADING, plf->hotreload_count);
    SDL_SetWindowTitle(plf->window, title);

    module_load(module, platform_api);

    snprintf(title, sizeof(title), "[%d] " WINDOW_TITLE, plf->hotreload_count);
    SDL_SetWindowTitle(plf->window, title);

    // TODO BUG: This doesn't currently work, must fix!
    if(ret == 2) {
        snprintf(title, sizeof(title), "[%d] " WINDOW_TITLE_FAILED, plf->hotreload_count);
        SDL_SetWindowTitle(plf->window, title);
    }
}
#endif

#if WITH_DEV
static void do_dev_input(UpdateInfo &info, UpdateInfo &info_prev)
{
    info.devinput_prev = info_prev.devinput;
    {
        int x, y;
        uint32_t state = SDL_GetMouseState(&x, &y);

        info.devinput.mouse[0] = x;
        info.devinput.mouse[1] = y;
        info.devinput.mouse_button[0] = state & SDL_BUTTON(SDL_BUTTON_LEFT);
        info.devinput.mouse_button[1] = state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
        info.devinput.mouse_button[2] = state & SDL_BUTTON(SDL_BUTTON_RIGHT);

        SDL_GetRelativeMouseState(&x, &y);
        info.devinput.mouse_rel[0] = x;
        info.devinput.mouse_rel[1] = y;

        int numkeys;
        const uint8_t *keys = SDL_GetKeyboardState(&numkeys);
        info.devinput.alt_key = keys[SDL_SCANCODE_LALT];
        info.devinput.shift_key = keys[SDL_SCANCODE_LSHIFT];
        info.devinput.w = keys[SDL_SCANCODE_W];
        info.devinput.a = keys[SDL_SCANCODE_A];
        info.devinput.s = keys[SDL_SCANCODE_S];
        info.devinput.d = keys[SDL_SCANCODE_D];
    }
}
#endif

static void handle_event_platform(const SDL_Event &event, Platform *plf, PlatformApi *plf_api, Module *module)
{
    switch(event.type) {
    case SDL_QUIT:
        plf->running = false;
        break;
#if WITH_HOTRELOAD
    case SDL_KEYDOWN:
        if(event.key.keysym.scancode == SDL_SCANCODE_F5)
            recompile(plf, module, plf_api);
        break;
#endif
    }
}

struct ButtonMap {
    ButtonState *map[SDL_NUM_SCANCODES];
};

static ButtonMap create_button_map(UpdateInfo *upd)
{
    ButtonMap button_map = {};
    button_map.map[SDL_SCANCODE_SPACE] = &upd->input.fire;

    return button_map;
}

static void handle_event_game(const SDL_Event &event, const ButtonMap &button_map, UpdateInfo *upd)
{
    switch(event.type) {
    /*
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        if(button_map.map[event.key.keysym.scancode] && (event.key.repeat == 0)) {
            button_map.map[event.key.keysym.scancode]->transitions += 1;
        }
        break;
        */
    default:
        break;
    }
}

static void finalize_game_input(GameInputs *inputs, GameInputs *prev_inputs)
{
    int numkeys;
    const uint8_t *keys = SDL_GetKeyboardState(&numkeys);

    int mouse_x, mouse_y, mouse_rel_x, mouse_rel_y;
    uint32_t mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_GetRelativeMouseState(&mouse_rel_x, &mouse_rel_y);

    (void)mouse_buttons;

    inputs->fire.down = keys[SDL_SCANCODE_SPACE];
    inputs->fire.last_down = prev_inputs->fire.down;

    inputs->pitch.value = math::clamp(prev_inputs->pitch.value - (float)mouse_rel_y * 0.004f, -1.0f, 1.0f);
    inputs->roll.value = math::clamp(prev_inputs->roll.value + (float)mouse_rel_x * 0.004f, -1.0f, 1.0f);

    if(mouse_buttons & SDL_BUTTON_MIDDLE) {
        inputs->pitch.value = 0.0f;
        inputs->roll.value = 0.0f;
    }

    inputs->pitch.delta = inputs->pitch.value - prev_inputs->pitch.value;
    inputs->roll.delta = inputs->pitch.value - prev_inputs->pitch.value;
}

int main(int, char **)
{
    UpdateInfo info_prev = {};
    Platform plf = {};
    Module module = {};

    PlatformApi plf_api = {};
    plf_api.gl_get_proc_address = SDL_GL_GetProcAddress;
    plf_api.window_width = 800;
    plf_api.window_height = 600;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        panic(SDL_GetError());
    }

    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG); // debug output

    plf.window = SDL_CreateWindow(WINDOW_TITLE,
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  plf_api.window_width, plf_api.window_height,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    sdl_assume_ptr(plf.window);

    plf.gl_context = SDL_GL_CreateContext(plf.window);
    sdl_assume_ptr(plf.gl_context);

    gladLoadGLLoader(SDL_GL_GetProcAddress);
	debug_enable_gl_callback();

    SDL_GL_SetSwapInterval(1);

#if WITH_DEV
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(plf.window, plf.gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    plf_api.dear_imgui_ctx = (void *)ImGui::GetCurrentContext();
#endif

    module_load(&module, &plf_api);
    module.api.init();

    plf.running = true;
    while(plf.running) {
        UpdateInfo info = {};
        ButtonMap button_map = create_button_map(&info); // Ugh

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
#if WITH_DEV
            ImGui_ImplSDL2_ProcessEvent(&event);
#endif
            handle_event_platform(event, &plf, &plf_api, &module);
            handle_event_game(event, button_map, &info);
        }

        finalize_game_input(&info.input, &info_prev.input);

#if WITH_DEV
        do_dev_input(info, info_prev);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(plf.window);
        ImGui::NewFrame();
#endif

        module.api.update(&info); // TODO: Proper game loop
        module.api.render();

#if WITH_DEV
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

        SDL_GL_SwapWindow(plf.window);
        fflush(stdout);
        info_prev = info;
    }

    module.api.quit();

    SDL_GL_DeleteContext(plf.gl_context);
    SDL_DestroyWindow(plf.window);

    SDL_Quit();
    return 0;
}
