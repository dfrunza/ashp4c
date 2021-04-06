#include "arena.h"
#include "cst.h"

#define DEBUG_ENABLED 1

external struct Arena arena;
internal int tab_level = 0;
internal int tab_size = 2;

enum ValueType {
  Value_None,
  Value_Int,
  Value_String,
  Value_Id,
  Value_Ref,
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
    case Cst_BaseType:
      return "Cst_BaseType";
    case Cst_ConstDecl:
      return "Cst_ConstDecl";
    case Cst_ExternDecl:
      return "Cst_ExternDecl";
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
    case Cst_Bool:
      return "Cst_Bool";
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
    case Cst_SelectExpr:
      return "Cst_SelectExpr";
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
    case Cst_KvPair:
      return "Cst_KvPair";
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
  } else if (type == Value_Id) {
    struct Cst* cst = va_arg(value, struct Cst*);
    printf("\"$%d\"", cst->id);
  } else if (type == Value_Ref) {
    struct Cst* cst = va_arg(value, struct Cst*);
    if (cst) {
      if (cst->next_node) {
        list_open();
        while (cst) {
          printf("\"$%d\"", cst->id);
          cst = cst->next_node;
          if (cst) {
            printf(", ");
          }
        }
        list_close();
      } else {
        printf("\"$%d\"", cst->id);
      }
    } else {
      printf("null");
    }
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
  print_prop("id", Value_Id, cst);
  print_prop("kind", Value_String, cst_kind_to_string(cst->kind));
  print_prop("line_nr", Value_Int, cst->line_nr);
}

internal void
dump_NonTypeName(struct Cst* name)
{
  assert(name->kind == Cst_NonTypeName);
  object_start();
  print_prop_common(name);
  print_prop("name", Value_String, cst_getattr(name, "name"));
  bool* is_dotprefixed = cst_getattr(name, "is_dotprefixed");
  if (is_dotprefixed) {
    print_prop("is_dotprefixed", Value_String, *is_dotprefixed ? "true" : "false" );
  }
  object_end();
}

internal void
dump_TypeName(struct Cst* name)
{
  assert(name->kind == Cst_TypeName);
  object_start();
  print_prop_common(name);
  print_prop("name", Value_String, cst_getattr(name, "name"));
  bool* is_dotprefixed = cst_getattr(name, "is_dotprefixed");
  if (is_dotprefixed) {
    print_prop("is_dotprefixed", Value_String, *is_dotprefixed ? "true" : "false" );
  }
  object_end();
}

internal void
dump_Error(struct Cst* error)
{
  assert(error->kind == Cst_Error);
  object_start();
  print_prop_common(error);
  print_prop("id_list", Value_Ref, cst_getattr(error, "id_list"));
  object_end();
  dump_Cst(cst_getattr(error, "id_list"));
}

internal void
dump_MatchKind(struct Cst* match_kind)
{
  assert(match_kind->kind == Cst_MatchKind);
  object_start();
  print_prop_common(match_kind);
  print_prop("id_list", Value_Ref, cst_getattr(match_kind, "id_list"));
  object_end();
  dump_Cst(cst_getattr(match_kind, "id_list"));
}

internal void
dump_Control(struct Cst* control)
{
  assert(control->kind == Cst_Control);
  object_start();
  print_prop_common(control);
  print_prop("type_decl", Value_Ref, cst_getattr(control, "type_decl"));
  print_prop("ctor_params", Value_Ref, cst_getattr(control, "ctor_params"));
  print_prop("local_decls", Value_Ref, cst_getattr(control, "local_decls"));
  print_prop("apply_stmt", Value_Ref, cst_getattr(control, "apply_stmt"));
  object_end();
  dump_Cst(cst_getattr(control, "type_decl"));
  dump_Cst(cst_getattr(control, "ctor_params"));
  dump_Cst(cst_getattr(control, "local_decls"));
  dump_Cst(cst_getattr(control, "apply_stmt"));
}

internal void
dump_ControlType(struct Cst* control)
{
  assert(control->kind == Cst_ControlType);
  object_start();
  print_prop_common(control);
  print_prop("name", Value_Ref, cst_getattr(control, "name"));
  print_prop("type_params", Value_Ref, cst_getattr(control, "type_params"));
  print_prop("params", Value_Ref, cst_getattr(control, "params"));
  object_end();
  dump_Cst(cst_getattr(control, "name"));
  dump_Cst(cst_getattr(control, "type_params"));
  dump_Cst(cst_getattr(control, "params"));
}

void
dump_Package(struct Cst* package)
{
  assert(package->kind == Cst_Package);
  object_start();
  print_prop_common(package);
  print_prop("name", Value_Ref, cst_getattr(package, "name"));
  print_prop("type_params", Value_Ref, cst_getattr(package, "type_params"));
  object_end();
  dump_Cst(cst_getattr(package, "name"));
  dump_Cst(cst_getattr(package, "type_params"));
  dump_Cst(cst_getattr(package, "params"));
}

