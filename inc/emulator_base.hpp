#ifndef EMULATOR_BASE_HPP
#define EMULATOR_BASE_HPP

#include <iostream>
#include <cstdint>
#include <SDL2/SDL.h>

enum EmulatorState {
    QUIT,
    RUNNING,
    PAUSED,
};

struct SDLResources {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

struct EmulatorConfig {
    uint32_t window_width;    // SDL window_width
    uint32_t window_height;   // SDL window_height
    uint32_t fg_color;        // Foreground color
    uint32_t bg_color;        // Background color
    uint32_t scale_factor;    // Scaling the emulator screen
    const char *rom_name;     // ROM file name
};

class EmulatorBase {
public:
    EmulatorBase();
    EmulatorBase(const EmulatorConfig&);
    ~EmulatorBase();

private:
    void initConfig(const EmulatorConfig&);
    bool initSDL();

protected:
    SDLResources m_sdl;
    EmulatorConfig m_emu_config;

    void clearScreen();

    virtual void handleInput() = 0;
    virtual void updateScreen() = 0;
};

#endif // EMULATOR_BASE_HPP
