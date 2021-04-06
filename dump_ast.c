#include "arena.h"
#include "ast.h"

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

internal void dump_Ast(struct Ast*);
internal void print_prop(char* name, enum ValueType type, ...);
internal void print_list_elem(enum ValueType type, ...);

internal char*
ast_kind_to_string(enum AstKind kind)
{
  switch (kind) {
    case Ast_NonTypeName:
      return "Ast_NonTypeName";
    case Ast_TypeName:
      return "Ast_TypeName";
    case Ast_BaseType:
      return "Ast_BaseType";
    case Ast_ConstDecl:
      return "Ast_ConstDecl";
    case Ast_ExternDecl:
      return "Ast_ExternDecl";
    case Ast_FunctionProto:
      return "Ast_FunctionProto";
    case Ast_ActionDecl:
      return "Ast_ActionDecl";
    case Ast_HeaderDecl:
      return "Ast_HeaderDecl";
    case Ast_HeaderUnionDecl:
      return "Ast_HeaderUnionDecl";
    case Ast_StructDecl:
      return "Ast_StructDecl";
    case Ast_EnumDecl:
      return "Ast_EnumDecl";
    case Ast_Parser:
      return "Ast_Parser";
    case Ast_Control:
      return "Ast_Control";
    case Ast_Package:
      return "Ast_Package";
    case Ast_Instantiation:
      return "Ast_Instantiation";
    case Ast_Error:
      return "Ast_Error";
    case Ast_MatchKind:
      return "Ast_MatchKind";
    case Ast_FunctionDecl:
      return "Ast_FunctionDecl";
    case Ast_Dontcare:
      return "Ast_Dontcare";
    case Ast_IntTypeSize:
      return "Ast_IntTypeSize";
    case Ast_Int:
      return "Ast_Int";
    case Ast_Bool:
      return "Ast_Bool";
    case Ast_StringLiteral:
      return "Ast_StringLiteral";
    case Ast_Tuple:
      return "Ast_Tuple";
    case Ast_HeaderStack:
      return "Ast_HeaderStack";
    case Ast_SpecdType:
      return "Ast_SpecdType";
    case Ast_StructField:
      return "Ast_StructField";
    case Ast_SpecdId:
      return "Ast_SpecdId";
    case Ast_ParserType:
      return "Ast_ParserType";
    case Ast_Argument:
      return "Ast_Argument";
    case Ast_VarDecl:
      return "Ast_VarDecl";
    case Ast_DirectApplic:
      return "Ast_DirectApplic";
    case Ast_ArrayIndex:
      return "Ast_ArrayIndex";
    case Ast_Parameter:
      return "Ast_Parameter";
    case Ast_Lvalue:
      return "Ast_Lvalue";
    case Ast_AssignmentStmt:
      return "Ast_AssignmentStmt";
    case Ast_MethodCallStmt:
      return "Ast_MethodCallStmt";
    case Ast_EmptyStmt:
      return "Ast_EmptyStmt";
    case Ast_Default:
      return "Ast_Default";
    case Ast_SelectExpr:
      return "Ast_SelectExpr";
    case Ast_SelectCase:
      return "Ast_SelectCase";
    case Ast_ParserState:
      return "Ast_ParserState";
    case Ast_ControlType:
      return "Ast_ControlType";
    case Ast_KeyElement:
      return "Ast_KeyElement";
    case Ast_ActionRef:
      return "Ast_ActionRef";
    case Ast_TableEntry:
      return "Ast_TableEntry";
    case Ast_TableProp_Key:
      return "Ast_TableProp_Key";
    case Ast_TableProp_Actions:
      return "Ast_TableProp_Actions";
    case Ast_TableProp_Entries:
      return "Ast_TableProp_Entries";
    case Ast_TableProp_SingleEntry:
      return "Ast_TableProp_SingleEntry";
    case Ast_TableDecl:
      return "Ast_TableDecl";
    case Ast_IfStmt:
      return "Ast_IfStmt";
    case Ast_ExitStmt:
      return "Ast_ExitStmt";
    case Ast_ReturnStmt:
      return "Ast_ReturnStmt";
    case Ast_SwitchLabel:
      return "Ast_SwitchLabel";
    case Ast_SwitchCase:
      return "Ast_SwitchCase";
    case Ast_SwitchStmt:
      return "Ast_SwitchStmt";
    case Ast_BlockStmt:
      return "Ast_BlockStmt";
    case Ast_ExpressionListExpr:
      return "Ast_ExpressionListExpr";
    case Ast_CastExpr:
      return "Ast_CastExpr";
    case Ast_UnaryExpr:
      return "Ast_UnaryExpr";
    case Ast_BinaryExpr:
      return "Ast_BinaryExpr";
    case Ast_KvPair:
      return "Ast_KvPair";
    case Ast_MemberSelectExpr:
      return "Ast_MemberSelectExpr";
    case Ast_IndexedArrayExpr:
      return "Ast_IndexedArrayExpr";
    case Ast_FunctionCallExpr:
      return "Ast_FunctionCallExpr";
    case Ast_TypeArgsExpr:
      return "Ast_TypeArgsExpr";
    case Ast_P4Program:
      return "Ast_P4Program";
    case Ast_TypeDecl:
      return "Ast_TypeDecl";
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
    struct Ast* ast = va_arg(value, struct Ast*);
    printf("\"$%d\"", ast->id);
  } else if (type == Value_Ref) {
    struct Ast* ast = va_arg(value, struct Ast*);
    if (ast) {
      if (ast->next_node) {
        list_open();
        while (ast) {
          printf("\"$%d\"", ast->id);
          ast = ast->next_node;
          if (ast) {
            printf(", ");
          }
        }
        list_close();
      } else {
        printf("\"$%d\"", ast->id);
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
print_prop_common(struct Ast* ast)
{
  print_prop("id", Value_Id, ast);
  print_prop("kind", Value_String, ast_kind_to_string(ast->kind));
  print_prop("line_nr", Value_Int, ast->line_nr);
}

internal void
dump_NonTypeName(struct Ast* name)
{
  assert(name->kind == Ast_NonTypeName);
  object_start();
  print_prop_common(name);
  print_prop("name", Value_String, ast_getattr(name, "name"));
  bool* is_dotprefixed = ast_getattr(name, "is_dotprefixed");
  if (is_dotprefixed) {
    print_prop("is_dotprefixed", Value_String, *is_dotprefixed ? "true" : "false" );
  }
  object_end();
}

internal void
dump_TypeName(struct Ast* name)
{
  assert(name->kind == Ast_TypeName);
  object_start();
  print_prop_common(name);
  print_prop("name", Value_String, ast_getattr(name, "name"));
  bool* is_dotprefixed = ast_getattr(name, "is_dotprefixed");
  if (is_dotprefixed) {
    print_prop("is_dotprefixed", Value_String, *is_dotprefixed ? "true" : "false" );
  }
  object_end();
}

internal void
dump_Error(struct Ast* error)
{
  assert(error->kind == Ast_Error);
  object_start();
  print_prop_common(error);
  print_prop("id_list", Value_Ref, ast_getattr(error, "id_list"));
  object_end();
  dump_Ast(ast_getattr(error, "id_list"));
}

internal void
dump_MatchKind(struct Ast* match_kind)
{
  assert(match_kind->kind == Ast_MatchKind);
  object_start();
  print_prop_common(match_kind);
  print_prop("id_list", Value_Ref, ast_getattr(match_kind, "id_list"));
  object_end();
  dump_Ast(ast_getattr(match_kind, "id_list"));
}

internal void
dump_Control(struct Ast* control)
{
  assert(control->kind == Ast_Control);
  object_start();
  print_prop_common(control);
  print_prop("type_decl", Value_Ref, ast_getattr(control, "type_decl"));
  print_prop("ctor_params", Value_Ref, ast_getattr(control, "ctor_params"));
  print_prop("local_decls", Value_Ref, ast_getattr(control, "local_decls"));
  print_prop("apply_stmt", Value_Ref, ast_getattr(control, "apply_stmt"));
  object_end();
  dump_Ast(ast_getattr(control, "type_decl"));
  dump_Ast(ast_getattr(control, "ctor_params"));
  dump_Ast(ast_getattr(control, "local_decls"));
  dump_Ast(ast_getattr(control, "apply_stmt"));
}

internal void
dump_ControlType(struct Ast* control)
{
  assert(control->kind == Ast_ControlType);
  object_start();
  print_prop_common(control);
  print_prop("name", Value_Ref, ast_getattr(control, "name"));
  print_prop("type_params", Value_Ref, ast_getattr(control, "type_params"));
  print_prop("params", Value_Ref, ast_getattr(control, "params"));
  object_end();
  dump_Ast(ast_getattr(control, "name"));
  dump_Ast(ast_getattr(control, "type_params"));
  dump_Ast(ast_getattr(control, "params"));
}

void
dump_Package(struct Ast* package)
{
  assert(package->kind == Ast_Package);
  object_start();
  print_prop_common(package);
  print_prop("name", Value_Ref, ast_getattr(package, "name"));
  print_prop("type_params", Value_Ref, ast_getattr(package, "type_params"));
  object_end();
  dump_Ast(ast_getattr(package, "name"));
  dump_Ast(ast_getattr(package, "type_params"));
  dump_Ast(ast_getattr(package, "params"));
}

void
dump_P4Program(struct Ast* prog)
{
  assert(prog->kind == Ast_P4Program);
  object_start();
  print_prop_common(prog);
  print_prop("decl_list", Value_Ref, ast_getattr(prog, "decl_list"));
  object_end();
  dump_Ast(ast_getattr(prog, "decl_list"));
}

void
dump_Instantiation(struct Ast* inst)
{
  assert(inst->kind == Ast_Instantiation);
  object_start();
  print_prop_common(inst);
  print_prop("type_ref", Value_Ref, ast_getattr(inst, "type_ref"));
  print_prop("args", Value_Ref, ast_getattr(inst, "args"));
  print_prop("name", Value_Ref, ast_getattr(inst, "name"));
  object_end();
  dump_Ast(ast_getattr(inst, "type_ref"));
  dump_Ast(ast_getattr(inst, "args"));
  dump_Ast(ast_getattr(inst, "name"));
}

void
dump_Parameter(struct Ast* param)
{
  assert(param->kind == Ast_Parameter);
  object_start();
  print_prop_common(param);
  char* dir_str = "AstDir_None";
  if (*(enum AstParamDirection*)ast_getattr(param, "direction") == AstParamDir_In) {
    dir_str = "AstParamDir_In";
  } else if (*(enum AstParamDirection*)ast_getattr(param, "direction") == AstParamDir_Out) {
    dir_str = "AstParamDir_Out";
  } else if (*(enum AstParamDirection*)ast_getattr(param, "direction") == AstParamDir_InOut) {
    dir_str = "AstParamDir_InOut";
  } else assert(*(enum AstParamDirection*)ast_getattr(param, "direction") == AstParamDir_None);
  print_prop("direction", Value_String, dir_str);
  print_prop("type", Value_Ref, ast_getattr(param, "type"));
  print_prop("name", Value_Ref, ast_getattr(param, "name"));
  print_prop("init_expr", Value_Ref, ast_getattr(param, "init_expr"));
  object_end();
  dump_Ast(ast_getattr(param, "type"));
  dump_Ast(ast_getattr(param, "name"));
  dump_Ast(ast_getattr(param, "init_expr"));
}

internal void
dump_ActionDecl(struct Ast* action)
{
  assert(action->kind == Ast_ActionDecl);
  object_start();
  print_prop_common(action);
  print_prop("name", Value_Ref, ast_getattr(action, "name"));
  print_prop("params", Value_Ref, ast_getattr(action, "params"));
  print_prop("stmt", Value_Ref, ast_getattr(action, "stmt"));
  object_end();
  dump_Ast(ast_getattr(action, "name"));
  dump_Ast(ast_getattr(action, "params"));
  dump_Ast(ast_getattr(action, "stmt"));
}

internal void
dump_BlockStmt(struct Ast* stmt)
{
  assert(stmt->kind == Ast_BlockStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("stmt_list", Value_Ref, ast_getattr(stmt, "stmt_list"));
  object_end();
  dump_Ast(ast_getattr(stmt, "stmt_list"));
}

internal void
dump_MethodCallStmt(struct Ast* stmt)
{
  assert(stmt->kind == Ast_MethodCallStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("lvalue", Value_Ref, ast_getattr(stmt, "lvalue"));
  print_prop("type_args", Value_Ref, ast_getattr(stmt, "type_args"));
  print_prop("args", Value_Ref, ast_getattr(stmt, "args"));
  object_end();
  dump_Ast(ast_getattr(stmt, "lvalue"));
  dump_Ast(ast_getattr(stmt, "type_args"));
  dump_Ast(ast_getattr(stmt, "args"));
}

internal void
dump_Lvalue(struct Ast* lvalue)
{
  assert(lvalue->kind == Ast_Lvalue);
  object_start();
  print_prop_common(lvalue);
  print_prop("name", Value_Ref, ast_getattr(lvalue, "name"));
  print_prop("expr", Value_Ref, ast_getattr(lvalue, "expr"));
  object_end();
  dump_Ast(ast_getattr(lvalue, "name"));
  dump_Ast(ast_getattr(lvalue, "expr"));
}

internal void
dump_Int(struct Ast* node)
{
  assert(node->kind == Ast_Int);
  object_start();
  print_prop_common(node);
  char flags_str[256];
  char* str = flags_str + sprintf(flags_str, "AstInteger_None");
  if ((*(int*)ast_getattr(node, "flags") & AstInteger_HasWidth) != 0) {
    str += sprintf(str, "|%s", "AstInteger_HasWidth");
  }
  if ((*(int*)ast_getattr(node, "flags") & AstInteger_IsSigned) != 0) {
    str += sprintf(str, "|%s", "AstInteger_IsSigned");
  }
  print_prop("flags", Value_String, flags_str);
  print_prop("width", Value_Int, *(int*)ast_getattr(node, "width"));
  print_prop("value", Value_Int, *(int*)ast_getattr(node, "value"));
  object_end();
}

internal void
dump_BaseType(struct Ast* type)
{
  assert(type->kind == Ast_BaseType);
  object_start();
  print_prop_common(type);
  char* type_str = "AstBaseType_None";
  if (*(enum AstBaseTypeKind*)ast_getattr(type, "base_type") == AstBaseType_Bool) {
    type_str = "AstBaseType_Bool";
  } else if (*(enum AstBaseTypeKind*)ast_getattr(type, "base_type") == AstBaseType_Error) {
    type_str = "AstBaseType_Error";
  } else if (*(enum AstBaseTypeKind*)ast_getattr(type, "base_type") == AstBaseType_Int) {
    type_str = "AstBaseType_Int";
  } else if (*(enum AstBaseTypeKind*)ast_getattr(type, "base_type") == AstBaseType_Bit) {
    type_str = "AstBaseType_Bit";
  } else if (*(enum AstBaseTypeKind*)ast_getattr(type, "base_type") == AstBaseType_Varbit) {
    type_str = "AstBaseType_Varbit";
  } else if (*(enum AstBaseTypeKind*)ast_getattr(type, "base_type") == AstBaseType_String) {
    type_str = "AstBaseType_String";
  }
  else assert(*(enum AstBaseTypeKind*)ast_getattr(type, "base_type") == AstBaseType_None);
  print_prop("base_type", Value_String, type_str);
  print_prop("size", Value_Ref, ast_getattr(type, "size"));
  object_end();
  dump_Ast(ast_getattr(type, "size"));
}

internal void
dump_ExternDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_ExternDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, ast_getattr(decl, "name"));
  print_prop("type_params", Value_Ref, ast_getattr(decl, "type_params"));
  print_prop("method_protos", Value_Ref, ast_getattr(decl, "method_protos"));
  object_end();
  dump_Ast(ast_getattr(decl, "name"));
  dump_Ast(ast_getattr(decl, "type_params"));
  dump_Ast(ast_getattr(decl, "method_protos"));
}

internal void
dump_FunctionProto(struct Ast* proto)
{
  assert(proto->kind == Ast_FunctionProto);
  object_start();
  print_prop_common(proto);
  print_prop("return_type", Value_Ref, ast_getattr(proto, "return_type"));
  print_prop("name", Value_Ref, ast_getattr(proto, "name"));
  print_prop("type_params", Value_Ref, ast_getattr(proto, "type_params"));
  print_prop("params", Value_Ref, ast_getattr(proto, "params"));
  object_end();
  dump_Ast(ast_getattr(proto, "return_type"));
  dump_Ast(ast_getattr(proto, "name"));
  dump_Ast(ast_getattr(proto, "type_params"));
  dump_Ast(ast_getattr(proto, "params"));
}

internal void
dump_FunctionDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_FunctionDecl);
  object_start();
  print_prop_common(decl);
  print_prop("proto", Value_Ref, ast_getattr(decl, "proto"));
  print_prop("stmt", Value_Ref, ast_getattr(decl, "stmt"));
  object_end();
  dump_Ast(ast_getattr(decl, "proto"));
  dump_Ast(ast_getattr(decl, "stmt"));
}

internal void
dump_TableDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_TableDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, ast_getattr(decl, "name"));
  print_prop("prop_list", Value_Ref, ast_getattr(decl, "prop_list"));
  object_end();
  dump_Ast(ast_getattr(decl, "name"));
  dump_Ast(ast_getattr(decl, "prop_list"));
}

