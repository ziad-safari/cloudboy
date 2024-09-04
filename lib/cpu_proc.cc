#include <cpu_proc.h>
#include <cpu.h>
#include <emu.h>
#include <bus.h>
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
            cpu.EMU->emu_cycles(1);
            bus_write16(*(cpu.EMU), cpu.mem_dest, cpu.fetched_data);
        } else {
            bus_write(*(cpu.EMU), cpu.mem_dest, cpu.fetched_data);
        }

        cpu.EMU->emu_cycles(1);
        
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
// Loading to High Ram
void CPUProc::proc_ldh() {
    if (cpu.cur_inst->reg_1 == RT_A) {
        cpu.cpu_set_reg(cpu.cur_inst->reg_1, bus_read(*(cpu.EMU), 0xFF00 | cpu.fetched_data));
    } else {
        bus_write(*(cpu.EMU), cpu.mem_dest, cpu.regs.a);
    }

    cpu.EMU->emu_cycles(1);
}
void CPUProc::proc_xor() {
    cpu.regs.a ^= cpu.fetched_data & 0xFF;
    cpu_set_flags(cpu.regs.a == 0, 0, 0, 0);
    // printf("register a value:%d\n",cpu.regs.a);
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

void CPUProc::goto_addr(u16 addr, bool pushpc) {
    if (check_cond()) {
        if (pushpc) {
            cpu.EMU->emu_cycles(2);
            cpu.stack_push16(cpu.regs.pc);
        }

        cpu.regs.pc = addr;
        cpu.EMU->emu_cycles(1);
    }
}
// jump
void CPUProc::proc_jp() {
    goto_addr(cpu.fetched_data, false);
}
// jump relative
void CPUProc::proc_jr() {
    // grab value, it might be negative
    char rel = (char) (cpu.fetched_data & 0xFF);
    u16 addr = cpu.regs.pc + rel;
    goto_addr(addr, false);
}
void CPUProc::proc_call() {
    goto_addr(cpu.fetched_data, true);
}
void CPUProc::proc_rst() {
    goto_addr(cpu.cur_inst->param, true);
}
void CPUProc::proc_ret() {
    if (cpu.cur_inst->cond != CT_NONE) cpu.EMU->emu_cycles(1);

    if (check_cond()) {
        u16 lo = cpu.stack_pop();
        cpu.EMU->emu_cycles(1);
        u16 hi = cpu.stack_pop();
        cpu.EMU->emu_cycles(1);

        u16 n = (hi << 8) | lo;  
        cpu.regs.pc = n;

        cpu.EMU->emu_cycles(1);
    }
}
void CPUProc::proc_reti() {
    cpu.int_master_enabled = true;
    proc_ret();
}
void CPUProc::proc_pop() {
    u16 lo = cpu.stack_pop();
    cpu.EMU->emu_cycles(1);
    u16 hi = cpu.stack_pop();
    cpu.EMU->emu_cycles(1);

    u16 n = (hi << 8) | lo;

    cpu.cpu_set_reg(cpu.cur_inst->reg_1, n);

    if (cpu.cur_inst->reg_1 == RT_AF) {
        cpu.cpu_set_reg(cpu.cur_inst->reg_1, n & 0xFFF0);
    }
}

void CPUProc::proc_push() {
    u16 hi = (cpu.cpu_read_reg(cpu.cur_inst->reg_1) >> 8) & 0xFF;
    cpu.EMU->emu_cycles(1);
    cpu.stack_push(hi);

    u16 lo = cpu.cpu_read_reg(cpu.cur_inst->reg_1) & 0xFF;
    cpu.EMU->emu_cycles(1);
    cpu.stack_push(lo);
    
    cpu.EMU->emu_cycles(1);
}
static const std::unordered_map<in_type, CPUProc::IN_PROC> processors = {
    {IN_NONE, CPUProc::proc_none},
    {IN_NOP, CPUProc::proc_nop},
    {IN_LD, CPUProc::proc_load},
    {IN_LDH, CPUProc::proc_ldh},
    {IN_JP, CPUProc::proc_jp},      
    {IN_DI, CPUProc::proc_di},
    {IN_JR, CPUProc::proc_jr},
    {IN_CALL, CPUProc::proc_call},
    {IN_XOR, CPUProc::proc_xor},
    {IN_POP, CPUProc::proc_pop},
    {IN_PUSH, CPUProc::proc_push}
};

CPUProc::IN_PROC CPUProc::inst_get_processor(in_type type) {
    auto it = processors.find(type);
    if (it != processors.end()) {
        return it->second;
    }// TODO Find something to return for not found case
    return nullptr;
}