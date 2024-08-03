
/* Syntactic analysis */

UnboundedArray* tokenize_source_text(SourceText* source_text, Arena* storage);
Ast*            parse_program(char* source_file, UnboundedArray* tokens, Arena* storage, Scope* root_scope);

/* Semantic analysis */

void drypass(char* source_file, Ast* ast);
Set* build_opened_scopes(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope);
Set* build_symtable(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope, Set* opened_scopes,
        Set** decl_table);
void build_type_env(Arena* storage, char* source_file, Ast* p4program, Scope* root_scope, UnboundedArray* type_array,
        Set* type_env, Set* opened_scopes, Set* enclosing_scopes, Set* decl_table);
Set* build_potential_types(Arena* storage, char* source_file, Ast* ast, Scope* root_scope, Set* opened_scopes,
        Set* enclosing_scopes, Set* type_env, Set* decl_table);

