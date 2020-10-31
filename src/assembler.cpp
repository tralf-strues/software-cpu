#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_specification.h"
#include "assembler_specification.h"

#define PRINT_ERROR(message) printf("Syntax ERROR: " message); \
                             printCurrentLine(assembler);      \
                             return false;   

#define PRINT_ERROR1(message, param1) printf("Syntax ERROR: " message, param1); \
                                      printCurrentLine(assembler);              \
                                      return false; 

#define PRINT_ERROR2(message, param1, param2) printf("Syntax ERROR: " message, param1, param2); \
                                              printCurrentLine(assembler);                      \
                                              return false;                            

const char*  DEFAULT_BYTECODE_FILE_NAME = "bin/bytecode.bcd";
const char*  TOKEN_DELIMS               = " ;\t";
const size_t MAX_LINE_LENGTH            = 128;

bool   translateAssemblyLine   (Assembler* assembler, const char* line, size_t passNum);
bool   makeAssemblyPass        (Assembler* assembler, size_t passNum);
bool   processArgument         (Assembler* assembler, bool isControlFlow, char* currToken, size_t passNum);
bool   processCompoundToken    (Assembler* assembler, bool isControlFlow, char* currToken, size_t passNum);
void   processNumbericToken    (Assembler* assembler, char* currToken);
bool   processCompoundArgument (Assembler* assembler, char* currArg);
bool   processLabel            (Assembler* assembler, size_t labelLength, char* cmd, size_t passNum);
size_t currBytecodeOfs         (Assembler* assembler);
void   createNewLabel          (Assembler* assembler, const char* labelName, size_t labelLength);
Label* getLabel                (Assembler* assembler, const char* labelName);
bool   isValidNumericToken     (const char* token);
bool   isValidRegisterToken    (const char* token);
size_t getExtraArgsCount       (const char* start, const char* commentStart);
void   printCurrentLine        (Assembler* assembler);

int main(int argc, char* argv[])
{
    Assembler assembler = {};

    AssemblerInitError asmInitError = initAssembler(&assembler, argc, argv);
    if (asmInitError != ASSEMBLER_INIT_NO_ERROR) {  return asmInitError; } 

    bool isTranslatedSuccessfuly = translateAssemblyFile(&assembler);
    if (!isTranslatedSuccessfuly) { printf("Assembler finished with an error.\n"); }

    finishAssembler(&assembler);

    return !isTranslatedSuccessfuly;
}

#define ASM_INIT_ERROR(error) printf("Assembler error: %s\n", #error); return error;

AssemblerInitError initAssembler(Assembler* assembler, int argc, char* argv[])
{
    if (assembler == NULL) { ASM_INIT_ERROR(ASSEMBLER_INIT_NULL_PTR_PARAMETER); }
    if (argv      == NULL) { ASM_INIT_ERROR(ASSEMBLER_INIT_ARGS_EMPTY); }
    if (argc      <= 1   ) { ASM_INIT_ERROR(ASSEMBLER_INIT_ASY_FILE_UNSPECIFIED); }

    const char* assemblyFileName = NULL;
    const char* bytecodeFileName = DEFAULT_BYTECODE_FILE_NAME;
    assemblyFileName = argv[1];
    if (argc >= 3)
    {
        bytecodeFileName = argv[2];
    }
 
    if (assemblyFileName == NULL) { ASM_INIT_ERROR(ASSEMBLER_INIT_ASY_FILE_UNSPECIFIED); }
    if (bytecodeFileName == NULL) { ASM_INIT_ERROR(ASSEMBLER_INIT_BCD_FILE_UNSPECIFIED); }

    assembler->assembly = readTextFromFile(assemblyFileName);
    if (assembler->assembly == NULL) { ASM_INIT_ERROR(ASSEMBLER_INIT_ASSEMBLY_FILE_READ_ERROR); }

    assembler->bytecodeFile = fopen(bytecodeFileName, "wb");
    if (assembler->bytecodeFile == NULL) { ASM_INIT_ERROR(ASSEMBLER_INIT_BYTECODE_FILE_WRITE_ERROR); }

    assembler->bytecode = newDynamicArray();
    if (assembler->bytecode == NULL) { ASM_INIT_ERROR(ASSEMBLER_INIT_NOT_ENOUGH_MEMORY); }

    assembler->labels = newLabelArray();
    if (assembler->labels == NULL) { ASM_INIT_ERROR(ASSEMBLER_INIT_NOT_ENOUGH_MEMORY); }

    return ASSEMBLER_INIT_NO_ERROR;
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

    if (assembler->labels != NULL)
    {
        for (size_t i = 0; i < assembler->labels->iteratorPos; i++)
        {
            free(get(assembler->labels, i).name);    
        }
   
        deleteLabelArray(assembler->labels);
        assembler->labels = NULL;
    }
}

