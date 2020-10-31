#pragma once

typedef char da_elem_t;
#include "../libs/dynamic_array.h"

enum DisassemblerInitError
{
	DISASSEMBLER_INIT_NO_ERROR,
	DISASSEMBLER_INIT_NULL_PTR_PARAMETER,
	DISASSEMBLER_INIT_ARGS_EMPTY,
	DISASSEMBLER_INIT_DISASY_FILE_UNSPECIFIED,
	DISASSEMBLER_INIT_BCD_FILE_UNSPECIFIED,
	DISASSEMBLER_INIT_NOT_ENOUGH_MEMORY,
	DISASSEMBLER_INIT_BYTECODE_FILE_READ_ERROR,
	DISASSEMBLER_INIT_DISASSEMBLY_FILE_WRITE_ERROR
};

struct Disassembler
{
    unsigned char* bytecode        = NULL;
    size_t         bytecodeSize    = 0;
    DynamicArray*  disassembled    = NULL;
    FILE*          disassemblyFile = NULL;
};

DisassemblerInitError initDisassembler   (Disassembler* disassembler, int argc, char* argv[]);
void                  finishDisassembler (Disassembler* disassembler);
bool                  disassemble        (Disassembler* disassembler);