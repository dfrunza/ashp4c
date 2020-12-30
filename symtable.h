#pragma once
#include "dp4c.h"

void sym_init();
Namespace_Entry* sym_get_namespace(char* name);
Ident* sym_get_var(char* name);
Ident* sym_add_var(char* name, Ast* ast);
Ident* sym_get_type(char* name);
Ident* sym_add_type(char* name, Ast* ast);
Ident* sym_add_typevar(char* name, Ast* ast);
Ident* sym_get_error_type();
void sym_remove_error_kw();
void sym_add_error_var();
void sym_remove_error_var();
void sym_add_error_kw();
int scope_push_level();
int scope_pop_level();
bool sym_ident_is_declared(Ident* ident);
