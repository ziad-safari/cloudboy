#include <cpu_proc.h>
#include <cpu.h>
#include <emu.h>
#include <cart.h>
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
    if (cpu.dest_is_mem) {
        //LD (BC), A for instance...

        if (cpu.cur_inst->reg_2 >= RT_AF) {
            //if 16 bit register...
            emu_cycles(1);
            //bus_write16(cpu.mem_dest, cpu.fetched_data);
        } else {
            bus_write(cpu.mem_dest, cpu.fetched_data);
        }

        return;
    }

    if (cpu.cur_inst->mode == AM_HL_SPR) {
        u8 hflag = (cpu.cpu_read_reg(cpu.cur_inst->reg_2) & 0xF) + 
            (cpu.fetched_data & 0xF) >= 0x10;

        u8 cflag = (cpu.cpu_read_reg(cpu.cur_inst->reg_2) & 0xFF) + 
            (cpu.fetched_data & 0xFF) >= 0x100;

        cpu_set_flags(0, 0, hflag, cflag);
        cpu.cpu_set_reg(cpu.cur_inst->reg_1, 
            cpu.cpu_read_reg(cpu.cur_inst->reg_2) + (char)cpu.fetched_data);

        return;
    }

    cpu.cpu_set_reg(cpu.cur_inst->reg_1, cpu.fetched_data);
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