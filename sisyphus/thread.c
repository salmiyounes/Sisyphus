#define _POSIX_C_SOURCE 199309L

#include "thread.h"
#include "thpool.h"
#include "time.h"

void *thread_start(void *arg) {
  Thread_d *thread_d = (Thread_d *)arg;

  thread_d->score = best_move(thread_d->search, thread_d->board, thread_d->move,
                              thread_d->debug);

  return NULL;
}

void thread_stop(Search *search) { search->stop = true; }

void thread_init(Search *search, ChessBoard *board, Move *result,
                 float duration, bool debug) {
  Thread_d *thread_d = (Thread_d *)calloc(1, sizeof(Thread_d));

  if (thread_d == NULL) {
    err("thread_init(): Could not allocate memory for thread_d");
  }

  thread_d->board = board;
  thread_d->search = search;
  thread_d->move = result;
  thread_d->score = -INF;
  thread_d->debug = debug;

  threadpool thpool_p;
  if ((thpool_p = thpool_init(1)) == NULL) {
    free(thread_d);
  }

  if (thpool_add_work(thpool_p, (void *)thread_start, (void *)thread_d) == -1)
    goto cleanup;

  struct timespec ts;
  ts.tv_sec = (time_t)duration;
  ts.tv_nsec = (long)((duration - (time_t)duration) * 1000000000L);
  nanosleep(&ts, NULL);

  thread_stop(search);

cleanup:
  thpool_destroy(thpool_p);
  free(thread_d);
}