internal void
dump_TableProp_Actions(struct Ast* actions)
{
  assert(actions->kind == Ast_TableProp_Actions);
  object_start();
  print_prop_common(actions);
  print_prop("action_list", Value_Ref, ast_getattr(actions, "action_list"));
  object_end();
  dump_Ast(ast_getattr(actions, "action_list"));
}

internal void
dump_TableProp_SingleEntry(struct Ast* entry)
{
  assert(entry->kind == Ast_TableProp_SingleEntry);
  object_start();
  print_prop_common(entry);
  print_prop("name", Value_Ref, ast_getattr(entry, "name"));
  print_prop("init_expr", Value_Ref, ast_getattr(entry, "init_expr"));
  object_end();
  dump_Ast(ast_getattr(entry, "name"));
  dump_Ast(ast_getattr(entry, "init_expr"));
}

internal void
dump_ActionRef(struct Ast* ref)
{
  assert(ref->kind == Ast_ActionRef);
  object_start();
  print_prop_common(ref);
  print_prop("name", Value_Ref, ast_getattr(ref, "name"));
  print_prop("args", Value_Ref, ast_getattr(ref, "args"));
  object_end();
  dump_Ast(ast_getattr(ref, "name"));
  dump_Ast(ast_getattr(ref, "args"));
}

