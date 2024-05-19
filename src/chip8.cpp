#include "../inc/chip8.hpp"

#include <iostream>
#include <fstream>
#include <cstring>

CHIP8::CHIP8(const EmulatorConfig &emu_config) : EmulatorBase(emu_config) {
    std::cout << "Initializing CHIP-8...\n";

    m_emu_state = RUNNING;

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

            /*
                Keyboard mapping:

                (original)     (emulator)
                1 2 3 C   ->   1 2 3 4
                4 5 6 D   ->   Q W E R
                7 8 9 E   ->   A S D F
                A 0 B F   ->   Z X C V
            */

            // Set key state to 1 if key pressed
            case SDLK_1:    m_input_keys[0x1] = 1; break;
            case SDLK_2:    m_input_keys[0x2] = 1; break;
            case SDLK_3:    m_input_keys[0x3] = 1; break;
            case SDLK_4:    m_input_keys[0xC] = 1; break;
            case SDLK_q:    m_input_keys[0x4] = 1; break;
            case SDLK_w:    m_input_keys[0x5] = 1; break;
            case SDLK_e:    m_input_keys[0x6] = 1; break;
            case SDLK_r:    m_input_keys[0xD] = 1; break;
            case SDLK_a:    m_input_keys[0x7] = 1; break;
            case SDLK_s:    m_input_keys[0x8] = 1; break;
            case SDLK_d:    m_input_keys[0x9] = 1; break;
            case SDLK_f:    m_input_keys[0xE] = 1; break;
            case SDLK_z:    m_input_keys[0xA] = 1; break;
            case SDLK_x:    m_input_keys[0x0] = 1; break;
            case SDLK_c:    m_input_keys[0xB] = 1; break;
            case SDLK_v:    m_input_keys[0xF] = 1; break;
            }
            break;

        case SDL_KEYUP:
            switch(event.key.keysym.sym)
            {
            // Set key state to 0 if key released
            case SDLK_1:    m_input_keys[0x1] = 0; break;
            case SDLK_2:    m_input_keys[0x2] = 0; break;
            case SDLK_3:    m_input_keys[0x3] = 0; break;
            case SDLK_4:    m_input_keys[0xC] = 0; break;
            case SDLK_q:    m_input_keys[0x4] = 0; break;
            case SDLK_w:    m_input_keys[0x5] = 0; break;
            case SDLK_e:    m_input_keys[0x6] = 0; break;
            case SDLK_r:    m_input_keys[0xD] = 0; break;
            case SDLK_a:    m_input_keys[0x7] = 0; break;
            case SDLK_s:    m_input_keys[0x8] = 0; break;
            case SDLK_d:    m_input_keys[0x9] = 0; break;
            case SDLK_f:    m_input_keys[0xE] = 0; break;
            case SDLK_z:    m_input_keys[0xA] = 0; break;
            case SDLK_x:    m_input_keys[0x0] = 0; break;
            case SDLK_c:    m_input_keys[0xB] = 0; break;
            case SDLK_v:    m_input_keys[0xF] = 0; break;
            }
            break;
        }
    }
}

void CHIP8::INSTR_00E0() {
    memset(m_display, 0, sizeof(m_display));
}

void CHIP8::INSTR_00EE() {
    if(m_stack_pointer == 0) {
        std::cerr << "ERROR: Stack underflow!\n";
        exit(EXIT_FAILURE);
    }

    --m_stack_pointer;

    m_pc = m_stack[m_stack_pointer];
}

void CHIP8::INSTR_1NNN() {
    uint16_t nnn_address = m_inst.opcode & 0x0FFF;
    m_pc = nnn_address;
}

void CHIP8::INSTR_2NNN() {
    if(m_stack_pointer == 16) {
        std::cerr << "ERROR: Stack overflow!\n";
        exit(EXIT_FAILURE);
    }

    m_stack[m_stack_pointer] = m_pc;

    ++m_stack_pointer;

    m_pc = m_inst.opcode & 0x0FFF;
}

void CHIP8::INSTR_3XNN() {
    if(m_registers[m_inst.X] == (m_inst.opcode & 0x00FF)) {
        m_pc += 2;
    }
}

void CHIP8::INSTR_4XNN() {
    if(m_registers[m_inst.X] != (m_inst.opcode & 0x00FF)) {
        m_pc += 2;
    }
}

void CHIP8::INSTR_5XY0() {
    if(m_registers[m_inst.X] == m_registers[m_inst.Y]){
        m_pc += 2;
    }
}

void CHIP8::INSTR_6XNN() {
    m_registers[m_inst.X] = (m_inst.opcode & 0x00FF);
}

void CHIP8::INSTR_7XNN() {
    m_registers[m_inst.X] += (m_inst.opcode & 0x00FF);
}

void CHIP8::INSTR_8XY0() {
    m_registers[m_inst.X] = m_registers[m_inst.Y];
}

