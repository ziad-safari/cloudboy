#include <instructions.h>
#include <cpu.h>

//this is an array of size 0x100 = 256 (16*16)
instruction instructions[0x100] = {
    //NO OP
    [0x00] = {IN_NOP,AM_IMP},
    //d8 -> register Decrement Reg B
    [0x05] = {IN_DEC,AM_R, RT_B},
    // Load Reg C
    [0x0E] = {IN_LD, AM_R_D8, RT_C},
    //XOR of A
    [0XAF] = {IN_XOR, AM_R, RT_A},
    //jump instr

    [0xC3] = {IN_JP,AM_D16},
    [0xF3] = {IN_DI},
};
//0x00
//instruction[0x00]
instruction *instruction_by_opcode(u8 opcode) {
    if (instructions[opcode].type == IN_NONE) {
        return NULL;
    }
    return &instructions[opcode];
}
char *inst_lookup[] = {
    "<NONE>",
    "NOP",
    "LD",
    "INC",
    "DEC",
    "RLCA",
    "ADD",
    "RRCA",
    "STOP",
    "RLA",
    "JR",
    "RRA",
    "DAA",
    "CPL",
    "SCF",
    "CCF",
    "HALT",
    "ADC",
    "SUB",
    "SBC",
    "AND",
    "XOR",
    "OR",
    "CP",
    "POP",
    "JP",
    "PUSH",
    "RET",
    "CB",
    "CALL",
    "RETI",
    "LDH",
    "JPHL",
    "DI",
    "EI",
    "RST",
    "IN_ERR",
    "IN_RLC", 
    "IN_RRC",
    "IN_RL", 
    "IN_RR",
    "IN_SLA", 
    "IN_SRA",
    "IN_SWAP", 
    "IN_SRL",
    "IN_BIT", 
    "IN_RES", 
    "IN_SET"
};
char *inst_name(in_type t) {
    return inst_lookup[t];
}