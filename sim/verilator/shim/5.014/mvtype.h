#pragma once

#include <stdlib.h>
#include <string.h>


/// @brief get C heap space for pointer array
/// @param len 
/// @return (void*)[] 
void *alloc_ptr_array(unsigned int len)
{
    return calloc(len, sizeof(void *));
}

/// @brief set pos-th pointer for array
/// @param ain pointer array
/// @param pos i-th
/// @param val value pointer
void set_ptr_array(void *ain, unsigned int pos, void *val)
{
    void **a = (void **) ain;
    a[pos] = val;
}

/// @brief get pos-th pointer of array
/// @param ain pointer array
/// @param pos i-th
/// @return value pointer 
void *get_ptr_array(void *ain, unsigned int pos)
{
    void **a = (void **) ain;
    return a[pos];
}

/// @brief free space of pointer array AND its elements' space
/// @param ain 
void free_ptr_array(void *ain)
{
    void **a = (void **) ain;
    unsigned int i;

    if (!a)
        return;
    for (i = 0; a[i]; i++) {
        free(a[i]);
    }
    free(a);
}

/// @brief force cast uintptr to char*
/// @param in uint pointer
/// @return char*
char *uintptr_to_string(void *in)
{
    return (char *) in;
}

/// @brief dump char* from C managed heap to Go memory usage
/// @param in 
/// @return char*
void *string_to_uintptr(char *in)
{
    return strdup(in);
}