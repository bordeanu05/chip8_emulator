#ifndef _EMULATOR_BASE_HPP_
#define _EMULATOR_BASE_HPP_

#include <iostream>
#include <cstdint>
#include <SDL2/SDL.h>

typedef enum {
    QUIT,
    RUNNING,
    PAUSED,
} EmulatorState;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} SDLT;

typedef struct {
    uint32_t window_width;    // SDL window_width
    uint32_t window_height;   // SDL window_height
    uint32_t fg_color;        // Foreground color
    uint32_t bg_color;        // Background color
    uint32_t scale_factor;    // Scaling the emulator screen
    const char *rom_name;     // ROM file name
} EmulatorConfigT;

class EmulatorBase {
public:
    EmulatorBase();
    EmulatorBase(const EmulatorConfigT &emu_config);
    ~EmulatorBase();

protected:
    SDLT m_sdl;
    EmulatorConfigT m_emu_config;

    void initConfig(const EmulatorConfigT &emu_config);
    bool initSDL();

    void clearScreen();
};

#endif // _EMULATOR_BASE_HPP_