internal void
dump_FunctionCallExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_FunctionCallExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr", Value_Ref, ast_getattr(expr, "expr"));
  print_prop("args", Value_Ref, ast_getattr(expr, "args"));
  object_end();
  dump_Ast(ast_getattr(expr, "expr"));
  dump_Ast(ast_getattr(expr, "args"));
}

internal void
dump_HeaderDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_HeaderDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, ast_getattr(decl, "name"));
  print_prop("fields", Value_Ref, ast_getattr(decl, "fields"));
  object_end();
  dump_Ast(ast_getattr(decl, "name"));
  dump_Ast(ast_getattr(decl, "fields"));
}

internal void
dump_HeaderStack(struct Ast* stack)
{
  assert(stack->kind == Ast_HeaderStack);
  object_start();
  print_prop_common(stack);
  print_prop("name", Value_Ref, ast_getattr(stack, "name"));
  print_prop("stack_expr", Value_Ref, ast_getattr(stack, "stack_expr"));
  object_end();
  dump_Ast(ast_getattr(stack, "name"));
  dump_Ast(ast_getattr(stack, "stack_expr"));
}

internal void
dump_StructField(struct Ast* field)
{
  assert(field->kind == Ast_StructField);
  object_start();
  print_prop_common(field);
  print_prop("type", Value_Ref, ast_getattr(field, "type"));
  print_prop("name", Value_Ref, ast_getattr(field, "name"));
  object_end();
  dump_Ast(ast_getattr(field, "type"));
  dump_Ast(ast_getattr(field, "name"));
}

