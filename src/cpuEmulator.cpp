#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "softwareCpuSpecification.h"

struct FileHeader
{
    short  signature = 0;
    short  version   = 0;
    size_t fileSize  = 0;
};

CpuError executeProgram     (CPU* cpu);
CpuError executeNextCommand (CPU* cpu);

int main(int argc, char* argv[])
{
    if (argc == 1 || (bytecodeFile = fopen(argv[1], "r")) == NULL) return CPU_INVALID_INPUT_FILE;

    stack = newStack();

    CpuError executionResult = executeProgram();

    deleteStack(stack);
    fclose(bytecodeFile);

    printf("\nexited with code %d\n", executionResult);

    return executionResult;
}

CpuError executeProgram(CPU* cpu)
{
    char* currLine = (char*) calloc(MAX_LINE_LENGTH, sizeof(char));
    assert(currLine != NULL);

    CpuError lineExecutionResult = CPU_NO_ERROR;
    while (haltReached == false && lineExecutionResult == CPU_NO_ERROR && fgets(currLine, MAX_LINE_LENGTH, bytecodeFile) != NULL)
    {
        lineExecutionResult = executeNextCommand(currLine);
    }

    free(currLine);

    return lineExecutionResult;
}

CpuError executeNextCommand(CPU* cpu)
{
    assert(line != NULL);
    
    if (line[0] == '\n') { return CPU_NO_ERROR; }
    
    const char* currToken = strtok((char*) line, " \n");
    if (line[0] == '\n' || currToken == NULL) { return CPU_NO_ERROR; }
    
    CpuOperator cpuOperator = (CpuOperator) strtol(currToken, NULL, 16);
    
    double register1 = 0;
    double register2 = 0;
    switch (cpuOperator)
    {
        case IN:
        {
            scanf("%lf", &register1);
            stackPush(stack, register1);
            break;
        }
    
        case OUT:
        {
            if (stack->size == 0) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            printf("%lf\n", stackPop(stack));
            break;
        }

        case ADD:
        {
            if (stack->size < 2) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            stackPush(stack, stackPop(stack) + stackPop(stack));
            break;
        }

        case SUB:
        {
            if (stack->size < 2) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            stackPush(stack, -stackPop(stack) + stackPop(stack));
            break;
        }

        case MUL:
        {
            if (stack->size < 2) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            stackPush(stack, stackPop(stack) * stackPop(stack));
            break;
        }

        case DIV:
        {
            if (stack->size < 2) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            register2 = stackPop(stack);
            register1 = stackPop(stack);

            if (register2 == 0) { return CPU_MATH_ERROR; }

            stackPush(stack, register1 / register2);
            break;
        }

        case POW:
        {
            if (stack->size < 2) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            register2 = stackPop(stack);
            register1 = stackPop(stack);
            stackPush(stack, pow(register1, register2));
            break;
        }

        case SQRT:
        {
            if (stack->size == 0) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            register1 = stackPop(stack);

            printf("sqrt(%lf)\n", register1);

            if (register1 < 0) { return CPU_MATH_ERROR; }

            stackPush(stack, sqrt(register1));
            break;
        }

        case SIN:
        {
            if (stack->size == 0) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            stackPush(stack, sin(stackPop(stack)));
            break;
        }

        case COS:
        {
            if (stack->size == 0) { return CPU_NOT_ENOUGH_VALUES_FOR_OPERATION; }

            stackPush(stack, cos(stackPop(stack)));
            break;
        }

        case PUSH:
        {
            stackPush(stack, strtold(strtok(NULL, " ;\n"), NULL));
            break;
        }

        case HALT:
        {
            haltReached = true;
            break;
        }
    }

    return CPU_NO_ERROR;
}