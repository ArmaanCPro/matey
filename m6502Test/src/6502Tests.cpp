#include "6502.h"
#include <gtest/gtest.h>

using namespace m6502;

class m6502Test1 : public testing::Test
{
public:
    Mem mem;
    CPU cpu;
    virtual void SetUp() override
    {
        cpu.reset(mem);
    }
    virtual void TearDown() override
    {
        cpu.reset(mem);
    }
    
    // using pointers to member variables. ugly but worth it.
    void TestLoadRegisterImmediate(uint8_t opcode, uint8_t CPU::*RegisterToTest);
    void TestLoadRegisterZeroPage(uint8_t opcode, uint8_t CPU::*RegisterToTest);
    void TestLoadRegisterZeroPageX(uint8_t opcode, uint8_t CPU::*RegisterToTest);
    void TestLoadRegisterZeroPageY(uint8_t opcode, uint8_t CPU::*RegisterToTest);
    void TestLoadRegisterAbsolute(uint8_t opcode, uint8_t CPU::*RegisterToTest);
    void TestLoadRegisterAbsoluteX(uint8_t opcode, uint8_t CPU::*RegisterToTest);
    void TestLoadRegisterAbsoluteY(uint8_t opcode, uint8_t CPU::*RegisterToTest);
    void TestLoadRegisterAbsoluteXPageCrossing(uint8_t opcode, uint8_t CPU::*RegisterToTest);
    void TestLoadRegisterAbsoluteYPageCrossing(uint8_t opcode, uint8_t CPU::*RegisterToTest);
};

// works for A, X, and Y registers
static void VerifyUnmodifiedFlagsFromLoadRegister(const CPU& cpu, const CPU& CPUCopy)
{
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
}

// using test fixtures
TEST_F(m6502Test1, TheCPUDoesNothingWhenWeExecuteZeroCycles)
{
    // given:
    constexpr int32_t NUM_CYCLES = 0;

    // when:
    const int32_t CyclesUsed = cpu.execute(NUM_CYCLES, mem);

    // then:
    EXPECT_EQ(CyclesUsed, 0);
}

TEST_F( m6502Test1, CPUCanExecuteMoreCyclesThanRequestedIfRequiredByTheInstruction)
{
    // given:
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x84;
    CPU CPUCopy = cpu;
    constexpr int32_t NUM_CYCLES = 1;

    // when:
    int32_t cyclesUsed = cpu.execute(NUM_CYCLES, mem);

    // then:
    EXPECT_EQ(cyclesUsed, 2);
}

void m6502Test1::TestLoadRegisterImmediate(uint8_t opcode, uint8_t CPU::*RegisterToTest) // pointer to a member variable. ugly syntax but worth it
{
    // given:
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x84;
    CPU CPUCopy = cpu; 

    // when:
    int32_t cyclesUsed = cpu.execute(2, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x84);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N); // 0x84 should be negative because bit 7 is true
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAImmediateCanLoadAValueIntoTheARegister)
{
    TestLoadRegisterImmediate(CPU::INS_LDA_IM, &CPU::A);
}

TEST_F( m6502Test1, LDXImmediateCanLoadAValueIntoTheXRegister)
{
    TestLoadRegisterImmediate(CPU::INS_LDX_IM, &CPU::X);
}

TEST_F( m6502Test1, LDYImmediateCanLoadAValueIntoTheYRegister)
{
    TestLoadRegisterImmediate(CPU::INS_LDY_IM, &CPU::Y);
}

