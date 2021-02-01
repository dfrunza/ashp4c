#define DEBUG_ENABLED 1

#include "syntax.h"

external Arena arena;
internal int tab_level = 0;
internal int tab_size = 2;

enum ValueType {
  Value_None,
  Value_Int,
  Value_String,
};

internal char*
cst_kind_to_string(enum CstKind kind)
{
  switch (kind) {
    case Cst_NonTypeName:
      return "Cst_NonTypeName";
    case Cst_TypeName:
      return "Cst_TypeName";
    case Cst_PrefixedTypeName:
      return "Cst_PrefixedTypeName";
    case Cst_BaseType:
      return "Cst_BaseType";
    case Cst_DotPrefixedName:
      return "Cst_DotPrefixedName";
    case Cst_ConstDecl:
      return "Cst_ConstDecl";
    case Cst_ExternDecl:
      return "Cst_ExternDecl";
    case Cst_Constructor:
      return "Cst_Constructor";
    case Cst_FunctionProto:
      return "Cst_FunctionProto";
    case Cst_ActionDecl:
      return "Cst_ActionDecl";
    case Cst_HeaderDecl:
      return "Cst_HeaderDecl";
    case Cst_HeaderUnionDecl:
      return "Cst_HeaderUnionDecl";
    case Cst_StructDecl:
      return "Cst_StructDecl";
    case Cst_EnumDecl:
      return "Cst_EnumDecl";
    case Cst_Parser:
      return "Cst_Parser";
    case Cst_Control:
      return "Cst_Control";
    case Cst_Package:
      return "Cst_Package";
    case Cst_Instantiation:
      return "Cst_Instantiation";
    case Cst_Error:
      return "Cst_Error";
    case Cst_MatchKind:
      return "Cst_MatchKind";
    case Cst_FunctionDecl:
      return "Cst_FunctionDecl";
    case Cst_Dontcare:
      return "Cst_Dontcare";
    case Cst_IntTypeSize:
      return "Cst_IntTypeSize";
    case Cst_Int:
      return "Cst_Int";
    case Cst_StringLiteral:
      return "Cst_StringLiteral";
    case Cst_Tuple:
      return "Cst_Tuple";
    case Cst_HeaderStack:
      return "Cst_HeaderStack";
    case Cst_SpecdType:
      return "Cst_SpecdType";
    case Cst_StructField:
      return "Cst_StructField";
    case Cst_SpecdId:
      return "Cst_SpecdId";
    case Cst_ParserType:
      return "Cst_ParserType";
    case Cst_Argument:
      return "Cst_Argument";
    case Cst_VarDecl:
      return "Cst_VarDecl";
    case Cst_DirectApplic:
      return "Cst_DirectApplic";
    case Cst_ArrayIndex:
      return "Cst_ArrayIndex";
    case Cst_ParamDir:
      return "Cst_ParamDir";
    case Cst_Parameter:
      return "Cst_Parameter";
    case Cst_Lvalue:
      return "Cst_Lvalue";
    case Cst_AssignmentStmt:
      return "Cst_AssignmentStmt";
    case Cst_MethodCallStmt:
      return "Cst_MethodCallStmt";
    case Cst_EmptyStmt:
      return "Cst_EmptyStmt";
    case Cst_Default:
      return "Cst_Default";
    case Cst_SelectCase:
      return "Cst_SelectCase";
    case Cst_ParserState:
      return "Cst_ParserState";
    case Cst_ControlType:
      return "Cst_ControlType";
    case Cst_KeyElement:
      return "Cst_KeyElement";
    case Cst_ActionRef:
      return "Cst_ActionRef";
    case Cst_TableEntry:
      return "Cst_TableEntry";
    case Cst_TableProp_Key:
      return "Cst_TableProp_Key";
    case Cst_TableProp_Actions:
      return "Cst_TableProp_Actions";
    case Cst_TableProp_Entries:
      return "Cst_TableProp_Entries";
    case Cst_TableProp_SingleEntry:
      return "Cst_TableProp_SingleEntry";
    case Cst_TableDecl:
      return "Cst_TableDecl";
    case Cst_IfStmt:
      return "Cst_IfStmt";
    case Cst_ExitStmt:
      return "Cst_ExitStmt";
    case Cst_ReturnStmt:
      return "Cst_ReturnStmt";
    case Cst_SwitchLabel:
      return "Cst_SwitchLabel";
    case Cst_SwitchCase:
      return "Cst_SwitchCase";
    case Cst_SwitchStmt:
      return "Cst_SwitchStmt";
    case Cst_BlockStmt:
      return "Cst_BlockStmt";
    case Cst_ExpressionListExpr:
      return "Cst_ExpressionListExpr";
    case Cst_CastExpr:
      return "Cst_CastExpr";
    case Cst_UnaryExpr:
      return "Cst_UnaryExpr";
    case Cst_BinaryExpr:
      return "Cst_BinaryExpr";
    case Cst_MemberSelectExpr:
      return "Cst_MemberSelectExpr";
    case Cst_IndexedArrayExpr:
      return "Cst_IndexedArrayExpr";
    case Cst_FunctionCallExpr:
      return "Cst_FunctionCallExpr";
    case Cst_TypeArgsExpr:
      return "Cst_TypeArgsExpr";
    case Cst_P4Program:
      return "Cst_P4Program";
    default:
      assert(0);
  }
  return 0;
}

