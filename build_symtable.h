#pragma once
#include "arena.h"
#include "ast.h"


void build_symtable_program(struct Ast* ast, struct Arena* symtable_storage);

