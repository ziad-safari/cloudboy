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
static bool is_16_bit(reg_type rt) {
    return rt >= RT_AF;
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
reg_type rt_lookup[] = {
    RT_B,
    RT_C,
    RT_D,
    RT_E,
    RT_H,
    RT_L,
    RT_HL,
    RT_A
};

reg_type decode_reg(u8 reg) {
    if (reg > 0b111) {
        return RT_NONE;
    }

    return rt_lookup[reg];
}

void CPUProc::proc_cb() {
    u8 op = cpu.fetched_data;
    reg_type reg = decode_reg(op & 0b111);
    u8 bit = (op >> 3) & 0b111;
    u8 bit_op = (op >> 6) & 0b11;
    u8 reg_val = cpu.cpu_read_reg8(reg);

    cpu.EMU->emu_cycles(1);

    if (reg == RT_HL) {
        cpu.EMU->emu_cycles(2);
    }

    switch(bit_op) {
        case 1:
            //BIT
            cpu_set_flags(!(reg_val & (1 << bit)), 0, 1, -1);
            return;

        case 2:
            //RST
            reg_val &= ~(1 << bit);
            cpu.cpu_set_reg8(reg, reg_val);
            return;

        case 3:
            //SET
            reg_val |= (1 << bit);
            cpu.cpu_set_reg8(reg, reg_val);
            return;
    }

    bool flagC = cpu.CPU_FLAG_C();

    switch(bit) {
        case 0: {
            //RLC
            bool setC = false;
            u8 result = (reg_val << 1) & 0xFF;

            if ((reg_val & (1 << 7)) != 0) {
                result |= 1;
                setC = true;
            }

            cpu.cpu_set_reg8(reg, result);
            cpu_set_flags(result == 0, false, false, setC);
        } return;

        case 1: {
            //RRC
            u8 old = reg_val;
            reg_val >>= 1;
            reg_val |= (old << 7);

            cpu.cpu_set_reg8(reg, reg_val);
            cpu_set_flags(!reg_val, false, false, old & 1);
        } return;

        case 2: {
            //RL
            u8 old = reg_val;
            reg_val <<= 1;
            reg_val |= flagC;

            cpu.cpu_set_reg8(reg, reg_val);
            cpu_set_flags(!reg_val, false, false, !!(old & 0x80));
        } return;

        case 3: {
            //RR
            u8 old = reg_val;
            reg_val >>= 1;

            reg_val |= (flagC << 7);

            cpu.cpu_set_reg8(reg, reg_val);
            cpu_set_flags(!reg_val, false, false, old & 1);
        } return;

        case 4: {
            //SLA
            u8 old = reg_val;
            reg_val <<= 1;

            cpu.cpu_set_reg8(reg, reg_val);
            cpu_set_flags(!reg_val, false, false, !!(old & 0x80));
        } return;

        case 5: {
            //SRA
            u8 u = (int8_t)reg_val >> 1;
            cpu.cpu_set_reg8(reg, u);
            cpu_set_flags(!u, 0, 0, reg_val & 1);
        } return;

        case 6: {
            //SWAP
            reg_val = ((reg_val & 0xF0) >> 4) | ((reg_val & 0xF) << 4);
            cpu.cpu_set_reg8(reg, reg_val);
            cpu_set_flags(reg_val == 0, false, false, false);
        } return;

        case 7: {
            //SRL
            u8 u = reg_val >> 1;
            cpu.cpu_set_reg8(reg, u);
            cpu_set_flags(!u, 0, 0, reg_val & 1);
        } return;
    }

    fprintf(stderr, "ERROR: INVALID CB: %02X", op);
    NO_IMPL
}

void CPUProc::proc_and() {
    cpu.regs.a &= cpu.fetched_data;
    cpu_set_flags(cpu.regs.a == 0, 0, 1, 0);
}

void CPUProc::proc_xor() {
    cpu.regs.a ^= cpu.fetched_data & 0xFF;
    cpu_set_flags(cpu.regs.a == 0, 0, 0, 0);
}

void CPUProc::proc_or() {
    cpu.regs.a |= cpu.fetched_data & 0xFF;
    cpu_set_flags(cpu.regs.a == 0, 0, 0, 0);
}

void CPUProc::proc_cp() {
    int n = (int)cpu.regs.a - (int)cpu.fetched_data;

    cpu_set_flags(n == 0, 1, 
        ((int)cpu.regs.a & 0x0F) - ((int)cpu.fetched_data & 0x0F) < 0, n < 0);
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

void CPUProc::proc_inc() {
    u16 val = cpu.cpu_read_reg(cpu.cur_inst->reg_1) + 1;

    if (is_16_bit(cpu.cur_inst->reg_1)) {
        cpu.EMU->emu_cycles(1);
    }

    if (cpu.cur_inst->reg_1 == RT_HL && cpu.cur_inst->mode == AM_MR) {
        val = bus_read(*cpu.EMU, cpu.cpu_read_reg(RT_HL)) + 1;
        val &= 0xFF;
        bus_write(*cpu.EMU, cpu.cpu_read_reg(RT_HL), val);
    } else {
        cpu.cpu_set_reg(cpu.cur_inst->reg_1, val);
        val = cpu.cpu_read_reg(cpu.cur_inst->reg_1);
    }

    if ((cpu.opcode & 0x03) == 0x03) {
        return;
    }

    cpu_set_flags(val == 0, 0, (val & 0x0F) == 0, -1);
}

void CPUProc::proc_dec() {
    u16 val = cpu.cpu_read_reg(cpu.cur_inst->reg_1) - 1;

    if (is_16_bit(cpu.cur_inst->reg_1)) {
        cpu.EMU->emu_cycles(1);
    }

    if (cpu.cur_inst->reg_1 == RT_HL && cpu.cur_inst->mode == AM_MR) {
        val = bus_read(*cpu.EMU, cpu.cpu_read_reg(RT_HL)) - 1;
        bus_write(*cpu.EMU, cpu.cpu_read_reg(RT_HL), val);
    } else {
        cpu.cpu_set_reg(cpu.cur_inst->reg_1, val);
        val = cpu.cpu_read_reg(cpu.cur_inst->reg_1);
    }

    if ((cpu.opcode & 0x0B) == 0x0B) {
        return;
    }

    cpu_set_flags(val == 0, 1, (val & 0x0F) == 0x0F, -1);
}

void CPUProc::proc_sub() {
    u16 val = cpu.cpu_read_reg(cpu.cur_inst->reg_1) - cpu.fetched_data;

    int z = val == 0;
    int h = ((int)cpu.cpu_read_reg(cpu.cur_inst->reg_1) & 0xF) - ((int)cpu.fetched_data & 0xF) < 0;
    int c = ((int)cpu.cpu_read_reg(cpu.cur_inst->reg_1)) - ((int)cpu.fetched_data) < 0;

    cpu.cpu_set_reg(cpu.cur_inst->reg_1, val);
    cpu_set_flags(z, 1, h, c);
}

void CPUProc::proc_sbc() {
    u8 val = cpu.fetched_data + cpu.CPU_FLAG_C();

    int z = cpu.cpu_read_reg(cpu.cur_inst->reg_1) - val == 0;

    int h = ((int)cpu.cpu_read_reg(cpu.cur_inst->reg_1) & 0xF) 
        - ((int)cpu.fetched_data & 0xF) - ((int)cpu.CPU_FLAG_C()) < 0;
    int c = ((int)cpu.cpu_read_reg(cpu.cur_inst->reg_1)) 
        - ((int)cpu.fetched_data) - ((int)cpu.CPU_FLAG_C()) < 0;

    cpu.cpu_set_reg(cpu.cur_inst->reg_1, cpu.cpu_read_reg(cpu.cur_inst->reg_1) - val);
    cpu_set_flags(z, 1, h, c);
}

void CPUProc::proc_adc() {
    u16 u = cpu.fetched_data;
    u16 a = cpu.regs.a;
    u16 c = cpu.CPU_FLAG_C();

    cpu.regs.a = (a + u + c) & 0xFF;

    cpu_set_flags(cpu.regs.a == 0, 0, 
        (a & 0xF) + (u & 0xF) + c > 0xF,
        a + u + c > 0xFF);
}

void CPUProc::proc_add() {
    u32 val = cpu.cpu_read_reg(cpu.cur_inst->reg_1) + cpu.fetched_data;

    bool is_16bit = is_16_bit(cpu.cur_inst->reg_1);

    if (is_16bit) {
        cpu.EMU->emu_cycles(1);
    }

    if (cpu.cur_inst->reg_1 == RT_SP) {
        val = cpu.cpu_read_reg(cpu.cur_inst->reg_1) + (int8_t)cpu.fetched_data;
    }

    int z = (val & 0xFF) == 0;
    int h = (cpu.cpu_read_reg(cpu.cur_inst->reg_1) & 0xF) + (cpu.fetched_data & 0xF) >= 0x10;
    int c = (int)(cpu.cpu_read_reg(cpu.cur_inst->reg_1) & 0xFF) + (int)(cpu.fetched_data & 0xFF) >= 0x100;

    if (is_16bit) {
        z = -1;
        h = (cpu.cpu_read_reg(cpu.cur_inst->reg_1) & 0xFFF) + (cpu.fetched_data & 0xFFF) >= 0x1000;
        u32 n = ((u32)cpu.cpu_read_reg(cpu.cur_inst->reg_1)) + ((u32)cpu.fetched_data);
        c = n >= 0x10000;
    }

    if (cpu.cur_inst->reg_1 == RT_SP) {
        z = 0;
        h = (cpu.cpu_read_reg(cpu.cur_inst->reg_1) & 0xF) + (cpu.fetched_data & 0xF) >= 0x10;
        c = (int)(cpu.cpu_read_reg(cpu.cur_inst->reg_1) & 0xFF) + (int)(cpu.fetched_data & 0xFF) > 0x100;
    }

    cpu.cpu_set_reg(cpu.cur_inst->reg_1, val & 0xFFFF);
    cpu_set_flags(z, 0, h, c);
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
    {IN_POP, CPUProc::proc_pop},
    {IN_PUSH, CPUProc::proc_push},
    {IN_RET, CPUProc::proc_ret},
    {IN_RST, CPUProc::proc_rst},
    {IN_DEC, CPUProc::proc_dec},
    {IN_INC, CPUProc::proc_inc},
    {IN_ADD, CPUProc::proc_add},
    {IN_ADC, CPUProc::proc_adc},
    {IN_SUB, CPUProc::proc_sub},
    {IN_SBC, CPUProc::proc_sbc},
    {IN_AND, CPUProc::proc_and},
    {IN_XOR, CPUProc::proc_xor},
    {IN_OR, CPUProc::proc_or},
    {IN_CP, CPUProc::proc_cp},
    {IN_CB, CPUProc::proc_cb},
    {IN_RETI, CPUProc::proc_reti}
};

CPUProc::IN_PROC CPUProc::inst_get_processor(in_type type) {
    auto it = processors.find(type);
    if (it != processors.end()) {
        return it->second;
    }// TODO Find something to return for not found case
    return nullptr;
}