void
dump_P4Program(struct Cst* prog)
{
  assert(prog->kind == Cst_P4Program);
  object_start();
  print_prop_common(prog);
  print_prop("decl_list", Value_Ref, cst_getattr(prog, "decl_list"));
  object_end();
  dump_Cst(cst_getattr(prog, "decl_list"));
}

void
dump_Instantiation(struct Cst* inst)
{
  assert(inst->kind == Cst_Instantiation);
  object_start();
  print_prop_common(inst);
  print_prop("type_ref", Value_Ref, cst_getattr(inst, "type_ref"));
  print_prop("args", Value_Ref, cst_getattr(inst, "args"));
  print_prop("name", Value_Ref, cst_getattr(inst, "name"));
  object_end();
  dump_Cst(cst_getattr(inst, "type_ref"));
  dump_Cst(cst_getattr(inst, "args"));
  dump_Cst(cst_getattr(inst, "name"));
}

void
dump_Parameter(struct Cst* param)
{
  assert(param->kind == Cst_Parameter);
  object_start();
  print_prop_common(param);
  char* dir_str = "CstDir_None";
  if (*(enum AstParamDirection*)cst_getattr(param, "direction") == AstParamDir_In) {
    dir_str = "AstParamDir_In";
  } else if (*(enum AstParamDirection*)cst_getattr(param, "direction") == AstParamDir_Out) {
    dir_str = "AstParamDir_Out";
  } else if (*(enum AstParamDirection*)cst_getattr(param, "direction") == AstParamDir_InOut) {
    dir_str = "AstParamDir_InOut";
  } else assert(*(enum AstParamDirection*)cst_getattr(param, "direction") == AstParamDir_None);
  print_prop("direction", Value_String, dir_str);
  print_prop("type", Value_Ref, cst_getattr(param, "type"));
  print_prop("name", Value_Ref, cst_getattr(param, "name"));
  print_prop("init_expr", Value_Ref, cst_getattr(param, "init_expr"));
  object_end();
  dump_Cst(cst_getattr(param, "type"));
  dump_Cst(cst_getattr(param, "name"));
  dump_Cst(cst_getattr(param, "init_expr"));
}

internal void
dump_ActionDecl(struct Cst* action)
{
  assert(action->kind == Cst_ActionDecl);
  object_start();
  print_prop_common(action);
  print_prop("name", Value_Ref, cst_getattr(action, "name"));
  print_prop("params", Value_Ref, cst_getattr(action, "params"));
  print_prop("stmt", Value_Ref, cst_getattr(action, "stmt"));
  object_end();
  dump_Cst(cst_getattr(action, "name"));
  dump_Cst(cst_getattr(action, "params"));
  dump_Cst(cst_getattr(action, "stmt"));
}

internal void
dump_BlockStmt(struct Cst* stmt)
{
  assert(stmt->kind == Cst_BlockStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("stmt_list", Value_Ref, cst_getattr(stmt, "stmt_list"));
  object_end();
  dump_Cst(cst_getattr(stmt, "stmt_list"));
}

internal void
dump_MethodCallStmt(struct Cst* stmt)
{
  assert(stmt->kind == Cst_MethodCallStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("lvalue", Value_Ref, cst_getattr(stmt, "lvalue"));
  print_prop("type_args", Value_Ref, cst_getattr(stmt, "type_args"));
  print_prop("args", Value_Ref, cst_getattr(stmt, "args"));
  object_end();
  dump_Cst(cst_getattr(stmt, "lvalue"));
  dump_Cst(cst_getattr(stmt, "type_args"));
  dump_Cst(cst_getattr(stmt, "args"));
}

internal void
dump_Lvalue(struct Cst* lvalue)
{
  assert(lvalue->kind == Cst_Lvalue);
  object_start();
  print_prop_common(lvalue);
  print_prop("name", Value_Ref, cst_getattr(lvalue, "name"));
  print_prop("expr", Value_Ref, cst_getattr(lvalue, "expr"));
  object_end();
  dump_Cst(cst_getattr(lvalue, "name"));
  dump_Cst(cst_getattr(lvalue, "expr"));
}

internal void
dump_Int(struct Cst* node)
{
  assert(node->kind == Cst_Int);
  object_start();
  print_prop_common(node);
  char flags_str[256];
  char* str = flags_str + sprintf(flags_str, "AstInteger_None");
  if ((*(int*)cst_getattr(node, "flags") & AstInteger_HasWidth) != 0) {
    str += sprintf(str, "|%s", "AstInteger_HasWidth");
  }
  if ((*(int*)cst_getattr(node, "flags") & AstInteger_IsSigned) != 0) {
    str += sprintf(str, "|%s", "AstInteger_IsSigned");
  }
  print_prop("flags", Value_String, flags_str);
  print_prop("width", Value_Int, *(int*)cst_getattr(node, "width"));
  print_prop("value", Value_Int, *(int*)cst_getattr(node, "value"));
  object_end();
}

internal void
dump_BaseType(struct Cst* type)
{
  assert(type->kind == Cst_BaseType);
  object_start();
  print_prop_common(type);
  char* type_str = "CstBaseType_None";
  if (*(enum CstBaseTypeKind*)cst_getattr(type, "base_type") == CstBaseType_Bool) {
    type_str = "CstBaseType_Bool";
  } else if (*(enum CstBaseTypeKind*)cst_getattr(type, "base_type") == CstBaseType_Error) {
    type_str = "CstBaseType_Error";
  } else if (*(enum CstBaseTypeKind*)cst_getattr(type, "base_type") == CstBaseType_Int) {
    type_str = "CstBaseType_Int";
  } else if (*(enum CstBaseTypeKind*)cst_getattr(type, "base_type") == CstBaseType_Bit) {
    type_str = "CstBaseType_Bit";
  } else if (*(enum CstBaseTypeKind*)cst_getattr(type, "base_type") == CstBaseType_Varbit) {
    type_str = "CstBaseType_Varbit";
  } else if (*(enum CstBaseTypeKind*)cst_getattr(type, "base_type") == CstBaseType_String) {
    type_str = "CstBaseType_String";
  }
  else assert(*(enum CstBaseTypeKind*)cst_getattr(type, "base_type") == CstBaseType_None);
  print_prop("base_type", Value_String, type_str);
  print_prop("size", Value_Ref, cst_getattr(type, "size"));
  object_end();
  dump_Cst(cst_getattr(type, "size"));
}

internal void
dump_ExternDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_ExternDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, cst_getattr(decl, "name"));
  print_prop("type_params", Value_Ref, cst_getattr(decl, "type_params"));
  print_prop("method_protos", Value_Ref, cst_getattr(decl, "method_protos"));
  object_end();
  dump_Cst(cst_getattr(decl, "name"));
  dump_Cst(cst_getattr(decl, "type_params"));
  dump_Cst(cst_getattr(decl, "method_protos"));
}

