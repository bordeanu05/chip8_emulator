#ifndef _CHIP8_HPP_
#define _CHIP8_HPP_

#include <cstdint>
#include <random>
#include <chrono>

#include "emulator_base.hpp"
#include "chip8_utils.hpp"

typedef struct {
    uint16_t opcode;
    uint16_t NNN;     // 12 bit address/constant
    uint8_t NN;       // 8 bit constant
    uint8_t N;        // 4 bit constant
    uint8_t X;        // 4 bit register identifier
    uint8_t Y;        // 4 bit register identifier
} InstructionT;

class CHIP8 : public EmulatorBase {
private:
    EmulatorBase m_emu_base;
    EmulatorState m_emu_state;

    uint8_t m_registers[16];
    uint8_t m_memory[MEMORY_SIZE];

    uint16_t m_index_register;
    uint16_t m_pc;
    uint16_t m_stack[16];

    uint8_t m_stack_pointer;
    uint8_t m_delay_timer;
    uint8_t m_sound_timer;
    uint8_t m_input_keys[16];

    uint32_t m_display[64 * 32];

    InstructionT m_inst;

public:
    CHIP8(const EmulatorConfigT &emu_config);

    bool loadROM(const char*);

    void handleInput();
    void emulateInstruction();
    void run();
};

#endif // _CHIP8_HPP_
