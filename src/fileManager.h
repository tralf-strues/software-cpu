#pragma once
#include <stdio.h>

struct Text;

size_t      getFileSize          (const char* fileName);
Text*       readTextFromFile     (const char* fileName);
bool        writeTextToFile      (FILE* file, Text* text);
void        deleteText           (Text* text);
                                 
const char* nextTextLine         (Text* text);
size_t      replaceAllOccurences (char* buffer, size_t bufferSize, char target, char replacement);