void CHIP8::INSTR_8XY1() {
    m_registers[m_inst.X] |= m_registers[m_inst.Y];
}

void CHIP8::INSTR_8XY2() {
    m_registers[m_inst.X] &= m_registers[m_inst.Y];
}

void CHIP8::INSTR_8XY3() {
    m_registers[m_inst.X] ^= m_registers[m_inst.Y];
}

void CHIP8::INSTR_8XY4() {
    const uint16_t sum = m_registers[m_inst.X] + m_registers[m_inst.Y];

    m_registers[0xF] = 0;

    if(sum > 0xFF) {
        m_registers[0xF] = 1;
    }

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
    m_registers[0xF] = m_registers[m_inst.X] & 0x1;
    m_registers[m_inst.X] >>= 1;
}

void CHIP8::INSTR_8XY7() {
    if(m_registers[m_inst.Y] > m_registers[m_inst.X]) {
        m_registers[0xF] = 1;
    }
    else {
        m_registers[0xF] = 0;
    }

    m_registers[m_inst.X] = m_registers[m_inst.Y] - m_registers[m_inst.X];
}

void CHIP8::INSTR_8XYE() {
    m_registers[0xF] = (m_registers[m_inst.X] & 0x80) >> 7;
    m_registers[m_inst.X] <<= 1;
}

void CHIP8::INSTR_9XY0() {
    if(m_registers[m_inst.X] != m_registers[m_inst.Y]) {
        m_pc += 2;
    }
}

void CHIP8::INSTR_ANNN() {
    m_index_register = m_inst.opcode & 0x0FFF;
}

void CHIP8::INSTR_BNNN() {
    m_pc = (m_inst.opcode & 0x0FFF) + m_registers[0x0];
}

void CHIP8::INSTR_CXNN() {
    const uint8_t rand_num = m_rand_byte(m_rand_gen) & 0xFF;
    m_registers[m_inst.X] = rand_num & (m_inst.opcode & 0x00FF);
}

void CHIP8::INSTR_DXYN() {
    const uint16_t width = 8;
    const uint16_t height = m_inst.opcode & 0x000F;

    // Set collision flag to 0
    m_registers[0xF] = 0;

    uint8_t x = m_registers[m_inst.X] % 64;
    uint8_t y = m_registers[m_inst.Y] % 32;

    for(uint32_t row = 0; row < height; ++row) {
        uint8_t spriteByte = m_memory[m_index_register + row];

        for(uint32_t col = 0; col < width; ++col) {
            uint8_t spritePixel = spriteByte & (0x80 >> col);
            uint32_t screenIndex = (y + row) * 64 + (x + col);

            if (spritePixel) {
                // Check if pixel is set, if so set the collision flag
                if (m_display[screenIndex] == 1) {
                    m_registers[0xF] = 1;
                }

                m_display[screenIndex] ^= 1;
            }
        }
    }
}


void CHIP8::INSTR_EX9E() {
    uint8_t key = m_registers[m_inst.X];

    if(m_input_keys[key]) {
        m_pc += 2;
    }
}

void CHIP8::INSTR_EXA1() {
    uint8_t key = m_registers[m_inst.X];

    if(!m_input_keys[key]) {
        m_pc += 2;
    }
}

void CHIP8::INSTR_FX07() {
    m_registers[m_inst.X] = m_delay_timer;
}

void CHIP8::INSTR_FX0A() {
    if(m_emu_state == PAUSED) {
        m_pc -= 2; // If emulator is paused, re-run the instruction
    }
    else {
        bool key_pressed = false;

        for(uint8_t i = 0; i < 16; ++i) {
            if(m_input_keys[i]) {
                m_registers[m_inst.X] = i;
                key_pressed = true;
                break;
            }
        }

        if(!key_pressed) {
            m_pc -= 2; // If no key is pressed, re-run the instruction
        }
    }
}

void CHIP8::INSTR_FX15() {
    m_delay_timer = m_registers[m_inst.X];
}

void CHIP8::INSTR_FX18() {
    m_sound_timer = m_registers[m_inst.X];
}

void CHIP8::INSTR_FX1E() {
    m_index_register += m_registers[m_inst.X];
}

void CHIP8::INSTR_FX29() {
    m_index_register = FONTSET_START_ADDRESS + (m_registers[m_inst.X] * 5);
}

void CHIP8::INSTR_FX33() {
    uint8_t val = m_registers[m_inst.X];

    m_memory[m_index_register + 2] = val % 10;
    val /= 10;

    m_memory[m_index_register + 1] = val % 10;
    val /= 10;

    m_memory[m_index_register] = val % 10;
}

void CHIP8::INSTR_FX55() {
    for(uint8_t i = 0; i <= m_inst.X; ++i) {
        m_memory[m_index_register + i] = m_registers[i];
    }
}

