#pragma once
#include <stdlib.h>
#include "display.h"

typedef double stk_elem_t;
#include "../libs/stack.h"

enum CpuError
{
    CPU_NO_ERROR,               
    CPU_INVALID_EXECUTABLE,
    CPU_NOT_ENOUGH_VALUES_FOR_OPERATION,
    CPU_MATH_ERROR,
    CPU_IO_ERROR,
    CPU_INVALID_CMD_ARGUMENT,
    CPU_REACHED_PROGRAM_END_NOT_HALTED,
    CPU_INVALID_COMMAND
};

enum CpuInitError
{
    CPU_INIT_NO_ERROR,
    CPU_INIT_NULL_PTR_PARAMETER,
    CPU_INIT_ARGS_EMPTY,
    CPU_INIT_BCD_FILE_UNSPECIFIED,
    CPU_INIT_BYTECODE_FILE_READ_ERROR,
    CPU_INIT_RAM_NOT_ENOUGH_MEMORY
};

enum CpuArgumentMasks
{
    CPU_ARGUMENT_MASK_CST = 1 << 0,
    CPU_ARGUMENT_MASK_REG = 1 << 1,
    CPU_ARGUMENT_MASK_RAM = 1 << 2
};

enum CpuArgumentType
{
    CPU_ARGUMENT_TYPE_CST              = CPU_ARGUMENT_MASK_CST,
    CPU_ARGUMENT_TYPE_REG              = CPU_ARGUMENT_MASK_REG,
    CPU_ARGUMENT_TYPE_REG_PLUS_CST     = CPU_ARGUMENT_MASK_REG | CPU_ARGUMENT_MASK_CST,
    CPU_ARGUMENT_TYPE_RAM_CST          = CPU_ARGUMENT_MASK_RAM | CPU_ARGUMENT_MASK_CST,
    CPU_ARGUMENT_TYPE_RAM_REG          = CPU_ARGUMENT_MASK_RAM | CPU_ARGUMENT_MASK_REG,
    CPU_ARGUMENT_TYPE_RAM_REG_PLUS_CST = CPU_ARGUMENT_MASK_RAM | CPU_ARGUMENT_MASK_REG | CPU_ARGUMENT_MASK_CST
};

enum CpuCommands
{
    #define DEFINE_CMD(name, number, args, isControlFlow, code) \
        CPU_CMD_##name = number,

    #include "cpu_commands.h"
    #undef DEFINE_CMD   
};

#define DEFINE_CMD(name, number, args, isControlFlow, code) +1
static const size_t CPU_COMMANDS_COUNT = 0 
                                         #include "cpu_commands.h"
                                         ;
#undef DEFINE_CMD

static const size_t CPU_REGISTERS_COUNT = 18;
static const size_t VRAM_START_INDEX    = 1024;
static const size_t VRAM_SIZE           = DISPLAY_DEFAULT_WIDTH * DISPLAY_DEFAULT_HEIGHT * 4;
static const size_t CPU_RAM_SIZE        = sizeof(double) * VRAM_START_INDEX + VRAM_SIZE;

struct RAM
{
    size_t         size     = 0;
    double*        cells    = NULL;
    unsigned char* vram     = NULL;
    size_t         vramSize = 0;
};

struct CPU
{
    CpuError status = CPU_NO_ERROR;

    Stack    stack        = {};
    Stack    callStack    = {};
    char*    program      = NULL;
    size_t   programBytes = 0;
    size_t   pc           = 0;
    bool     halt         = false;
    RAM      ram          = {};
    Display* display      = NULL;
    double   regs[CPU_REGISTERS_COUNT] = {};
};

CpuInitError initCpu        (CPU* cpu, int argc, char* argv[]);
void         deleteCpu      (CPU* cpu);
void         cpuSetError    (CPU* cpu, CpuError error);
CpuError     executeProgram (CPU* cpu);
void         dump           (CPU* cpu);

