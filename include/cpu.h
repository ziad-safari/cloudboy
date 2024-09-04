#pragma once

#include <common.h>
// #include <emu.h>
// #include <bus.h>
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
class emu;
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
    emu* EMU;
    // Current Opcode
    u8 opcode;
    instruction *cur_inst;

    // Flags
    bool halted;
    bool stepping;

    bool int_master_enabled;
    u8 ie_register;
    
    // Instruction Execution functions
    void fetch_instruction();
    void fetch_data();
    void execute();
public:
    CPU(emu* myemu);
    bool step();
    u16 cpu_read_reg(reg_type rt) const;
    void cpu_set_reg(reg_type rt, u16 val);
    u8 cpu_read_reg8(reg_type rt);
    void cpu_set_reg8(reg_type rt, u8 val);
    bool CPU_FLAG_Z() { return BIT(regs.f, 7);}
    bool CPU_FLAG_C() { return BIT(regs.f, 4);}
    u8 cpu_get_ie_register();
    void cpu_set_ie_register(u8 n);

    void stack_push(u8 data);
    void stack_push16(u16 data);
    u8 stack_pop();
    u16 stack_pop16();

    /*
    CPU::bus_read(int a, int b) {
        bus_read()
    }
    
    */
    // To handle procedure functions
    friend class CPUProc;
    // friend u8 bus_read(u16 address);
    // friend void bus_write(u16 address, u8 value);
    // friend u16 bus_read16(u16 address);
    // friend void bus_write16(u16 address, u16 value);

};





//define a type called IN_PROC which is a function pointer
// typedef void (*IN_PROC)(cpu_context *);

// Get the Process function for the instruction


// #define CPU_FLAG_C BIT(ctx->regs.f, 4)