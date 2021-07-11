#include "hash.h"
#include <memory.h>  // memset
#include <math.h>

static const uint32_t P = 257, Q = 4294967029;
static const uint32_t SIGMA = 2654435769;

internal uint32_t
fold_string(uint8_t* string)
{
  uint32_t K = 0;
  uint8_t* s;
  for(s = string; (*s); s++) {
    K = (P * K + (*s)) % Q;
  }
  return K;
}

internal uint32_t
fold_bytes(uint8_t* bytes, int length)
{
  uint32_t K = 0;
  uint8_t* b;
  int i;
  for(i = 0; i < length; i++) {
    K = (P * K + bytes[i]) % Q;
  }
  return K;
}

internal uint32_t
multiply_hash(uint32_t K, uint32_t m)
{
  uint64_t Ksigma = (uint64_t)K * (uint64_t)SIGMA;
  uint32_t h = ((uint32_t)Ksigma) >> (32 - m);  // 0 <= h < 2^{m}
  return h;
}

uint32_t
hash_string(uint8_t* string, uint32_t m)
{
  uint32_t K = fold_string(string);
  uint32_t h = multiply_hash(K, m) % ((1 << m) - 1);  // 0 <= h < 2^{m} - 1
  return h;
}

uint32_t
hash_bytes(uint8_t* bytes, int length, uint32_t m)
{
  uint32_t K = fold_bytes(bytes, length);
  uint32_t h = multiply_hash(K, m);
  return h;
}

