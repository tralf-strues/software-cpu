#pragma once
#include "../libs/file_manager.h"

typedef char da_elem_t;
#include "../libs/dynamic_array.h"

enum AssemblerInitError
{
	ASSEMBLER_INIT_NO_ERROR,
	ASSEMBLER_INIT_NULL_PTR_PARAMETER,
	ASSEMBLER_INIT_ARGS_EMPTY,
	ASSEMBLER_INIT_ASY_FILE_UNSPECIFIED,
	ASSEMBLER_INIT_BCD_FILE_UNSPECIFIED,
	ASSEMBLER_INIT_NOT_ENOUGH_MEMORY,
	ASSEMBLER_INIT_ASSEMBLY_FILE_READ_ERROR,
	ASSEMBLER_INIT_BYTECODE_FILE_WRITE_ERROR
};

const char* assemblerCommands[] = { 
									#define DEFINE_CMD(name, number, args, isControlFlow, code) #name,
									#include "cpu_commands.h"
									#undef DEFINE_CMD	
                                  };

struct Label
{
	char*  name  = NULL;
	double value = 0;
};

#undef DYNAMIC_ARRAY_H
#include "label_array.h"

struct Assembler
{
    Text*         assembly     = NULL;
    FILE*         bytecodeFile = NULL;
    DynamicArray* bytecode     = NULL;
    LabelArray*   labels       = NULL;
};

AssemblerInitError initAssembler         (Assembler* assembler, int argc, char* argv[]);
void               finishAssembler       (Assembler* assembler);
bool               translateAssemblyFile (Assembler* assembler);