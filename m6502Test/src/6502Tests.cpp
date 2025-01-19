#include "6502.h"
#include <gtest/gtest.h>

class m6502Test1 : public ::testing::Test
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

    }
};

// using test fixtures
TEST_F( m6502Test1, LDAImmediateCanLoadAValueIntoTheARegister)
{
    // given:
    // inlining a little program
    mem[0xFFFC] = CPU::INS_LDA_IM;
    mem[0xFFFD] = 0x84;
    // -------------------------

    // when:
    CPU CPUCopy = cpu; 
    int32_t cyclesUsed = cpu.execute(2, mem);

    // then:
    EXPECT_EQ(cpu.A, 0x84);
    EXPECT_EQ(cyclesUsed, 2);
    EXPECT_FALSE(cpu.Z);
    EXPECT_TRUE(cpu.N); // 0x84 should be negative because bit 7 is true
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
}

TEST_F( m6502Test1, LDAZeroPageCanLoadAValueIntoTheARegister)
{
    // given:
    // inlining a little program
    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x37;
    // -------------------------

    // when:
    CPU CPUCopy = cpu; 
    int32_t cyclesUsed = cpu.execute(3, mem);

    // then:
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, 3);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
}

TEST_F( m6502Test1, LDAZeroPageXCanLoadAValueIntoTheARegister)
{
    // given:
    cpu.X = 5;
    // inlining a little program
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x42;
    mem[0x0047] = 0x37; // we check 0x0047 because ZPX goes to the address 0x42 but adds 5 because the X register is 5
    // -------------------------

    // when:
    CPU CPUCopy = cpu; 
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
}

TEST_F( m6502Test1, LDAZeroPageXCanLoadAValueIntoTheARegisterWhenItWraps)
{
    // given:
    cpu.X = 0xFF;
    // inlining a little program
    mem[0xFFFC] = CPU::INS_LDA_ZPX;
    mem[0xFFFD] = 0x0080;
    mem[0x007F] = 0x37;
    // -------------------------

    // when:
    CPU CPUCopy = cpu; 
    int32_t cyclesUsed = cpu.execute(4, mem);

    // then:
    EXPECT_EQ(cpu.A, 0x37);
    EXPECT_EQ(cyclesUsed, 4);
    EXPECT_FALSE(cpu.Z);
    EXPECT_FALSE(cpu.N);
    EXPECT_EQ(cpu.C, CPUCopy.C);
    EXPECT_EQ(cpu.I, CPUCopy.I);
    EXPECT_EQ(cpu.D, CPUCopy.D);
    EXPECT_EQ(cpu.B, CPUCopy.B);
    EXPECT_EQ(cpu.V, CPUCopy.V);
}