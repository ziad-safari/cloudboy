#pragma once

#include <common.h>

//cartridge from 0x100 to 0x14F
typedef struct {
    u8 entry[4];
    u8 logo[0x30];

    char title[16];
    u16 new_lic_code;
    u8 sgb_flag;
    u8 type;
    u8 rom_size;
    u8 ram_size;
    u8 dest_code;
    u8 lic_code;
    u8 version;
    u8 checksum;
    u16 global_checksum;
} rom_header;

bool cart_load(char *cart);

u8 cart_read(u16 address);
void cart_write(u16 address, u8 value);


/*
Memory Map:
0x0000 - 0x3FFF : ROM Bank 0
0x4000 - 0x7FFF : ROM Bank 1 - Switchable
0x8000 - 0x97FF : CHR RAM
0x9800 - 0x9BFF : BG Map 1
0x9C00 - 0x9FFF : BG Map 2
0xA000 - 0xBFFF : Cartridge RAM
0xC000 - 0xCFFF : RAM Bank 0
0xD000 - 0xDFFF : RAM Bank 1-7 - switchable - Color only
0xE000 - 0xFDFF : Reserved - Echo RAM
0xFE00 - 0xFE9F : Object Attribute Memory
0xFEA0 - 0xFEFF : Reserved - Unusable
0xFF00 - 0xFF7F : I/O Registers
0xFF80 - 0xFFFE : Zero Page

*/

//16 bit address bus returns 8 bit data
u8 bus_read(u16 address);
void bus_write(u16 address, u8 value);

