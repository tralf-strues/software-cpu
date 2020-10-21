#pragma once

#include "softwareCpuSpecification.h"

const char* ASSEMBLY_OPERATORS[CPU_OPERATORS_COUNT] = {
                                                       // i/o
                                                       "in",
                                                       "out",
                                                       
                                                       // math
                                                       "add",
                                                       "sub",
                                                       "mul",
                                                       "div",
                                                      
                                                       "pow",
                                                       "sqrt",
                                                      
                                                       "sin",
                                                       "cos",
                                                      
                                                       // stack
                                                       "push",
                                                       "pop",
                                                      
                                                       // program-flow
                                                       "halt"
                                                      };
