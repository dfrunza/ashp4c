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

NameDeclaration* NameDeclaration::create(Arena* storage, char* strname)
{
  NameDeclaration* name_decl = storage->allocate<NameDeclaration>();
  name_decl->strname = strname;
  return name_decl;
}

NameDeclaration* NameEntry::get_declarations(enum NameSpace ns)
{
  NameDeclaration* decls = declarations[(int)ns >> 1];
  return decls;
}

void NameEntry::new_declaration(NameDeclaration* name_decl, enum NameSpace ns)
{
  name_decl->next_in_scope = declarations[(int)ns >> 1];
  declarations[(int)ns >> 1] = name_decl;
}
