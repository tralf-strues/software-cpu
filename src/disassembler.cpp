//#include <assert.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#include "softwareCpuSpecification.h"
//#include "assemblySpecification.h"
//
//const char*  DEFAULT_DISASSEMBLY_FILE_NAME = "assembly.asy";
//const size_t MAX_LINE_LENGTH               = 64;
//
//bool processDisassemblerFlags (int argc, char* argv[]);
//void translateBytecodeLine    (const char* line);
//void translateBytecodeFile    ();
//
//FILE* bytecodeFile = NULL;
//FILE* assemblyFile = NULL;
//
//int main(int argc, char* argv[])
//{
//    if (processDisassemblerFlags(argc, argv) == false) { return -1; }
//
//    translateBytecodeFile();
//
//    fclose(bytecodeFile);
//    fclose(assemblyFile);
//
//    return 0;
//}
//
//bool processDisassemblerFlags(int argc, char* argv[])
//{
//    assert(argv != NULL);
//
//    if (argc == 1)
//    {
//        printf("ERROR: assembly file unspecified\n");
//        return false;
//    }
//
//    // bytecode file
//    bytecodeFile = fopen(argv[1], "r");
//    if (bytecodeFile == NULL)
//    {
//        printf("ERROR: no file with name \"%s\" found.", argv[1]);
//        return false;
//    }
//
//    // assembly file
//    const char* assemblyFileName = DEFAULT_DISASSEMBLY_FILE_NAME;
//    if (argc == 3)
//    {
//        assemblyFileName = argv[2];
//    }
//
//    assemblyFile = fopen(assemblyFileName, "w");
//
//    return true;
//}
//
//void translateBytecodeFile()
//{
//    char* currLine = (char*) calloc(MAX_LINE_LENGTH, sizeof(char));
//    assert(currLine != NULL);
//
//    while (fgets(currLine, MAX_LINE_LENGTH, bytecodeFile) != NULL)
//    {
//        translateBytecodeLine(currLine);
//    }
//
//    free(currLine);
//}
//
//void translateBytecodeLine(const char* line)
//{
//    assert(line != NULL);
//
//    if (line[0] == '\n') { return; }
//
//    const char* currToken = strtok((char*) line, " \n");
//    if (currToken == NULL) { return; }
//
//    fprintf(assemblyFile, "%s ", ASSEMBLY_OPERATORS[strtol(currToken, NULL, 16)]);
//
//    while ((currToken = strtok(NULL, " \n")) != NULL)
//    {
//        fprintf(assemblyFile, "%s ", currToken);
//    }
//
//    fprintf(assemblyFile, "\n");
//}