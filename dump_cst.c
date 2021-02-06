#define DEBUG_ENABLED 1

#include "syntax.h"

external Arena arena;
internal int tab_level = 0;
internal int tab_size = 2;

enum ValueType {
  Value_None,
  Value_Int,
  Value_String,
  Value_Ref,
  Value_RefList,
};

internal void dump_Cst(struct Cst*);
internal void print_prop(char* name, enum ValueType type, ...);
internal void print_list_elem(enum ValueType type, ...);

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
    case Cst_TypeDecl:
      return "Cst_TypeDecl";
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
  printf("]");
}

internal void
print_nl()
{
  printf("\n");
}

internal void
print_value(enum ValueType type, va_list value)
{
  if (type == Value_Int) {
    int i = va_arg(value, int);
    printf("%d", i);
  } else if (type == Value_String) {
    char* s = va_arg(value, char*);
    printf("\"%s\"", s);
  } else if (type == Value_Ref) {
    struct Cst* cst = va_arg(value, struct Cst*);
    if (cst) {
      printf("\"$%d\"", cst->id);
    } else {
      printf("null");
    }
  } else if (type == Value_RefList) {
    struct Cst* cst = va_arg(value, struct Cst*);
    list_open();
    while (cst) {
      print_list_elem(Value_Ref, cst);
      cst = cst->link.next_node;
      if (cst) {
        printf(", ");
      }
    }
    list_close();
  } else assert(0);
}

internal void
print_prop(char* name, enum ValueType type, ...)
{
  indent_right();
  printf("%s: ", name);
  if (type != Value_None) {
    va_list value;
    va_start(value, type);
    print_value(type, value);
    print_nl();
    va_end(value);
  }
}

internal void
print_list_elem(enum ValueType type, ...)
{
  if (type != Value_None) {
    va_list value;
    va_start(value, type);
    print_value(type, value);
    va_end(value);
  }
}

internal void
print_prop_common(struct Cst* cst)
{
  print_prop("id", Value_Ref, cst);
  print_prop("kind", Value_String, cst_kind_to_string(cst->kind));
  print_prop("line_nr", Value_Int, cst->line_nr);
}

internal void
dump_NonTypeName(struct Cst_NonTypeName* name)
{
  object_start();
  print_prop_common((struct Cst*)name);
  print_prop("name", Value_String, name->name);
  object_end();
}

internal void
dump_TypeName(struct Cst_TypeName* name)
{
  object_start();
  print_prop_common((struct Cst*)name);
  print_prop("name", Value_String, name->name);
  object_end();
}

internal void
dump_Error(struct Cst_Error* error)
{
  object_start();
  print_prop_common((struct Cst*)error);
  print_prop("id_list", Value_RefList, error->id_list);
  object_end();
  dump_Cst(error->id_list);
}

internal void
dump_Control(struct Cst_Control* control)
{
  object_start();
  print_prop_common((struct Cst*)control);
  print_prop("type_decl", Value_Ref, control->type_decl);
  print_prop("ctor_params", Value_RefList, control->ctor_params);
  print_prop("local_decls", Value_RefList, control->local_decls);
  print_prop("apply_stmt", Value_Ref, control->apply_stmt);
  object_end();
  dump_Cst(control->type_decl);
  dump_Cst(control->ctor_params);
  dump_Cst(control->local_decls);
  dump_Cst(control->apply_stmt);
}

internal void
dump_ControlType(struct Cst_ControlType* control)
{
  object_start();
  print_prop_common((struct Cst*)control);
  print_prop("name", Value_Ref, control->name);
  print_prop("type_params", Value_RefList, control->type_params);
  print_prop("params", Value_RefList, control->params);
  object_end();
  dump_Cst(control->name);
  dump_Cst(control->type_params);
  dump_Cst(control->params);
}

void
dump_Package(struct Cst_Package* package)
{
  object_start();
  print_prop_common((struct Cst*)package);
  print_prop("name", Value_Ref, package->name);
  print_prop("type_params", Value_RefList, package->type_params);
  object_end();
  dump_Cst(package->name);
  dump_Cst(package->type_params);
  dump_Cst(package->params);
}

void
dump_P4Program(struct Cst_P4Program* prog)
{
  object_start();
  print_prop_common((struct Cst*)prog);
  print_prop("decl_list", Value_RefList, prog->decl_list);
  object_end();
  dump_Cst(prog->decl_list);
}

void
dump_Instantiation(struct Cst_Instantiation* inst)
{
  object_start();
  print_prop_common((struct Cst*)inst);
  print_prop("type", Value_Ref, inst->type);
  print_prop("args", Value_RefList, inst->args);
  print_prop("name", Value_Ref, inst->name);
  object_end();
  dump_Cst(inst->type);
  dump_Cst(inst->args);
  dump_Cst(inst->name);
}

void
dump_Parameter(struct Cst_Parameter* param)
{
  object_start();
  print_prop_common((struct Cst*)param);
  print_prop("direction", Value_Ref, param->direction);
  print_prop("type", Value_Ref, param->type);
  print_prop("name", Value_Ref, param->name);
  print_prop("init_expr", Value_Ref, param->init_expr);
  object_end();
  dump_Cst(param->direction);
  dump_Cst(param->type);
  dump_Cst(param->name);
  dump_Cst(param->init_expr);
}

internal void
dump_Cst(struct Cst* cst)
{
  while (cst) {
    if (cst->kind == Cst_Control) {
      dump_Control((struct Cst_Control*)cst);
    } else if (cst->kind == Cst_Package) {
      dump_Package((struct Cst_Package*)cst);
    } else if (cst->kind == Cst_Error) {
      dump_Error((struct Cst_Error*)cst);
    } else if (cst->kind == Cst_Instantiation) {
      dump_Instantiation((struct Cst_Instantiation*)cst);
    } else if (cst->kind == Cst_NonTypeName) {
      dump_NonTypeName((struct Cst_NonTypeName*)cst);
    } else if (cst->kind == Cst_TypeName) {
      dump_TypeName((struct Cst_TypeName*)cst);
    } else if (cst->kind == Cst_Parameter) {
      dump_Parameter((struct Cst_Parameter*)cst);
    } else if (cst->kind == Cst_ControlType) {
      dump_ControlType((struct Cst_ControlType*)cst);
    } else {
      printf("TODO: %s\n", cst_kind_to_string(cst->kind));
    }
    cst = cst->link.next_node;
  }
}

