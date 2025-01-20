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
    /*for (auto& entry : instructionTable)
    {
        entry = [&](int32_t& cycles, Mem& memory)
        {
            //std::cerr << "[ERROR] Unknown opcode." << std::endl;
        };
    }*/

    // cycle accurate performance isn't a big concern of mine currently, hence I'm using shared handlers
    // these shared handlers have a switch statement, but the few nanosecond runtime cost is a fine tradeoff for code maintenance
    // maybe I'll change it later
    
    /** LDA family */
    // ----------------------
    instructionTable[INS_LDA_IM] = [&](int32_t& cycles, Mem& memory) {
        handle_lda(cycles, memory, AddressingMode::Immediate);
    };
    instructionTable[INS_LDA_ZP] = [&](int32_t& cycles, Mem& memory) {
        handle_lda(cycles, memory, AddressingMode::ZeroPage);
    };
    instructionTable[INS_LDA_ZPX] = [&](int32_t& cycles, Mem& memory) {
        handle_lda(cycles, memory, AddressingMode::ZeroPageX);
    };
    instructionTable[INS_LDA_ABS] = [&](int32_t& cycles, Mem& memory) {
        handle_lda(cycles, memory, AddressingMode::Absolute);
    };
    instructionTable[INS_LDA_ABSX] = [&](int32_t& cycles, Mem& memory) {
        handle_lda(cycles, memory, AddressingMode::AbsoluteX);
    };
    instructionTable[INS_LDA_ABSY] = [&](int32_t& cycles, Mem& memory) {
        handle_lda(cycles, memory, AddressingMode::AbsoluteY);
    };
    instructionTable[INS_LDA_INDX] = [&](int32_t& cycles, Mem& memory) {
        handle_lda(cycles, memory, AddressingMode::IndirectX);
    };
    instructionTable[INS_LDA_INDY] = [&](int32_t& cycles, Mem& memory) {
        handle_lda(cycles, memory, AddressingMode::IndirectY);
    };
    // ---------------------------------------------

    /** LDX Family */
    // ------------------
    instructionTable[INS_LDX_IM] = [&](int32_t& cycles, Mem& memory)
    {
        handle_ldx(cycles, memory, AddressingMode::Immediate);
    };
    // -------------------

    /** LDY Family */
    // ----------------------
    instructionTable[INS_LDY_IM] = [&](int32_t& cycles, Mem& memory)
    {
        handle_ldy(cycles, memory, AddressingMode::Immediate);
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
    instructionTable[0xEA] = [&](int32_t& cycles, Mem& memory) { cycles--; };
}

void m6502::CPU::handle_lda(int32_t& cycles, Mem& memory, AddressingMode mode)
{
    uint16_t address = 0;
    switch (mode)
    {
    case AddressingMode::Immediate:
        A = fetch_byte(cycles, memory);
        zn_set_status(A);
        return;

    case AddressingMode::ZeroPage:
        address = fetch_byte(cycles, memory);
        break;

    case AddressingMode::ZeroPageX:
        address = wrap_zero_page(fetch_byte(cycles, memory) + X);
        cycles--; // Extra cycle for indexed access
        break;

    case AddressingMode::Absolute:
        address = fetch_word(cycles, memory);
        break;

    case AddressingMode::AbsoluteX:
    {
        uint16_t baseAddr = fetch_word(cycles, memory);
        address = baseAddr + X;
        if (crosses_page_boundary(address, baseAddr)) cycles--;
        break;
    }

    case AddressingMode::AbsoluteY:
    {
        uint16_t baseAddr = fetch_word(cycles, memory);
        address = baseAddr + Y;
        if (crosses_page_boundary(address, baseAddr)) cycles--;
        break;
    }

    case AddressingMode::IndirectX:
    {
        uint8_t zpAddr = wrap_zero_page(fetch_byte(cycles, memory) + X);
        cycles--; // Extra cycle for zero-page indexing
        address = peek_word(zpAddr, cycles, memory);
        break;
    }

    case AddressingMode::IndirectY:
    {
        uint8_t zpAddr = fetch_byte(cycles, memory);
        uint16_t baseAddr = peek_word(zpAddr, cycles, memory);
        address = baseAddr + Y;
        if (crosses_page_boundary(address, baseAddr)) cycles--;
        break;
    }
    }
    // Perform the LDA operation and update flags
    A = peek_byte(address, cycles, memory);
    zn_set_status(A);
}

void m6502::CPU::handle_ldx(int32_t& cycles, Mem& memory, AddressingMode addressingMode)
{
    uint16_t address = 0;
    switch (addressingMode)
    {
    case AddressingMode::Immediate:
    {
        X = fetch_byte(cycles, memory);
        zn_set_status(X);
    } break;
    }
}

void m6502::CPU::handle_ldy(int32_t& cycles, Mem& memory, AddressingMode addressingMode)
{
    uint16_t address = 0;
    switch (addressingMode)
    {
    case AddressingMode::Immediate:
    {
        Y = fetch_byte(cycles, memory);
        zn_set_status(Y);
    } break;
    }
}