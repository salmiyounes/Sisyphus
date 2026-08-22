#ifndef THREAD_H
#define THREAD_H

#include "search.h"
#include "types.h"
#include <stdbool.h>

void thread_init(Search *search, ChessBoard *board, Move *result,
                 float duration, bool debug); // Initialize search thread
void thread_stop(Search *search);             // Stop search thread

#endif // THREAD_H