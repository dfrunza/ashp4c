#pragma once
#include "token.h"
#include "ast.h"


void build_ast_program(struct Ast** p4program_, int* ast_node_count_, struct UnboundedArray* tokens_array_, struct Arena* ast_storage_);