internal void
dump_VarDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_VarDecl);
  object_start();
  print_prop_common(decl);
  print_prop("type", Value_Ref, ast_getattr(decl, "type"));
  print_prop("name", Value_Ref, ast_getattr(decl, "name"));
  print_prop("init_expr", Value_Ref, ast_getattr(decl, "init_expr"));
  object_end();
  dump_Ast(ast_getattr(decl, "type"));
  dump_Ast(ast_getattr(decl, "name"));
  dump_Ast(ast_getattr(decl, "init_expr"));
}

internal void
dump_AssignmentStmt(struct Ast* stmt)
{
  assert(stmt->kind == Ast_AssignmentStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("lvalue", Value_Ref, ast_getattr(stmt, "lvalue"));
  print_prop("expr", Value_Ref, ast_getattr(stmt, "expr"));
  object_end();
  dump_Ast(ast_getattr(stmt, "lvalue"));
  dump_Ast(ast_getattr(stmt, "expr"));
}

internal void
dump_ArrayIndex(struct Ast* index)
{
  assert(index->kind == Ast_ArrayIndex);
  object_start();
  print_prop_common(index);
  print_prop("index", Value_Ref, ast_getattr(index, "index"));
  print_prop("colon_index", Value_Ref, ast_getattr(index, "colon_index"));
  object_end();
  dump_Ast(ast_getattr(index, "index"));
  dump_Ast(ast_getattr(index, "colon_index"));
}

internal void
dump_MemberSelectExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_MemberSelectExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr", Value_Ref, ast_getattr(expr, "expr"));
  print_prop("member_name", Value_Ref, ast_getattr(expr, "member_name"));
  object_end();
  dump_Ast(ast_getattr(expr, "expr"));
  dump_Ast(ast_getattr(expr, "member_name"));
}

