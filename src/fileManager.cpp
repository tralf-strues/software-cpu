#include "fileManager.h"

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

struct Text
{
    size_t bytesCount   = 0;
    char*  buffer       = NULL;
    char** lines        = NULL;
    size_t linesCount   = 0;
    size_t currPos      = 0;
};

//-----------------------------------------------------------------------------
//! Returns size in bytes of a file.
//!
//! @param [in] filename  name of the file to get size of
//!
//! @return size of a file in bytes or 0 if an error occured.
//-----------------------------------------------------------------------------
size_t getFileSize(const char* fileName)
{
    assert(fileName != NULL);

    struct stat fileStat = {};
    if (stat((char*) fileName, &fileStat) == -1) { return 0; }

    return (size_t) fileStat.st_size;
}

//-----------------------------------------------------------------------------
//! Reads Text from file. 
//!
//! @param [in] fileName name of the file from which to read Text
//!
//! @note uses dynamic memory, so after finishing to work with the read text
//!       it's recommended to call deleteText(...) function in order to free
//!       used memory.
//!
//! @return a pointer to the text read or NULL if an error occurred.
//-----------------------------------------------------------------------------
Text* readTextFromFile(const char* fileName)
{
    assert(fileName != NULL);

    FILE* file = fopen(fileName, "r");
    assert(file != NULL);

    Text* text = (Text*) calloc(1, sizeof(Text));
    assert(text != NULL);

    text->bytesCount = getFileSize(fileName);
    text->buffer     = (char*) calloc(text->bytesCount, sizeof(char));
    assert(text->buffer != NULL);

    text->bytesCount = fread(text->buffer, sizeof(char), text->bytesCount, file);
    if (text->bytesCount == 0) 
    {
        fclose(file);
        free  (text);

        return NULL;
    }

    char* newTextBuffer = (char*) realloc(text->buffer, sizeof(char) * text->bytesCount);
    assert(newTextBuffer != NULL);
    text->buffer = newTextBuffer;

    text->linesCount = replaceAllOccurences(text->buffer, text->bytesCount, '\n', '\0') + 1;
    text->lines      = (char**) calloc(text->linesCount, sizeof(char*));
    assert(text->lines != NULL);

    size_t currLinePos = 0;
    text->lines[currLinePos++] = &text->buffer[0];
    for (size_t i = 0; i < text->bytesCount - 1; i++)
    {
        if (text->buffer[i] == '\0') { 
            text->lines[currLinePos++] = &text->buffer[i + 1]; 
        }
    }

    text->currPos = 0;

    fclose(file);

    return text;
}

//-----------------------------------------------------------------------------
//! Writes Text to file. 
//!
//! @param [in] file to which to write Text
//! @param [in] text Text which is to be written to file
//!
//!
//! @return a pointer to the text read or NULL if an error occurred.
//-----------------------------------------------------------------------------
bool writeTextToFile(FILE* file, Text* text)
{
    assert(file != NULL);
    assert(text != NULL);

    replaceAllOccurences(text->buffer, text->bytesCount, '\0', '\n');

    return fwrite(text->buffer, sizeof(char), text->bytesCount, file) == text->bytesCount;
}

//-----------------------------------------------------------------------------
//! Frees memory used for text. 
//!
//! @param [in] text Text object to be deleted
//!
//-----------------------------------------------------------------------------
void deleteText(Text* text)
{
    assert(text != NULL);

    text->bytesCount = 0;
    text->currPos    = 0;
    text->linesCount = 0;

    if (text->buffer != NULL)
    {
        free(text->buffer);
        text->buffer = NULL;
    }

    if (text->lines != NULL)
    {
        free(text->lines);
        text->lines = NULL;
    }

    free(text);
}

//-----------------------------------------------------------------------------
//! Returns next line of text.
//!
//! @param [in] text 
//!
//! @return pointer to the next line's first symbol or NULL in case end of text
//          is reached or an error occured.
//-----------------------------------------------------------------------------
const char* nextTextLine(Text* text)
{
    assert(text         != NULL);
    assert(text->lines  != NULL);
    assert(text->buffer != NULL);

    if (text->currPos >= text->linesCount)
    {
        return NULL;
    }

    return text->lines[text->currPos++];
}

//-----------------------------------------------------------------------------
//! Replaces all bytes with value 'target' in buffer with 'replacement'.
//!
//! @param [in] buffer       bytes-array to be examined and altered
//! @param [in] bufferSize   number of bytes in buffer
//! @param [in] target       specifies what bytes in buffer to replace
//! @param [in] replacement  specifies what value target bytes to replace with
//!
//! @note Undefined behavior if buffer is of lesser size than bufferSize.
//!
//! @return number of replaced bytes.
//-----------------------------------------------------------------------------
size_t replaceAllOccurences(char* buffer, size_t bufferSize, char target, char replacement)
{
    assert(buffer != NULL);

    size_t numOfOccurences = 0;
    for (size_t i = 0; i < bufferSize; i++)
    {
        if (buffer[i] == target)
        {
            buffer[i] = replacement;
            numOfOccurences++;
        }
    }

    return numOfOccurences;
}