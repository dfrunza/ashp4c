#pragma once
#include "ast.h"


struct UnboundedArray* collect_name_ref_program(struct Ast* ast, struct Arena* name_ref_storage);
