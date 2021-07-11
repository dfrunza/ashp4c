#pragma once
#include "basic.h"
#include "token.h"
#include <stdint.h>


void lex_tokenize(char* text_, int text_size_, struct Arena* arena_, struct UnboundedArray* tokens_array_);
