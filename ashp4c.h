#pragma once

struct UnboundedArray* lex_tokenize(char* text, int text_size, struct Arena* lexeme_storage, struct Arena* tokens_storage);
struct Scope* build_symtable(struct Ast_P4Program* p4program, struct Arena* symtable_storage, struct Hashmap** nameref_map);
struct Ast_P4Program* build_ast(struct UnboundedArray* tokens_array, struct Arena* ast_storage);
void resolve_nameref(struct Ast_P4Program* p4program, struct Hashmap* nameref_map);
struct Hashmap* build_type(struct Ast_P4Program* p4program, struct Scope* root_scope,
                           struct Hashmap* nameref_map, struct Arena* type_storage);
