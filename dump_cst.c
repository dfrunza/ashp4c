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
  print_prop("id_list", Value_Ref, error->id_list);
  object_end();
  dump_Cst(error->id_list);
}

internal void
dump_Control(struct Cst_Control* control)
{
  object_start();
  print_prop_common((struct Cst*)control);
  print_prop("type_decl", Value_Ref, control->type_decl);
  print_prop("ctor_params", Value_Ref, control->ctor_params);
  print_prop("local_decls", Value_Ref, control->local_decls);
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
  print_prop("type_params", Value_Ref, control->type_params);
  print_prop("params", Value_Ref, control->params);
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
  print_prop("type_params", Value_Ref, package->type_params);
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
  print_prop("decl_list", Value_Ref, prog->decl_list);
  object_end();
  dump_Cst(prog->decl_list);
}

void
dump_Instantiation(struct Cst_Instantiation* inst)
{
  object_start();
  print_prop_common((struct Cst*)inst);
  print_prop("type", Value_Ref, inst->type);
  print_prop("args", Value_Ref, inst->args);
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
dump_ActionDecl(struct Cst_ActionDecl* action)
{
  object_start();
  print_prop_common((struct Cst*)action);
  print_prop("name", Value_Ref, action->name);
  print_prop("params", Value_Ref, action->params);
  print_prop("stmt", Value_Ref, action->stmt);
  object_end();
  dump_Cst(action->name);
  dump_Cst(action->params);
  dump_Cst(action->stmt);
}

internal void
dump_BlockStmt(struct Cst_BlockStmt* stmt)
{
  object_start();
  print_prop_common((struct Cst*)stmt);
  print_prop("stmt_list", Value_Ref, stmt->stmt_list);
  object_end();
  dump_Cst(stmt->stmt_list);
}

internal void
dump_MethodCallStmt(struct Cst_MethodCallStmt* stmt)
{
  object_start();
  print_prop_common((struct Cst*)stmt);
  print_prop("lvalue", Value_Ref, stmt->lvalue);
  print_prop("type_args", Value_Ref, stmt->type_args);
  print_prop("args", Value_Ref, stmt->args);
  object_end();
  dump_Cst(stmt->lvalue);
  dump_Cst(stmt->type_args);
  dump_Cst(stmt->args);
}

internal void
dump_Lvalue(struct Cst_Lvalue* lvalue)
{
  object_start();
  print_prop_common((struct Cst*)lvalue);
  print_prop("name", Value_Ref, lvalue->name);
  print_prop("expr", Value_Ref, lvalue->expr);
  object_end();
  dump_Cst(lvalue->name);
  dump_Cst(lvalue->expr);
}

internal void
dump_Int(struct Cst_Int* node)
{
  object_start();
  print_prop_common((struct Cst*)node);
  char flags_str[256];
  char* str = flags_str + sprintf(flags_str, "AstInteger_None");
  if ((node->flags & AstInteger_HasWidth) != 0) {
    str += sprintf(str, "|%s", "AstInteger_HasWidth");
  }
  if ((node->flags & AstInteger_IsSigned) != 0) {
    str += sprintf(str, "|%s", "AstInteger_IsSigned");
  }
  print_prop("flags", Value_String, flags_str);
  print_prop("width", Value_Int, node->width);
  print_prop("value", Value_Int, node->value);
  object_end();
}

internal void
dump_ParamDir(struct Cst_ParamDir* dir)
{
  object_start();
  print_prop_common((struct Cst*)dir);
  char* dir_str = "CstDir_None";
  if (dir->dir_kind == AstDir_In) {
    dir_str = "AstDir_In";
  } else if (dir->dir_kind == AstDir_Out) {
    dir_str = "AstDir_Out";
  } else if (dir->dir_kind == AstDir_InOut) {
    dir_str = "AstDir_InOut";
  } else assert(dir->dir_kind == AstDir_None);
  print_prop("dir", Value_String, dir_str);
  object_end();
}

internal void
dump_BaseType(struct Cst_BaseType* type)
{
  object_start();
  print_prop_common((struct Cst*)type);
  char* type_str = "AstBaseType_None";
  if (type->base_type == AstBaseType_Bool) {
    type_str = "AstBaseType_Bool";
  } else if (type->base_type == AstBaseType_Error) {
    type_str = "AstBaseType_Error";
  } else if (type->base_type == AstBaseType_Int) {
    type_str = "AstBaseType_Int";
  } else if (type->base_type == AstBaseType_Bit) {
    type_str = "AstBaseType_Bit";
  } else if (type->base_type == AstBaseType_Varbit) {
    type_str = "AstBaseType_Varbit";
  } else assert(type->base_type == AstBaseType_None);
  print_prop("base_type", Value_String, type_str);
  print_prop("size", Value_Ref, type->size);
  object_end();
  dump_Cst(type->size);
}

internal void
dump_ExternDecl(struct Cst_ExternDecl* decl)
{
  object_start();
  print_prop_common((struct Cst*)decl);
  print_prop("name", Value_Ref, decl->name);
  print_prop("type_params", Value_Ref, decl->type_params);
  print_prop("method_protos", Value_Ref, decl->method_protos);
  object_end();
  dump_Cst(decl->name);
  dump_Cst(decl->type_params);
  dump_Cst(decl->method_protos);
}

internal void
dump_Constructor(struct Cst_Constructor* ctor)
{
  object_start();
  print_prop_common((struct Cst*)ctor);
  print_prop("params", Value_Ref, ctor->params);
  object_end();
  dump_Cst(ctor->params);
}

internal void
dump_FunctionProto(struct Cst_FunctionProto* proto)
{
  object_start();
  print_prop_common((struct Cst*)proto);
  print_prop("return_type", Value_Ref, proto->return_type);
  print_prop("name", Value_Ref, proto->name);
  print_prop("type_params", Value_Ref, proto->type_params);
  print_prop("params", Value_Ref, proto->params);
  object_end();
  dump_Cst(proto->return_type);
  dump_Cst(proto->name);
  dump_Cst(proto->type_params);
  dump_Cst(proto->params);
}

internal void
dump_TableDecl(struct Cst_TableDecl* decl)
{
  object_start();
  print_prop_common((struct Cst*)decl);
  print_prop("name", Value_Ref, decl->name);
  print_prop("prop_list", Value_Ref, decl->prop_list);
  object_end();
  dump_Cst(decl->name);
  dump_Cst(decl->prop_list);
}

internal void
dump_TableProp_Actions(struct Cst_TableProp_Actions* actions)
{
  object_start();
  print_prop_common((struct Cst*)actions);
  print_prop("action_list", Value_Ref, actions->action_list);
  object_end();
  dump_Cst(actions->action_list);
}

internal void
dump_TableProp_SingleEntry(struct Cst_TableProp_SingleEntry* entry)
{
  object_start();
  print_prop_common((struct Cst*)entry);
  print_prop("name", Value_Ref, entry->name);
  print_prop("init_expr", Value_Ref, entry->init_expr);
  object_end();
  dump_Cst(entry->name);
  dump_Cst(entry->init_expr);
}

internal void
dump_ActionRef(struct Cst_ActionRef* ref)
{
  object_start();
  print_prop_common((struct Cst*)ref);
  print_prop("name", Value_Ref, ref->name);
  print_prop("args", Value_Ref, ref->args);
  object_end();
  dump_Cst(ref->name);
  dump_Cst(ref->args);
}

internal void
dump_FunctionCallExpr(struct Cst_FunctionCallExpr* expr)
{
  object_start();
  print_prop_common((struct Cst*)expr);
  print_prop("expr", Value_Ref, expr->expr);
  print_prop("args", Value_Ref, expr->args);
  object_end();
  dump_Cst(expr->expr);
  dump_Cst(expr->args);
}

internal void
dump_HeaderDecl(struct Cst_HeaderDecl* decl)
{
  object_start();
  print_prop_common((struct Cst*)decl);
  print_prop("name", Value_Ref, decl->name);
  print_prop("fields", Value_Ref, decl->fields);
  object_end();
  dump_Cst(decl->name);
  dump_Cst(decl->fields);
}

internal void
dump_HeaderStack(struct Cst_HeaderStack* stack)
{
  object_start();
  print_prop_common((struct Cst*)stack);
  print_prop("name", Value_Ref, stack->name);
  print_prop("stack_expr", Value_Ref, stack->stack_expr);
  object_end();
  dump_Cst(stack->name);
  dump_Cst(stack->stack_expr);
}

internal void
dump_StructField(struct Cst_StructField* field)
{
  object_start();
  print_prop_common((struct Cst*)field);
  print_prop("type", Value_Ref, field->type);
  print_prop("name", Value_Ref, field->name);
  object_end();
  dump_Cst(field->type);
  dump_Cst(field->name);
}

internal void
dump_VarDecl(struct Cst_VarDecl* decl)
{
  object_start();
  print_prop_common((struct Cst*)decl);
  print_prop("type", Value_Ref, decl->type);
  print_prop("name", Value_Ref, decl->name);
  print_prop("init_expr", Value_Ref, decl->init_expr);
  object_end();
  dump_Cst(decl->type);
  dump_Cst(decl->name);
  dump_Cst(decl->init_expr);
}

internal void
dump_AssignmentStmt(struct Cst_AssignmentStmt* stmt)
{
  object_start();
  print_prop_common((struct Cst*)stmt);
  print_prop("lvalue", Value_Ref, stmt->lvalue);
  print_prop("expr", Value_Ref, stmt->expr);
  object_end();
  dump_Cst(stmt->lvalue);
  dump_Cst(stmt->expr);
}

internal void
dump_ArrayIndex(struct Cst_ArrayIndex* index)
{
  object_start();
  print_prop_common((struct Cst*)index);
  print_prop("index", Value_Ref, index->index);
  print_prop("colon_index", Value_Ref, index->colon_index);
  object_end();
  dump_Cst(index->index);
  dump_Cst(index->colon_index);
}

internal void
dump_MemberSelectExpr(struct Cst_MemberSelectExpr* expr)
{
  object_start();
  print_prop_common((struct Cst*)expr);
  print_prop("expr", Value_Ref, expr->expr);
  print_prop("member_name", Value_Ref, expr->member_name);
  object_end();
  dump_Cst(expr->expr);
  dump_Cst(expr->member_name);
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
  } else assert(op == AstOp_None);
  return op_str;
}

