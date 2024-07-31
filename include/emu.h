#pragma once

#include <common.h>
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

    CPU cpu;
    friend class CPU;

    public:
    /*
    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;
    */
    emu(int argc, char **argv); 
    int emu_run();
    void emu_cycles(int cpu_cycles);
};
// int emu_run(int argc, char **argv);

//get current emulator context
// emu_context *emu_get_context();

// void emu_cycles(int cpu_cycles);