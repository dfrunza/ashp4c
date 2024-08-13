Array* tokenize(Arena* storage, SourceText* source_text);
Ast*   parse(Arena* storage, char* source_file, Array* tokens, Scope** root_scope);
void   drypass(char* source_file, Ast* ast);
Map*   build_scopes(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope);
Map*   build_symtable(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope,
          Map* scope_map, Array** type_array);
Map*   build_type_env(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope,
          Array* type_array, Map* scope_map, Map* decl_map);
Map*   build_potential_types(Arena* storage, char* source_file, Ast* ast, Scope* root_scope,
          Map* scope_map, Map* decl_map, Map* type_env);

