#pragma once
#include "lex.h"
#include "cst.h"

struct Cst* build_cst(struct Token* tokens_, int token_count_);
