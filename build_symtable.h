#pragma once
#include "arena.h"
#include "ast.h"

void build_symtable(struct Ast_P4Program* p4program, struct Arena* scope_storage);