internal char*
expr_operator_to_string(enum AstExprOperator op)
{
  char* op_str = "Ast_Op_None";
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
dump_BinaryExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_BinaryExpr);
  object_start();
  print_prop_common(expr);
  print_prop("op", Value_String, expr_operator_to_string(*(enum AstExprOperator*)ast_getattr(expr, "op")));
  print_prop("left_operand", Value_Ref, ast_getattr(expr, "left_operand"));
  print_prop("right_operand", Value_Ref, ast_getattr(expr, "right_operand"));
  object_end();
  dump_Ast(ast_getattr(expr, "left_operand"));
  dump_Ast(ast_getattr(expr, "right_operand"));
}

internal void
dump_KvPair(struct Ast* kv_pair)
{
  assert(kv_pair->kind == Ast_KvPair);
  object_start();
  print_prop_common(kv_pair);
  print_prop("name", Value_Ref, ast_getattr(kv_pair, "name"));
  print_prop("expr", Value_Ref, ast_getattr(kv_pair, "expr"));
  object_end();
  dump_Ast(ast_getattr(kv_pair, "name"));
  dump_Ast(ast_getattr(kv_pair, "expr"));
}

internal void
dump_IndexedArrayExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_IndexedArrayExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr", Value_Ref, ast_getattr(expr, "expr"));
  print_prop("index_expr", Value_Ref, ast_getattr(expr, "index_expr"));
  object_end();
  dump_Ast(ast_getattr(expr, "expr"));
  dump_Ast(ast_getattr(expr, "index_expr"));
}

internal void
dump_HeaderUnionDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_HeaderUnionDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, ast_getattr(decl, "name"));
  print_prop("fields", Value_Ref, ast_getattr(decl, "fields"));
  object_end();
  dump_Ast(ast_getattr(decl, "name"));
  dump_Ast(ast_getattr(decl, "fields"));
}

internal void
dump_StringLiteral(struct Ast* string)
{
  assert(string->kind == Ast_StringLiteral);
  object_start();
  print_prop_common(string);
  print_prop("value", Value_String, ast_getattr(string, "value"));
  object_end();
}

internal void
dump_Tuple(struct Ast* tuple)
{
  assert(tuple->kind == Ast_Tuple);
  object_start();
  print_prop_common(tuple);
  print_prop("type_args", Value_Ref, ast_getattr(tuple, "type_args"));
  object_end();
  dump_Ast(ast_getattr(tuple, "type_args"));
}

internal void
dump_SpecdId(struct Ast* id)
{
  assert(id->kind == Ast_SpecdId);
  object_start();
  print_prop_common(id);
  print_prop("name", Value_Ref, ast_getattr(id, "name"));
  print_prop("init_expr", Value_Ref, ast_getattr(id, "init_expr"));
  object_end();
  dump_Ast(ast_getattr(id, "name"));
  dump_Ast(ast_getattr(id, "init_expr"));
}

internal void
dump_ExpressionListExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_ExpressionListExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr_list", Value_Ref, ast_getattr(expr, "expr_list"));
  object_end();
  dump_Ast(ast_getattr(expr, "expr_list"));
}

