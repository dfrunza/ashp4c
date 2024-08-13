Array* tokenize_source_text(Arena* storage, SourceText* source_text);
Ast*   parse_program(Arena* storage, char* source_file, Array* tokens, Scope* root_scope);
void   drypass(char* source_file, Ast* ast);
Map*   build_opened_scopes(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope);
Map*   build_symtable(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope, Map* opened_scopes,
          Map** decl_map);
void   build_type_env(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope, Array* type_array,
          Map* type_env, Map* opened_scopes, Map* enclosing_scopes, Map* decl_map);
Map*   build_potential_types(Arena* storage, char* source_file, Ast* ast, Scope* root_scope, Map* opened_scopes,
          Map* enclosing_scopes, Map* decl_map, Map* type_env);

