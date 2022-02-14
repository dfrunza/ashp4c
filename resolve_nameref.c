#include "arena.h"
#include "ast.h"
#include "symtable.h"


void resolve_nameref(struct Scope* scope);

void
resolve_nameref(struct Scope* scope)
{
  if (scope->right_sibling_scope) {
    resolve_nameref(scope->right_sibling_scope);
  }
  if (scope->first_child_scope) {
    resolve_nameref(scope->first_child_scope);
  }
}
