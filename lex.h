#pragma once

struct UnboundedArray* lex_tokenize(char* text, int text_size, struct Arena* lexeme_storage, struct Arena* tokens_storage);
