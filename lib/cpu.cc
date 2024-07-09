#include <cpu.h>
#include <cart.h>
#include <emu.h>
#include <iostream>
#include <iomanip>
#include <cpu_proc.h>
// cpu_context ctx = {0};
CPURegisters::CPURegisters() : a(0), f(0), b(0), c(0), d(0), e(0), h(0), l(0), pc(0), sp(0) {}
CPU::CPU() : regs(), fetched_data(0), mem_dest(0), dest_is_mem(false), opcode(0) {
    regs.pc = 0x100;
    regs.a = 0x01;
}

void CPU::fetch_instruction() {
    opcode = bus_read(regs.pc++);
    cur_inst = instruction_by_opcode(opcode);
}

void CPU::execute() {
    CPUProc cpuproc(*this);
    CPUProc::IN_PROC proc = cpuproc.inst_get_processor(cur_inst->type);
    if (!proc) {
        NO_IMPL
    }
    // proc(*this);
    (cpuproc.*proc)();
    // cpuproc.proc();
}

bool CPU::step() {
    if (!halted) {
        u16 pc = regs.pc;
        fetch_instruction();
        fetch_data();
        printf("%04X: %-7s (%02X %02X %02X) A: %02X B: %02X C: %02X\n", 
            pc, inst_name(cur_inst->type).c_str(), opcode,
            bus_read(pc + 1), bus_read(pc + 2), regs.a, regs.b, regs.c);

        if (cur_inst == NULL) {
            printf("Unknown Instruction! %02X\n", opcode);
            exit(-7);
        }
        execute();
    }
    return true;
}