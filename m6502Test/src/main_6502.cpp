#include "6502.h"
#include <iostream>

int main()
{
    Mem mem;
    CPU cpu;
    cpu.reset(mem);
    // inlining a little program
    mem[0xFFFC] = CPU::INS_JSR;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::INS_LDA_IM;
    mem[0x4243] = 0x69;
    // -------------------------
    cpu.execute(9, mem);
    std::cout << "A: " << "0x" << std::hex << (int)cpu.A << std::endl;
    return 0;
}