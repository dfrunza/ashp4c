#pragma once
#include "token.h"
#include "ast.h"


struct Ast* build_ast(struct UnboundedArray* tokens_array, struct Arena* ast_storage);
