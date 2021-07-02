#pragma once
#include "token.h"
#include "ast.h"

void build_AstTree(struct Ast** p4program_, int* ast_node_count_, struct Token* tokens_array_, int token_count_, struct Arena* ast_storage_, struct Arena* symtable_storage_);
