#include "6502.h"


int32_t m6502::CPU::execute(int32_t cycles, Mem& memory)
{
    const int32_t cyclesRequested = cycles;
    while (cycles > 0)
    {
        uint8_t opCode = fetch_byte(cycles, memory);
        auto handler = instructionTable[opCode];
        handler(cycles, memory);
    }

    return cyclesRequested - cycles; // number of cycles used
}

void m6502::CPU::initialize_instruction_table()
{
    // No operation. NOP takes 2 cycles, one for getting the opcode and one for the instruction itself.
    auto NOP_lambda = [](int32_t& cycles, Mem& memory) { cycles--; };

    for (auto& entry : instructionTable)
    {
        entry = NOP_lambda;
    }

    // cycle accurate performance isn't a big concern of mine currently, hence I'm using shared handlers
    // these shared handlers have a switch statement, but the few nanosecond runtime cost is a fine tradeoff for code maintenance
    // maybe I'll change it later
    
    /** LDA family */
    // ----------------------
    instructionTable[INS_LDA_IM] = [&](int32_t& cycles, Mem& memory) {
        A = fetch_byte(cycles, memory);
        zn_set_status(A);
    };
    instructionTable[INS_LDA_ZP] = [&](int32_t& cycles, Mem& memory) {
        uint8_t ZeroPageAddress = fetch_byte(cycles, memory);
        A = peek_byte(ZeroPageAddress, cycles, memory);
        zn_set_status(A);
    };
    instructionTable[INS_LDA_ZPX] = [&](int32_t& cycles, Mem& memory) {
        uint8_t ZeroPageAddress = fetch_byte(cycles, memory);
        ZeroPageAddress = wrap_zero_page(ZeroPageAddress + X);
        cycles--; // adding X to teh ZPA takes a cycle
        A = peek_byte(ZeroPageAddress, cycles, memory);
        zn_set_status(A);
    };
    instructionTable[INS_LDA_ABS] = [&](int32_t& cycles, Mem& memory) {
        uint16_t addr = fetch_word(cycles, memory);
        A = peek_byte(addr, cycles, memory);
        zn_set_status(A);
    };
    instructionTable[INS_LDA_ABSX] = [&](int32_t& cycles, Mem& memory) {
        uint16_t baseAddr = fetch_word(cycles, memory);
        uint16_t effectiveAddr = baseAddr + X;
        if (crosses_page_boundary(effectiveAddr, baseAddr))
        {
            cycles--;
        }
        A = peek_byte(effectiveAddr, cycles, memory);
        zn_set_status(A);
    };
    instructionTable[INS_LDA_ABSY] = [&](int32_t& cycles, Mem& memory) {
        uint16_t baseAddr = fetch_word(cycles, memory);
        uint16_t effectiveAddr = baseAddr + Y;
        if (crosses_page_boundary(effectiveAddr, baseAddr))
        {
            cycles--;
        }
        A = peek_byte(effectiveAddr, cycles, memory);
        zn_set_status(A);
    };
    instructionTable[INS_LDA_INDX] = [&](int32_t& cycles, Mem& memory) {
        uint8_t zpAddr = fetch_byte(cycles, memory);
        zpAddr = wrap_zero_page(zpAddr + X);
        cycles--; // adding x to zpAddr takes a cycle
        uint16_t effectiveAddr = peek_word(zpAddr, cycles, memory);
        A = peek_byte(effectiveAddr, cycles, memory);
        zn_set_status(A);
    };
    instructionTable[INS_LDA_INDY] = [&](int32_t& cycles, Mem& memory) {
        uint8_t zpAddr = fetch_byte(cycles, memory);
        uint16_t baseAddr = peek_word(zpAddr, cycles, memory);
        uint16_t effectiveAddr = baseAddr + Y;
        if (crosses_page_boundary(effectiveAddr, baseAddr))
        {
            cycles--;
        }
        A = peek_byte(effectiveAddr, cycles, memory);
        zn_set_status(A);
    };
    // ---------------------------------------------

    /** LDX Family */
    // ------------------
    instructionTable[INS_LDX_IM] = [&](int32_t& cycles, Mem& memory)
    {
        X = fetch_byte(cycles, memory);
        zn_set_status(X);
    };
    // -------------------

    /** LDY Family */
    // ----------------------
    instructionTable[INS_LDY_IM] = [&](int32_t& cycles, Mem& memory)
    {
        Y = fetch_byte(cycles, memory);
        zn_set_status(Y);
    };
    // ----------------------
    
    instructionTable[INS_JSR] = [&](int32_t& cycles, Mem& memory)
    {
        uint16_t SubAddr = fetch_word(cycles, memory);
        // push return point - 1 on to the stack
        memory.write_word(PC - 1, SP, cycles); // push return address on to the stack
        SP = wrap_stack_address(SP - 1); // We decrement SP by 1 but also need to enforce 8-bit stack pointer wrapping
        cycles--;
        PC = SubAddr;
        cycles--;
    };
    
    // NOP opcode
    instructionTable[0xEA] = NOP_lambda;
}