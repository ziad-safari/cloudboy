#include <cpu.h>
#include <bus.h>
#include <emu.h>

extern cpu_context ctx;

void fetch_data() {
    ctx.mem_dest = 0;
    ctx.dest_is_mem = false;
    
    if (ctx.cur_inst == NULL) {
        printf("Unknown Instruction! %02x\n", ctx.opcode);
        exit(-7);
    }

    switch(ctx.cur_inst->mode) {
        // Implied
        case AM_IMP: return;
        // Read Register 1
        case AM_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg_1);
            return;
        // Read Register 2
        case AM_R_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg_2);
            return;
        // Read Program Counter
        case AM_R_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;
        // Read virtual 16 bit address (AF, BC, DE)
        case AM_R_D16: 
         
        case AM_D16: {
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1);

            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            ctx.fetched_data = lo | (hi << 8);

            ctx.regs.pc += 2;

            return;
        }
        // Load a Register into a memory region i.e loading reg a into address of bc
        case AM_MR_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg_2);
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg_1);
            ctx.dest_is_mem = true; // flag

            if (ctx.cur_inst->reg_1 == RT_C) { // Special Case when writing to reg C
                ctx.mem_dest |= 0xFF00;
            }
            return;
        // Reading memory address to a register ie. loading address bc to reg a 
        case AM_R_MR: {
            u16 addr = cpu_read_reg(ctx.cur_inst->reg_2);

            if (ctx.cur_inst->reg_1 == RT_C) {
                addr |= 0xFF00;
            }

            ctx.fetched_data = bus_read(addr);
            emu_cycles(1);

        } return;
        // Load value into reg from the byte at HL and then increment HL
        case AM_R_HLI:
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.cur_inst->reg_2));
            emu_cycles(1);
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
            return;
        // Load value into reg from the byte at HL and then decrement HL
        case AM_R_HLD:
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.cur_inst->reg_2));
            emu_cycles(1);
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
            return;
        // Load value into reg from the byte pointed by HL and increment HL
        case AM_HLI_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg_2);
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg_1);
            ctx.dest_is_mem = true;
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
            break;
        // Load value into reg from the byte pointed by HL and decrement HL
        case AM_HLD_R:
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg_2);
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg_1);
            ctx.dest_is_mem = true;
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
            break;
        // Read data from program counter
        case AM_R_A8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;
        // Move from a register to A8
        case AM_A8_R:
            ctx.mem_dest = bus_read(ctx.regs.pc) | 0xFF00;
            ctx.dest_is_mem = true;        
            emu_cycles(1);
            ctx.regs.pc++;
            return;
        // Special case for loading stack pointer into HL incremented by R8
        case AM_HL_SPR:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;
        // read pc 
        case AM_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            return;
        // Moving a register into a 16bit address i.e reg a into address bc
        case AM_D16_R: {
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1);
            
            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            ctx.mem_dest = lo | (hi << 8);
            ctx.dest_is_mem = true;

            ctx.regs.pc += 2;
            ctx.fetched_data = cpu_read_reg(ctx.cur_inst->reg_2);
        }   return;
        // Loading value from program counter (and incrementing) into a register
        case AM_MR_D8:
            ctx.fetched_data = bus_read(ctx.regs.pc);
            emu_cycles(1);
            ctx.regs.pc++;
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg_1);
            ctx.dest_is_mem = true;
            return;
        //  Reading and writing a register
        case AM_MR:
            ctx.mem_dest = cpu_read_reg(ctx.cur_inst->reg_1);
            ctx.dest_is_mem = true;
            ctx.fetched_data = bus_read(cpu_read_reg(ctx.cur_inst->reg_1));
            emu_cycles(1);
            return;
        
        case AM_R_A16: { 
            u16 lo = bus_read(ctx.regs.pc);
            emu_cycles(1);
            
            u16 hi = bus_read(ctx.regs.pc + 1);
            emu_cycles(1);

            u16 addr = lo | (hi << 8);

            ctx.regs.pc += 2;
            ctx.fetched_data = bus_read(addr);
            emu_cycles(1);

        } return;
        default:
            printf("Unknown Addressing Mode! %d (%02X)\n", ctx.cur_inst->mode, ctx.opcode);
            exit(-7);
            return;
    }
}