#pragma once

struct Ast_P4Program* build_ast(struct UnboundedArray* tokens_array, struct Arena* ast_storage);