internal void
dump_IfStmt(struct Ast* stmt)
{
  assert(stmt->kind == Ast_IfStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("cond_expr", Value_Ref, ast_getattr(stmt, "cond_expr"));
  print_prop("stmt", Value_Ref, ast_getattr(stmt, "stmt"));
  print_prop("else_stmt", Value_Ref, ast_getattr(stmt, "else_stmt"));
  object_end();
  dump_Ast(ast_getattr(stmt, "cond_expr"));
  dump_Ast(ast_getattr(stmt, "stmt"));
  dump_Ast(ast_getattr(stmt, "else_stmt"));
}

internal void
dump_SwitchStmt(struct Ast* stmt)
{
  assert(stmt->kind == Ast_SwitchStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("expr", Value_Ref, ast_getattr(stmt, "expr"));
  print_prop("switch_cases", Value_Ref, ast_getattr(stmt, "switch_cases"));
  object_end();
  dump_Ast(ast_getattr(stmt, "expr"));
  dump_Ast(ast_getattr(stmt, "switch_cases"));
}

internal void
dump_SwitchCase(struct Ast* swcase)
{
  assert(swcase->kind == Ast_SwitchCase);
  object_start();
  print_prop_common(swcase);
  print_prop("label", Value_Ref, ast_getattr(swcase, "label"));
  print_prop("stmt", Value_Ref, ast_getattr(swcase, "stmt"));
  object_end();
  dump_Ast(ast_getattr(swcase, "label"));
  dump_Ast(ast_getattr(swcase, "stmt"));
}

internal void
dump_SwitchLabel(struct Ast* label)
{
  assert(label->kind == Ast_SwitchLabel);
  object_start();
  print_prop_common(label);
  print_prop("name", Value_Ref, ast_getattr(label, "name"));
  object_end();
  dump_Ast(ast_getattr(label, "name"));
}

internal void
dump_ReturnStmt(struct Ast* stmt)
{
  assert(stmt->kind == Ast_ReturnStmt);
  object_start();
  print_prop_common(stmt);
  print_prop("expr", Value_Ref, ast_getattr(stmt, "expr"));
  object_end();
  dump_Ast(ast_getattr(stmt, "expr"));
}

internal void
dump_CastExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_CastExpr);
  object_start();
  print_prop_common(expr);
  print_prop("to_type", Value_Ref, ast_getattr(expr, "to_type"));
  print_prop("expr", Value_Ref, ast_getattr(expr, "expr"));
  object_end();
  dump_Ast(ast_getattr(expr, "to_type"));
  dump_Ast(ast_getattr(expr, "expr"));
}

internal void
dump_UnaryExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_UnaryExpr);
  object_start();
  print_prop_common(expr);
  print_prop("op", Value_String, expr_operator_to_string(*(enum AstExprOperator*)ast_getattr(expr, "op")));
  print_prop("expr", Value_Ref, ast_getattr(expr, "expr"));
  object_end();
  dump_Ast(ast_getattr(expr, "expr"));
}

internal void
dump_Parser(struct Ast* parser)
{
  assert(parser->kind == Ast_Parser);
  object_start();
  print_prop_common(parser);
  print_prop("type_decl", Value_Ref, ast_getattr(parser, "type_decl"));
  print_prop("ctor_params", Value_Ref, ast_getattr(parser, "ctor_params"));
  print_prop("local_elements", Value_Ref, ast_getattr(parser, "local_elements"));
  print_prop("states", Value_Ref, ast_getattr(parser, "states"));
  object_end();
  dump_Ast(ast_getattr(parser, "type_decl"));
  dump_Ast(ast_getattr(parser, "ctor_params"));
  dump_Ast(ast_getattr(parser, "local_elements"));
  dump_Ast(ast_getattr(parser, "states"));
}

internal void
dump_ParserType(struct Ast* parser)
{
  assert(parser->kind == Ast_ParserType);
  object_start();
  print_prop_common(parser);
  print_prop("name", Value_Ref, ast_getattr(parser, "name"));
  print_prop("type_params", Value_Ref, ast_getattr(parser, "type_params"));
  print_prop("params", Value_Ref, ast_getattr(parser, "params"));
  object_end();
  dump_Ast(ast_getattr(parser, "name"));
  dump_Ast(ast_getattr(parser, "type_params"));
  dump_Ast(ast_getattr(parser, "params"));
}

internal void
dump_SpecdType(struct Ast* type)
{
  assert(type->kind == Ast_SpecdType);
  object_start();
  print_prop_common(type);
  print_prop("name", Value_Ref, ast_getattr(type, "name"));
  print_prop("type_args", Value_Ref, ast_getattr(type, "type_args"));
  object_end();
  dump_Ast(ast_getattr(type, "name"));
  dump_Ast(ast_getattr(type, "type_args"));
}

internal void
dump_TypeDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_TypeDecl);
  object_start();
  print_prop_common(decl);
  print_prop("is_typedef", Value_String, *(bool*)ast_getattr(decl, "is_typedef") ? "true" : "false");
  print_prop("type_ref", Value_Ref, ast_getattr(decl, "type_ref"));
  print_prop("name", Value_Ref, ast_getattr(decl, "name"));
  object_end();
  dump_Ast(ast_getattr(decl, "type_ref"));
  dump_Ast(ast_getattr(decl, "name"));
}

internal void
dump_StructDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_StructDecl);
  object_start();
  print_prop_common(decl);
  print_prop("name", Value_Ref, ast_getattr(decl, "name"));
  print_prop("fields", Value_Ref, ast_getattr(decl, "fields"));
  object_end();
  dump_Ast(ast_getattr(decl, "name"));
  dump_Ast(ast_getattr(decl, "fields"));
}

internal void
dump_ParserState(struct Ast* state)
{
  assert(state->kind == Ast_ParserState);
  object_start();
  print_prop_common(state);
  print_prop("name", Value_Ref, ast_getattr(state, "name"));
  print_prop("stmts", Value_Ref, ast_getattr(state, "stmts"));
  print_prop("trans_stmt", Value_Ref, ast_getattr(state, "trans_stmt"));
  object_end();
  dump_Ast(ast_getattr(state, "name"));
  dump_Ast(ast_getattr(state, "stmts"));
  dump_Ast(ast_getattr(state, "trans_stmt"));
}

