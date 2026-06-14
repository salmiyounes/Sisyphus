#ifndef TABLE_H
#define TABLE_H

#include "board.h"
#include "move.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Node types in transposition table
#define EXACT 1 // Exact evaluation score
#define ALPHA 2 // Upper bound score
#define BETA 3  // Lower bound score

// Function declarations for table operations
// Prefetch table entry for given position key
void table_prefetch(Table *table, bb key);

// Allocate memory for transposition table
int table_alloc(Table *table, int bits);

// Free table memory
void table_free(Table *table);

// Get table entry for a position
Entry *table_entry(Table *table, bb key);

// Get stored move from table
Move table_get_move(Table *table, bb key);

// Store move in table
void table_set_move(Table *table, bb key, int depth, Move move);

// Store position evaluation in table
void table_set(Table *table, bb key, int depth, int value, int flag);

// Retrieve position evaluation from table
int table_get(Table *table, bb key, int depth, int alpha, int beta, int *value);

#endif