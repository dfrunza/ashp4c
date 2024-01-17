#pragma once

SourceText      read_source_text(char* filename, Arena* storage);
UnboundedArray* tokenize_source_text(SourceText* source_text, Arena* storage);
Ast* parse_program(UnboundedArray* tokens, Arena* storage, Scope** root_scope);
void drypass(Ast* ast);
Set* build_open_scope(Ast* p4program, Scope* root_scope, Arena* storage);
void build_symtable(Ast* p4program, Scope* root_scope, Set* opened_scopes, Arena* storage);
Set* build_type_table(Ast* p4program, Scope* root_scope, UnboundedArray** type_array,
         Set* opened_scopes, Arena* storage);
void resolve_type_xref(Set* type_table, UnboundedArray* type_array);
Set* build_potential_types(Ast* ast, Set* type_table, Arena* storage);

