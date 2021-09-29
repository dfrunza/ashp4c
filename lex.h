#pragma once
#include "basic.h"
#include "token.h"
#include <stdint.h>


void lex_tokenize(char* text_, int text_size_, struct UnboundedArray* tokens_array_, struct Arena* lexeme_storage_, struct Arena* tokens_storage_);
