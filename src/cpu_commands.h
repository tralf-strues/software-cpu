#define CPU_PTR        cpu
#define STACK_PTR      (&CPU_PTR->stack)
#define RAM_CELLS      (CPU_PTR->ram.cells)
#define VRAM_CELLS     (CPU_PTR->ram.vram)
#define CALL_STACK_PTR (&CPU_PTR->callStack)
#define PC             CPU_PTR->pc
#define CODE           CPU_PTR->program
#define CURR_CODE      CPU_PTR->program[PC]

#define TO_DOUBLE(value)       *(double*)(&(value))       
#define CURR_VAL               TO_DOUBLE(CURR_CODE)
#define SET_REGISTER(i, value) CPU_PTR->regs[i] = value
#define GET_REGISTER(i)        CPU_PTR->regs[i]
#define VAL_NUM_BYTES          sizeof(double)
#define PC_SET(value)          PC = value
#define CPU_STOP               CPU_PTR->halt = true;
#define CPU_SET_ERROR(error)   cpuSetError(CPU_PTR, error); \
                               ASSERT_CPU_OK(CPU_PTR); 
    
#define STACK_PUSH(value)        stackPush(STACK_PTR, value)
#define STACK_POP                stackPop(STACK_PTR)
#define STACK_CHECK_SIZE(needed) if (STACK_PTR->size < needed) \
                                 { CPU_SET_ERROR(CPU_NOT_ENOUGH_VALUES_FOR_OPERATION); }

#define READ(dest)   if (scanf("%lg", &dest) != 1)   { CPU_SET_ERROR(CPU_IO_ERROR); }
#define WRITE(value) if (printf("%lg\n", value) < 0) { CPU_SET_ERROR(CPU_IO_ERROR); }

#define JUMP_TEMPLATE(condition)    PC += 2; /* to skip cmd and literal value flag */   \
                                    STACK_CHECK_SIZE(2);                                \
                                    double temp2 = STACK_POP;                           \
                                    double temp1 = STACK_POP;                           \
                                    if (temp1 condition temp2) { PC_SET(CURR_VAL); }    \
                                    else                       { PC += VAL_NUM_BYTES; } \

DEFINE_CMD(in, 0, 0, false,
            {
                double temp = 0;
                READ(temp);

                STACK_PUSH(temp);

                PC++;
            })

DEFINE_CMD(out, 1, 0, false,
            {
                STACK_CHECK_SIZE(1);

                WRITE(STACK_POP);

                PC++;
            })

DEFINE_CMD(add, 2, 0, false,
            {
                STACK_CHECK_SIZE(2);

                STACK_PUSH(STACK_POP + STACK_POP);

                PC++;
            })

DEFINE_CMD(sub, 3, 0, false,
            {
                STACK_CHECK_SIZE(2);

                double temp2 = STACK_POP;
                double temp1 = STACK_POP;
                STACK_PUSH(temp1 - temp2);
               
                PC++;
            })

DEFINE_CMD(mul, 4, 0, false,
            {
                STACK_CHECK_SIZE(2);

                double temp2 = STACK_POP;
                double temp1 = STACK_POP;

                STACK_PUSH(temp1 * temp2);
               
                PC++;
            })

DEFINE_CMD(div, 5, 0, false,
            {
                STACK_CHECK_SIZE(2);

                double temp2 = STACK_POP;
                double temp1 = STACK_POP;

                if (temp2 == 0) { CPU_SET_ERROR(CPU_MATH_ERROR); }

                STACK_PUSH(temp1 / temp2);

                PC++;
            })

DEFINE_CMD(pow, 6, 0, false,
            {
                STACK_CHECK_SIZE(2);
               
                double temp2 = STACK_POP;
                double temp1 = STACK_POP;
               
                STACK_PUSH(pow(temp1, temp2));
               
                PC++;
            })

DEFINE_CMD(sqrt, 7, 0, false,
            {
                STACK_CHECK_SIZE(1);

                double temp = STACK_POP;

                if (temp < 0) { CPU_SET_ERROR(CPU_MATH_ERROR); }
               
                STACK_PUSH(sqrt(temp));
               
                PC++;
            })

DEFINE_CMD(sin, 8, 0, false, 
            {
                STACK_CHECK_SIZE(1);
               
                STACK_PUSH(sin(STACK_POP));
               
                PC++;
            })

DEFINE_CMD(cos, 9, 0, false,
            {
                STACK_CHECK_SIZE(1);
                
                STACK_PUSH(cos(STACK_POP));
               
                PC++;
            })

