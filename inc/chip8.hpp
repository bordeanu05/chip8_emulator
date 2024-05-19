#ifndef CHIP8_HPP
#define CHIP8_HPP

#include <cstdint>
#include <random>
#include <chrono>

#include "emulator_base.hpp"
#include "chip8_utils.hpp"

struct Instruction{
    uint16_t opcode;  
    uint8_t X;        // 4 bit register identifier
    uint8_t Y;        // 4 bit register identifier
};

class CHIP8 : public EmulatorBase {
private:
    EmulatorState m_emu_state;

    uint8_t m_registers[16] = {};
    uint8_t m_memory[MEMORY_SIZE] = {};

    uint16_t m_index_register;
    uint16_t m_pc;
    uint16_t m_stack[16] = {};

    uint8_t m_stack_pointer;
    uint8_t m_delay_timer;
    uint8_t m_sound_timer;
    uint8_t m_input_keys[16] = {};

    uint32_t m_display[64 * 32] = {};

    Instruction m_inst = {};

    std::default_random_engine m_rand_gen;
    std::uniform_int_distribution<uint8_t> m_rand_byte;

public:
    CHIP8(const EmulatorConfig&);

    void run();

private:
    bool loadROM(const char*);

    void handleInput();
    void emulateInstruction();
    void updateScreen();

    // CHIP8 instructions
    void INSTR_00E0();
    void INSTR_00EE();
    void INSTR_1NNN();
    void INSTR_2NNN();
    void INSTR_3XNN();
    void INSTR_4XNN();
    void INSTR_5XY0();
    void INSTR_6XNN();
    void INSTR_7XNN();
    void INSTR_8XY0();
    void INSTR_8XY1();
    void INSTR_8XY2();
    void INSTR_8XY3();
    void INSTR_8XY4();
    void INSTR_8XY5();
    void INSTR_8XY6();
    void INSTR_8XY7();
    void INSTR_8XYE();
    void INSTR_9XY0();
    void INSTR_ANNN();
    void INSTR_BNNN();
    void INSTR_CXNN();
    void INSTR_DXYN();
    void INSTR_EX9E();
    void INSTR_EXA1();
    void INSTR_FX07();
    void INSTR_FX0A();
    void INSTR_FX15();
    void INSTR_FX18();
    void INSTR_FX1E();
    void INSTR_FX29();
    void INSTR_FX33();
    void INSTR_FX55();
    void INSTR_FX65();
};

#endif // CHIP8_HPP
