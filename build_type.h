#pragma once
#include "arena.h"
#include "ast.h"
#include "type.h"


struct Hashmap* build_type(struct Ast* p4program, struct Arena* type_storage);