internal void
dump_BinaryExpr(struct Cst_BinaryExpr* expr)
{
  object_start();
  print_prop_common((struct Cst*)expr);
  print_prop("op", Value_String, expr_operator_to_string(expr->op));
  print_prop("left_operand", Value_Ref, expr->left_operand);
  print_prop("right_operand", Value_Ref, expr->right_operand);
  object_end();
  dump_Cst(expr->left_operand);
  dump_Cst(expr->right_operand);
}

internal void
dump_ExpressionListExpr(struct Cst_ExpressionListExpr* expr)
{
  object_start();
  print_prop_common((struct Cst*)expr);
  print_prop("expr_list", Value_Ref, expr->expr_list);
  object_end();
  dump_Cst(expr->expr_list);
}

internal void
dump_IfStmt(struct Cst_IfStmt* stmt)
{
  object_start();
  print_prop_common((struct Cst*)stmt);
  print_prop("cond_expr", Value_Ref, stmt->cond_expr);
  print_prop("stmt", Value_Ref, stmt->stmt);
  print_prop("else_stmt", Value_Ref, stmt->else_stmt);
  object_end();
  dump_Cst(stmt->cond_expr);
  dump_Cst(stmt->stmt);
  dump_Cst(stmt->else_stmt);
}

internal void
dump_SwitchStmt(struct Cst_SwitchStmt* stmt)
{
  object_start();
  print_prop_common((struct Cst*)stmt);
  print_prop("expr", Value_Ref, stmt->expr);
  print_prop("switch_cases", Value_Ref, stmt->switch_cases);
  object_end();
  dump_Cst(stmt->expr);
  dump_Cst(stmt->switch_cases);
}

