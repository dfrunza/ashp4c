#pragma once
#include "dp4c.h"

void sym_init();
NamespaceInfo* sym_get_namespace(char* name);
IdentInfo_Type* sym_get_type(char* name);
IdentInfo_Type* sym_add_type(char* name);
IdentInfo_Selector* sym_add_selector(char* name);
IdentInfo_Type* sym_get_error_type();
IdentInfo_Selector* sym_get_selector(char* name);
void sym_remove_error_kw();
void sym_add_error_var();
void sym_remove_error_var();
void sym_add_error_kw();
int scope_push_level();
int scope_pop_level();
