#pragma once

#include <array>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <memory>

// modeling after the 6502 (see http://www.6502.org/users/obelisk/)

// 64 KB of memory
struct Mem
{
    static constexpr size_t MEM_SIZE = 64 * 1024; // 64 KB
    std::array<uint8_t, MEM_SIZE> mem;

    void initialize()
    {
      mem.fill(0);
    }

    // read one byte
    uint8_t operator[](size_t address) const
    {
      // we should assert if the address is valid or use mem.at(address)
      return mem[address];
    }

    // write 1 byte
    uint8_t& operator[](size_t address)
    {
      // we should assert if the address is valid or use mem.at(address)
      return mem[address];
    }

    // write 1 word to the stack. takes 2 cycles (1 for each byte)
    void write_word(uint16_t value, uint32_t address, uint32_t& cycles)
    {
        // least significant byte goes in first because little endian
        mem[address] = (value & 0xFF);
        mem[address + 1] = value >> 8;
        cycles -= 2;
    }

};

// 6502 microprocessor. 8 bit cpu, 16 bit memory bus, little endian
struct CPU {
    // the program counter
    uint16_t PC;
    // stack pointer
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
    uint8_t O : 1;
    uint8_t N : 1;

    void reset(Mem& mem)
    {
        // reset the program counter
        PC = 0xFFFC;
        // reset the stack pointer. stack pointer starts at 0x01FF and grows downward to 0x0100
        SP = 0x01FF;
        // reset the registers
        A = X = Y = 0;
        // reset the flags
        C = Z = I = D = B = O = N = 0;
        // initialize the memory. note that the CPU doesn't do anything else with it
        mem.initialize();
    }

    // fetches the byte of the PC. takes a cycle and increments program counter
    uint8_t fetch_byte(uint32_t& cycles, const Mem& memory)
    {
        uint8_t data = memory[PC];
        PC++;
        cycles--;
        return data;
    }

    // fetches the WORD (16 bit) of the PC. takes a cycle and increments program counter
    uint16_t fetch_word(uint32_t& cycles, const Mem& memory)
    {
        // 6502 is little endian, lower byte comes first
        uint16_t data = memory[PC] | (memory[PC + 1] << 8);
        PC += 2;
        cycles -= 2;
        // if I wanted to handle endianness, I would have to swap bytes here

        return data;
    }

    // peeks a byte at an address. takes a cycle but does not increment program counter
    uint8_t peek_byte(uint8_t address, uint32_t& cycles, const Mem& memory)
    {
        cycles--;
        return memory[address];
    }


  // opcodes
  static constexpr uint8_t
    INS_LDA_IM      = 0x00A9,   // LDA immediate
    INS_LDA_ZP      = 0x00A5,   // LDA Zero Page
    INS_LDA_ZPX     = 0x00B5,   // LDA Zero Page, X
    INS_JSR         = 0x0020    // JSR Absolute
    ;

    void LDASetStatus()
    {
        Z = (A == 0);
        N = (A & 0x80) != 0; // checks if the most significant digit of A is 1, for the 6502 it is checking for the 7th bit
    }

  void execute(uint32_t cycles, Mem& memory)
  {
    while (cycles > 0)
      {
        uint8_t opCode = fetch_byte(cycles, memory); // read opcode
        switch (opCode)
        {
        case INS_LDA_IM:
        {
            A = fetch_byte(cycles, memory);
            LDASetStatus();
        } break;
        case INS_LDA_ZP:
        {
            uint8_t ZeroPageAddress = fetch_byte(cycles, memory);
            A = peek_byte(ZeroPageAddress, cycles, memory);
            LDASetStatus();
        } break;
        case INS_LDA_ZPX:
        {
            uint8_t ZeroPageAddress = fetch_byte(cycles, memory);
            ZeroPageAddress = (ZeroPageAddress + X) & 0xFF; // wraps around in the zero page
            cycles--; // adding X to the ZeroPageAddress takes a cycle
            A = peek_byte(ZeroPageAddress, cycles, memory);
            LDASetStatus();
        } break;
        case INS_JSR:
        {
            uint16_t SubAddr = fetch_word(cycles, memory);
            // push return point - 1 on to the stack
            memory.write_word(PC - 1, SP, cycles); // push return address on to the stack
            SP--;
            cycles--;
            PC = SubAddr;
            cycles--;
        } break;

        default:
        {
            std::cerr << "Unknown opcode: " << "0x" << std::hex << (int)opCode << std::endl;
        } break;

        }
      }
  }
};