#include <array>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <memory>

// modeling after the 6502 (see http://www.6502.org/users/obelisk/)

// 16 bit bus with 64 KB of memory
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

  // cheeky cheating operator here. not to be used by the emulated cpu, instead it is for testing purposes.
  uint8_t& operator[](size_t address)
  {
    // we should assert if the address is valid or use mem.at(address)
    return mem[address];
  }
};

// 6502 microprocessor. 8 bit cpu, little endian
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
        // reset the stack pointer
        SP = 0x0100;
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
    // peeks a byte at an address. takes a cycle but does not increment program counter
    uint8_t peek_byte(uint8_t address, uint32_t& cycles, const Mem& memory)
    {
        cycles--;
        return memory[address];
    }


  // opcodes
  static constexpr uint8_t
    INS_LDA_IM = 0x00A9,    // LDA immediate
    INS_LDA_ZP = 0x00A5,    // LDA Zero Page
    INS_LDA_ZPX = 0x00B5;   // LDA Zero Page, X

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
            uint8_t val = fetch_byte(cycles, memory);
            A = val;
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

        } break;

        default:
        {
            std::cerr << "Unknown opcode: " << "0x" << std::hex << (int)opCode << std::endl;
        } break;

        }
      }
  }
};

int main()
{
    Mem mem;
    CPU cpu;
    cpu.reset(mem);
    // inlining a little program
    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x84;
    // -------------------------
    cpu.execute(3, mem);
    std::cout << "A: " << "0x" << std::hex << (int)cpu.A << std::endl;
    return 0;
}