#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//
void memset32(void *dest, uint16_t value, size_t count);

// String duplication function
char *xstrdup(const char *src);

// Random number generator using xorshift algorithm
uint64_t xorshift64();

// Get the current time in milliseconds
long gettimeinms();

#endif