void CHIP8::INSTR_FX65() {
    for(uint8_t i = 0; i <= m_inst.X; ++i) {
        m_registers[i] = m_memory[m_index_register + i];
    }
}

void CHIP8::emulateInstruction() {
    // Get next opcode from memory
    m_inst.opcode = (m_memory[m_pc] << 8) | m_memory[m_pc + 1];

    // Fill instruction format
    m_inst.X = (m_inst.opcode & 0x0F00) >> 8;
    m_inst.Y = (m_inst.opcode & 0x00F0) >> 4;

    // Pre-increment PC for next opcode
    m_pc += 2;

    // Emulate opcode
    switch(m_inst.opcode & 0xF000u) {
        case 0x0000:
            switch(m_inst.opcode & 0x00FFu) {
                case 0x00E0: INSTR_00E0(); break;
                case 0x00EE: INSTR_00EE(); break;
                default:
                    std::cerr << "Unsupported opcode: " << m_inst.opcode << "\n";
                    exit(EXIT_FAILURE);
            }
            break;
        case 0x1000: INSTR_1NNN(); break;
        case 0x2000: INSTR_2NNN(); break;
        case 0x3000: INSTR_3XNN(); break;
        case 0x4000: INSTR_4XNN(); break;
        case 0x5000: INSTR_5XY0(); break;
        case 0x6000: INSTR_6XNN(); break;
        case 0x7000: INSTR_7XNN(); break;
        case 0x8000:
            switch(m_inst.opcode & 0x000Fu) {
                case 0x0000: INSTR_8XY0(); break;
                case 0x0001: INSTR_8XY1(); break;
                case 0x0002: INSTR_8XY2(); break;
                case 0x0003: INSTR_8XY3(); break;
                case 0x0004: INSTR_8XY4(); break;
                case 0x0005: INSTR_8XY5(); break;
                case 0x0006: INSTR_8XY6(); break;
                case 0x0007: INSTR_8XY7(); break;
                case 0x000E: INSTR_8XYE(); break;
                default:
                    std::cerr << "Unsupported opcode: " << m_inst.opcode << "\n";
                    exit(EXIT_FAILURE);
            }
            break;
        case 0x9000: INSTR_9XY0(); break;
        case 0xA000: INSTR_ANNN(); break;
        case 0xB000: INSTR_BNNN(); break;
        case 0xC000: INSTR_CXNN(); break;
        case 0xD000: INSTR_DXYN(); break;
        case 0xE000:
            switch(m_inst.opcode & 0x00FFu) {
                case 0x009E: INSTR_EX9E(); break;
                case 0x00A1: INSTR_EXA1(); break;
                default:
                    std::cerr << "Unsupported opcode: " << m_inst.opcode << "\n";
                    exit(EXIT_FAILURE);
            }
            break;
        case 0xF000:
            switch(m_inst.opcode & 0x00FFu) {
                case 0x0007: INSTR_FX07(); break;
                case 0x000A: INSTR_FX0A(); break;
                case 0x0015: INSTR_FX15(); break;
                case 0x0018: INSTR_FX18(); break;
                case 0x001E: INSTR_FX1E(); break;
                case 0x0029: INSTR_FX29(); break;
                case 0x0033: INSTR_FX33(); break;
                case 0x0055: INSTR_FX55(); break;
                case 0x0065: INSTR_FX65(); break;
                default:
                    std::cerr << "Unsupported opcode: " << m_inst.opcode << "\n";
                    exit(EXIT_FAILURE);
            }
            break;
        default:
            std::cerr << "Unsupported opcode: " << m_inst.opcode << "\n";
            exit(EXIT_FAILURE);
    }

    // Update timers

    if(m_delay_timer > 0) {
        --m_delay_timer;
    }

    if(m_sound_timer > 0) {
        if(m_sound_timer == 1) {
            std::cout << '\a'; // Beep
        }

        --m_sound_timer;
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
            SDL_Rect pixel = {static_cast<int32_t>(x * m_emu_config.scale_factor),
                              static_cast<int32_t>(y * m_emu_config.scale_factor),
                              static_cast<int32_t>(m_emu_config.scale_factor),
                              static_cast<int32_t>(m_emu_config.scale_factor)};
            SDL_RenderFillRect(m_sdl.renderer, &pixel);
        }
    }

    SDL_RenderPresent(m_sdl.renderer);
}

void CHIP8::run() {
    std::cout << "Running CHIP8 emulator...\n";

    clearScreen();
    memset(m_display, 0, sizeof(m_display));
    
    while(m_emu_state != QUIT) {
        handleInput();

        if(m_emu_state == PAUSED) {
            continue;
        }

        emulateInstruction();

        clearScreen();

        updateScreen();

        SDL_Delay(1); // Workaround to slow down the emulator
    }
}
