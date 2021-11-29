#pragma once
#include "basic.h"
#include "token.h"
#include <stdint.h>


struct UnboundedArray* lex_tokenize(char* text_, int text_size_, struct Arena* lexeme_storage_, struct Arena* tokens_storage_);