internal void
dump_SwitchCase(struct Cst_SwitchCase* swcase)
{
  object_start();
  print_prop_common((struct Cst*)swcase);
  print_prop("label", Value_Ref, swcase->label);
  print_prop("stmt", Value_Ref, swcase->stmt);
  object_end();
  dump_Cst(swcase->label);
  dump_Cst(swcase->stmt);
}

internal void
dump_SwitchLabel(struct Cst_SwitchLabel* label)
{
  object_start();
  print_prop_common((struct Cst*)label);
  print_prop("name", Value_Ref, label->name);
  object_end();
  dump_Cst(label->name);
}

internal void
dump_ReturnStmt(struct Cst_ReturnStmt* stmt)
{
  object_start();
  print_prop_common((struct Cst*)stmt);
  print_prop("expr", Value_Ref, stmt);
  object_end();
  dump_Cst(stmt->expr);
}

internal void
dump_CastExpr(struct Cst_CastExpr* expr)
{
  object_start();
  print_prop_common((struct Cst*)expr);
  print_prop("to_type", Value_Ref, expr->to_type);
  print_prop("expr", Value_Ref, expr->expr);
  object_end();
  dump_Cst(expr->to_type);
  dump_Cst(expr->expr);
}

internal void
dump_UnaryExpr(struct Cst_UnaryExpr* expr)
{
  object_start();
  print_prop_common((struct Cst*)expr);
  print_prop("op", Value_String, expr_operator_to_string(expr->op));
  print_prop("expr", Value_Ref, expr->expr);
  object_end();
  dump_Cst(expr->expr);
}

