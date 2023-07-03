#pragma once

UnboundedArray* tokenize_text(char* text, int text_size, Arena* lexeme_storage, Arena* tokens_storage);
Ast_P4Program* parse_tokens(UnboundedArray* tokens, Arena* ast_storage);
void node_id_pass(Ast_P4Program* p4program);
Scope* name_decl_pass(Ast_P4Program* p4program, Arena* decl_storage);
Hashmap* build_type_decl(Ast_P4Program* p4program, Scope* root_scope, Arena* type_storage);
Hashmap* build_potential_type(Ast_P4Program* p4program, Scope* root_scope, Arena* type_storage);
Hashmap* select_type(Ast_P4Program* p4program, Scope* root_scope_, Hashmap* potential_types, Arena* type_storage);