internal void
dump_FunctionProto(struct Cst* proto)
{
  assert(proto->kind == Cst_FunctionProto);
  object_start();
  print_prop_common(proto);
  print_prop("return_type", Value_Ref, cst_getattr(proto, "return_type"));
  print_prop("name", Value_Ref, cst_getattr(proto, "name"));
  print_prop("type_params", Value_Ref, cst_getattr(proto, "type_params"));
  print_prop("params", Value_Ref, cst_getattr(proto, "params"));
  object_end();
  dump_Cst(cst_getattr(proto, "return_type"));
  dump_Cst(cst_getattr(proto, "name"));
  dump_Cst(cst_getattr(proto, "type_params"));
  dump_Cst(cst_getattr(proto, "params"));
}

internal void
dump_FunctionDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_FunctionDecl);
  object_start();
  print_prop_common(decl);
  print_prop("proto", Value_Ref, cst_getattr(decl, "proto"));
  print_prop("stmt", Value_Ref, cst_getattr(decl, "stmt"));
  object_end();
  dump_Cst(cst_getattr(decl, "proto"));
  dump_Cst(cst_getattr(decl, "stmt"));
}

internal void
dump_TableDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_TableDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, cst_getattr(decl, "name"));
  print_prop("prop_list", Value_Ref, cst_getattr(decl, "prop_list"));
  object_end();
  dump_Cst(cst_getattr(decl, "name"));
  dump_Cst(cst_getattr(decl, "prop_list"));
}

internal void
dump_TableProp_Actions(struct Cst* actions)
{
  assert(actions->kind == Cst_TableProp_Actions);
  object_start();
  print_prop_common(actions);
  print_prop("action_list", Value_Ref, cst_getattr(actions, "action_list"));
  object_end();
  dump_Cst(cst_getattr(actions, "action_list"));
}

internal void
dump_TableProp_SingleEntry(struct Cst* entry)
{
  assert(entry->kind == Cst_TableProp_SingleEntry);
  object_start();
  print_prop_common(entry);
  print_prop("name", Value_Ref, cst_getattr(entry, "name"));
  print_prop("init_expr", Value_Ref, cst_getattr(entry, "init_expr"));
  object_end();
  dump_Cst(cst_getattr(entry, "name"));
  dump_Cst(cst_getattr(entry, "init_expr"));
}

internal void
dump_ActionRef(struct Cst* ref)
{
  assert(ref->kind == Cst_ActionRef);
  object_start();
  print_prop_common(ref);
  print_prop("name", Value_Ref, cst_getattr(ref, "name"));
  print_prop("args", Value_Ref, cst_getattr(ref, "args"));
  object_end();
  dump_Cst(cst_getattr(ref, "name"));
  dump_Cst(cst_getattr(ref, "args"));
}

internal void
dump_FunctionCallExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_FunctionCallExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr", Value_Ref, cst_getattr(expr, "expr"));
  print_prop("args", Value_Ref, cst_getattr(expr, "args"));
  object_end();
  dump_Cst(cst_getattr(expr, "expr"));
  dump_Cst(cst_getattr(expr, "args"));
}