internal void
dump_Parser(struct Cst_Parser* parser)
{
  object_start();
  print_prop_common((struct Cst*)parser);
  print_prop("type_decl", Value_Ref, parser->type_decl);
  print_prop("ctor_params", Value_Ref, parser->ctor_params);
  print_prop("local_elements", Value_Ref, parser->local_elements);
  print_prop("states", Value_Ref, parser->states);
  object_end();
  dump_Cst(parser->type_decl);
  dump_Cst(parser->ctor_params);
  dump_Cst(parser->local_elements);
  dump_Cst(parser->states);
}

internal void
dump_ParserType(struct Cst_ParserType* parser)
{
  object_start();
  print_prop_common((struct Cst*)parser);
  print_prop("name", Value_Ref, parser->name);
  print_prop("type_params", Value_Ref, parser->type_params);
  print_prop("params", Value_Ref, parser->params);
  object_end();
  dump_Cst(parser->name);
  dump_Cst(parser->type_params);
  dump_Cst(parser->params);
}

internal void
dump_SpecdType(struct Cst_SpecdType* type)
{
  object_start();
  print_prop_common((struct Cst*)type);
  print_prop("name", Value_Ref, type->name);
  print_prop("type_args", Value_Ref, type->type_args);
  object_end();
  dump_Cst(type->name);
  dump_Cst(type->type_args);
}

internal void
dump_TypeDecl(struct Cst_TypeDecl* decl)
{
  object_start();
  print_prop_common((struct Cst*)decl);
  print_prop("is_typedef", Value_String, decl->is_typedef ? "true" : "false");
  print_prop("type_ref", Value_Ref, decl->type_ref);
  print_prop("name", Value_Ref, decl->name);
  object_end();
  dump_Cst(decl->type_ref);
  dump_Cst(decl->name);
}

internal void
dump_StructDecl(struct Cst_StructDecl* decl)
{
  object_start();
  print_prop_common((struct Cst*)decl);
  print_prop("name", Value_Ref, decl->name);
  print_prop("fields", Value_Ref, decl->fields);
  object_end();
  dump_Cst(decl->name);
  dump_Cst(decl->fields);
}

internal void
dump_ParserState(struct Cst_ParserState* state)
{
  object_start();
  print_prop_common((struct Cst*)state);
  print_prop("name", Value_Ref, state->name);
  print_prop("stmts", Value_Ref, state->stmts);
  print_prop("trans_stmt", Value_Ref, state->trans_stmt);
  object_end();
  dump_Cst(state->name);
  dump_Cst(state->stmts);
  dump_Cst(state->trans_stmt);
}

internal void
dump_SelectExpr(struct Cst_SelectExpr* expr)
{
  object_start();
  print_prop_common((struct Cst*)expr);
  print_prop("expr_list", Value_Ref, expr->expr_list);
  print_prop("case_list", Value_Ref, expr->case_list);
  object_end();
  dump_Cst(expr->expr_list);
  dump_Cst(expr->case_list);
}

internal void
dump_SelectCase(struct Cst_SelectCase* select)
{
  object_start();
  print_prop_common((struct Cst*)select);
  print_prop("keyset", Value_Ref, select->keyset);
  print_prop("name", Value_Ref, select->name);
  object_end();
  dump_Cst(select->keyset);
  dump_Cst(select->name);
}

internal void
dump_Default(struct Cst_Default* deflt)
{
  object_start();
  print_prop_common((struct Cst*)deflt);
  object_end();
}

internal void
dump_IntTypeSize(struct Cst_IntTypeSize* size)
{
  object_start();
  print_prop_common((struct Cst*)size);
  print_prop("size", Value_Ref, size->size);
  object_end();
  dump_Cst(size->size);
}

internal void
dump_DotPrefixedName(struct Cst_DotPrefixedName* name)
{
  object_start();
  print_prop_common((struct Cst*)name);
  print_prop("name", Value_Ref, name->name);
  object_end();
  dump_Cst(name->name);
}

