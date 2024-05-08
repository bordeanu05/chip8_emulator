#include "../inc/chip8.hpp"

int main() {
    EmulatorConfigT emu_config = {64, 32,      // Original CHIP8 resolution
                                       0xFFFFFFFF,  //
                                       0x00FFFFFF,  //
                                       20,
                                       "ibm.ch8"};  // ROM file name
    CHIP8 emu(emu_config);
    emu.run();
}
