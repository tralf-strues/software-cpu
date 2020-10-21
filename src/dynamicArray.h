#pragma once

#include <assert.h>
#include <stdlib.h>

const size_t DYNAMIC_ARRAY_DEFAULT_SIZE      = 64;
const double DYNAMIC_ARRAY_EXPAND_MULTIPLIER = 1.8;

struct DynamicArray
{
    da_elem_t* data         = NULL;
    size_t     size         = 0;
    size_t     iteratorPos  = 0;
};

DynamicArray* newDynamicArray(size_t size)
{
    DynamicArray* dynamicArray = (DynamicArray*) calloc(1, sizeof(DynamicArray));
    assert(dynamicArray != NULL);

    dynamicArray->data = (da_elem_t*) calloc(size, sizeof(da_elem_t));
    assert(dynamicArray->data != NULL);

    dynamicArray->size = size;

    return dynamicArray;
}

DynamicArray* newDynamicArray()
{
    return newDynamicArray(DYNAMIC_ARRAY_DEFAULT_SIZE);
}

void deleteDynamicArray(DynamicArray* dynamicArray)
{
    assert(dynamicArray != NULL);

    dynamicArray->size     = 0;

    free(dynamicArray->data);
    dynamicArray->data = NULL;

    free(dynamicArray);
}

void resize(DynamicArray* dynamicArray, size_t newSize)
{
    assert(dynamicArray       != NULL);
    assert(dynamicArray->data != NULL);

    da_elem_t* newData = (da_elem_t*) realloc(dynamicArray->data, newSize * sizeof(da_elem_t));
    assert(newData != NULL);

    dynamicArray->data = newData;
    dynamicArray->size = newSize;
}

void shrinkToFit(DynamicArray* dynamicArray)
{
    assert(dynamicArray != NULL);

    resize(dynamicArray, dynamicArray->size);
}

da_elem_t* at(DynamicArray* dynamicArray, size_t i)
{
    assert(dynamicArray != NULL);
    assert(i < dynamicArray->size);

    return &dynamicArray->data[i];
}

da_elem_t get(DynamicArray* dynamicArray, size_t i)
{
    return *at(dynamicArray, i);
}

void set(DynamicArray* dynamicArray, size_t i, da_elem_t value)
{
    assert(dynamicArray != NULL);

    *at(dynamicArray, i) = value;
}

size_t spaceLeft(DynamicArray* dynamicArray)
{
    assert(dynamicArray != NULL);

    return dynamicArray->size - dynamicArray->iteratorPos;
}

void pushBack(DynamicArray* dynamicArray, da_elem_t value)
{
    assert(dynamicArray != NULL);

    if (dynamicArray->iteratorPos + 1 > dynamicArray->size)
    {
        resize(dynamicArray, dynamicArray->size * DYNAMIC_ARRAY_EXPAND_MULTIPLIER);
    }

    set(dynamicArray, dynamicArray->iteratorPos++, value);
}

void pushBack(DynamicArray* dynamicArray, void* value, size_t typeSize)
{
    assert(dynamicArray != NULL);

    while (spaceLeft(dynamicArray) * sizeof(da_elem_t) < typeSize)
    {
        resize(dynamicArray, dynamicArray->size * DYNAMIC_ARRAY_EXPAND_MULTIPLIER);
    }

    memcpy(&dynamicArray->data[dynamicArray->iteratorPos], value, typeSize);

    dynamicArray->iteratorPos += typeSize;
}

da_elem_t popBack(DynamicArray* dynamicArray)
{
    assert(dynamicArray != NULL);

    return get(dynamicArray, dynamicArray->iteratorPos--);
}