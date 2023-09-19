#pragma once

SourceText*     read_source_text(char* filename, Arena* storage);
UnboundedArray* tokenize_source_text(SourceText* source_text, Arena* storage);
Ast_P4Program*  parse_program(UnboundedArray* tokens, Arena* storage, Scope** root_scope);
void            pass_dry(Ast_P4Program* ast);
void            pass_name_decl(Ast_P4Program* ast, Scope* root_scope, Hashmap** scope_map, Hashmap** field_map, Arena* storage);
Hashmap*        pass_type_decl(Ast_P4Program* ast, Arena* storage);
Hashmap*        pass_potential_type(Ast_P4Program* ast, Scope* root_scope, Arena* storage,
      Hashmap* scope_map, Hashmap* type_table);

