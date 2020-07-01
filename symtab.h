#pragma once
#include "basic.h"
#include "arena.h"
#include "lex.h"

void sym_init(Arena* arena_);
NamespaceInfo* sym_get_namespace(char* name);
IdentInfo_Type* sym_get_type(char* name);
int sym_scope_get_level();
int sym_scope_push_level();
int sym_scope_pop_level();
IdentInfo_Type* sym_add_type(char* name);
IdentInfo_Selector* sym_add_selector(char* name);
IdentInfo* sym_add_error_code(char* name);
IdentInfo_Type* sym_get_error_type();
IdentInfo_Selector* sym_get_selector(char* name);