DEFINE_CMD(push, 10, 1, false,
            {
                PC++;
                char mode = CURR_CODE;
                PC++;

                double argument = 0;

                if (mode & CPU_ARGUMENT_MASK_REG) { argument += GET_REGISTER(CURR_CODE - 1); PC += 1; }

                if (mode & CPU_ARGUMENT_MASK_CST) { argument += TO_DOUBLE(CODE[PC]); PC += VAL_NUM_BYTES; } 

                if (mode & CPU_ARGUMENT_MASK_RAM) 
                { 
                    if ((size_t) argument >= VRAM_START_INDEX)
                        argument = VRAM_CELLS[(size_t) argument - VRAM_START_INDEX];
                    else
                        argument = RAM_CELLS[(size_t) argument];
                }

                STACK_PUSH(argument);
            })

DEFINE_CMD(pop, 11, 1, false,
            {
                STACK_CHECK_SIZE(1); 

                PC++;
                char mode = CURR_CODE;
                PC++;

                if ((mode & (CPU_ARGUMENT_MASK_REG | CPU_ARGUMENT_MASK_RAM)) == 0) 
                { 
                    CPU_SET_ERROR(CPU_INVALID_CMD_ARGUMENT); 
                }

                if ((mode & CPU_ARGUMENT_MASK_RAM) == 0)
                {
                    SET_REGISTER(CURR_CODE - 1, STACK_POP);
                    PC++;
                }
                else
                {
                    double argument = 0;

                    if (mode & CPU_ARGUMENT_MASK_REG) { argument += GET_REGISTER(CURR_CODE - 1); PC += 1; }

                    if (mode & CPU_ARGUMENT_MASK_CST) { argument += TO_DOUBLE(CODE[PC]); PC += VAL_NUM_BYTES; }    

                    if ((size_t) argument >= VRAM_START_INDEX)
                        VRAM_CELLS[(size_t) argument - VRAM_START_INDEX] = (unsigned char) STACK_POP;
                    else
                        RAM_CELLS[(size_t)argument] = STACK_POP;
                }                    
            })

DEFINE_CMD(call, 12, 1, true,
            {
                PC += 2; // to skip cmd and literal value flag

                double temp = CURR_VAL;

                stackPush(CALL_STACK_PTR, PC + VAL_NUM_BYTES);
                PC_SET(temp);
            })

DEFINE_CMD(ret, 13, 0, false,
            {
                PC_SET(stackPop(CALL_STACK_PTR));
            })

DEFINE_CMD(jmp, 14, 1, true,
            {
                PC += 2; // to skip cmd and literal value flag
                PC_SET(CURR_VAL);
            })

DEFINE_CMD(jae, 15, 1, true,
            {
                JUMP_TEMPLATE(>=)
            })

DEFINE_CMD(ja, 16, 1, true,
            {
                JUMP_TEMPLATE(>)
            })

DEFINE_CMD(jb, 17, 1, true,
            {
                JUMP_TEMPLATE(<)
            })

DEFINE_CMD(jbe, 18, 1, true,
            {
                JUMP_TEMPLATE(<=)
            })

DEFINE_CMD(je, 19, 1, true,
            {
                JUMP_TEMPLATE(==)
            })

DEFINE_CMD(jne, 20, 1, true,
            {
                JUMP_TEMPLATE(!=)              
            })

DEFINE_CMD(upd, 21, 0, false,
            {
                updateDisplay(CPU_PTR->display, CPU_PTR->ram.vram);       
                PC++;           
            })

DEFINE_CMD(clr, 22, 0, false,
            {
                clearVRAM(CPU_PTR->ram.vram, getDisplayBufferSize(CPU_PTR->display));       
                PC++;           
            })

DEFINE_CMD(abs, 23, 0, false,
            {
                STACK_CHECK_SIZE(1); 

                STACK_PUSH(abs(STACK_POP));

                PC++;           
            })

DEFINE_CMD(flr, 24, 0, false,
            {
                STACK_CHECK_SIZE(1); 

                STACK_PUSH(floor(STACK_POP));

                PC++;           
            })

DEFINE_CMD(hlt, 25, 0, false,
            {
                CPU_STOP;
            })

#undef CPU_PTR                 
#undef STACK_PTR
#undef CALL_STACK_PTR
#undef PC               
#undef CURR_CODE               
#undef SET_REGISTER
#undef CPU_SET_ERROR  
#undef CPU_STOP                

#undef CMD_NUM_BYTES           
#undef VAL_NUM_BYTES           
#undef PC_INCREMENT     

#undef STACK_PUSH      
#undef STACK_POP               
#undef STACK_CHECK_SIZE

#undef READ             
#undef WRITE            