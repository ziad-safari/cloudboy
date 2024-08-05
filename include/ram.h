#pragma once

#include <common.h>
// #include <bus.h>

class emu;
class Ram {
    u8 wram[0x2000];
    u8 hram[0x80];
    emu* EMU;
public: 
    Ram(emu* myemu);
    u8 wram_read(u16 address);
    void wram_write(u16 address, u8 value);

    u8 hram_read(u16 address);
    void hram_write(u16 address, u8 value);
    // friend u8 bus_read(u16 address);
    // friend void bus_write(u16 address, u8 value);
    // friend u16 bus_read16(u16 address);
    // friend void bus_write16(u16 address, u16 value);
};

