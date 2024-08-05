#pragma once

#include <common.h>
#include <bus.h>
#include <ram.h>
#include <cpu.h>
typedef struct {
    bool paused;
    bool running;
    u64 ticks;
} emu_context;
class emu {
    private:
    bool paused;
    bool running;
    u64 ticks;
    int argc;
    char **argv;

    CPU* cpu;
    Ram* ram;
    // friend class CPU;

    public:
    /*
    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;
    */
    emu(int argc, char **argv); 
    int emu_run();
    void emu_cycles(int cpu_cycles);
    friend u8 bus_read(emu &EMU, u16 address);
    friend void bus_write(emu &EMU, u16 address, u8 value);
    friend u16 bus_read16(emu &EMU, u16 address);
    friend void bus_write16(emu &EMU, u16 address, u16 value);
};
// int emu_run(int argc, char **argv);

//get current emulator context
// emu_context *emu_get_context();

// void emu_cycles(int cpu_cycles);