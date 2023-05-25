#pragma once

UnboundedArray* tokenize_text(char* text, int text_size, Arena* lexeme_storage, Arena* tokens_storage);
Scope* build_symtable(Ast_P4Program* p4program, Arena* symtable_storage);
Ast_P4Program* parse_tokens(UnboundedArray* tokens, Arena* ast_storage);
Hashmap* build_possible_type(Ast_P4Program* p4program, Scope* root_scope, Arena* type_storage);
Hashmap* select_type(Ast_P4Program* p4program, Scope* root_scope_, Hashmap* potential_types, Arena* type_storage);