internal void
dump_HeaderDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_HeaderDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, cst_getattr(decl, "name"));
  print_prop("fields", Value_Ref, cst_getattr(decl, "fields"));
  object_end();
  dump_Cst(cst_getattr(decl, "name"));
  dump_Cst(cst_getattr(decl, "fields"));
}

internal void
dump_HeaderStack(struct Cst* stack)
{
  assert(stack->kind == Cst_HeaderStack);
  object_start();
  print_prop_common(stack);
  print_prop("name", Value_Ref, cst_getattr(stack, "name"));
  print_prop("stack_expr", Value_Ref, cst_getattr(stack, "stack_expr"));
  object_end();
  dump_Cst(cst_getattr(stack, "name"));
  dump_Cst(cst_getattr(stack, "stack_expr"));
}

internal void
dump_StructField(struct Cst* field)
{
  assert(field->kind == Cst_StructField);
  object_start();
  print_prop_common(field);
  print_prop("type", Value_Ref, cst_getattr(field, "type"));
  print_prop("name", Value_Ref, cst_getattr(field, "name"));
  object_end();
  dump_Cst(cst_getattr(field, "type"));
  dump_Cst(cst_getattr(field, "name"));
}

internal void
dump_VarDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_VarDecl);
  object_start();
  print_prop_common(decl);
  print_prop("type", Value_Ref, cst_getattr(decl, "type"));
  print_prop("name", Value_Ref, cst_getattr(decl, "name"));
  print_prop("init_expr", Value_Ref, cst_getattr(decl, "init_expr"));
  object_end();
  dump_Cst(cst_getattr(decl, "type"));
  dump_Cst(cst_getattr(decl, "name"));
  dump_Cst(cst_getattr(decl, "init_expr"));
}

internal void
dump_AssignmentStmt(struct Cst* stmt)
{
  assert(stmt->kind == Cst_AssignmentStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("lvalue", Value_Ref, cst_getattr(stmt, "lvalue"));
  print_prop("expr", Value_Ref, cst_getattr(stmt, "expr"));
  object_end();
  dump_Cst(cst_getattr(stmt, "lvalue"));
  dump_Cst(cst_getattr(stmt, "expr"));
}

internal void
dump_ArrayIndex(struct Cst* index)
{
  assert(index->kind == Cst_ArrayIndex);
  object_start();
  print_prop_common(index);
  print_prop("index", Value_Ref, cst_getattr(index, "index"));
  print_prop("colon_index", Value_Ref, cst_getattr(index, "colon_index"));
  object_end();
  dump_Cst(cst_getattr(index, "index"));
  dump_Cst(cst_getattr(index, "colon_index"));
}

internal void
dump_MemberSelectExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_MemberSelectExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr", Value_Ref, cst_getattr(expr, "expr"));
  print_prop("member_name", Value_Ref, cst_getattr(expr, "member_name"));
  object_end();
  dump_Cst(cst_getattr(expr, "expr"));
  dump_Cst(cst_getattr(expr, "member_name"));
}

internal char*
expr_operator_to_string(enum AstExprOperator op)
{
  char* op_str = "Cst_Op_None";
  if (op == AstBinOp_ArAdd) {
    op_str = "AstBinOp_ArAdd";
  } else if (op == AstBinOp_ArSub) {
    op_str = "AstBinOp_ArSub";
  } else if (op == AstBinOp_ArMul) {
    op_str = "AstBinOp_ArMul";
  } else if (op == AstBinOp_ArDiv) {
    op_str = "AstBinOp_ArDiv";
  } else if (op == AstBinOp_LogAnd) {
    op_str = "AstBinOp_LogAnd";
  } else if (op == AstBinOp_LogOr) {
    op_str = "AstBinOp_LogOr";
  } else if (op == AstBinOp_LogEqual) {
    op_str = "AstBinOp_LogEqual";
  } else if (op == AstBinOp_LogNotEqual) {
    op_str = "AstBinOp_LogNotEqual";
  } else if (op == AstBinOp_LogLess) {
    op_str = "AstBinOp_LogLess";
  } else if (op == AstBinOp_LogGreater) {
    op_str = "AstBinOp_LogGreater";
  } else if (op == AstBinOp_LogLessEqual) {
    op_str = "AstBinOp_LogLessEqual";
  } else if (op == AstBinOp_LogGreaterEqual) {
    op_str = "AstBinOp_LogGreaterEqual";
  } else if (op == AstBinOp_BitAnd) {
    op_str = "AstBinOp_BitAnd";
  } else if (op == AstBinOp_BitOr) {
    op_str = "AstBinOp_BitOr";
  } else if (op == AstBinOp_BitXor) {
    op_str = "AstBinOp_BitXor";
  } else if (op == AstBinOp_BitShLeft) {
    op_str = "AstBinOp_BitShLeft";
  } else if (op == AstBinOp_BitShRight) {
    op_str = "AstBinOp_BitShRight";
  } else if (op == AstUnOp_LogNot) {
    op_str = "AstUnOp_LogNot";
  } else if (op == AstUnOp_BitNot) {
    op_str = "AstUnOp_BitNot";
  } else if (op == AstUnOp_ArMinus) {
    op_str = "AstUnOp_ArMinus";
  } else if (op == AstBinOp_Mask) {
    op_str = "AstBinOp_Mask";
  }
  else assert(op == AstOp_None);
  return op_str;
}

