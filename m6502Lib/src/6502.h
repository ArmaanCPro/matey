#pragma once

#include <array>
#include <iostream>
#include <cstdint>
#include <memory>
#include <functional>

// modeling after the 6502 (see http://www.6502.org/users/obelisk/)
// uint8_t = byte
// uint16_t = word

namespace m6502
{
    struct Mem;
    class CPU;
    
    //using InstructionHandler = void (CPU::*)(int32_t& cycles, Mem& mem);
    // Instruction Handler is a function pointer taking a ref to cycles and memory. 
    using InstructionHandler = std::function<void(int32_t& cycles, Mem& memory)>;
    
    enum class AddressingMode : uint8_t {
        Immediate,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
        Absolute,
        AbsoluteX,
        AbsoluteY,
        IndirectX,
        IndirectY
    };
}

// 64 KB of memory
struct m6502::Mem
{
    static constexpr size_t MEM_SIZE = 64 * 1024; // 64 KB
    std::array<uint8_t, MEM_SIZE> mem;

    void initialize()
    {
        mem.fill(0);
    }

    // read one byte
    inline uint8_t operator[](size_t address) const
    {
        // we should assert if the address is valid or use mem.at(address)
        return mem[address];
    }

    // write 1 byte
    inline uint8_t& operator[](size_t address)
    {
        // we should assert if the address is valid or use mem.at(address)
        return mem[address];
    }

    // write 1 word to the stack. takes 2 cycles (1 for each byte)
    inline void write_word(uint16_t value, uint32_t address, int32_t& cycles)
    {
        // least significant byte goes in first because little endian
        mem[address] = (value & 0xFF);
        mem[address + 1] = value >> 8;
        cycles -= 2;
    }

};

// 6502 microprocessor. 8-bit cpu, 16-bit memory bus, little endian
class m6502::CPU
{
public:
    // the program counter
    uint16_t PC;
    // stack pointer
    //uint16_t SP; // should be uint8_t, but that creates some truncation issues. will fix later
    uint8_t SP;
    
    // registers
    uint8_t A;
    uint8_t X;
    uint8_t Y;

    // flags
    uint8_t C : 1;
    uint8_t Z : 1;
    uint8_t I : 1;
    uint8_t D : 1;
    uint8_t B : 1;
    uint8_t V : 1;
    uint8_t N : 1;

    void reset(Mem& mem)
    {
        // reset the program counter
        PC = 0xFFFC;
        // reset the stack pointer. stack pointer starts at 0x01FF and grows downward to 0x0100
        SP = 0xFF;
        // reset the registers
        A = X = Y = 0;
        // reset the flags
        C = Z = I = D = B = V = N = 0;
        // initialize the memory. note that the CPU doesn't do anything else with it
        mem.initialize();
        
        initialize_instruction_table();
    }


    // opcodes
    static constexpr uint8_t
        //LDA
        INS_LDA_IM      = 0x00A9,   // LDA immediate
        INS_LDA_ZP      = 0x00A5,   // LDA Zero Page
        INS_LDA_ZPX     = 0x00B5,   // LDA Zero Page, X
        INS_LDA_ABS     = 0x00AD,   // LDA Absolute
        INS_LDA_ABSX    = 0x00BD,   // LDA Absolute, X
        INS_LDA_ABSY    = 0x00B9,   // LDA Absolute, Y
        INS_LDA_INDX    = 0x00A1,   // LDA Indirect, X
        INS_LDA_INDY    = 0x00B1,   // LDA Indirect, X
        // LDX
        INS_LDX_IM      = 0x00A2,   // LDX Immediate
        INS_LDX_ZP      = 0x00A6,   // LDX Zero Page
        INS_LDX_ZPY     = 0x00B6,   // LDX Zero Page, Y
        INS_LDX_ABS     = 0x00AE,   // LDX Absolute
        INS_LDX_ABSY    = 0x00BE,   // LDX Absolute, Y
    
        // LDY
        INS_LDY_IM      = 0x00A0,   // LDY Immediate
        INS_LDY_ZP      = 0x00A4,   // LDX Zero Page
        INS_LDY_ZPX     = 0x00B4,   // LDY Zero Page, X
        INS_LDY_ABS     = 0x00AC,   // LDY Absolute
        INS_LDY_ABSX    = 0x00BC,   // LDY Absolute, X
    
        INS_JSR         = 0x0020    // JSR Absolute
    ;
    
    /** @return the number of cycles it took*/
    int32_t execute(int32_t cycles, Mem& memory);

private:

    // 6502 has 256 total opcodes
    InstructionHandler instructionTable[256];

    void initialize_instruction_table();

    // these grouped handlers have a switch statement based on addressingMoed increasing runtime cost. be aware of it if exact nanosecond cycle emulation is desired
    void handle_lda(int32_t& cycles, Mem& memory, AddressingMode addressingMode);
    void handle_ldx(int32_t& cycles, Mem& memory, AddressingMode addressingMode);
    void handle_ldy(int32_t& cycles, Mem& memory, AddressingMode addressingMode);

protected:
    
    inline uint16_t get_stack_address(uint8_t stackPointer)
    {
        return 0x0100 | stackPointer; // Stacks are always within page 0x01
    }

    inline uint8_t wrap_stack_address(uint8_t stackPointer)
    {
        return stackPointer & 0xFF; // enforce 8 bit stack pointer wrapping
    }

    // fetches the byte of the PC. takes a cycle and increments program counter
    inline uint8_t fetch_byte(int32_t& cycles, const Mem& memory)
    {
        cycles--;
        return memory[PC++];
    }

    // fetches the WORD (16 bit) of the PC. takes 2 cycles and increments program counter by 2
    inline uint16_t fetch_word(int32_t& cycles, const Mem& memory)
    {
        // 6502 is little endian, lower byte comes first
        uint16_t data = memory[PC] | (uint16_t)(memory[PC + 1] << 8u); // bitshift promotes memory[PC + 1] to an unsigned int so we cast it back
        PC += 2;
        cycles -= 2;
        // if I wanted to handle endianness, I would have to swap bytes here

        return data;
    }

    // peeks a byte at an address. takes a cycle but does not increment program counter
    inline uint8_t peek_byte(uint16_t address, int32_t& cycles, const Mem& memory)
    {
        cycles--;
        return memory[address];
    }
    // peeks a word at an address. takes 2 cycles but does not change program counter
    inline uint16_t peek_word(uint16_t address, int32_t& cycles, const Mem& memory)
    {
        cycles -= 2;
        return memory[address] | (uint16_t)(memory[address + 1] << 8u); // could also do peek_byte(address) | (peek_byte(address + 1) << 8) and not change the cycles here
    }


    #pragma region helpers

    // previously LDASetStatus(), but we can specify a register we want to pass in for this one.
    inline void zn_set_status(uint8_t value)
    {
        Z = (value == 0);
        N = (value & 0x80) != 0;    // checks if the most significant digit of A is 1, for the 6502 it is checking for the 7th bit
    }

    /** order doesn't actually matter. this basically extracts the high byte and checks for equivalence. the high byte represents the page #, i.e. 4401 vs 4501 are on different pages. */
    inline bool crosses_page_boundary(uint16_t newAddr, uint16_t baseAddr)
    {
        return (newAddr & 0xFF00) != (baseAddr & 0xFF00);   // see also: if (effectiveAddr - baseAddr >= 0xFF)
    }
    
    inline uint8_t wrap_zero_page(uint16_t address)
    {
        return address & 0x00FF; // Enforces zero-page boundaries
    }

    #pragma endregion
};