bool translateAssemblyFile(Assembler* assembler)
{
    assert(assembler               != NULL);
    assert(assembler->assembly     != NULL);
    assert(assembler->bytecode     != NULL);
    assert(assembler->bytecodeFile != NULL);
    
    if (!makeAssemblyPass(assembler, 1)) { return false; }
    if (!makeAssemblyPass(assembler, 2)) { return false; }

    size_t numOfBytesToWrite = assembler->bytecode->iteratorPos;
    if (fwrite(assembler->bytecode->data, sizeof(char), numOfBytesToWrite, assembler->bytecodeFile) != numOfBytesToWrite)
    {
        printf("Couldn't write to file.\n");
        return false;
    }
   
    return true;
}

// passNum starting from 1
bool makeAssemblyPass(Assembler* assembler, size_t passNum)
{
    assert(assembler               != NULL);
    assert(assembler->assembly     != NULL);
    assert(assembler->bytecode     != NULL);
    assert(assembler->bytecodeFile != NULL);

    resetTextToStart(assembler->assembly);
    assembler->bytecode->iteratorPos = 0;

    const char* currLine = NULL;
    while ((currLine = nextTextLine(assembler->assembly)) != NULL)
    {
        if (!translateAssemblyLine(assembler, currLine, passNum)) { return false; }
    }

    return true;
}

