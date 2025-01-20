#include "6502.h"


int32_t m6502::CPU::execute(int32_t cycles, Mem& memory)
{
        
    const int32_t cyclesRequested = cycles;
    while (cycles > 0)
    {
        uint8_t opCode = fetch_byte(cycles, memory); // read opcode
        switch (opCode)
        {
        case INS_LDA_IM:
        {
            A = fetch_byte(cycles, memory);
            zn_set_status(A);
        } break;
        case INS_LDA_ZP:
        {
            uint8_t ZeroPageAddress = fetch_byte(cycles, memory);
            A = peek_byte(ZeroPageAddress, cycles, memory);
            zn_set_status(A);
        } break;
        case INS_LDA_ZPX:
        {
            uint8_t ZeroPageAddress = fetch_byte(cycles, memory);
            ZeroPageAddress = wrap_zero_page(ZeroPageAddress + X);
            cycles--; // adding X to the ZeroPageAddress takes a cycle
            A = peek_byte(ZeroPageAddress, cycles, memory);
            zn_set_status(A);
        } break;
        case INS_LDA_ABS:
        {
            uint16_t addr = fetch_word(cycles, memory);
            A = peek_byte(addr, cycles, memory);
            zn_set_status(A);
        } break;
        case INS_LDA_ABSX:
        {
            uint16_t baseAddr = fetch_word(cycles, memory);
            uint16_t effectiveAddr = baseAddr + X;
            if (crosses_page_boundary(effectiveAddr, baseAddr))
            {
                cycles--;
            }
            A = peek_byte(effectiveAddr, cycles, memory);
            zn_set_status(A);
        } break;
        case INS_LDA_ABSY:
        {
            uint16_t baseAddr = fetch_word(cycles, memory);
            uint16_t effectiveAddr = baseAddr + Y;
            if (crosses_page_boundary(effectiveAddr, baseAddr))
            {
                cycles--;
            }
            A = peek_byte(effectiveAddr, cycles, memory);
            zn_set_status(A);
        } break;
        case INS_LDA_INDX:
        {
            uint8_t zpAddr = fetch_byte(cycles, memory);
            zpAddr = wrap_zero_page(zpAddr + X);
            cycles--; // adding x to zpAddr takes a cycle
            uint16_t effectiveAddr = peek_word(zpAddr, cycles, memory);
            A = peek_byte(effectiveAddr, cycles, memory);
            zn_set_status(A);
        } break;
        case INS_LDA_INDY:
        {
            uint8_t zpAddr = fetch_byte(cycles, memory);
            uint16_t baseAddr = peek_word(zpAddr, cycles, memory);
            uint16_t effectiveAddr = baseAddr + Y;
            if (crosses_page_boundary(effectiveAddr, baseAddr))
            {
                cycles--;
            }
            A = peek_byte(effectiveAddr, cycles, memory);
            zn_set_status(A);
        } break;
        case INS_JSR:
        {
            uint16_t SubAddr = fetch_word(cycles, memory);
            // push return point - 1 on to the stack
            memory.write_word(PC - 1, SP, cycles); // push return address on to the stack
            SP = wrap_stack_address(SP - 1); // We decrement SP by 1 but also need to enforce 8-bit stack pointer wrapping
            cycles--;
            PC = SubAddr;
            cycles--;
        } break;

        case 0xEA: // NOP opcode
        {
            cycles--; // It takes exactly 2 cycles
        } break;

        default:
        {
            std::cerr << "[ERROR] Unknown opcode: 0x" << std::hex << (int)opCode << std::endl;
            return cyclesRequested - cycles; // Stop executing gracefully
        } break;
                
        }
    }

    return cyclesRequested - cycles; // number of cycles used
}