internal void
dump_TypeArgsExpr(struct Cst_TypeArgsExpr* expr)
{
  object_start();
  print_prop_common((struct Cst*)expr);
  print_prop("expr", Value_Ref, expr->expr);
  print_prop("type_args", Value_Ref, expr->type_args);
  object_end();
  dump_Cst(expr->expr);
  dump_Cst(expr->type_args);
}

internal void
dump_ConstDecl(struct Cst_ConstDecl* decl)
{
  object_start();
  print_prop_common((struct Cst*)decl);
  print_prop("type", Value_Ref, decl->type);
  print_prop("name", Value_Ref, decl->name);
  print_prop("expr", Value_Ref, decl->expr);
  object_end();
  dump_Cst(decl->type);
  dump_Cst(decl->name);
  dump_Cst(decl->expr);
}

internal void
dump_Bool(struct Cst_Bool* node)
{
  object_start();
  print_prop_common((struct Cst*)node);
  print_prop("value", Value_Int, node->value);
  object_end();
}

internal void
dump_TableProp_Entries(struct Cst_TableProp_Entries* entries)
{
  object_start();
  print_prop_common((struct Cst*)entries);
  print_prop("is_const", Value_Int, entries->is_const);
  print_prop("entries", Value_Ref, entries->entries);
  object_end();
  dump_Cst(entries->entries);
}

internal void
dump_TableEntry(struct Cst_TableEntry* entry)
{
  object_start();
  print_prop_common((struct Cst*)entry);
  print_prop("keyset", Value_Ref, entry->keyset);
  print_prop("action", Value_Ref, entry->action);
  object_end();
  dump_Cst(entry->keyset);
  dump_Cst(entry->action);
}

internal void
dump_TableProp_Key(struct Cst_TableProp_Key* key)
{
  object_start();
  print_prop_common((struct Cst*)key);
  print_prop("keyelem_list", Value_Ref, key->keyelem_list);
  object_end();
  dump_Cst(key->keyelem_list);
}

