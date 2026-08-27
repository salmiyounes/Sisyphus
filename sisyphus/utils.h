#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include <stdlib.h>
#include <string.h>

// Generic swap function for any data type
void swap_any(void *a, void *b, size_t s);

// String duplication function
char *xstrdup(const char *src);

// Random number generator using xorshift algorithm
bb xorshift64();

// Get the current time in milliseconds
long gettimeinms();

#endif