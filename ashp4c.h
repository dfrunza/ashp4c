#pragma once

UnboundedArray* tokenize_text(char* text, int text_size, Arena* lexeme_storage, Arena* tokens_storage);
Ast_P4Program*  parse_tokens(UnboundedArray* tokens, Arena* ast_storage, Scope** root_scope);
void     pass_dry(Ast_P4Program* p4program, Scope* root_scope);
void     pass_name_decl(Ast_P4Program* p4program, Arena* storage, Scope* root_scope);
Hashmap* pass_type_decl(Ast_P4Program* p4program, Arena* storage, Scope* root_scope);
Hashmap* pass_potential_type(Ast_P4Program* p4program, Arena* storage, Hashmap* type_table);
/*
Hashmap* select_type(Ast_P4Program* p4program, Scope* root_scope_, Hashmap* potential_types, Arena* type_storage);*/
