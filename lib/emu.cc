#include <stdio.h>
#include <emu.h>
#include <cart.h>
#include <cpu.h>
//sdl for graphics library
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

//address bus maps address to cartridge

static emu_context ctx;

emu_context *emu_get_context() {
    return &ctx;
}

void delay(u32 ms) {
    SDL_Delay(ms);
}

int emu_run(int argc, char **argv) {
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
    CPU cpu;
    
    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;

    //game loop
    while(ctx.running) {
        if (ctx.paused) {
            //delay 10ms
            delay(10);
            continue;
        }
        
        //1 step of cpu, terminate if it fails
        if (!cpu.step()) {
            printf("CPU Stopped\n");
            return -3;
        }

        ctx.ticks++;
    }
    return 0;
}

void emu_cycles(int cpu_cycles) {
    //TODO... Will eventually use to synchronize the Timer with the CPU and the PPU
}
