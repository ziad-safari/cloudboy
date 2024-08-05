#include <cpu.h>
#include <bus.h>

void CPU::stack_push(u8 data) {
    regs.sp--;
    bus_write(*EMU, regs.sp,data);
}
void CPU::stack_push(u16 data) {
    stack_push((data >> 8) & 0xFF);
    stack_push(data & 0xFF);
}
u8 CPU::stack_pop() {
    return bus_read(*EMU,regs.sp++);
}
u16 CPU::stack_pop16() {
    u16 lo = stack_pop();
    u16 hi = stack_pop();
    return (hi << 8) | lo;
}