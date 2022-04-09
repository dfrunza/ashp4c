#pragma once
#include "arena.h"
#include "ast.h"


void resolve_nameref(struct Ast* p4program, struct Hashmap* nameref_map);
