#include "../inc/chip8.hpp"

#include <iostream>
#include <fstream>
#include <cstring>

CHIP8::CHIP8(const EmulatorConfigT &emu_config) : EmulatorBase(emu_config) {
    std::cout << "Initializing CHIP-8...\n";

    // Set emulator state to RUNNING
    m_emu_state = RUNNING;

    // Set PC to the starting address
    m_pc = START_ADDRESS;

    // Load font into memory
    for(uint32_t i = 0; i < FONTSET_SIZE; ++i) {
        m_memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // Load ROM file
    if(!loadROM(emu_config.rom_name)) {
        std::cerr << "Failed to load ROM!\n";
        exit(EXIT_FAILURE); 
    }

    // Setup random engine
    m_rand_gen = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
    m_rand_byte = std::uniform_int_distribution<uint8_t>(0, 255);

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

// Clear screen
void CHIP8::INSTR_00E0() {
    memset(m_display, 0, sizeof(m_display));
    EmulatorBase::clearScreen();
}

// Return from subroutine
void CHIP8::INSTR_00EE() {
    if(m_stack_pointer == 0) {
        std::cerr << "ERROR: Stack underflow!\n";
        exit(EXIT_FAILURE);
    }

    // Decrement stack pointer
    --m_stack_pointer;

    // Set PC to the address at the top of the stack
    m_pc = m_stack[m_stack_pointer];
}

// Jump to address NNN
void CHIP8::INSTR_1NNN() {
    uint16_t nnn_address = m_inst.opcode & 0x0FFF;
    m_pc = nnn_address;
}

// Call subroutine at NNN
void CHIP8::INSTR_2NNN() {
    if(m_stack_pointer == 16) {
        std::cerr << "ERROR: Stack overflow!\n";
        exit(EXIT_FAILURE);
    }

    // Store current PC on the stack
    m_stack[m_stack_pointer] = m_pc;

    // Increment stack pointer
    ++m_stack_pointer;

    // Set PC to NNN
    m_pc = m_inst.opcode & 0x0FFF;
}

// Skip next instruction if register X equals NN
void CHIP8::INSTR_3XNN() {
    if(m_registers[m_inst.X] == (m_inst.opcode & 0x00FF)) {
        m_pc += 2;
    }
}

// Skip next instruction if register X does not equal NN
void CHIP8::INSTR_4XNN() {
    if(m_registers[m_inst.X] != (m_inst.opcode & 0x00FF)) {
        m_pc += 2;
    }
}

// Skip next instruction if register X equals register Y
void CHIP8::INSTR_5XY0() {
    if(m_registers[m_inst.X] == m_registers[m_inst.Y]){
        m_pc += 2;
    }
}

// Set register X to NN
void CHIP8::INSTR_6XNN() {
    m_registers[m_inst.X] = (m_inst.opcode & 0x00FF);
}

// Add NN to register X
void CHIP8::INSTR_7XNN() {
    m_registers[m_inst.X] += (m_inst.opcode & 0x00FF);
}

// Set register X to register Y
void CHIP8::INSTR_8XY0() {
    m_registers[m_inst.X] = m_registers[m_inst.Y];
}

// Set register X to (register X | register Y)
void CHIP8::INSTR_8XY1() {
    m_registers[m_inst.X] = m_registers[m_inst.X] | m_registers[m_inst.Y];
}

// Set register X to (register X & register Y)
void CHIP8::INSTR_8XY2() {
    m_registers[m_inst.X] = m_registers[m_inst.X] & m_registers[m_inst.Y];
}

// Set register X to (register X ^ register Y)
void CHIP8::INSTR_8XY3() {
    m_registers[m_inst.X] = m_registers[m_inst.X] ^ m_registers[m_inst.Y];
}

// Set register X to sum of register X and register Y
void CHIP8::INSTR_8XY4() {
    const uint16_t sum = m_registers[m_inst.X] + m_registers[m_inst.Y];

    // Set carry register to 0 (the last register)
    m_registers[0xF] = 0;

    // Check if sum can be put into 1 byte
    if(sum > 0xFF) {
        // Carry 1
        m_registers[0xF] = 1;
    }

    // Store least segnificant 8 bits
    m_registers[m_inst.X] = sum & 0xFF;
}

void CHIP8::INSTR_8XY5() {
    m_registers[0xF] = 0;

    if(m_registers[m_inst.X] > m_registers[m_inst.Y]) {
        m_registers[0xF] = 1;
    }

    m_registers[m_inst.X] -= m_registers[m_inst.Y];
}

void CHIP8::INSTR_8XY6() {

}

void CHIP8::INSTR_8XY7() {

}

void CHIP8::INSTR_8XYE() {

}

void CHIP8::INSTR_9XY0() {

}

// Set index register to NNN
void CHIP8::INSTR_ANNN() {
    m_index_register = m_inst.opcode & 0x0FFF;
}

// Jump to address NNN + register 0
void CHIP8::INSTR_BNNN() {
    m_pc = (m_inst.opcode & 0x0FFF) + m_registers[0x0];
}

// Set register X to a random number
void CHIP8::INSTR_CXNN() {
    const uint8_t rand_num = m_rand_byte(m_rand_gen) & 0xFF;
    m_registers[m_inst.X] = rand_num;
}

// Draw N height sprite at coords X, Y
void CHIP8::INSTR_DXYN() {
    uint16_t width = 8;
    uint16_t height = m_inst.opcode & 0x000F;

    // Set collision flag to 0
    m_registers[0xF] = 0;

    // Loop through each row of the sprite
    for(uint16_t row = 0; row < height; ++row) {
        // Get sprite byte
        uint8_t sprite_byte = m_memory[m_index_register + row];

        // Loop through each column of the sprite
        for(uint16_t col = 0; col < width; ++col) {
            // Check if pixel is set
            if((sprite_byte & 0x80) > 0) {
                // If the corresponding pixel on the display is also set then set collision flag to 1
                if(m_display[m_registers[m_inst.X] + col + ((m_registers[m_inst.Y] + row) * 64)] == 1) {
                    m_registers[0xF] = 1;
                }

                // Toggle the corresponding pixel on the display
                m_display[m_registers[m_inst.X] + col + ((m_registers[m_inst.Y] + row) * 64)] ^= 1;
            }

            sprite_byte <<= 1;
        }
    }
}

void CHIP8::INSTR_EX9E() {

}

void CHIP8::INSTR_EXA1() {

}

void CHIP8::INSTR_FX07() {

}

void CHIP8::INSTR_FX0A() {

}

void CHIP8::INSTR_FX15() {

}

void CHIP8::INSTR_FX18() {

}

void CHIP8::INSTR_FX1E() {

}

void CHIP8::INSTR_FX29() {

}

void CHIP8::INSTR_FX33() {

}

void CHIP8::INSTR_FX55() {

}

void CHIP8::INSTR_FX65() {

}

void CHIP8::emulateInstruction() {
    // Get next opcode from memory
    m_inst.opcode = (m_memory[m_pc] << 8) | m_memory[m_pc + 1];
    // Pre-increment PC for next opcode
    m_pc += 2;

    // Fill instruction format
    m_inst.X = (m_inst.opcode & 0x0F00) >> 8;
    m_inst.Y = (m_inst.opcode & 0x00F0) >> 4;

    // Emulate opcode
    switch(m_inst.opcode & 0xF000) {
    case 0x0000:
        switch (m_inst.opcode)
        {
        case 0x00E0:
            INSTR_00E0();
            break;
        case 0x00EE:
            INSTR_00EE();
            break;
        }
        break;
    
    case 0x1000:
        INSTR_1NNN();
        break;

    case 0x2000:
        INSTR_2NNN();
        break;

    case 0x3000:
        INSTR_3XNN();
        break;

    case 0x4000:
        INSTR_4XNN();
        break;

    case 0x5000:
        INSTR_5XY0();
        break;

    case 0x6000:
        INSTR_6XNN();
        break;

    case 0x7000:
        INSTR_7XNN();
        break;

    case 0x8000:
        switch (m_inst.opcode & 0x000F)
        {
        case 0x0000:
            INSTR_8XY0();
            break;
        case 0x0001:
            INSTR_8XY1();
            break;
        case 0x0002:
            INSTR_8XY2();
            break;
        case 0x0003:
            INSTR_8XY3();
            break;
        case 0x0004:
            INSTR_8XY4();
            break;
        case 0x0005:
            INSTR_8XY5();
            break;
        case 0x0006:
            INSTR_8XY6();
            break;
        case 0x0007:
            INSTR_8XY7();
            break;
        case 0x000E:
            INSTR_8XYE();
            break;
        }
        break;

    case 0x9000:
        INSTR_9XY0();
        break;

    case 0xA000:
        INSTR_ANNN();
        break;

    case 0xB000:
        INSTR_BNNN();
        break;

    case 0xC000:
        INSTR_CXNN();
        break;

    case 0xD000:
        INSTR_DXYN();
        break;

    case 0xE000:
        switch (m_inst.opcode & 0x00FF)
        {
        case 0x009E:
            INSTR_EX9E();
            break;
        case 0x00A1:
            INSTR_EXA1();
            break;
        }
        break;

    case 0xF000:
        switch (m_inst.opcode & 0x00FF)
        {
        case 0x0007:
            INSTR_FX07();
            break;
        case 0x000A:
            INSTR_FX0A();
            break;
        case 0x0015:
            INSTR_FX15();
            break;
        case 0x0018:
            INSTR_FX18();
            break;
        case 0x001E:
            INSTR_FX1E();
            break;
        case 0x0029:
            INSTR_FX29();
            break;
        case 0x0033:
            INSTR_FX33();
            break;
        case 0x0055:
            INSTR_FX55();
            break;
        case 0x0065:
            INSTR_FX65();
            break;
        }
        break;

    default:
        break; // Unimplemented or invalid
    }
}

void CHIP8::updateScreen() {
    // Loop over each pixel in the chip8 display
    for(uint32_t y = 0; y < 32; ++y) {
        for(uint32_t x = 0; x < 64; ++x) {
            // If pixel is set then set color to white, else set color to black
            if(m_display[x + (y * 64)] == 1) {
                SDL_SetRenderDrawColor(m_sdl.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
            else {
                SDL_SetRenderDrawColor(m_sdl.renderer, 0x00, 0x00, 0x00, 0xFF);
            }

            // Draw pixel
            SDL_Rect pixel = {x * m_emu_config.scale_factor, y * m_emu_config.scale_factor, m_emu_config.scale_factor, m_emu_config.scale_factor};
            SDL_RenderFillRect(m_sdl.renderer, &pixel);
        }
    }

    SDL_RenderPresent(m_sdl.renderer);
}

void CHIP8::run() {
    std::cout << "Running CHIP8 emulator...\n";
    
    while(m_emu_state != QUIT) {
        handleInput();

        emulateInstruction();

        SDL_Delay(16);

        EmulatorBase::clearScreen();

        updateScreen();
    }
}
