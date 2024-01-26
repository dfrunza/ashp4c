
/* Syntactic analysis */

SourceText      read_source_text(char* filename, Arena* storage);
UnboundedArray* tokenize_source_text(SourceText* source_text, Arena* storage);
Ast* parse_program(UnboundedArray* tokens, Arena* storage, Scope** root_scope);

/* Semantic analysis */

void drypass(Ast* ast);
Set* build_open_scope(Ast* p4program, Scope* root_scope, Arena* storage);
Set* build_symtable(Ast* p4program, Scope* root_scope, Set* opened_scopes, Arena* storage);
Set* build_type_table(Ast* p4program, Scope* root_scope, Set* opened_scopes, Set* enclosing_scopes, Arena* storage);
Set* build_potential_types(Ast* ast, Scope* root_scope,
        Set* opened_scopes,  Set* enclosing_scopes, Set* type_table, Arena* storage);
void Debug_potential_types(Ast* p4program, Set* potential_types);