internal void
dump_KeyElement(struct Cst_KeyElement* elem)
{
  object_start();
  print_prop_common((struct Cst*)elem);
  print_prop("expr", Value_Ref, elem->expr);
  print_prop("name", Value_Ref, elem->name);
  object_end();
  dump_Cst(elem->expr);
  dump_Cst(elem->name);
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
    } else if (cst->kind == Cst_ActionDecl) {
      dump_ActionDecl((struct Cst_ActionDecl*)cst);
    } else if (cst->kind == Cst_BlockStmt) {
      dump_BlockStmt((struct Cst_BlockStmt*)cst);
    } else if (cst->kind == Cst_MethodCallStmt) {
      dump_MethodCallStmt((struct Cst_MethodCallStmt*)cst);
    } else if (cst->kind == Cst_Lvalue) {
      dump_Lvalue((struct Cst_Lvalue*)cst);
    } else if (cst->kind == Cst_Int) {
      dump_Int((struct Cst_Int*)cst);
    } else if (cst->kind == Cst_ParamDir) {
      dump_ParamDir((struct Cst_ParamDir*)cst);
    } else if (cst->kind == Cst_BaseType) {
      dump_BaseType((struct Cst_BaseType*)cst);
    } else if (cst->kind == Cst_ExternDecl) {
      dump_ExternDecl((struct Cst_ExternDecl*)cst);
    } else if (cst->kind == Cst_Constructor) {
      dump_Constructor((struct Cst_Constructor*)cst);
    } else if (cst->kind == Cst_FunctionProto) {
      dump_FunctionProto((struct Cst_FunctionProto*)cst);
    } else if (cst->kind == Cst_TableDecl) {
      dump_TableDecl((struct Cst_TableDecl*)cst);
    } else if (cst->kind == Cst_TableProp_Actions) {
      dump_TableProp_Actions((struct Cst_TableProp_Actions*)cst);
    } else if (cst->kind == Cst_TableProp_SingleEntry) {
      dump_TableProp_SingleEntry((struct Cst_TableProp_SingleEntry*)cst);
    } else if (cst->kind == Cst_ActionRef) {
      dump_ActionRef((struct Cst_ActionRef*)cst);
    } else if (cst->kind == Cst_FunctionCallExpr) {
      dump_FunctionCallExpr((struct Cst_FunctionCallExpr*)cst);
    } else if (cst->kind == Cst_HeaderDecl) {
      dump_HeaderDecl((struct Cst_HeaderDecl*)cst);
    } else if (cst->kind == Cst_HeaderStack) {
      dump_HeaderStack((struct Cst_HeaderStack*)cst);
    } else if (cst->kind == Cst_StructField) {
      dump_StructField((struct Cst_StructField*)cst);
    } else if (cst->kind == Cst_VarDecl) {
      dump_VarDecl((struct Cst_VarDecl*)cst);
    } else if (cst->kind == Cst_AssignmentStmt) {
      dump_AssignmentStmt((struct Cst_AssignmentStmt*)cst);
    } else if (cst->kind == Cst_ArrayIndex) {
      dump_ArrayIndex((struct Cst_ArrayIndex*)cst);
    } else if (cst->kind == Cst_MemberSelectExpr) {
      dump_MemberSelectExpr((struct Cst_MemberSelectExpr*)cst);
    } else if (cst->kind == Cst_BinaryExpr) {
      dump_BinaryExpr((struct Cst_BinaryExpr*)cst);
    } else if (cst->kind == Cst_ExpressionListExpr) {
      dump_ExpressionListExpr((struct Cst_ExpressionListExpr*)cst);
    } else if (cst->kind == Cst_IfStmt) {
      dump_IfStmt((struct Cst_IfStmt*)cst);
    } else if (cst->kind == Cst_SwitchStmt) {
      dump_SwitchStmt((struct Cst_SwitchStmt*)cst);
    } else if (cst->kind == Cst_SwitchCase) {
      dump_SwitchCase((struct Cst_SwitchCase*)cst);
    } else if (cst->kind == Cst_SwitchLabel) {
      dump_SwitchLabel((struct Cst_SwitchLabel*)cst);
    } else if (cst->kind == Cst_ReturnStmt) {
      dump_ReturnStmt((struct Cst_ReturnStmt*)cst);
    } else if (cst->kind == Cst_CastExpr) {
      dump_CastExpr((struct Cst_CastExpr*)cst);
    } else if (cst->kind == Cst_UnaryExpr) {
      dump_UnaryExpr((struct Cst_UnaryExpr*)cst);
    } else if (cst->kind == Cst_Parser) {
      dump_Parser((struct Cst_Parser*)cst);
    } else if (cst->kind == Cst_ParserType) {
      dump_ParserType((struct Cst_ParserType*)cst);
    } else if (cst->kind == Cst_SpecdType) {
      dump_SpecdType((struct Cst_SpecdType*)cst);
    } else if (cst->kind == Cst_TypeDecl) {
      dump_TypeDecl((struct Cst_TypeDecl*)cst);
    } else if (cst->kind == Cst_StructDecl) {
      dump_StructDecl((struct Cst_StructDecl*)cst);
    } else if (cst->kind == Cst_ParserState) {
      dump_ParserState((struct Cst_ParserState*)cst);
    } else if (cst->kind == Cst_SelectExpr) {
      dump_SelectExpr((struct Cst_SelectExpr*)cst);
    } else if (cst->kind == Cst_SelectCase) {
      dump_SelectCase((struct Cst_SelectCase*)cst);
    } else if (cst->kind == Cst_Default) {
      dump_Default((struct Cst_Default*)cst);
    } else if (cst->kind == Cst_IntTypeSize) {
      dump_IntTypeSize((struct Cst_IntTypeSize*)cst);
    } else if (cst->kind == Cst_DotPrefixedName) {
      dump_DotPrefixedName((struct Cst_DotPrefixedName*)cst);
    } else if (cst->kind == Cst_TypeArgsExpr) {
      dump_TypeArgsExpr((struct Cst_TypeArgsExpr*)cst);
    } else if (cst->kind == Cst_ConstDecl) {
      dump_ConstDecl((struct Cst_ConstDecl*)cst);
    } else if (cst->kind == Cst_Bool) {
      dump_Bool((struct Cst_Bool*)cst);
    } else if (cst->kind == Cst_TableProp_Entries) {
      dump_TableProp_Entries((struct Cst_TableProp_Entries*)cst);
    } else if (cst->kind == Cst_TableEntry) {
      dump_TableEntry((struct Cst_TableEntry*)cst);
    } else if (cst->kind == Cst_TableProp_Key) {
      dump_TableProp_Key((struct Cst_TableProp_Key*)cst);
    } else if (cst->kind == Cst_KeyElement) {
      dump_KeyElement((struct Cst_KeyElement*)cst);
    }
    else {
      printf("TODO: %s\n", cst_kind_to_string(cst->kind));
    }
    cst = cst->next_node;
  }
}

