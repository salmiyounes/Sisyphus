#include "utils.h"

void swap_any(void *a, void *b, size_t s) {
  void *temp = malloc(s);
  memcpy(temp, a, s);
  memcpy(a, b, s);
  memcpy(b, temp, s);
  free(temp);
}

char *strdup(const char *src) {
  if (src == NULL)
    return NULL;

  size_t len = strlen(src) + 1;
  void *new_s = malloc(len);

  if (new_s == NULL)
    return NULL;

  return (char *)memcpy(new_s, src, len);
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
