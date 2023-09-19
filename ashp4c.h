#pragma once

SourceText*     read_source_text(char* filename, Arena* storage);
UnboundedArray* tokenize_source_text(SourceText* source_text, Arena* storage);
Ast_P4Program*  parse_program(UnboundedArray* tokens, Scope** root_scope, Arena* storage);
void            drypass(Ast_P4Program* ast);
void            name_decl(Ast_P4Program* ast, Scope* root_scope,
                  Hashmap** scope_map, Hashmap** field_map, Arena* storage);
Hashmap*        type_decl(Ast_P4Program* ast, Arena* storage);
Hashmap*        potential_type(Ast_P4Program* ast, Scope* root_scope, 
                  Hashmap* scope_map, Hashmap* type_table, Arena* storage);

