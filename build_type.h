#pragma once
#include "arena.h"
#include "ast.h"
#include "type.h"

struct Hashmap* build_type(struct Ast_P4Program* p4program, struct Arena* type_storage);
struct Type* type_get(struct Hashmap* map, uint32_t id);
void type_add(struct Hashmap* map, struct Type* type, uint32_t id);
