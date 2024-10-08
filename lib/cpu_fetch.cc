#include <cpu.h>
#include <cart.h>
#include <emu.h>
#include <bus.h>

void CPU::fetch_data() {
    mem_dest = 0;
    dest_is_mem = false;
    
    if (cur_inst == NULL) {
        printf("Unknown Instruction! %02x\n", opcode);
        exit(-7);
    }

    switch(cur_inst->mode) {
        // Implied
        case AM_IMP: return;
        // Read Register 1
        case AM_R:
            fetched_data = cpu_read_reg(cur_inst->reg_1);
            return;
        // Read Register 2
        case AM_R_R:
            fetched_data = cpu_read_reg(cur_inst->reg_2);
            return;
        // Read Program Counter
        //Direct Load 8 bit value
        case AM_R_D8:
            fetched_data = bus_read(*EMU, regs.pc);
            EMU->emu_cycles(1);
            regs.pc++;
            return;
        // Read virtual 16 bit address (AF, BC, DE)
        case AM_R_D16: 
            
        case AM_D16: {
            u16 lo = bus_read(*EMU, regs.pc);
            EMU->emu_cycles(1);

            u16 hi = bus_read(*EMU, regs.pc + 1);
            EMU->emu_cycles(1);

            fetched_data = lo | (hi << 8);

            regs.pc += 2;

            return;
        }
        // Load a Register into a memory region i.e loading reg a into address of bc
        case AM_MR_R:
            fetched_data = cpu_read_reg(cur_inst->reg_2);
            mem_dest = cpu_read_reg(cur_inst->reg_1);
            dest_is_mem = true; // flag

            if (cur_inst->reg_1 == RT_C) { // Special Case when writing to reg C
                mem_dest |= 0xFF00; //this is for I/O ports since 0XFF00 - 0xFF7F is mem mapped I/O
            }
            return;
        // Reading memory address to a register ie. loading address bc to reg a 
        case AM_R_MR: {
            u16 addr = cpu_read_reg(cur_inst->reg_2);

            if (cur_inst->reg_1 == RT_C) {
                addr |= 0xFF00;
            }

            fetched_data = bus_read(*EMU, addr);
            EMU->emu_cycles(1);

        } return;
        // Load value into reg from the byte at HL and then increment HL
        case AM_R_HLI:
            fetched_data = bus_read(*EMU, cpu_read_reg(cur_inst->reg_2));
            EMU->emu_cycles(1);
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
            return;
        // Load value into reg from the byte at HL and then decrement HL
        case AM_R_HLD:
            fetched_data = bus_read(*EMU, cpu_read_reg(cur_inst->reg_2));
            EMU->emu_cycles(1);
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
            return;
        // Load value into reg from the byte pointed by HL and increment HL
        case AM_HLI_R:
            fetched_data = cpu_read_reg(cur_inst->reg_2);
            mem_dest = cpu_read_reg(cur_inst->reg_1);
            dest_is_mem = true;
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) + 1);
            break;
        // Load value into reg from the byte pointed by HL and decrement HL
        case AM_HLD_R:
            fetched_data = cpu_read_reg(cur_inst->reg_2);
            mem_dest = cpu_read_reg(cur_inst->reg_1);
            dest_is_mem = true;
            cpu_set_reg(RT_HL, cpu_read_reg(RT_HL) - 1);
            break;
        // Read data from program counter
        case AM_R_A8:
            fetched_data = bus_read(*EMU, regs.pc);
            EMU->emu_cycles(1);
            regs.pc++;
            return;
        // Move from a register to A8
        case AM_A8_R:
            mem_dest = bus_read(*EMU, regs.pc) | 0xFF00;
            dest_is_mem = true;        
            EMU->emu_cycles(1);
            regs.pc++;
            return;
        // Special case for loading stack pointer into HL incremented by R8
        case AM_HL_SPR:
            fetched_data = bus_read(*EMU, regs.pc);
            EMU->emu_cycles(1);
            regs.pc++;
            return;
        // read pc 
        case AM_D8:
            fetched_data = bus_read(*EMU, regs.pc);
            EMU->emu_cycles(1);
            regs.pc++;
            return;
        // Temp no IMPL
        case AM_A16_R:
        // Moving a register into a 16bit address i.e reg a into address bc
        case AM_D16_R: {
            u16 lo = bus_read(*EMU, regs.pc);
            EMU->emu_cycles(1);
            
            u16 hi = bus_read(*EMU, regs.pc + 1);
            EMU->emu_cycles(1);

            mem_dest = lo | (hi << 8);
            dest_is_mem = true;

            regs.pc += 2;
            fetched_data = cpu_read_reg(cur_inst->reg_2);
        }   return;
        // Loading value from program counter (and incrementing) into a register
        case AM_MR_D8:
            fetched_data = bus_read(*EMU, regs.pc);
            EMU->emu_cycles(1);
            regs.pc++;
            mem_dest = cpu_read_reg(cur_inst->reg_1);
            dest_is_mem = true;
            return;
        //  Reading and writing a register
        case AM_MR:
            mem_dest = cpu_read_reg(cur_inst->reg_1);
            dest_is_mem = true;
            fetched_data = bus_read(*EMU, cpu_read_reg(cur_inst->reg_1));
            EMU->emu_cycles(1);
            return;
        
        case AM_R_A16: { 
            u16 lo = bus_read(*EMU, regs.pc);
            EMU->emu_cycles(1);
            
            u16 hi = bus_read(*EMU, regs.pc + 1);
            EMU->emu_cycles(1);

            u16 addr = lo | (hi << 8);

            regs.pc += 2;
            fetched_data = bus_read(*EMU, addr);
            EMU->emu_cycles(1);
        } return;
        default:
            printf("Unknown Addressing Mode! %d (%02X)\n", cur_inst->mode, opcode);
            exit(-7);
            return;
    }
}