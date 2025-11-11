#include "frontend.h"
#include "ashp4c.h"

void DryPass::do_pass(Ast* ast)
{
  visit_p4program(ast);
}