bool translateAssemblyLine(Assembler* assembler, const char* line, size_t passNum)
{
    assert(assembler != NULL);
    assert(line      != NULL);
    static char tempLineCopy[MAX_LINE_LENGTH];
    strcpy(tempLineCopy, line);

    const char* commentStart = strchr(tempLineCopy, ';');

    char* currToken = strtok((char*) tempLineCopy, TOKEN_DELIMS);
    if (currToken == NULL || (commentStart != NULL && currToken >= commentStart)) { return true; }

    char* cmd = currToken;

    #define DEFINE_CMD(name, number, args, isControlFlow, code) \
            if (strcmp(cmd, #name) == 0)                                                                     \
            {                                                                                                \
                pushBack(assembler->bytecode, CPU_CMD_##name);                                               \
                currToken = strtok(NULL, TOKEN_DELIMS);                                                      \
                for (size_t i = 0; i < args; i++, currToken = strtok(NULL, " ;\t"))                          \
                    if (!processArgument(assembler, isControlFlow, currToken, passNum))                      \
                        return false;                                                                        \
                                                                                                             \
                size_t extraArgsCount = getExtraArgsCount(currToken, commentStart);                          \
                if (extraArgsCount != 0)                                                                     \
                {                                                                                            \
                    PRINT_ERROR2("invalid number of arguments: %d instead of %d: ",                          \
                                 args + extraArgsCount, args);                                               \
                }                                                                                            \
                                                                                                             \
            }                                                                                                \
            else                                                                                                                                                                      
  
    #include "cpu_commands.h"
    /* else */ 
    {
        size_t labelLength = strlen(cmd);
        if (cmd[labelLength - 1] == ':')
        {
            if (!processLabel(assembler, labelLength, cmd, passNum)) { return false; }
        } 
        else
        {
            PRINT_ERROR1("unrecognized command '%s': ", cmd);
        }
    }
    #undef DEFINE_CMD   

    return true;
}

bool processArgument(Assembler* assembler, bool isControlFlow, char* currToken, size_t passNum)
{
    assert(assembler != NULL);
    assert(currToken != NULL);

    if (!isValidNumericToken(currToken))                                                     
    {                                                                                        
        if (!processCompoundToken(assembler, isControlFlow, currToken, passNum))                   
            return false;                                                                    
    }                                                                                        
    else { processNumbericToken(assembler, currToken); }    

    return true;                                 
}

bool processCompoundToken(Assembler* assembler, bool isControlFlow, char* currToken, size_t passNum)
{
    assert(assembler != NULL);
    assert(currToken != NULL);

    double temp = 0;

    if (isControlFlow && currToken[0] != ':')                                     
    {                                                                                
        PRINT_ERROR("invalid label indicator, no ':' found: ");                                                          
    }                                                                     
                                                                                  
    Label* label = getLabel(assembler, &currToken[1]);                            
                                                                                  
    if (isControlFlow &&                                                
        passNum > 1   &&                                                
        label != NULL)                                                            
    {                                                                             
        pushBack(assembler->bytecode, CPU_ARGUMENT_TYPE_CST);         
        pushBack(assembler->bytecode, &(label->value), sizeof(temp));           
    }                                                                             
    else if (isControlFlow &&                                                     
             passNum > 1)                                             
    {                                                                                      
        PRINT_ERROR1("no label '%s' found: ", &currToken[1]);                                                             
    }                                                                             
    else if (isControlFlow)                                                       
    {                                                                             
        temp = -1;                                                                
        pushBack(assembler->bytecode, CPU_ARGUMENT_TYPE_CST);               
        pushBack(assembler->bytecode, &temp, sizeof(temp));                       
    }                                                                             
    else                                                                          
    {                                                                             
        if (!processCompoundArgument(assembler, currToken)) { return false; };                   
    } 

    return true;                                                                            
}

void processNumbericToken(Assembler* assembler, char* currToken)
{
    assert(assembler != NULL);
    assert(currToken != NULL);

    double temp = 0;

    sscanf(currToken, "%lg", &temp);                                                   
    pushBack(assembler->bytecode, CPU_ARGUMENT_TYPE_CST);               
    pushBack(assembler->bytecode, &temp, sizeof(temp));                                
}

bool processCompoundArgument(Assembler* assembler, char* currArg)
{
    assert(assembler != NULL);
    assert(currArg   != NULL);

    bool isRam = currArg[0] == '['; 
    if (isRam) { currArg++; }; // to skip '['

    char*  firstArg        = strtok(currArg, "+");
    char*  secondArg       = strtok(NULL, "+");
    size_t firstArgLength  = strlen(firstArg);
    size_t secondArgLength = 0;

    if (secondArg == NULL && isRam)
    {
        if (firstArg[firstArgLength - 1] != ']') { PRINT_ERROR("no ']' found in ram argument: "); }

        firstArg[firstArgLength - 1] = '\0';
    }
    else if (isRam)
    {
        secondArgLength = strlen(secondArg);

        if (secondArg[secondArgLength - 1] != ']') { PRINT_ERROR("no ']' found in ram argument: "); }

        secondArg[secondArgLength - 1] = '\0';
    }

    bool is1stNum = isValidNumericToken(firstArg);
    bool is1stReg = isValidRegisterToken(firstArg);
    bool is2ndNum = isValidNumericToken(secondArg);
    bool is2ndReg = isValidRegisterToken(secondArg);

    if ((is1stNum && is2ndNum) || (is1stReg && is2ndReg))
    {
        PRINT_ERROR("two arguments of the same type (only register + const is supported): ");
    }

    if (is1stNum && is2ndReg)
    {
        PRINT_ERROR("const before register (only register + const is supported): ");
    }

    char argType = 0;

    if (isRam)                { argType |= CPU_ARGUMENT_MASK_RAM; }
    if (is1stNum || is2ndNum) { argType |= CPU_ARGUMENT_MASK_CST; }
    if (is1stReg || is2ndReg) { argType |= CPU_ARGUMENT_MASK_REG; }

    if (argType == 0) { PRINT_ERROR("invalid argument: "); }

    pushBack(assembler->bytecode, argType);

    double temp = 0;
    if (isValidNumericToken(firstArg))
    {
        sscanf(firstArg, "%lg", &temp);                                                             
        pushBack(assembler->bytecode, &temp, sizeof(temp));   
    }
    else if (isValidRegisterToken(firstArg))
    {                                                                                  
        pushBack(assembler->bytecode, firstArg[1] - 'a' + 1);
    }

    if (secondArg != NULL)
    {
        if (isValidNumericToken(secondArg))
        {
            sscanf(secondArg, "%lg", &temp);                                                             
            pushBack(assembler->bytecode, &temp, sizeof(temp));
        }   
        else if (isValidRegisterToken(secondArg))
        {                                                                                 
            pushBack(assembler->bytecode, secondArg[1] - 'a' + 1);
        }
    }

    return true;
}

bool processLabel(Assembler* assembler, size_t labelLength, char* cmd, size_t passNum)
{
    char* currToken = NULL;

    if ((currToken = strtok(NULL, TOKEN_DELIMS)) != NULL)
    {
        PRINT_ERROR1("commands after label definition: '%s' ", currToken);
    }
    else
    {
        cmd[--labelLength] = '\0';
        bool labelDefined = getLabel(assembler, cmd) != NULL;
        if (passNum == 1 && labelDefined)
        {
            PRINT_ERROR1("label defined more than once: '%s': ", cmd);
        }    
        else if (passNum == 1)
        {
            createNewLabel(assembler, cmd, labelLength);
        }
    }

    return true;
}

size_t currBytecodeOfs(Assembler* assembler)
{
    assert(assembler           != NULL);
    assert(assembler->bytecode != NULL);

    return assembler->bytecode->iteratorPos - 1;
}

void createNewLabel(Assembler* assembler, const char* labelName, size_t labelLength)
{
    assert(assembler != NULL);
    assert(labelName != NULL);

    Label newLabel = {};
    newLabel.value = currBytecodeOfs(assembler) + 1;

    newLabel.name  = (char*) calloc(labelLength + 1, sizeof(char));
    assert(newLabel.name != NULL);

    strcpy(newLabel.name, labelName);
    newLabel.name[labelLength] = '\0';

    pushBack(assembler->labels, newLabel);
}

Label* getLabel(Assembler* assembler, const char* labelName)
{
    assert(assembler != NULL);
    assert(labelName != NULL);

    for (size_t i = 0; i < assembler->labels->iteratorPos; i++)
    {
        if (strcmp(get(assembler->labels, i).name, labelName) == 0) 
        {
            return at(assembler->labels, i);    
        }       
    }

    return NULL;
}

bool isValidNumericToken(const char* token)
{
    if (token == NULL) { return false; }

    double temp = 0;
    return sscanf(token, "%lg", &temp) == 1;
}

bool isValidRegisterToken(const char* token)
{
    if (token == NULL) { return false; }

    return strlen(token) == 3                    && 
           token[0] == 'r'                       && 
           token[1] >= 'a'                       && 
           token[1] <  'a' + CPU_REGISTERS_COUNT &&
           token[2] == 'x';
}

size_t getExtraArgsCount(const char* start, const char* commentStart)
{
    if (start == NULL || (commentStart != NULL && start >= commentStart))
    {
        return 0;
    }

    size_t extraArgsCount = 1;

    char* currToken = strtok(NULL, TOKEN_DELIMS);

    while (currToken != NULL)
    {
        if (commentStart != NULL && currToken >= commentStart) { break; }

        extraArgsCount++;

        currToken = strtok(NULL, TOKEN_DELIMS);
    }

    return extraArgsCount;
}

void printCurrentLine(Assembler* assembler)
{
    size_t currLineNumber = getCurrentLineNumber(assembler->assembly);
    printf("line %u\n%5u | %s\n", currLineNumber + 1, 
                                  currLineNumber + 1, 
                                  getLine(assembler->assembly, currLineNumber));
}