#include <stdio.h>
#include <emu.h>
#include <cart.h>
#include <cpu.h>
//sdl for graphics library
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

//address bus maps address to cartridge

// static emu_context ctx;

// emu_context *emu_get_context() {
//     return &ctx;
// }

void delay(u32 ms) {
    SDL_Delay(ms);
}
emu::emu(int argc, char **argv) 
: argc{argc}, argv{argv}, running{true}, paused{false}, ticks{0}, cpu{new CPU(this)}  {}

int emu::emu_run() {
    //no rom file
    if (argc < 2) {
        printf("Usage: emu <rom_file>\n");
        return -1;
    }

    //failed to load rom file
    if (!cart_load(argv[1])) {
        printf("Failed to load ROM file: %s\n", argv[1]);
        return -2;
    }

    printf("Cart loaded..\n");

    SDL_Init(SDL_INIT_VIDEO);
    printf("SDL INIT\n");
    //true type fonts
    TTF_Init();
    printf("TTF INIT\n");
    
    // Initialize the CPU 
    // CPU cpu;
    // ctx.running = true;
    // ctx.paused = false;
    // ctx.ticks = 0;

    //game loop
    while(running) {
        if (paused) {
            //delay 10ms
            delay(10);
            continue;
        }
        
        //1 step of cpu, terminate if it fails
        if (!cpu->step()) {
            printf("CPU Stopped\n");
            return -3;
        }

        ticks++;
    }
    delete cpu;
    return 0;
}

void emu::emu_cycles(int cpu_cycles) {
    //TODO... Will eventually use to synchronize the Timer with the CPU and the PPU
}
