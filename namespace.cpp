#include <basic.h>
#include <namespace.h>

char* NameSpace_to_string(enum NameSpace ns)
{
  switch (ns) {
    case NameSpace::VAR: return "VAR";
    case NameSpace::TYPE: return "TYPE";
    case NameSpace::KEYWORD: return "KEYWORD";

    default: return "?";
  }
  assert(0);
  return 0;
}

