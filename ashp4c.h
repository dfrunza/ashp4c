Array* tokenize_source_text(Arena* storage, SourceText* source_text);
Ast*   parse_program(Arena* storage, char* source_file, Array* tokens, Scope* root_scope);
void   drypass(char* source_file, Ast* ast);
Map*   build_scopes(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope);
void   build_symtable(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope,
          Map* scop_map, Map** decl_map);
void   build_type_env(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope, Array* type_array,
          Map* scop_map, Map* decl_map, Map* type_env);
Map*   build_potential_types(Arena* storage, char* source_file, Ast* ast, Scope* root_scope,
          Map* scop_map, Map* decl_map, Map* type_env);

