#include <cpu.h>

u16 reverse(u16 n) {
    return ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
}

u16 CPU::cpu_read_reg(reg_type rt) const {
    switch(rt) {
        case RT_A: return regs.a;
        case RT_F: return regs.f;
        case RT_B: return regs.b;
        case RT_C: return regs.c;
        case RT_D: return regs.d;
        case RT_E: return regs.e;
        case RT_H: return regs.h;
        case RT_L: return regs.l;

        case RT_AF: return reverse(*((u16 *)&regs.a));
        case RT_BC: return reverse(*((u16 *)&regs.b));
        case RT_DE: return reverse(*((u16 *)&regs.d));
        case RT_HL: return reverse(*((u16 *)&regs.h));

        case RT_PC: return regs.pc;
        case RT_SP: return regs.sp;
        default: return 0;
    }
}

void CPU::cpu_set_reg(reg_type rt, u16 val) {
    switch(rt) {
        case RT_A: regs.a = val & 0xFF; break;
        case RT_F: regs.f = val & 0xFF; break;
        case RT_B: regs.b = val & 0xFF; break;
        case RT_C: {
             regs.c = val & 0xFF;
        } break;
        case RT_D: regs.d = val & 0xFF; break;
        case RT_E: regs.e = val & 0xFF; break;
        case RT_H: regs.h = val & 0xFF; break;
        case RT_L: regs.l = val & 0xFF; break;

        case RT_AF: *((u16 *)&regs.a) = reverse(val); break;
        case RT_BC: *((u16 *)&regs.b) = reverse(val); break;
        case RT_DE: *((u16 *)&regs.d) = reverse(val); break;
        case RT_HL: {
         *((u16 *)&regs.h) = reverse(val); 
         break;
        }

        case RT_PC: regs.pc = val; break;
        case RT_SP: regs.sp = val; break;
        case RT_NONE: break;
    }
}