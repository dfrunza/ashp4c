#pragma once
#include "basic.h"
#include "token.h"
#include <stdint.h>


struct UnboundedArray* lex_tokenize(char* text, int text_size, struct Arena* lexeme_storage, struct Arena* tokens_storage);
