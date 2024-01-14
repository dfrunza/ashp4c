#pragma once

SourceText      read_source_text(char* filename, Arena* storage);
UnboundedArray* tokenize_source_text(SourceText* source_text, Arena* storage);
Ast*            parse_program(UnboundedArray* tokens, Arena* storage, Scope** root_scope);
void            drypass(Ast* ast);
Hashmap*        build_open_scope(Ast* p4program, Scope* root_scope, Arena* storage);
void            build_symtable(Ast* p4program, Scope* root_scope, Hashmap* opened_scopes, Arena* storage);
/*
void            build_name_decl(Ast* ast, Scope* root_scope,
                    Hashmap** scope_map, Hashmap** field_map, Arena* storage); */
Hashmap*        build_type_table(Ast* p4program, Scope* root_scope, UnboundedArray** type_array,
                    Hashmap* opened_scopes, Arena* storage);
void            resolve_type_idref(Hashmap* type_table, UnboundedArray* type_array);
void            resolve_type_nameref(Hashmap* type_table, UnboundedArray* type_array);
void            detect_type_cycle(UnboundedArray* type_array);
Hashmap*        build_potential_types(Ast* ast, Scope* root_scope, Hashmap* scope_map, Arena* storage);