internal void
dump_SelectExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_SelectExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr_list", Value_Ref, ast_getattr(expr, "expr_list"));
  print_prop("case_list", Value_Ref, ast_getattr(expr, "case_list"));
  object_end();
  dump_Ast(ast_getattr(expr, "expr_list"));
  dump_Ast(ast_getattr(expr, "case_list"));
}

internal void
dump_SelectCase(struct Ast* select)
{
  assert(select->kind == Ast_SelectCase);
  object_start();
  print_prop_common(select);
  print_prop("keyset", Value_Ref, ast_getattr(select, "keyset"));
  print_prop("name", Value_Ref, ast_getattr(select, "name"));
  object_end();
  dump_Ast(ast_getattr(select, "keyset"));
  dump_Ast(ast_getattr(select, "name"));
}

internal void
dump_Default(struct Ast* deflt)
{
  assert(deflt->kind == Ast_Default);
  object_start();
  print_prop_common(deflt);
  object_end();
}

internal void
dump_IntTypeSize(struct Ast* size)
{
  assert(size->kind == Ast_IntTypeSize);
  object_start();
  print_prop_common(size);
  print_prop("size", Value_Ref, ast_getattr(size, "size"));
  object_end();
  dump_Ast(ast_getattr(size, "size"));
}

internal void
dump_TypeArgsExpr(struct Ast* expr)
{
  assert(expr->kind == Ast_TypeArgsExpr);
  object_start();
  print_prop_common(expr);
  print_prop("expr", Value_Ref, ast_getattr(expr, "expr"));
  print_prop("type_args", Value_Ref, ast_getattr(expr, "type_args"));
  object_end();
  dump_Ast(ast_getattr(expr, "expr"));
  dump_Ast(ast_getattr(expr, "type_args"));
}

internal void
dump_ConstDecl(struct Ast* decl)
{
  assert(decl->kind == Ast_ConstDecl);
  object_start();
  print_prop_common(decl);
  print_prop("type_ref", Value_Ref, ast_getattr(decl, "type_ref"));
  print_prop("name", Value_Ref, ast_getattr(decl, "name"));
  print_prop("expr", Value_Ref, ast_getattr(decl, "expr"));
  object_end();
  dump_Ast(ast_getattr(decl, "type_ref"));
  dump_Ast(ast_getattr(decl, "name"));
  dump_Ast(ast_getattr(decl, "expr"));
}

internal void
dump_Bool(struct Ast* node)
{
  assert(node->kind == Ast_Bool);
  object_start();
  print_prop_common(node);
  print_prop("value", Value_Int, ast_getattr(node, "value"));
  object_end();
}

internal void
dump_TableProp_Entries(struct Ast* entries)
{
  assert(entries->kind == Ast_TableProp_Entries);
  object_start();
  print_prop_common(entries);
  print_prop("is_const", Value_Int, *(bool*)ast_getattr(entries, "is_const"));
  print_prop("entries", Value_Ref, ast_getattr(entries, "entries"));
  object_end();
  dump_Ast(ast_getattr(entries, "entries"));
}

internal void
dump_TableEntry(struct Ast* entry)
{
  assert(entry->kind == Ast_TableEntry);
  object_start();
  print_prop_common(entry);
  print_prop("keyset", Value_Ref, ast_getattr(entry, "keyset"));
  print_prop("action", Value_Ref, ast_getattr(entry, "action"));
  object_end();
  dump_Ast(ast_getattr(entry, "keyset"));
  dump_Ast(ast_getattr(entry, "action"));
}

internal void
dump_TableProp_Key(struct Ast* key)
{
  assert(key->kind == Ast_TableProp_Key);
  object_start();
  print_prop_common(key);
  print_prop("keyelem_list", Value_Ref, ast_getattr(key, "keyelem_list"));
  object_end();
  dump_Ast(ast_getattr(key, "keyelem_list"));
}

internal void
dump_KeyElement(struct Ast* elem)
{
  assert(elem->kind == Ast_KeyElement);
  object_start();
  print_prop_common(elem);
  print_prop("expr", Value_Ref, ast_getattr(elem, "expr"));
  print_prop("name", Value_Ref, ast_getattr(elem, "name"));
  object_end();
  dump_Ast(ast_getattr(elem, "expr"));
  dump_Ast(ast_getattr(elem, "name"));
}

