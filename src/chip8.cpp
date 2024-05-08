#include "../inc/chip8.hpp"

#include <iostream>
#include <fstream>
#include <cstring>

CHIP8::CHIP8(const EmulatorConfigT &emu_config) : EmulatorBase(emu_config) {
    std::cout << "Initializing CHIP-8...\n";

    m_emu_state = RUNNING;

    m_pc = START_ADDRESS;

    for(uint32_t i = 0; i < FONTSET_SIZE; ++i) {
        m_memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    if(!loadROM(emu_config.rom_name)) {
        std::cerr << "Failed to load ROM!\n";
        exit(EXIT_FAILURE); 
    }

    std::cout << "Succesfully initialized CHIP-8!\n";
}

bool CHIP8::loadROM(const char *rom_name) {
    std::cout << "Loading ROM " << rom_name << "...\n";

    // Open ROM file
    FILE *rom = fopen(rom_name, "rb");
    if(!rom) {
        std::cerr << "ERROR: Failed to open ROM file " << rom_name << "!\n";
        return false;
    }

    // Check ROM file size
    fseek(rom, 0, SEEK_END);
    const size_t rom_size = ftell(rom);
    const size_t max_size = sizeof(m_memory) - START_ADDRESS;
    rewind(rom);

    if(rom_size > max_size) {
        std::cerr << "ERROR: Current ROM file size(" << rom_size << 
                      ") is greater than the maximum allowed size(" << max_size << ")!\n";
        return false;
    }

    if(fread(&m_memory[START_ADDRESS], rom_size, 1, rom) != 1) {
        std::cerr << "ERROR: Could no read ROM file into emulated memory\n";
        return false;
    }

    // Close ROM file
    fclose(rom);

    std::cout << "Succesfully loaded ROM " << rom_name << "!\n";
    return true;
}

void CHIP8::handleInput() {
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        switch(event.type)
        {
        case SDL_QUIT:  // Quit if window exited
            m_emu_state = QUIT;
            break;

        case SDL_KEYDOWN:
            switch(event.key.keysym.sym)
            {
            case SDLK_ESCAPE:    // Quit if ESC key pressed
                m_emu_state = QUIT;
                return;
            case SDLK_SPACE:    // Toggle between PAUSED/RUNNING states
                if(m_emu_state == PAUSED) {
                    m_emu_state = RUNNING;
                    std::cout << "Emulator resumed\n";
                }
                else {
                    m_emu_state = PAUSED;
                    std::cout << "Emulator paused\n";
                }
                break;
            }
            break;

        case SDL_KEYUP:
            break;
        }
    }
}

void CHIP8::emulateInstruction() {
    // Get next opcode from memory
    m_inst.opcode = (m_memory[m_pc] << 8) | m_memory[m_pc + 1];
    m_pc += 2; // Pre-increment PC for next opcode

    // Fill instruction format
    m_inst.NNN = m_inst.opcode & 0x0FFF;
    m_inst.NN = m_inst.opcode & 0x0FF;
    m_inst.N = m_inst.opcode & 0x0F;
    m_inst.X = (m_inst.opcode >> 8) & 0x0F;
    m_inst.Y = (m_inst.opcode >> 4) & 0x0F;

    // Emulate opcode
    switch((m_inst.opcode >> 12) & 0x0F) {
    case 0x00:
        break;
    
    /// TODO: Add chip8 instructions

    default:
        break; // Unimplemented or invalid
    }
}

void CHIP8::run() {
    std::cout << "Running CHIP8 emulator...\n";
    
    while(m_emu_state != QUIT) {
        handleInput();

        // CHIP8 logic

        // Delay

        clearScreen();  // from EmulatorBase

        updateScreen(); // from EmulatorBase
    }
}
