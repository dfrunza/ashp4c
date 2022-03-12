#pragma once


void resolve_nameref_type(struct Hashmap* nameref_table, struct UnboundedArray* type_names);
void resolve_nameref_var(struct Hashmap* nameref_table, struct UnboundedArray* var_names);
