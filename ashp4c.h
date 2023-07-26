#pragma once

UnboundedArray* tokenize_text(char* text, int text_size, Arena* lexeme_storage, Arena* tokens_storage);
Ast_P4Program*  parse_tokens(UnboundedArray* tokens, Arena* ast_storage);
int    pass_node_id(Ast_P4Program* p4program);
void   pass_name_decl(Ast_P4Program* p4program, Arena* storage);
void   pass_type_decl(Ast_P4Program* p4program, Arena* storage);
void   pass_potential_type(Ast_P4Program* p4program, Arena* storage);
/*
Hashmap* select_type(Ast_P4Program* p4program, Scope* root_scope_, Hashmap* potential_types, Arena* type_storage);*/
