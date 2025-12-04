#include <ast.h>
#include <passes/drypass.h>

void DryPass::do_pass(Ast* ast)
{
  visit_p4program(ast);
}
