#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cpu_specification.h"
#include "display.h"
#include "../libs/file_manager.h"
#include "../libs/log_generator.h"

#define CPU_DONT_DUMP_RAM true

#ifdef CPU_DEBUG_MODE
#define STACK_DEBUG_MODE
#define ASSERT_CPU_OK(cpu_ptr) if (cpu_ptr->status != CPU_NO_ERROR)            \
                               {                                               \
                                   printf("CPU error: %d\n", cpu_ptr->status); \
                                   assert(!"OK");                              \
                               }                                               \
                               ASSERT_STACK_OK(&cpu_ptr->stack);

#else
#define ASSERT_CPU_OK(cpu_ptr)
#endif

void clearVRAM(unsigned char* vram, size_t vramSize);

int main(int argc, char* argv[])
{
   	CPU cpu = {};

   	CpuInitError cpuInitError = initCpu(&cpu, argc, argv);
   	if (cpuInitError != CPU_INIT_NO_ERROR) { return cpuInitError; } 

    CpuError executionResult = executeProgram(&cpu);

   	deleteCpu(&cpu);
   	return executionResult;
}

#define CPU_INIT_ERROR(error) printf("Cpu error: %s\n", #error); return error;

void printStack(CPU* cpu)
{
	for (size_t i = 0; i < cpu->stack.size; i++)
		printf("%lg ", cpu->stack.dynamicArray[i]);

	printf("\n");
}

CpuInitError initCpu(CPU* cpu, int argc, char* argv[])
{
   	if (cpu  == NULL) { CPU_INIT_ERROR(CPU_INIT_NULL_PTR_PARAMETER);   }
   	if (argv == NULL) { CPU_INIT_ERROR(CPU_INIT_ARGS_EMPTY);           }
   	if (argc <= 1   ) { CPU_INIT_ERROR(CPU_INIT_BCD_FILE_UNSPECIFIED); }

   	const char* bytecodeFileName = argv[1];
   	if (bytecodeFileName == NULL) { CPU_INIT_ERROR(CPU_INIT_BCD_FILE_UNSPECIFIED); }

   	FILE* bytecodeFile = fopen(bytecodeFileName, "rb");
   	size_t programBytes = getFileSize(bytecodeFileName);
   	if (bytecodeFile == NULL || programBytes == 0) { CPU_INIT_ERROR(CPU_INIT_BYTECODE_FILE_READ_ERROR); }

   	cpu->programBytes = programBytes;
   	cpu->program = (char*) calloc(programBytes, sizeof(char));
   	if (fread(cpu->program, sizeof(char), programBytes, bytecodeFile) != programBytes) 
   	{ 
   		CPU_INIT_ERROR(CPU_INIT_BYTECODE_FILE_READ_ERROR); 
   	}

   	stackDefaultConstruct(&cpu->stack);
   	stackDefaultConstruct(&cpu->callStack);

   	fclose(bytecodeFile);

    cpu->ram.size  = CPU_RAM_SIZE;
    cpu->ram.cells = (double*) calloc(cpu->ram.size, sizeof(char));
    if (cpu->ram.cells == NULL) { CPU_INIT_ERROR(CPU_INIT_RAM_NOT_ENOUGH_MEMORY); }
    cpu->ram.vram  = ((unsigned char*) cpu->ram.cells) + sizeof(double) * VRAM_START_INDEX; 
    cpu->display   = newDisplay();

   	return CPU_INIT_NO_ERROR;
}

void deleteCpu(CPU* cpu)
{
	assert(cpu != NULL);

	stackDestruct(&cpu->stack);
	stackDestruct(&cpu->callStack);

    free(cpu->program);
    free(cpu->ram.cells);

    deleteDisplay(cpu->display);
}

void cpuSetError(CPU* cpu, CpuError error)
{
	assert(cpu != NULL);

	cpu->status = error;
}

CpuError executeProgram(CPU* cpu)
{
	assert(cpu != NULL);

	#define DEFINE_CMD(name, number, args, isControlFlow, code)	\
	            case CPU_CMD_##name:                            \
				{                                               \
                    ASSERT_CPU_OK(cpu);                         \
					code                                        \
                    ASSERT_CPU_OK(cpu);                         \
				    break;                                      \
				}                

	while(!cpu->halt)
	{
		if (cpu->pc >= cpu->programBytes) { cpuSetError(cpu, CPU_REACHED_PROGRAM_END_NOT_HALTED); return CPU_REACHED_PROGRAM_END_NOT_HALTED; }

		switch(cpu->program[cpu->pc])
		{
			#include "cpu_commands.h"

			default:
			{
				cpuSetError(cpu, CPU_INVALID_COMMAND);
				return CPU_INVALID_COMMAND;
			}
		}
	}

	#undef DEFINE_CMD

    return cpu->status;
}

void clearVRAM(unsigned char* vram, size_t vramSize)
{
    assert(vram != NULL);

    for (size_t i = 0; i < vramSize; i++)
    {
        vram[i] = 0;
    }
}

