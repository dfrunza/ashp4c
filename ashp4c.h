#pragma once
#include "foundation.h"
#include "frontend.h"
#include "ast_visitor.h"

struct DryPass : AstVisitor {
  void do_pass(Ast* ast);
};

void tokenize(Lexer* lexer, SourceText* source_text);
void parse(Parser* parser);
void builtin_methods(BuiltinMethodBuilder* builder, Ast* ast);
void scope_hierarchy(ScopeBuilder* scope_builder);
void name_bind(NameBinder* name_binder);
void declared_types(TypeChecker* type_checker);
void potential_types(TypeChecker* type_checker);
void select_type(TypeChecker* type_checker);

