#pragma once
#include "arena.h"
#include "ast.h"


void build_nameref(struct Ast* p4program, struct Hashmap* nameref_table_, struct UnboundedArray* type_names_, struct UnboundedArray* var_names_, struct UnboundedArray* member_names_);