#define CPU_ERROR_STRING(errorStatus) #errorStatus
static size_t CPU_DUMP_ERROR_NAME_STARTING_INDEX = 9;
static size_t CPU_DUMP_ERROR_STRING_LENGTH       = 128;
#define CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(errorStatus) snprintf(&errorString[CPU_DUMP_ERROR_NAME_STARTING_INDEX], \
                                                                                   CPU_DUMP_ERROR_STRING_LENGTH,        \
                                                                                   CPU_ERROR_STRING(errorStatus));
void dump(CPU* cpu)
{
    assert(cpu != NULL);

    if (!isLogInitialized())
    {
        initLog();
    }

    char errorString[CPU_DUMP_ERROR_STRING_LENGTH];
    if (cpu->status == CPU_NO_ERROR)
    {
        snprintf(errorString, CPU_DUMP_ERROR_STRING_LENGTH, CPU_ERROR_STRING(CPU_NO_ERROR));
    }
    else
    {
        snprintf(errorString, CPU_DUMP_ERROR_STRING_LENGTH, "ERROR %d: ", cpu->status);

        switch(cpu->status)
        {
            case CPU_NO_ERROR:
                CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(CPU_NO_ERROR);
            break;

            case CPU_INVALID_EXECUTABLE:
                CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(CPU_INVALID_EXECUTABLE);
            break;

            case CPU_NOT_ENOUGH_VALUES_FOR_OPERATION:
                CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(CPU_NOT_ENOUGH_VALUES_FOR_OPERATION);
            break;

            case CPU_MATH_ERROR:
                CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(CPU_MATH_ERROR);
            break;

            case CPU_IO_ERROR:
                CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(CPU_IO_ERROR);
            break;

            case CPU_INVALID_CMD_ARGUMENT:
                CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(CPU_INVALID_CMD_ARGUMENT);
            break;

            case CPU_REACHED_PROGRAM_END_NOT_HALTED:
                CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(CPU_REACHED_PROGRAM_END_NOT_HALTED);
            break;

            case CPU_INVALID_COMMAND:
                CPU_DUMP_CAT_ERROR_NAME_TO_ERROR_STRING(CPU_INVALID_COMMAND);
            break;
        }
    }
    
    logWriteMessageStart(LOG_COLOR_BLACK);
    logWrite("CPU (");
    logWrite(errorString, cpu->status == CPU_NO_ERROR ? LOG_COLOR_GREEN : LOG_COLOR_RED);

    logWrite(") [0x%X] "

                 "\n"
                 "{\n"  

                 "   programBytes = %lu\n"
                 "   pc           = %lu\n"
                 "   halt         = %d\n\n"
                 "   regs [0x%X]\n"
                 "   {\n"
                 ,
                 cpu, 
                 cpu->programBytes,
                 cpu->pc,
                 (int) cpu->halt,
                 cpu->regs
    );

    for (char i = 0; i < CPU_REGISTERS_COUNT; i++)
    {
        logWrite("       [r%cx]\t= %lg\n", 'a' + i, cpu->regs[i]);
    }

    logWrite("   }\n"
             "   program\n"
             "   {\n"
             "       ");

    for (size_t i = 0; i < cpu->programBytes; i++)
    {
        if (i == cpu->pc)
            logWrite("%.4lu|", LOG_COLOR_BLUE, i);
        else
            logWrite("%.4lu|", i);
    }

    logWrite("\n       ");

    for (size_t i = 0; i < cpu->programBytes * 5; i++)
    {
        logWrite("-");
    }

    logWrite("\n       ");

    for (size_t i = 0; i < cpu->programBytes; i++)
    {
        if (i == cpu->pc)
            logWrite("%.4u|", LOG_COLOR_BLUE, (unsigned char) cpu->program[i]);
        else
            logWrite("%.4u|", (unsigned char) cpu->program[i]);
    }

    logWrite("\n       ");

    for (size_t i = 0; i < 5 * cpu->pc; i++ )
    {
        logWrite(" ");
    }

    logWrite("^\n", LOG_COLOR_BLUE);
    logWrite("   }\n\n");

    if (!CPU_DONT_DUMP_RAM)
    {
    
        logWrite("   ram\n"
                 "   {\n");
    
        for (size_t i = 0; i < cpu->ram.size; i++)
        {
            if (i < VRAM_START_INDEX)
            {
                logWrite("       [%lu]\t= %lg\n", 
                         i, cpu->ram.cells[i]);
            }
            else 
            {
                logWrite("       [%lu]\t= %u\n", LOG_COLOR_GREEN, 
                         i, cpu->ram.vram[i - VRAM_START_INDEX]);
            }
        }
    
        logWrite("   }\n");

    }

    logWrite("}\n");

    logWriteMessageEnd();

    dump(&cpu->stack);
    dump(&cpu->callStack);
}