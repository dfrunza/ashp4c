Array* tokenize(Arena* storage, SourceText* source_text);
Ast*   parse(Arena* storage, char* source_file, Array* tokens, Scope** root_scope);
void   drypass(char* source_file, Ast* ast);
Map*   scope_hierarchy(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope);
Map*   name_binding(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope,
          Map* scope_map, Array** type_array);
Map*   declared_types(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope,
          Array* type_array, Map* scope_map, Map* decl_map);
Map*   potential_types(Arena* storage, char* source_file, Ast* ast, Scope* root_scope,
          Map* scope_map, Map* decl_map, Map* type_env);

