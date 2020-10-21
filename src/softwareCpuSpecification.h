#pragma once
#include <stdlib.h>
typedef double stk_elem_t;
#include "stack.h"

const size_t CPU_OPERATORS_COUNT = 13;

enum CpuOperator
{
    // i/o
    IN = 0x00000000,
    OUT,

    // math
    ADD,
    SUB,
    MUL,
    DIV,
         
    POW,
    SQRT,

    SIN,
    COS,

    // stack
    PUSH,
    POP,

    // program-flow
    HALT
};

enum CpuOperatorType
{
    PUSH_TO_STACK,
    PUSH_TO_REGISTER
};

enum CpuRegisterIndicator
{
    RAX_INDICATOR = 0,
    RBX_INDICATOR = 1,
    RCX_INDICATOR = 2,
    RDX_INDICATOR = 3,
};

enum CpuError
{
    CPU_NO_ERROR,
    CPU_INVALID_INPUT_FILE,
    CPU_NOT_ENOUGH_VALUES_FOR_OPERATION,
    CPU_MATH_ERROR
};

struct CPU
{
    CpuError status = CPU_NO_ERROR;

    Stack*   stack  = NULL;
    char*    code   = NULL;
    size_t   pc     = 0;

    double   rax    = 0;
    double   rbx    = 0;
    double   rcx    = 0;
    double   rdx    = 0;
};

