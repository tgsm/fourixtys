#include <SDL2/SDL.h>
#include "frontend/sdl.h"
#include "n64.h"

static constexpr std::size_t DefaultScreenWidth = 640;
static constexpr std::size_t DefaultScreenHeight = 480;

SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
SDL_Texture* g_texture = nullptr;
SDL_Event g_event {};

bool g_running = false;

void handle_frontend_events() {
    while (SDL_PollEvent(&g_event)) {
        switch (g_event.type) {
            case SDL_QUIT:
                g_running = false;
                break;
        }
    }
}

int main_SDL(std::span<std::string_view> args) {
    PIF pif(args[0]);
    GamePak gamepak(args[1]);

    if (!gamepak.swap_bytes_for_endianness()) {
        return 1;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        LFATAL("Failed to initialize SDL: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    g_window = SDL_CreateWindow("n64emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DefaultScreenWidth, DefaultScreenHeight, 0);
    if (!g_window) {
        LFATAL("Failed to create SDL window: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    g_renderer = SDL_CreateRenderer(g_window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        LFATAL("Failed to create SDL renderer: {}", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 1;
    }

    N64 n64(pif, gamepak);

    g_running = true;
    while (g_running) {
        n64.run();
        handle_frontend_events();
    }

    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}
