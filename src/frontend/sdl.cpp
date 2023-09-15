#include <SDL2/SDL.h>
#include "frontend/sdl.h"
#include "n64.h"

static constexpr std::size_t DefaultScreenWidth = 320;
static constexpr std::size_t DefaultScreenHeight = 240;
static constexpr std::size_t DefaultScreenScale = 2;

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

void render_screen(const N64& n64) {
    SDL_DestroyTexture(g_texture);

    const VI& vi = n64.mmu().vi();

    const auto width = Common::bit_range<11, 0>(vi.width());
    const auto v_video = vi.vstart();
    const auto v_start = Common::bit_range<25, 16>(v_video);
    const auto v_end = Common::bit_range<9, 0>(v_video);
    const auto height = (v_end - v_start) / 2;
    const auto origin = Common::bit_range<23, 0>(vi.origin());
    const auto color_format = Common::bit_range<1, 0>(vi.control());
    LINFO("Draw: width={}, height={}, origin={:08X}, color_format={}", width, height, origin, color_format);

    switch (color_format) {
        case 0: // Blank
            break;

        case 3:
            g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, width, height);
            if (!g_texture) {
                LERROR("Draw: failed to create SDL texture: {} (color_format={}, width={}, height={})", SDL_GetError(), width, height);
                return;
            }

            SDL_UpdateTexture(g_texture, nullptr, n64.mmu().rdram().data() + origin, width * 4);
            SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
            SDL_RenderPresent(g_renderer);

            break;

        default:
            UNIMPLEMENTED_MSG("Draw: Unimplemented color format {}", color_format);
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

    g_window = SDL_CreateWindow("n64emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DefaultScreenWidth * DefaultScreenScale, DefaultScreenHeight * DefaultScreenScale, 0);
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
    }

    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}
