#pragma once
#include "token.h"
#include "cst.h"

struct Cst* build_cst(struct Token* tokens_, int token_count_);
