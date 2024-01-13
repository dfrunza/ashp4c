#pragma once

SourceText      read_source_text(char* filename, Arena* storage);
UnboundedArray* tokenize_source_text(SourceText* source_text, Arena* storage);
Ast*            parse_program(UnboundedArray* tokens, Arena* storage, Scope** root_scope);
void            drypass(Ast* ast);
Hashmap*        pass_open_scope(Ast* ast, Scope* root_scope, Arena* storage);
/*
void            pass_name_decl(Ast* ast, Scope* root_scope,
                  Hashmap** scope_map, Hashmap** field_map, Arena* storage); */
Hashmap*        pass_type_decl(Ast* ast, Scope* root_scope, Hashmap* opened_scopes, Arena* storage);
Hashmap*        pass_potential_types(Ast* ast, Scope* root_scope, Hashmap* scope_map, Arena* storage);

