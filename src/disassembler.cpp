#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\libs\file_manager.h"

#include "cpu_specification.h"
#include "assembler_specification.h"
#include "disassembler_specification.h"

const char*  DEFAULT_DISASSEMBLY_FILE_NAME = "bin/disassembly.asy";
const size_t MAX_LITERAL_STR_LENGTH        = 32;

void pushBackStringToken(DynamicArray* arr, const char* token);

int main(int argc, char* argv[])
{
    Disassembler disassembler = {};

    DisassemblerInitError disasmInitError = initDisassembler(&disassembler, argc, argv);
    if (disasmInitError != DISASSEMBLER_INIT_NO_ERROR) {  return disasmInitError; } 

    bool isDisassembledSuccessfuly = disassemble(&disassembler);
    if (!isDisassembledSuccessfuly) { printf("Disassembler finished with an error.\n"); }

    finishDisassembler(&disassembler);

    return !isDisassembledSuccessfuly;
}

#define DISASM_INIT_ERROR(error) printf("Disassembler error: %s\n", #error); return error;

DisassemblerInitError initDisassembler(Disassembler* disassembler, int argc, char* argv[])
{
    if (disassembler == NULL) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_NULL_PTR_PARAMETER);   }
    if (argv         == NULL) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_ARGS_EMPTY);           }
    if (argc         <= 1   ) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_BCD_FILE_UNSPECIFIED); }

    const char* bytecodeFileName    = NULL;
    const char* disassemblyFileName = DEFAULT_DISASSEMBLY_FILE_NAME;
    bytecodeFileName = argv[1];
    if (argc >= 3)
    {
        disassemblyFileName = argv[2];
    }

    if (bytecodeFileName    == NULL) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_BCD_FILE_UNSPECIFIED); }
    if (disassemblyFileName == NULL) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_DISASSEMBLY_FILE_WRITE_ERROR); }

    disassembler->bytecodeSize = getFileSize(bytecodeFileName);
    disassembler->bytecode     = (unsigned char*) calloc(disassembler->bytecodeSize, sizeof(unsigned char));
    if (disassembler->bytecode == NULL) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_NOT_ENOUGH_MEMORY); }

    FILE* bytecodeFile = fopen(bytecodeFileName, "rb");
    if (bytecodeFile == NULL) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_BYTECODE_FILE_READ_ERROR); }

    if (fread(disassembler->bytecode, sizeof(unsigned char), disassembler->bytecodeSize, bytecodeFile) != disassembler->bytecodeSize) 
  	{ 
  		DISASM_INIT_ERROR(DISASSEMBLER_INIT_BYTECODE_FILE_READ_ERROR);
  	}

    fclose(bytecodeFile);

    disassembler->disassembled = newDynamicArray();
    if (disassembler->disassembled == NULL) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_NOT_ENOUGH_MEMORY); }

    disassembler->disassemblyFile = fopen(disassemblyFileName, "w");
    if (disassembler->disassemblyFile == NULL) { DISASM_INIT_ERROR(DISASSEMBLER_INIT_DISASSEMBLY_FILE_WRITE_ERROR); }

    return DISASSEMBLER_INIT_NO_ERROR;
}

void finishDisassembler(Disassembler* disassembler)
{
    assert(disassembler != NULL);

    if (disassembler->bytecode != NULL)
    {
        free(disassembler->bytecode);
        disassembler->bytecode = NULL;
    }

    disassembler->bytecodeSize = 0;

    if (disassembler->disassembled != NULL)
    {
        deleteDynamicArray(disassembler->disassembled);
        disassembler->disassembled = NULL;
    }

    if (disassembler->disassemblyFile != NULL)
    {
        fclose(disassembler->disassemblyFile);
        disassembler->disassemblyFile = NULL;
    }
}

bool disassemble(Disassembler* disassembler)
{
    assert(disassembler                  != NULL);
    assert(disassembler->bytecode        != NULL);
    assert(disassembler->disassembled    != NULL);
    assert(disassembler->disassemblyFile != NULL);

    size_t        numOfArgs = 0;
    unsigned char argType   = 0;
    double        temp      = 0;

    char* auxBuffer = (char*) calloc(MAX_LITERAL_STR_LENGTH, sizeof(char));
    if (auxBuffer == NULL) { printf("ERROR: Not enough RAM.\n"); return false; }

    for (unsigned char* currByte = disassembler->bytecode; currByte - disassembler->bytecode < disassembler->bytecodeSize;)
    {
   	    // command number
   	    if (*currByte >= CPU_COMMANDS_COUNT) { printf("ERROR: Invalid command number (%u).\n", *currByte); return false; }

   	    pushBackStringToken(disassembler->disassembled, assemblerCommands[*currByte]);

   	    #define DEFINE_CMD(name, number, args, isControlFlow, code) if (*currByte == number) { numOfArgs = args; }
   	    #include "cpu_commands.h"
   	    #undef DEFINE_CMD

        currByte++;
   	    for (size_t j = 0; j < numOfArgs; j++)
   	    {
            argType = *currByte;
            currByte++; 

            if (argType == 0) { printf("ERROR: Invalid argument type (%u).\n", argType); return false; }

            // ram specifier
            if ((argType & CPU_ARGUMENT_MASK_RAM) != 0)
            {
                pushBackStringToken(disassembler->disassembled, " [");
            }

            // register specifier
            if ((argType & CPU_ARGUMENT_MASK_REG) != 0)
            {
                pushBackStringToken(disassembler->disassembled, " r");
                pushBack(disassembler->disassembled, 'a' + *currByte - 1);
                pushBackStringToken(disassembler->disassembled, "x ");

                currByte++;
            }

            // both register and const specifier
            if ((argType & CPU_ARGUMENT_MASK_REG) != 0 && (argType & CPU_ARGUMENT_MASK_CST) != 0)
            {
                pushBackStringToken(disassembler->disassembled, " + ");
            }

            // const specifier
            if ((argType & CPU_ARGUMENT_MASK_CST) != 0)
            {

                temp = *((double*) currByte);
                sprintf(auxBuffer, " %lg ", temp);
                pushBackStringToken(disassembler->disassembled, auxBuffer);

                currByte += sizeof(temp);
            }

            // ram specifier
            if ((argType & CPU_ARGUMENT_MASK_RAM) != 0)
            {
                pushBackStringToken(disassembler->disassembled, " ] ");
            }
   	    }

   	    pushBack(disassembler->disassembled, '\n');
    }

    if (fwrite(disassembler->disassembled->data, 
   	           sizeof(char), 
   	           disassembler->disassembled->iteratorPos,  
   	           disassembler->disassemblyFile) != disassembler->disassembled->iteratorPos)
    {
        printf("Couldn't write to file.\n");
        return false;
    }

    free(auxBuffer);

    return true;
}

void pushBackStringToken(DynamicArray* arr, const char* token)
{
	assert(arr   != NULL);
	assert(token != NULL);

	for (size_t j = 0; token[j] != '\0'; j++)
    {
   	    pushBack(arr, token[j]);
    }
}