internal void
dump_BinaryExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_BinaryExpr);
  object_start();
  print_prop_common(expr);
  print_prop("op", Value_String, expr_operator_to_string(*(enum AstExprOperator*)cst_getattr(expr, "op")));
  print_prop("left_operand", Value_Ref, cst_getattr(expr, "left_operand"));
  print_prop("right_operand", Value_Ref, cst_getattr(expr, "right_operand"));
  object_end();
  dump_Cst(cst_getattr(expr, "left_operand"));
  dump_Cst(cst_getattr(expr, "right_operand"));
}

internal void
dump_KvPair(struct Cst* kv_pair)
{
  assert(kv_pair->kind == Cst_KvPair);
  object_start();
  print_prop_common(kv_pair);
  print_prop("name", Value_Ref, cst_getattr(kv_pair, "name"));
  print_prop("expr", Value_Ref, cst_getattr(kv_pair, "expr"));
  object_end();
  dump_Cst(cst_getattr(kv_pair, "name"));
  dump_Cst(cst_getattr(kv_pair, "expr"));
}

internal void
dump_IndexedArrayExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_IndexedArrayExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr", Value_Ref, cst_getattr(expr, "expr"));
  print_prop("index_expr", Value_Ref, cst_getattr(expr, "index_expr"));
  object_end();
  dump_Cst(cst_getattr(expr, "expr"));
  dump_Cst(cst_getattr(expr, "index_expr"));
}

internal void
dump_HeaderUnionDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_HeaderUnionDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, cst_getattr(decl, "name"));
  print_prop("fields", Value_Ref, cst_getattr(decl, "fields"));
  object_end();
  dump_Cst(cst_getattr(decl, "name"));
  dump_Cst(cst_getattr(decl, "fields"));
}

internal void
dump_StringLiteral(struct Cst* string)
{
  assert(string->kind == Cst_StringLiteral);
  object_start();
  print_prop_common(string);
  print_prop("value", Value_String, cst_getattr(string, "value"));
  object_end();
}

internal void
dump_Tuple(struct Cst* tuple)
{
  assert(tuple->kind == Cst_Tuple);
  object_start();
  print_prop_common(tuple);
  print_prop("type_args", Value_Ref, cst_getattr(tuple, "type_args"));
  object_end();
  dump_Cst(cst_getattr(tuple, "type_args"));
}

internal void
dump_SpecdId(struct Cst* id)
{
  assert(id->kind == Cst_SpecdId);
  object_start();
  print_prop_common(id);
  print_prop("name", Value_Ref, cst_getattr(id, "name"));
  print_prop("init_expr", Value_Ref, cst_getattr(id, "init_expr"));
  object_end();
  dump_Cst(cst_getattr(id, "name"));
  dump_Cst(cst_getattr(id, "init_expr"));
}

internal void
dump_ExpressionListExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_ExpressionListExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr_list", Value_Ref, cst_getattr(expr, "expr_list"));
  object_end();
  dump_Cst(cst_getattr(expr, "expr_list"));
}

internal void
dump_IfStmt(struct Cst* stmt)
{
  assert(stmt->kind == Cst_IfStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("cond_expr", Value_Ref, cst_getattr(stmt, "cond_expr"));
  print_prop("stmt", Value_Ref, cst_getattr(stmt, "stmt"));
  print_prop("else_stmt", Value_Ref, cst_getattr(stmt, "else_stmt"));
  object_end();
  dump_Cst(cst_getattr(stmt, "cond_expr"));
  dump_Cst(cst_getattr(stmt, "stmt"));
  dump_Cst(cst_getattr(stmt, "else_stmt"));
}

internal void
dump_SwitchStmt(struct Cst* stmt)
{
  assert(stmt->kind == Cst_SwitchStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("expr", Value_Ref, cst_getattr(stmt, "expr"));
  print_prop("switch_cases", Value_Ref, cst_getattr(stmt, "switch_cases"));
  object_end();
  dump_Cst(cst_getattr(stmt, "expr"));
  dump_Cst(cst_getattr(stmt, "switch_cases"));
}

internal void
dump_SwitchCase(struct Cst* swcase)
{
  assert(swcase->kind == Cst_SwitchCase);
  object_start();
  print_prop_common(swcase);
  print_prop("label", Value_Ref, cst_getattr(swcase, "label"));
  print_prop("stmt", Value_Ref, cst_getattr(swcase, "stmt"));
  object_end();
  dump_Cst(cst_getattr(swcase, "label"));
  dump_Cst(cst_getattr(swcase, "stmt"));
}

internal void
dump_SwitchLabel(struct Cst* label)
{
  assert(label->kind == Cst_SwitchLabel);
  object_start();
  print_prop_common(label);
  print_prop("name", Value_Ref, cst_getattr(label, "name"));
  object_end();
  dump_Cst(cst_getattr(label, "name"));
}

internal void
dump_ReturnStmt(struct Cst* stmt)
{
  assert(stmt->kind == Cst_ReturnStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("expr", Value_Ref, cst_getattr(stmt, "expr"));
  object_end();
  dump_Cst(cst_getattr(stmt, "expr"));
}

internal void
dump_CastExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_CastExpr);
  object_start();
  print_prop_common(expr);
  print_prop("to_type", Value_Ref, cst_getattr(expr, "to_type"));
  print_prop("expr", Value_Ref, cst_getattr(expr, "expr"));
  object_end();
  dump_Cst(cst_getattr(expr, "to_type"));
  dump_Cst(cst_getattr(expr, "expr"));
}

internal void
dump_UnaryExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_UnaryExpr);
  object_start();
  print_prop_common(expr);
  print_prop("op", Value_String, expr_operator_to_string(*(enum AstExprOperator*)cst_getattr(expr, "op")));
  print_prop("expr", Value_Ref, cst_getattr(expr, "expr"));
  object_end();
  dump_Cst(cst_getattr(expr, "expr"));
}

