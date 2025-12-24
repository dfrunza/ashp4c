#include <basic.h>
#include <namespace.h>

char* NameSpace_to_string(enum NameSpace ns)
{
  switch (ns) {
    case NameSpace::Var: return "VAR";
    case NameSpace::Type: return "TYPE";
    case NameSpace::Keyword: return "KEYWORD";

    default: return "?";
  }
  assert(0);
  return 0;
}

