#pragma once
#include "token.h"
#include "ast.h"


struct Ast* build_ast_program(struct UnboundedArray* tokens_array_, struct Arena* ast_storage_);
