#pragma once
#include "basic.h"
#include "token.h"
#include <stdint.h>

struct TokenSequence {
  struct Token* tokens;
  int count;
  struct Arena* arena;
};

struct SourceText {
  char* text;
  int size;
  struct Arena* arena;
};

struct TokenSequence lex_tokenize(struct SourceText* source);