internal void
dump_Parser(struct Cst* parser)
{
  assert(parser->kind == Cst_Parser);
  object_start();
  print_prop_common(parser);
  print_prop("type_decl", Value_Ref, cst_getattr(parser, "type_decl"));
  print_prop("ctor_params", Value_Ref, cst_getattr(parser, "ctor_params"));
  print_prop("local_elements", Value_Ref, cst_getattr(parser, "local_elements"));
  print_prop("states", Value_Ref, cst_getattr(parser, "states"));
  object_end();
  dump_Cst(cst_getattr(parser, "type_decl"));
  dump_Cst(cst_getattr(parser, "ctor_params"));
  dump_Cst(cst_getattr(parser, "local_elements"));
  dump_Cst(cst_getattr(parser, "states"));
}

internal void
dump_ParserType(struct Cst* parser)
{
  assert(parser->kind == Cst_ParserType);
  object_start();
  print_prop_common(parser);
  print_prop("name", Value_Ref, cst_getattr(parser, "name"));
  print_prop("type_params", Value_Ref, cst_getattr(parser, "type_params"));
  print_prop("params", Value_Ref, cst_getattr(parser, "params"));
  object_end();
  dump_Cst(cst_getattr(parser, "name"));
  dump_Cst(cst_getattr(parser, "type_params"));
  dump_Cst(cst_getattr(parser, "params"));
}

internal void
dump_SpecdType(struct Cst* type)
{
  assert(type->kind == Cst_SpecdType);
  object_start();
  print_prop_common(type);
  print_prop("name", Value_Ref, cst_getattr(type, "name"));
  print_prop("type_args", Value_Ref, cst_getattr(type, "type_args"));
  object_end();
  dump_Cst(cst_getattr(type, "name"));
  dump_Cst(cst_getattr(type, "type_args"));
}

internal void
dump_TypeDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_TypeDecl);
  object_start();
  print_prop_common(decl);
  print_prop("is_typedef", Value_String, *(bool*)cst_getattr(decl, "is_typedef") ? "true" : "false");
  print_prop("type_ref", Value_Ref, cst_getattr(decl, "type_ref"));
  print_prop("name", Value_Ref, cst_getattr(decl, "name"));
  object_end();
  dump_Cst(cst_getattr(decl, "type_ref"));
  dump_Cst(cst_getattr(decl, "name"));
}

internal void
dump_StructDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_StructDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, cst_getattr(decl, "name"));
  print_prop("fields", Value_Ref, cst_getattr(decl, "fields"));
  object_end();
  dump_Cst(cst_getattr(decl, "name"));
  dump_Cst(cst_getattr(decl, "fields"));
}

internal void
dump_ParserState(struct Cst* state)
{
  assert(state->kind == Cst_ParserState);
  object_start();
  print_prop_common(state);
  print_prop("name", Value_Ref, cst_getattr(state, "name"));
  print_prop("stmts", Value_Ref, cst_getattr(state, "stmts"));
  print_prop("trans_stmt", Value_Ref, cst_getattr(state, "trans_stmt"));
  object_end();
  dump_Cst(cst_getattr(state, "name"));
  dump_Cst(cst_getattr(state, "stmts"));
  dump_Cst(cst_getattr(state, "trans_stmt"));
}

internal void
dump_SelectExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_SelectExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr_list", Value_Ref, cst_getattr(expr, "expr_list"));
  print_prop("case_list", Value_Ref, cst_getattr(expr, "case_list"));
  object_end();
  dump_Cst(cst_getattr(expr, "expr_list"));
  dump_Cst(cst_getattr(expr, "case_list"));
}

