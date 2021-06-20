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

struct PlatformInputState {
    int mouse_x, mouse_y;
    int mouse_rel_x, mouse_rel_y;
    uint32_t mouse_buttons;
    const uint8_t *keys;
};

static PlatformInputState get_platform_input_state()
{
    PlatformInputState state = {};
    state.mouse_buttons = SDL_GetMouseState(&state.mouse_x, &state.mouse_y);
    SDL_GetRelativeMouseState(&state.mouse_rel_x, &state.mouse_rel_y);

    int numkeys;
    state.keys = SDL_GetKeyboardState(&numkeys);
    (void)numkeys;

    return state;
}

#if WITH_DEV
static void do_dev_input(UpdateInfo &info, UpdateInfo &info_prev, const PlatformInputState &state)
{
    info.devinput_prev = info_prev.devinput;

    info.devinput.mouse[0] = state.mouse_x;
    info.devinput.mouse[1] = state.mouse_y;
    info.devinput.mouse_button[0] = state.mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
    info.devinput.mouse_button[1] = state.mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
    info.devinput.mouse_button[2] = state.mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

    info.devinput.mouse_rel[0] = state.mouse_rel_x;
    info.devinput.mouse_rel[1] = state.mouse_rel_y;

    info.devinput.alt_key = state.keys[SDL_SCANCODE_LALT];
    info.devinput.shift_key = state.keys[SDL_SCANCODE_LSHIFT];
    info.devinput.w = state.keys[SDL_SCANCODE_W];
    info.devinput.a = state.keys[SDL_SCANCODE_A];
    info.devinput.s = state.keys[SDL_SCANCODE_S];
    info.devinput.d = state.keys[SDL_SCANCODE_D];
    info.devinput.f2_key = state.keys[SDL_SCANCODE_F2];
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

static void handle_event_game(const SDL_Event &event, UpdateInfo *upd)
{
    switch(event.type) {
    default:
        break;
    }
}

static void finalize_game_input(GameInputs *inputs, GameInputs *prev_inputs, const PlatformInputState &state, PlatformOptions *platform_options)
{
    SDL_bool lock_mouse = platform_options->lock_mouse ? SDL_TRUE : SDL_FALSE;
    bool lock_mouse_needs_toggle = lock_mouse != SDL_GetRelativeMouseMode();
    if(lock_mouse_needs_toggle) {
        SDL_SetRelativeMouseMode(lock_mouse);
    }

    inputs->fire.down = state.keys[SDL_SCANCODE_SPACE];
    inputs->fire.last_down = prev_inputs->fire.down;

    inputs->pitch.value = math::clamp(prev_inputs->pitch.value - (float)state.mouse_rel_y * 0.004f, -1.0f, 1.0f);
    inputs->roll.value = math::clamp(prev_inputs->roll.value + (float)state.mouse_rel_x * 0.004f, -1.0f, 1.0f);

    const float throttle_dir = state.keys[SDL_SCANCODE_W] ? 1.0f : (state.keys[SDL_SCANCODE_S] ? -1.0f : 0.0f);
    const float throttle_add = throttle_dir * 0.01f;
    inputs->throttle.value = math::clamp(prev_inputs->throttle.value + throttle_add, 0.0f, 1.0f);

    const float yaw_dir = state.keys[SDL_SCANCODE_D] ? 1.0f : (state.keys[SDL_SCANCODE_A] ? -1.0f : 0.0f);
    const float yaw_add = yaw_dir * 0.05f;
    const float yaw_mul = yaw_dir == 0.0f ? 0.65f : 1.0f;
    inputs->yaw.value = math::clamp(prev_inputs->yaw.value * yaw_mul + yaw_add, -1.0f, 1.0f);

    if(state.mouse_buttons & SDL_BUTTON_MIDDLE || lock_mouse_needs_toggle) {
        inputs->pitch.value = 0.0f;
        inputs->roll.value = 0.0f;
    }

    inputs->pitch.delta = inputs->pitch.value - prev_inputs->pitch.value;
    inputs->roll.delta = inputs->roll.value - prev_inputs->roll.value;
    inputs->throttle.delta = inputs->throttle.value - prev_inputs->throttle.value;
    inputs->yaw.delta = inputs->yaw.value - prev_inputs->yaw.value;
}

int main(int, char **)
{
    UpdateInfo info_prev = {};
    Platform plf = {};
    Module module = {};
    PlatformOptions options = {};

    PlatformApi plf_api = {};
    plf_api.gl_get_proc_address = SDL_GL_GetProcAddress;
    plf_api.window_width = 1024;
    plf_api.window_height = 768;

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
    module.api.init(&options);

    plf.running = true;
    while(plf.running) {
        UpdateInfo info = {};
        info.delta_time = 1.0f / 60.0f; // TODO: Don't just assume 60FPS
        info_prev.delta_time = info.delta_time;

        SDL_GetWindowSize(plf.window, &plf_api.window_width, &plf_api.window_height);

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
#if WITH_DEV
            ImGui_ImplSDL2_ProcessEvent(&event);
#endif
            handle_event_platform(event, &plf, &plf_api, &module);
            handle_event_game(event, &info);
        }

        PlatformInputState input_state = get_platform_input_state();
        finalize_game_input(&info.input, &info_prev.input, input_state, &options);

#if WITH_DEV
        do_dev_input(info, info_prev, input_state);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(plf.window);
        ImGui::NewFrame();
#endif

        module.api.update(&info, &options); // TODO: Proper game loop
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
