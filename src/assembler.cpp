#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileManager.h"
#include "softwareCpuSpecification.h"
#include "assemblySpecification.h"
typedef char da_elem_t;
#include "dynamicArray.h"

const char*  DEFAULT_BYTECODE_FILE_NAME = "bytecode.bcd";
const size_t MAX_LINE_LENGTH            = 64;

struct Assembler
{
    Text*         assembly     = NULL;
    FILE*         bytecodeFile = NULL;
    DynamicArray* bytecode     = NULL;
};

bool                 processTerminalFlags  (Assembler* assembler, int argc, char* argv[]);
Assembler*           initAssembler         (Assembler* assembler, const char* assemblyFileName, const char* bytecodeFileName);
void                 finishAssembler       (Assembler* assembler);
bool                 translateAssemblyFile (Assembler* assembler);
void                 translateAssemblyLine (Assembler* assembler, const char* line);
CpuOperator          getOperatorNumber     (const char* operatorString);
CpuRegisterIndicator getRegisterIndicator  (const char* registerString);


int main(int argc, char* argv[])
{
    Assembler assembler = {};

    if (processTerminalFlags(&assembler, argc, argv) == false) { return -1; }

    translateAssemblyFile(&assembler);

    return 0;
}

bool processTerminalFlags(Assembler* assembler, int argc, char* argv[])
{
    assert(argv != NULL);

    if (argc <= 1)
    {
        printf("ERROR: assembly file unspecified\n");
        return false;
    }

    const char* bytecodeFileName = DEFAULT_BYTECODE_FILE_NAME;
    if (argc == 3)
    {
        bytecodeFileName = argv[2];
    }

    return initAssembler(assembler, argv[1], bytecodeFileName) != NULL;
}

Assembler* initAssembler(Assembler* assembler, const char* assemblyFileName, const char* bytecodeFileName)
{
    assert(assembler        != NULL);
    assert(assemblyFileName != NULL);
    assert(bytecodeFileName != NULL);

    assembler->assembly = readTextFromFile(assemblyFileName);
    if (assembler->assembly == NULL)     { return NULL; }
    
    assembler->bytecodeFile = fopen(bytecodeFileName, "wb");
    if (assembler->bytecodeFile == NULL) { return NULL; }

    assembler->bytecode = newDynamicArray();
    if (assembler->bytecode == NULL)     { return NULL; }

    return assembler;
}

void finishAssembler(Assembler* assembler)
{
    assert(assembler != NULL);

    if (assembler->assembly != NULL)
    {
        deleteText(assembler->assembly);
        assembler->assembly = NULL;
    }

    if (assembler->bytecodeFile != NULL)
    {
        fclose(assembler->bytecodeFile);
        assembler->bytecodeFile = NULL;
    }

    if (assembler->bytecode != NULL)
    {
        deleteDynamicArray(assembler->bytecode);
        assembler->bytecode = NULL;
    }
}

bool translateAssemblyFile(Assembler* assembler)
{
    assert(assembler != NULL);

    const char* currLine = NULL;
    while ((currLine = nextTextLine(assembler->assembly)) != NULL)
    {
        translateAssemblyLine(assembler, currLine);
    }

    size_t numOfBytesToWrite = assembler->bytecode->iteratorPos;
    if (fwrite(assembler->bytecode->data, sizeof(char), numOfBytesToWrite, assembler->bytecodeFile) != numOfBytesToWrite)
    {
        return false;
    }

    return true;
}

void translateAssemblyLine(Assembler* assembler, const char* line)
{
    assert(assembler != NULL);

    //const char* currLine     = nextTextLine(assembler->assembly);
    const char* commentStart = strchr(line, ';');

    const char* currToken = strtok((char*) line, " ;");
    if (currToken == NULL || (commentStart != NULL && currToken >= commentStart)) { return; }

    CpuOperator cpuOperator = getOperatorNumber(currToken);
    pushBack(assembler->bytecode, (char) cpuOperator);
    double value = 0;
    if (commentStart != NULL && (currToken = strtok(NULL, " ;")) > commentStart) 
    { 
        if (cpuOperator == PUSH)
        {
            if (sscanf(currToken, "%lg", &value) == 1)
            {
                pushBack(assembler->bytecode, &value, sizeof(value));
            }
            else
            {
                pushBack(assembler->bytecode, getRegisterIndicator(currToken));
            }
        }
    }
    else
    {
        while ((currToken = strtok(NULL, " ;")) != NULL)
        {
            if (commentStart != NULL && currToken > commentStart) { break; }

            sscanf(currToken, "%lg", &value);
            pushBack(assembler->bytecode, &value, sizeof(value));
        }
    }
}

CpuOperator getOperatorNumber(const char* operatorString)
{
    assert(operatorString != NULL);

    for (size_t i = 0; i < CPU_OPERATORS_COUNT; i++)
    {
        if (strcmp(operatorString, ASSEMBLY_OPERATORS[i]) == 0)
        {
            return (CpuOperator) i;
        }
    }

    return HALT;
}

CpuRegisterIndicator getRegisterIndicator(const char* registerString)
{
    assert(registerString != NULL);

    switch(registerString[1])
    {
        case 'a':
            return RAX_INDICATOR;

        case 'b':
            return RBX_INDICATOR;

        case 'c':
            return RCX_INDICATOR;

        case 'd':
            return RDX_INDICATOR;
    }
}