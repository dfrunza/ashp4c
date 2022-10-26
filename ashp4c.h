#pragma once

UnboundedArray* lex_tokenize(char* text, int text_size, Arena* lexeme_storage, Arena* tokens_storage);
Scope* build_symtable(Ast_P4Program* p4program, Arena* symtable_storage, Hashmap** nameref_map);
Ast_P4Program* build_ast(UnboundedArray* tokens_array, Arena* ast_storage);
void resolve_nameref(Ast_P4Program* p4program, Hashmap* nameref_map);
Hashmap* build_type(Ast_P4Program* p4program, Scope* root_scope, Hashmap* nameref_map, Arena* type_storage);
