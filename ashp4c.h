void   tokenize(Lexer* lexer, SourceText* source_text);
void   parse(Parser* parser);
void   drypass(char* source_file, Ast* ast);
void   builtin_methods(Arena* storage, char* source_file, Ast* ast);
Map*   scope_hierarchy(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope);
Map*   name_bind(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope,
          Map* scope_map, Array** type_array);
Map*   declared_types(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope,
          Array* type_array, Map* scope_map, Map* decl_map);
Map*   potential_types(Arena* storage, char* source_file, Ast* ast, Scope* root_scope,
          Map* scope_map, Map* decl_map, Map* type_env);
void   select_type(Arena* storage_, char* source_file_, Ast* p4program, Scope* root_scope_,
          Array* type_array, Map* scope_map_, Map* decl_map_, Map* type_env_, Map* potype_map);

