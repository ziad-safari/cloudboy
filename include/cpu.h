#pragma once

#include <common.h>
#include <instructions.h>
/**************************
CPU registers:
a: accumulator
f: flags
b - l: general purpose
pc: program counter
sp: stack pointer
**************************/

class CPUProc; 

class CPURegisters {
public:
    u8 a;
    u8 f;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 h;
    u8 l;
    u16 pc;
    u16 sp;
    CPURegisters();
    // ~CPURegisters();
};

class CPU {
private:
    CPURegisters regs;

    //current fetch...
    u16 fetched_data;
    u16 mem_dest;
    bool dest_is_mem;

    // Current Opcode
    u8 opcode;
    instruction *cur_inst;

    // Flags
    bool halted;
    bool stepping;

    bool int_master_enabled;

    // Instruction Execution functions
    void fetch_instruction();
    void fetch_data();
    void execute();
public:
    CPU();
    bool step();
    u16 cpu_read_reg(reg_type rt) const;
    void cpu_set_reg(reg_type rt, u16 val);
    bool CPU_FLAG_Z() { return BIT(regs.f, 7);}
    bool CPU_FLAG_C() { return BIT(regs.f, 4);}

    // To handle procedure functions
    friend class CPUProc;
};





//define a type called IN_PROC which is a function pointer
// typedef void (*IN_PROC)(cpu_context *);

// Get the Process function for the instruction


// #define CPU_FLAG_C BIT(ctx->regs.f, 4)