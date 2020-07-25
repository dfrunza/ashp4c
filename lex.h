#pragma once
#include <stdio.h>
#include "basic.h"
#include "arena.h"

void lex_lexeme_init(char* input_text);
void lex_next_token();
int lex_line_nr();


