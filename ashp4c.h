#pragma once

struct UnboundedArray* lex_tokenize(char* text, int text_size, struct Arena* lexeme_storage, struct Arena* tokens_storage);
void build_symtable(struct Ast_P4Program* p4program, struct Arena* symtable_storage);
struct Ast_P4Program* build_ast(struct UnboundedArray* tokens_array, struct Arena* ast_storage);
void resolve_nameref(struct Ast_P4Program* p4program);
struct Hashmap* build_type(struct Ast_P4Program* p4program, struct Arena* type_storage);
struct Type* type_get(struct Hashmap* map, uint32_t id);
void type_add(struct Hashmap* map, struct Type* type, uint32_t id);
