
/* Syntactic analysis */

SourceText      read_source_text(char* filename, Arena* storage);
UnboundedArray* tokenize_source_text(SourceText* source_text, Arena* storage);
Ast*            parse_program(char* source_file, UnboundedArray* tokens, Arena* storage, Scope* root_scope);

/* Semantic analysis */

void drypass(char* source_file, Ast* ast);
Set* build_open_scope(char* source_file, Ast* p4program, Scope* root_scope, Arena* storage);
Set* build_symtable(char* source_file, Ast* p4program, Scope* root_scope, Set* opened_scopes,
          Set** decl_table, Arena* storage);
void build_type_table(char* source_file, Ast* p4program, Scope* root_scope, UnboundedArray* type_array,
          Set* type_table, Set* opened_scopes, Set* enclosing_scopes, Set* decl_table, Arena* storage);
Set* build_potential_types(char* source_file, Ast* ast, Scope* root_scope, Set* opened_scopes,
          Set* enclosing_scopes, Set* type_table, Set* decl_table, Arena* storage);