internal void
dump_Ast(struct Ast* ast)
{
  while (ast) {
    if (ast->kind == Ast_Control) {
      dump_Control(ast);
    } else if (ast->kind == Ast_Package) {
      dump_Package(ast);
    } else if (ast->kind == Ast_Error) {
      dump_Error(ast);
    } else if (ast->kind == Ast_MatchKind) {
      dump_MatchKind(ast);
    } else if (ast->kind == Ast_Instantiation) {
      dump_Instantiation(ast);
    } else if (ast->kind == Ast_NonTypeName) {
      dump_NonTypeName(ast);
    } else if (ast->kind == Ast_TypeName) {
      dump_TypeName(ast);
    } else if (ast->kind == Ast_Parameter) {
      dump_Parameter(ast);
    } else if (ast->kind == Ast_ControlType) {
      dump_ControlType(ast);
    } else if (ast->kind == Ast_ActionDecl) {
      dump_ActionDecl(ast);
    } else if (ast->kind == Ast_BlockStmt) {
      dump_BlockStmt(ast);
    } else if (ast->kind == Ast_MethodCallStmt) {
      dump_MethodCallStmt(ast);
    } else if (ast->kind == Ast_Lvalue) {
      dump_Lvalue(ast);
    } else if (ast->kind == Ast_Int) {
      dump_Int(ast);
    } else if (ast->kind == Ast_BaseType) {
      dump_BaseType(ast);
    } else if (ast->kind == Ast_ExternDecl) {
      dump_ExternDecl(ast);
    } else if (ast->kind == Ast_FunctionProto) {
      dump_FunctionProto(ast);
    } else if (ast->kind == Ast_FunctionDecl) {
      dump_FunctionDecl(ast);
    } else if (ast->kind == Ast_TableDecl) {
      dump_TableDecl(ast);
    } else if (ast->kind == Ast_TableProp_Actions) {
      dump_TableProp_Actions(ast);
    } else if (ast->kind == Ast_TableProp_SingleEntry) {
      dump_TableProp_SingleEntry(ast);
    } else if (ast->kind == Ast_ActionRef) {
      dump_ActionRef(ast);
    } else if (ast->kind == Ast_FunctionCallExpr) {
      dump_FunctionCallExpr(ast);
    } else if (ast->kind == Ast_HeaderDecl) {
      dump_HeaderDecl(ast);
    } else if (ast->kind == Ast_HeaderStack) {
      dump_HeaderStack(ast);
    } else if (ast->kind == Ast_StructField) {
      dump_StructField(ast);
    } else if (ast->kind == Ast_VarDecl) {
      dump_VarDecl(ast);
    } else if (ast->kind == Ast_AssignmentStmt) {
      dump_AssignmentStmt(ast);
    } else if (ast->kind == Ast_ArrayIndex) {
      dump_ArrayIndex(ast);
    } else if (ast->kind == Ast_MemberSelectExpr) {
      dump_MemberSelectExpr(ast);
    } else if (ast->kind == Ast_BinaryExpr) {
      dump_BinaryExpr(ast);
    } else if (ast->kind == Ast_ExpressionListExpr) {
      dump_ExpressionListExpr(ast);
    } else if (ast->kind == Ast_IfStmt) {
      dump_IfStmt(ast);
    } else if (ast->kind == Ast_SwitchStmt) {
      dump_SwitchStmt(ast);
    } else if (ast->kind == Ast_SwitchCase) {
      dump_SwitchCase(ast);
    } else if (ast->kind == Ast_SwitchLabel) {
      dump_SwitchLabel(ast);
    } else if (ast->kind == Ast_ReturnStmt) {
      dump_ReturnStmt(ast);
    } else if (ast->kind == Ast_CastExpr) {
      dump_CastExpr(ast);
    } else if (ast->kind == Ast_UnaryExpr) {
      dump_UnaryExpr(ast);
    } else if (ast->kind == Ast_Parser) {
      dump_Parser(ast);
    } else if (ast->kind == Ast_ParserType) {
      dump_ParserType(ast);
    } else if (ast->kind == Ast_SpecdType) {
      dump_SpecdType(ast);
    } else if (ast->kind == Ast_TypeDecl) {
      dump_TypeDecl(ast);
    } else if (ast->kind == Ast_StructDecl) {
      dump_StructDecl(ast);
    } else if (ast->kind == Ast_ParserState) {
      dump_ParserState(ast);
    } else if (ast->kind == Ast_SelectExpr) {
      dump_SelectExpr(ast);
    } else if (ast->kind == Ast_SelectCase) {
      dump_SelectCase(ast);
    } else if (ast->kind == Ast_Default) {
      dump_Default(ast);
    } else if (ast->kind == Ast_IntTypeSize) {
      dump_IntTypeSize(ast);
    } else if (ast->kind == Ast_TypeArgsExpr) {
      dump_TypeArgsExpr(ast);
    } else if (ast->kind == Ast_ConstDecl) {
      dump_ConstDecl(ast);
    } else if (ast->kind == Ast_Bool) {
      dump_Bool(ast);
    } else if (ast->kind == Ast_TableProp_Entries) {
      dump_TableProp_Entries(ast);
    } else if (ast->kind == Ast_TableEntry) {
      dump_TableEntry(ast);
    } else if (ast->kind == Ast_TableProp_Key) {
      dump_TableProp_Key(ast);
    } else if (ast->kind == Ast_KeyElement) {
      dump_KeyElement(ast);
    } else if (ast->kind == Ast_KvPair) {
      dump_KvPair(ast);
    } else if (ast->kind == Ast_IndexedArrayExpr) {
      dump_IndexedArrayExpr(ast);
    } else if (ast->kind == Ast_SpecdId) {
      dump_SpecdId(ast);
    } else if (ast->kind == Ast_Tuple) {
      dump_Tuple(ast);
    } else if (ast->kind == Ast_StringLiteral) {
      dump_StringLiteral(ast);
    } else if (ast->kind == Ast_HeaderUnionDecl) {
      dump_HeaderUnionDecl(ast);
    }
    else {
      printf("TODO: %s\n", ast_kind_to_string(ast->kind));
    }
    ast = ast->next_node;
  }
}

