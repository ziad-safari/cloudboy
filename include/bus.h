#pragma once

#include <common.h>
// #include <emu.h>
class emu;
// //16 bit address bus returns 8 bit data
    u8 bus_read(emu &EMU, u16 address);
    void bus_write(emu &EMU, u16 address, u8 value);
    u16 bus_read16(emu &EMU, u16 address);
    void bus_write16(emu &EMU, u16 address, u16 value);