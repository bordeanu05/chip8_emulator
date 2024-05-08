#include "../inc/emulator_base.hpp"

EmulatorBase::EmulatorBase() {}

EmulatorBase::EmulatorBase(const EmulatorConfigT &emu_config) {
    std::cout << "Initializing emulator base..." << '\n';

    initConfig(emu_config);

    if(!initSDL()) {
        exit(EXIT_FAILURE);
    }

    std::cout << "Succesfully initialized emulator base!\n";
}

// Clean up after terminating the program
EmulatorBase::~EmulatorBase() {
    SDL_DestroyRenderer(m_sdl.renderer);
    SDL_DestroyWindow(m_sdl.window);
    SDL_Quit();
}

void EmulatorBase::initConfig(const EmulatorConfigT &emu_config) {
    m_emu_config = (EmulatorConfigT)emu_config;
}

bool EmulatorBase::initSDL() {
    // Initialize SDL subsystems
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("Could not initialize SDL subsystems! %s\n", SDL_GetError());
        return false;
    }

    // Initialize SDL Window
    m_sdl.window = SDL_CreateWindow("CHIP8 Emulator",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    m_emu_config.window_width * m_emu_config.scale_factor,
                                    m_emu_config.window_height * m_emu_config.scale_factor,
                                    0);

    if(!m_sdl.window) {
        SDL_Log("Could not create SDL Window %s", SDL_GetError());
        return false;
    }

    // Initialize SDL Renderer
    m_sdl.renderer = SDL_CreateRenderer(m_sdl.window, -1, SDL_RENDERER_ACCELERATED);

    if(!m_sdl.renderer) {
        SDL_Log("Could not create SDL Renderer %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void EmulatorBase::clearScreen() {
    const uint8_t rgba[4] = {
        uint8_t((m_emu_config.bg_color << 24) & 0x00FFFFFF),
        uint8_t((m_emu_config.bg_color << 16) & 0xFF00FFFF),
        uint8_t((m_emu_config.bg_color <<  8) & 0xFFFF00FF),
        uint8_t((m_emu_config.bg_color <<  0) & 0xFFFFFF00)
    };

    SDL_SetRenderDrawColor(m_sdl.renderer, rgba[0], rgba[1], rgba[2], rgba[3]);
    SDL_RenderClear(m_sdl.renderer);
}