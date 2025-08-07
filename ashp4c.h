void   tokenize(Lexer* lexer, SourceText* source_text);
void   parse(Parser* parser);
void   drypass(Ast* ast);
void   builtin_methods(Arena* storage, char* source_file, Ast* ast);
void   scope_hierarchy(ScopeBuilder* scope_builder);
void   name_bind(NameBinder* name_binder);
void   declared_types(TypeChecker* type_checker);
void   potential_types(TypeChecker* type_checker);
void   select_type(TypeChecker* type_checker);