internal void
dump_SelectCase(struct Cst* select)
{
  assert(select->kind == Cst_SelectCase);
  object_start();
  print_prop_common(select);
  print_prop("keyset", Value_Ref, cst_getattr(select, "keyset"));
  print_prop("name", Value_Ref, cst_getattr(select, "name"));
  object_end();
  dump_Cst(cst_getattr(select, "keyset"));
  dump_Cst(cst_getattr(select, "name"));
}

internal void
dump_Default(struct Cst* deflt)
{
  assert(deflt->kind == Cst_Default);
  object_start();
  print_prop_common(deflt);
  object_end();
}

internal void
dump_IntTypeSize(struct Cst* size)
{
  assert(size->kind == Cst_IntTypeSize);
  object_start();
  print_prop_common(size);
  print_prop("size", Value_Ref, cst_getattr(size, "size"));
  object_end();
  dump_Cst(cst_getattr(size, "size"));
}

internal void
dump_TypeArgsExpr(struct Cst* expr)
{
  assert(expr->kind == Cst_TypeArgsExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr", Value_Ref, cst_getattr(expr, "expr"));
  print_prop("type_args", Value_Ref, cst_getattr(expr, "type_args"));
  object_end();
  dump_Cst(cst_getattr(expr, "expr"));
  dump_Cst(cst_getattr(expr, "type_args"));
}

internal void
dump_ConstDecl(struct Cst* decl)
{
  assert(decl->kind == Cst_ConstDecl);
  object_start();
  print_prop_common(decl);
  print_prop("type_ref", Value_Ref, cst_getattr(decl, "type_ref"));
  print_prop("name", Value_Ref, cst_getattr(decl, "name"));
  print_prop("expr", Value_Ref, cst_getattr(decl, "expr"));
  object_end();
  dump_Cst(cst_getattr(decl, "type_ref"));
  dump_Cst(cst_getattr(decl, "name"));
  dump_Cst(cst_getattr(decl, "expr"));
}

internal void
dump_Bool(struct Cst* node)
{
  assert(node->kind == Cst_Bool);
  object_start();
  print_prop_common(node);
  print_prop("value", Value_Int, cst_getattr(node, "value"));
  object_end();
}

internal void
dump_TableProp_Entries(struct Cst* entries)
{
  assert(entries->kind == Cst_TableProp_Entries);
  object_start();
  print_prop_common(entries);
  print_prop("is_const", Value_Int, *(bool*)cst_getattr(entries, "is_const"));
  print_prop("entries", Value_Ref, cst_getattr(entries, "entries"));
  object_end();
  dump_Cst(cst_getattr(entries, "entries"));
}

internal void
dump_TableEntry(struct Cst* entry)
{
  assert(entry->kind == Cst_TableEntry);
  object_start();
  print_prop_common(entry);
  print_prop("keyset", Value_Ref, cst_getattr(entry, "keyset"));
  print_prop("action", Value_Ref, cst_getattr(entry, "action"));
  object_end();
  dump_Cst(cst_getattr(entry, "keyset"));
  dump_Cst(cst_getattr(entry, "action"));
}

internal void
dump_TableProp_Key(struct Cst* key)
{
  assert(key->kind == Cst_TableProp_Key);
  object_start();
  print_prop_common(key);
  print_prop("keyelem_list", Value_Ref, cst_getattr(key, "keyelem_list"));
  object_end();
  dump_Cst(cst_getattr(key, "keyelem_list"));
}

internal void
dump_KeyElement(struct Cst* elem)
{
  assert(elem->kind == Cst_KeyElement);
  object_start();
  print_prop_common(elem);
  print_prop("expr", Value_Ref, cst_getattr(elem, "expr"));
  print_prop("name", Value_Ref, cst_getattr(elem, "name"));
  object_end();
  dump_Cst(cst_getattr(elem, "expr"));
  dump_Cst(cst_getattr(elem, "name"));
}

internal void
dump_Cst(struct Cst* cst)
{
  while (cst) {
    if (cst->kind == Cst_Control) {
      dump_Control(cst);
    } else if (cst->kind == Cst_Package) {
      dump_Package(cst);
    } else if (cst->kind == Cst_Error) {
      dump_Error(cst);
    } else if (cst->kind == Cst_MatchKind) {
      dump_MatchKind(cst);
    } else if (cst->kind == Cst_Instantiation) {
      dump_Instantiation(cst);
    } else if (cst->kind == Cst_NonTypeName) {
      dump_NonTypeName(cst);
    } else if (cst->kind == Cst_TypeName) {
      dump_TypeName(cst);
    } else if (cst->kind == Cst_Parameter) {
      dump_Parameter(cst);
    } else if (cst->kind == Cst_ControlType) {
      dump_ControlType(cst);
    } else if (cst->kind == Cst_ActionDecl) {
      dump_ActionDecl(cst);
    } else if (cst->kind == Cst_BlockStmt) {
      dump_BlockStmt(cst);
    } else if (cst->kind == Cst_MethodCallStmt) {
      dump_MethodCallStmt(cst);
    } else if (cst->kind == Cst_Lvalue) {
      dump_Lvalue(cst);
    } else if (cst->kind == Cst_Int) {
      dump_Int(cst);
    } else if (cst->kind == Cst_BaseType) {
      dump_BaseType(cst);
    } else if (cst->kind == Cst_ExternDecl) {
      dump_ExternDecl(cst);
    } else if (cst->kind == Cst_FunctionProto) {
      dump_FunctionProto(cst);
    } else if (cst->kind == Cst_FunctionDecl) {
      dump_FunctionDecl(cst);
    } else if (cst->kind == Cst_TableDecl) {
      dump_TableDecl(cst);
    } else if (cst->kind == Cst_TableProp_Actions) {
      dump_TableProp_Actions(cst);
    } else if (cst->kind == Cst_TableProp_SingleEntry) {
      dump_TableProp_SingleEntry(cst);
    } else if (cst->kind == Cst_ActionRef) {
      dump_ActionRef(cst);
    } else if (cst->kind == Cst_FunctionCallExpr) {
      dump_FunctionCallExpr(cst);
    } else if (cst->kind == Cst_HeaderDecl) {
      dump_HeaderDecl(cst);
    } else if (cst->kind == Cst_HeaderStack) {
      dump_HeaderStack(cst);
    } else if (cst->kind == Cst_StructField) {
      dump_StructField(cst);
    } else if (cst->kind == Cst_VarDecl) {
      dump_VarDecl(cst);
    } else if (cst->kind == Cst_AssignmentStmt) {
      dump_AssignmentStmt(cst);
    } else if (cst->kind == Cst_ArrayIndex) {
      dump_ArrayIndex(cst);
    } else if (cst->kind == Cst_MemberSelectExpr) {
      dump_MemberSelectExpr(cst);
    } else if (cst->kind == Cst_BinaryExpr) {
      dump_BinaryExpr(cst);
    } else if (cst->kind == Cst_ExpressionListExpr) {
      dump_ExpressionListExpr(cst);
    } else if (cst->kind == Cst_IfStmt) {
      dump_IfStmt(cst);
    } else if (cst->kind == Cst_SwitchStmt) {
      dump_SwitchStmt(cst);
    } else if (cst->kind == Cst_SwitchCase) {
      dump_SwitchCase(cst);
    } else if (cst->kind == Cst_SwitchLabel) {
      dump_SwitchLabel(cst);
    } else if (cst->kind == Cst_ReturnStmt) {
      dump_ReturnStmt(cst);
    } else if (cst->kind == Cst_CastExpr) {
      dump_CastExpr(cst);
    } else if (cst->kind == Cst_UnaryExpr) {
      dump_UnaryExpr(cst);
    } else if (cst->kind == Cst_Parser) {
      dump_Parser(cst);
    } else if (cst->kind == Cst_ParserType) {
      dump_ParserType(cst);
    } else if (cst->kind == Cst_SpecdType) {
      dump_SpecdType(cst);
    } else if (cst->kind == Cst_TypeDecl) {
      dump_TypeDecl(cst);
    } else if (cst->kind == Cst_StructDecl) {
      dump_StructDecl(cst);
    } else if (cst->kind == Cst_ParserState) {
      dump_ParserState(cst);
    } else if (cst->kind == Cst_SelectExpr) {
      dump_SelectExpr(cst);
    } else if (cst->kind == Cst_SelectCase) {
      dump_SelectCase(cst);
    } else if (cst->kind == Cst_Default) {
      dump_Default(cst);
    } else if (cst->kind == Cst_IntTypeSize) {
      dump_IntTypeSize(cst);
    } else if (cst->kind == Cst_TypeArgsExpr) {
      dump_TypeArgsExpr(cst);
    } else if (cst->kind == Cst_ConstDecl) {
      dump_ConstDecl(cst);
    } else if (cst->kind == Cst_Bool) {
      dump_Bool(cst);
    } else if (cst->kind == Cst_TableProp_Entries) {
      dump_TableProp_Entries(cst);
    } else if (cst->kind == Cst_TableEntry) {
      dump_TableEntry(cst);
    } else if (cst->kind == Cst_TableProp_Key) {
      dump_TableProp_Key(cst);
    } else if (cst->kind == Cst_KeyElement) {
      dump_KeyElement(cst);
    } else if (cst->kind == Cst_KvPair) {
      dump_KvPair(cst);
    } else if (cst->kind == Cst_IndexedArrayExpr) {
      dump_IndexedArrayExpr(cst);
    } else if (cst->kind == Cst_SpecdId) {
      dump_SpecdId(cst);
    } else if (cst->kind == Cst_Tuple) {
      dump_Tuple(cst);
    } else if (cst->kind == Cst_StringLiteral) {
      dump_StringLiteral(cst);
    } else if (cst->kind == Cst_HeaderUnionDecl) {
      dump_HeaderUnionDecl(cst);
    }
    else {
      printf("TODO: %s\n", cst_kind_to_string(cst->kind));
    }
    cst = cst->next_node;
  }
}

