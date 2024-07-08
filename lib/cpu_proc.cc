#include <cpu_proc.h>
#include <cpu.h>
#include <emu.h>
#include <unordered_map>

//process and execute CPU instructions...
void CPUProc::proc_none() {
    printf("INVALID INSTRUCTION!\n");
    exit(-7);
}
void CPUProc::proc_nop() {

}
// Disable interrupts for various IO and other instructions
void CPUProc::proc_di() {
    cpu.int_master_enabled = false;
}
// Loading procedure
void CPUProc::proc_load() {
    
}
bool CPUProc::check_cond() {
    bool z = cpu.CPU_FLAG_Z();
    bool c = cpu.CPU_FLAG_Z();

    switch(cpu.cur_inst->cond) {
        case CT_NONE: return true;
        case CT_C: return c;
        case CT_NC: return !c;
        case CT_Z: return z;
        case CT_NZ: return !z;
    }
    //should never reach this
    return false;
}
void CPUProc::cpu_set_flags( char z, char n, char h, char c)  {
    //f flags = 1001    
     if (z != -1) {
        BIT_SET(cpu.regs.f, 7, z);
    }

    if (n != -1) {
        BIT_SET(cpu.regs.f, 6, n);
    }

    if (h != -1) {
        BIT_SET(cpu.regs.f, 5, h);
    }

    if (c != -1) {
        BIT_SET(cpu.regs.f, 4, c);
    }
}
void CPUProc::proc_jp() {
    //if conditional var is true then jump
    if (check_cond()) {
        cpu.regs.pc = cpu.fetched_data;
        emu_cycles(1);
    }
}
void CPUProc::proc_xor() {
    cpu.regs.a ^= cpu.fetched_data & 0xFF;
    cpu_set_flags(cpu.regs.a == 0, 0, 0, 0);
    printf("register a value:%d\n",cpu.regs.a);
}
static const std::unordered_map<in_type, CPUProc::IN_PROC> processors = {
    {IN_NONE, CPUProc::proc_none},
    {IN_NOP, CPUProc::proc_nop},
    {IN_LD, CPUProc::proc_load},
    {IN_JP, CPUProc::proc_jp},      
    {IN_DI, CPUProc::proc_di},
    {IN_XOR, CPUProc::proc_xor}
};

CPUProc::IN_PROC CPUProc::inst_get_processor(in_type type) {
    auto it = processors.find(type);
    if (it != processors.end()) {
        return it->second;
    }// TODO Find something to return for not found case
}