#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>

void swap_any(void *a, void *b, size_t s) {
  void *temp = malloc(s);
  memcpy(temp, a, s);
  memcpy(a, b, s);
  memcpy(b, temp, s);
  free(temp);
}

char *xstrdup(const char *src) {
  char *s2;
  size_t len;
  len = strlen(src);
  if (!(s2 = malloc(len + 1)))
    die("fatal error: out of memory");
  return memcpy(s2, src, len + 1);
}

bb xorshift64() {
  // https://en.wikipedia.org/wiki/Xorshift
  // https://vigna.di.unimi.it/ftp/papers/xorshift.pdf
  static bb x = U64(1);
  x ^= x >> 12;
  x ^= x << 25;
  x ^= x >> 27;
  return x * U64(0x2545F4914F6CDD1D);
}

long gettimeinms() {
  struct timeval time;
  gettimeofday(&time, NULL);

  return time.tv_sec * 1000 + time.tv_usec / 1000;
}