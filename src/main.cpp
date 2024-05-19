#include "../inc/chip8.hpp"

int main(int argc, char **argv) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <ROM file>" << '\n';
        exit(EXIT_FAILURE);
    }

    EmulatorConfig emu_config = {
        64, 32,      // Original CHIP8 resolution
        0xFFFFFFFF,  // Foreground color (white)
        0x00FFFFFF,  // Background color (black)
        20,          // Scale factor
        argv[1]      // ROM file name
    };

    CHIP8 emulator(emu_config);
    emulator.run();

    return 0;
}
