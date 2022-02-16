#pragma once
#include "arena.h"
#include "ast.h"


struct Hashmap* build_symtable(struct Ast* p4program, struct Arena* symtable_storage);