TEST_F( m6502Test1, LDAImmediateCanLoadTheZeroFlag)
{
    // given:
    cpu.A = 0x69;
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x0;
    CPU CPUCopy = cpu;

    // when:
    cpu.execute(2, mem);

    // then:
    EXPECT_TRUE(cpu.Z);     // loading 0 into our A register should make this flag true!
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

void m6502Test1::TestLoadRegisterZeroPage(uint8_t opcode, uint8_t CPU::*RegisterToTest)
{
    // given:
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x37;
    CPU CPUCopy = cpu; 

    // when:
    int32_t cyclesUsed = cpu.execute(3, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x37);
    EXPECT_EQ(cyclesUsed, 3);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAZeroPageCanLoadAValueIntoTheARegister)
{
    TestLoadRegisterZeroPage(CPU::INS_LDA_ZP, &CPU::A);
}

TEST_F( m6502Test1, LDXZeroPageCanLoadAValueIntoTheXRegister)
{
    TestLoadRegisterZeroPage(CPU::INS_LDX_ZP, &CPU::X);
}

TEST_F( m6502Test1, LDYZeroPageCanLoadAValueIntoTheYRegister)
{
    TestLoadRegisterZeroPage(CPU::INS_LDY_ZP, &CPU::Y);
}

void m6502Test1::TestLoadRegisterZeroPageX(uint8_t opcode, uint8_t CPU::*RegisterToTest)
{
    // given:
    cpu.X = 5;
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x42;
    mem[0x0047] = 0x37; // we check 0x0047 because ZPX goes to the address 0x42 but adds 5 because the X register is 5
    CPU CPUCopy = cpu; 

    // when:
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

void m6502Test1::TestLoadRegisterZeroPageY(uint8_t opcode, uint8_t CPU::*RegisterToTest)
{
    // given:
    cpu.Y = 5;
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x42;
    mem[0x0047] = 0x37; // we check 0x0047 because ZPX goes to the address 0x42 but adds 5 because the X register is 5
    CPU CPUCopy = cpu; 

    // when:
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAZeroPageXCanLoadAValueIntoTheARegister)
{
    TestLoadRegisterZeroPageX(CPU::INS_LDA_ZPX, &CPU::A);
}

TEST_F( m6502Test1, LDXZeroPageYCanLoadAValueIntoTheXRegister)
{
    TestLoadRegisterZeroPageY(CPU::INS_LDX_ZPY, &CPU::X);
}

TEST_F( m6502Test1, LDYZeroPageXCanLoadAValueIntoTheYRegister)
{
    TestLoadRegisterZeroPageX(CPU::INS_LDY_ZPX, &CPU::Y);
}

TEST_F( m6502Test1, LDAZeroPageXCanLoadAValueIntoTheARegisterWhenItWraps)
{
    // given:
    cpu.X = 0xFF;
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x0080;
    mem[0x007F] = 0x37;
    CPU CPUCopy = cpu; 

    // when:
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

void m6502Test1::TestLoadRegisterAbsolute(uint8_t opcode, uint8_t CPU::*RegisterToTest)
{
    // given:
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44; // [ 0x4480
    mem[0x4480] = 0x37;
    CPU CPUCopy = cpu;

    // when:
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAAbsoluteCanLoadAValueIntoTheARegister)
{
    TestLoadRegisterAbsolute(CPU::INS_LDA_ABS, &CPU::A);
}

TEST_F( m6502Test1, LDXAbsoluteCanLoadAValueIntoTheXRegister)
{
    TestLoadRegisterAbsolute(CPU::INS_LDX_ABS, &CPU::X);
}

TEST_F( m6502Test1, LDYAbsoluteCanLoadAValueIntoTheYRegister)
{
    TestLoadRegisterAbsolute(CPU::INS_LDY_ABS, &CPU::Y);
}

void m6502Test1::TestLoadRegisterAbsoluteX(uint8_t opcode, uint8_t CPU::*RegisterToTest)
{
    // given:
    cpu.X = 1;
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44; // [ 0x4480
    mem[0x4481] = 0x37;
    CPU CPUCopy = cpu;

    // when:
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAAbsoluteXCanLoadAValueIntoTheARegister)
{
    TestLoadRegisterAbsoluteX(CPU::INS_LDA_ABSX, &CPU::A);
}

TEST_F( m6502Test1, LDYAbsoluteXCanLoadAValueIntoTheYRegister)
{
    TestLoadRegisterAbsoluteX(CPU::INS_LDY_ABSX, &CPU::Y);
}

void m6502Test1::TestLoadRegisterAbsoluteXPageCrossing(uint8_t opcode, uint8_t CPU::*RegisterToTest)
{
    // given:
    cpu.X = 0xFF;
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x02;
    mem[0xFFFE] = 0x44; // [ 0x4402
    mem[0x4501] = 0x37; // 0x4402 + 0xFF crosses page boundary
    CPU CPUCopy = cpu;

    // when:
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x37);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAAbsoluteXCanLoadAValueIntoTheARegisterWhenItCrossesAPageBoundary)
{
    TestLoadRegisterAbsoluteXPageCrossing(CPU::INS_LDA_ABSX, &CPU::A);
}

TEST_F( m6502Test1, LDYAbsoluteXCanLoadAValueIntoTheYRegisterWhenItCrossesAPageBoundary)
{
    TestLoadRegisterAbsoluteXPageCrossing(CPU::INS_LDY_ABSX, &CPU::Y);
}

void m6502Test1::TestLoadRegisterAbsoluteY(uint8_t opcode, uint8_t CPU::*RegisterToTest)
{
    // given:
    cpu.Y = 1;
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x80;
    mem[0xFFFE] = 0x44; // [ 0x4480
    mem[0x4481] = 0x37;
    CPU CPUCopy = cpu;

    // when:
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAAbsoluteYCanLoadAValueIntoTheARegister)
{
    TestLoadRegisterAbsoluteY(CPU::INS_LDA_ABSY, &CPU::A);
}

TEST_F( m6502Test1, LDXAbsoluteYCanLoadAValueIntoTheXRegister)
{
    TestLoadRegisterAbsoluteY(CPU::INS_LDX_ABSY, &CPU::X);
}

void m6502Test1::TestLoadRegisterAbsoluteYPageCrossing(uint8_t opcode, uint8_t CPU::*RegisterToTest)
{
    // given:
    cpu.Y = 0xFF;
    mem[0xFFFC] = opcode;
    mem[0xFFFD] = 0x02;
    mem[0xFFFE] = 0x44; // [ 0x4402
    mem[0x4501] = 0x37; // 0x4402 + 0xFF crosses page boundary
    CPU CPUCopy = cpu;

    // when:
    int32_t cyclesUsed = cpu.execute(5, mem);

    // then:
    EXPECT_EQ(cpu.*RegisterToTest, 0x37);
    EXPECT_EQ(cyclesUsed, 5);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAAbsoluteYCanLoadAValueIntoTheARegisterWhenItCrossesAPageBoundary)
{
    TestLoadRegisterAbsoluteYPageCrossing(CPU::INS_LDA_ABSY, &CPU::A);
}

TEST_F( m6502Test1, LDXAbsoluteYCanLoadAValueIntoTheXRegisterWhenItCrossesAPageBoundary)
{
    TestLoadRegisterAbsoluteYPageCrossing(CPU::INS_LDX_ABSY, &CPU::X);
}

TEST_F( m6502Test1, LDAIndirectXCanLoadAValueIntoTheARegister)
{
    // given:
    cpu.X = 0x04;
    mem[0xFFFC] = CPU::INS_LDA_INDX;
    mem[0xFFFD] = 0x02;
    mem[0x0006] = 0x00; // 0x2 + 0x4
    mem[0x0007] = 0x80;
    mem[0x8000] = 0x37;
    constexpr uint32_t EXPECTED_CYCLES = 6;
    CPU CPUCopy = cpu;

    // when:
    int32_t cyclesUsed = cpu.execute(EXPECTED_CYCLES, mem);

    // then:
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAIndirectYCanLoadAValueIntoTheARegister)
{
    // given:
    cpu.Y = 4;
    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0xFFFD] = 0x02;
    mem[0x0002] = 0x00; 
    mem[0x0003] = 0x80; 
    mem[0x8004] = 0x37; // 0x8000 + 0x4
    constexpr uint32_t EXPECTED_CYCLES = 5;
    CPU CPUCopy = cpu;

    // when:
    int32_t cyclesUsed = cpu.execute(EXPECTED_CYCLES, mem);

    // then:
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}

TEST_F( m6502Test1, LDAIndirectYCanLoadAValueIntoTheARegisterWhenItCrossesAPageBoundary)
{
    // given:
    cpu.Y = 0xFF;
    mem[0xFFFC] = CPU::INS_LDA_INDY;
    mem[0xFFFD] = 0x02;
    mem[0x0002] = 0x02; 
    mem[0x0003] = 0x80; 
    mem[0x8101] = 0x37; // 0x8002 + 0xFF
    constexpr uint32_t EXPECTED_CYCLES = 6;
    CPU CPUCopy = cpu;

    // when:
    int32_t cyclesUsed = cpu.execute(EXPECTED_CYCLES, mem);

    // then:
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, EXPECTED_CYCLES);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    VerifyUnmodifiedFlagsFromLoadRegister(cpu, CPUCopy);
}