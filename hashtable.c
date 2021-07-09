#include "hashtable.h"
#include <memory.h>  // memset
#include <math.h>

static const uint32_t JAS_HASH_P = 257, JAS_HASH_Q = 4294967029;
static const uint32_t JAS_HASH_SIGMA = 2654435769;

internal uint32_t
fold_string(uint8_t* string)
{
  uint64_t K = 0;
  uint8_t* s;
  for(s = string; (*s); s++) {
    K = (JAS_HASH_P * K + (*s)) % JAS_HASH_Q;
  }
  return K;
}

internal uint32_t
fold_bytes(uint8_t* bytes, int length)
{
  uint64_t K = 0;
  uint8_t* b;
  int i;
  for(i = 0; i < length; i++) {
    K = (JAS_HASH_P * K + bytes[i]) % JAS_HASH_Q;
  }
  return K;
}

internal uint32_t
multiply_hash(uint32_t K, uint32_t m)
{
  uint64_t Ksigma = K * JAS_HASH_SIGMA;
  uint32_t h = (Ksigma & 0xffffffff) >> (32 - m);
  return h;
}

uint32_t
hash_string(uint8_t* string, uint32_t m)
{
  uint64_t K = fold_string(string);
  uint32_t h = multiply_hash(K, m);
  return h;
}

uint32_t
hash_bytes(uint8_t* bytes, int length, uint32_t m)
{
  uint64_t K = fold_bytes(bytes, length);
  uint32_t h = multiply_hash(K, m);
  return h;
}

