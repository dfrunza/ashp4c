#pragma once
#include "basic.h"
#include "token.h"
#include <stdint.h>

struct TokenSequence {
  struct Token* tokens;
  int count;
};

struct TokenSequence lex_tokenize(char* input_text_, int input_size_);