internal void
indent_right()
{
  int i = 0;
  for (; i < tab_level*tab_size; i++) {
    printf(" ");
  }
}

internal void
object_start()
{
  printf("{\n");
  tab_level++;
}

internal void
object_end()
{
  tab_level--;
  printf("}\n");
}

internal void
list_open()
{
  printf("[");
}

internal void
list_close()
{
  printf("]\n");
}

internal void
newline()
{
  printf("\n");
}

internal void
print_prop(char* name, enum ValueType type, ...)
{
  indent_right();
  printf("%s: ", name);
  if (type != Value_None) {
    va_list value;
    va_start(value, type);
    if (type == Value_Int) {
      vprintf("%d\n", value);
    } else if (type == Value_String) {
      vprintf("\"%s\"\n", value);
    }
    va_end(value);
  }
}

internal void
print_list_elem(enum ValueType type, ...)
{
  if (type != Value_None) {
    va_list value;
    va_start(value, type);
    if (type == Value_Int) {
      vprintf("%d, ", value);
    } else if (type == Value_String) {
      vprintf("\"%s\", ", value);
    }
    va_end(value);
  }
}

internal void
print_cst_common(struct Cst* cst)
{
  print_prop("id", Value_Int, cst->id);
  print_prop("kind", Value_String, cst_kind_to_string(cst->kind));
  print_prop("line_nr", Value_Int, cst->line_nr);
}

internal void
dump_NonTypeName(struct Cst_NonTypeName* name)
{
  object_start();
  print_cst_common((struct Cst*)name);
  print_prop("name", Value_String, name->name);
  object_end();
}

internal void
dump_TypeName(struct Cst_TypeName* name)
{
  object_start();
  print_cst_common((struct Cst*)name);
  print_prop("name", Value_String, name->name);
  object_end();
}

internal void
dump_Error(struct Cst_Error* error)
{
  object_start();
  print_cst_common((struct Cst*)error);
  print_prop("id_list", Value_None);
  list_open();
  struct Cst* id = error->id_list;
  while (id) {
    print_list_elem(Value_Int, id->id);
    id = id->link.next_node;
  }
  list_close();
  object_end();

  id = error->id_list;
  while (id) {
    if (id->kind == Cst_NonTypeName) {
      dump_NonTypeName((struct Cst_NonTypeName*)id);
    } else if (id->kind == Cst_TypeName) {
      dump_TypeName((struct Cst_TypeName*)id);
    } else assert(0);
    id = id->link.next_node;
  }
}

void
dump_P4Program(struct Cst_P4Program* prog)
{
  object_start();
  print_cst_common((struct Cst*)prog);
  print_prop("decl_list", Value_None);
  list_open();
  struct Cst* decl = prog->decl_list;
  while (decl) {
    print_list_elem(Value_Int, decl->id);
    decl = decl->link.next_node;
  }
  list_close();
  object_end();

  decl = prog->decl_list;
  while (decl) {
    if (decl->kind == Cst_ConstDecl) {
      printf("TODO: Cst_ConstDecl\n");
    } else if (decl->kind == Cst_ExternDecl) {
      printf("TODO: Cst_ExternDecl\n");
    } else if (decl->kind == Cst_FunctionProto) {
      printf("TODO: Cst_FunctionProto\n");
    } else if (decl->kind == Cst_ActionDecl) {
      printf("TODO: Cst_ActionDecl\n");
    } else if (decl->kind == Cst_HeaderDecl) {
      printf("TODO: Cst_HeaderDecl\n");
    } else if (decl->kind == Cst_HeaderUnionDecl) {
      printf("TODO: Cst_HeaderUnionDecl\n");
    } else if (decl->kind == Cst_StructDecl) {
      printf("TODO: Cst_StructDecl\n");
    } else if (decl->kind == Cst_EnumDecl) {
      printf("TODO: Cst_EnumDecl\n");
    } else if (decl->kind == Cst_TypeDecl) {
      printf("TODO: Cst_TypeDecl\n");
    } else if (decl->kind == Cst_Parser) {
      printf("TODO: Cst_Parser\n");
    } else if (decl->kind == Cst_Control) {
      printf("TODO: Cst_Control\n");
    } else if (decl->kind == Cst_Package) {
      printf("TODO: Cst_Package\n");
    } else if (decl->kind == Cst_Instantiation) {
      printf("TODO: Cst_Instantiation\n");
    } else if (decl->kind == Cst_Error) {
      dump_Error((struct Cst_Error*)decl);
    } else if (decl->kind == Cst_MatchKind) {
      printf("TODO: Cst_MatchKind\n");
    } else if (decl->kind == Cst_FunctionDecl) {
      printf("TODO: Cst_FunctionDecl\n");
    } else assert(0);
    decl = decl->link.next_node;
  }
}
