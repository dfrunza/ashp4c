#include "arena.h"
#include "ast.h"


internal int tab_level = 0;
internal int tab_size = 2;


enum ValueType {
  Value_NONE_,
  Value_Integer,
  Value_String,
  Value_Id,
  Value_IdList,
};

internal void print_prop(char* name, enum ValueType type, ...);


internal char*
ast_kind_to_string(enum AstKind kind)
{
  switch (kind) {
    case Ast_Name:
      return "Ast_Name";
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
    case Ast_TupleKeyset:
      return "Ast_TupleKeyset";
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
    case Ast_DirectApplication:
      return "Ast_DirectApplication";
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
      assert(kind == Ast_NONE_);
  }
  return "Ast_NONE_";
}

internal char*
expr_operator_to_string(enum AstExprOperator op)
{
  switch (op) {
    case AstExprOp_Add:
      return "AstExprOp_Add";
    case AstExprOp_Sub:
      return "AstExprOp_Sub";
    case AstExprOp_Mul:
      return "AstExprOp_Mul";
    case AstExprOp_Div:
      return "AstExprOp_Div";
    case AstExprOp_And:
      return "AstExprOp_And";
    case AstExprOp_Or:
      return "AstExprOp_Or";
    case AstExprOp_Equal:
      return "AstExprOp_Equal";
    case AstExprOp_NotEqual:
      return "AstExprOp_NotEqual";
    case AstExprOp_Less:
      return "AstExprOp_Less";
    case AstExprOp_Greater:
      return "AstExprOp_Greater";
    case AstExprOp_LessEqual:
      return "AstExprOp_LessEqual";
    case AstExprOp_GreaterEqual:
      return "AstExprOp_GreaterEqual";
    case AstExprOp_BitAnd:
      return "AstExprOp_BitAnd";
    case AstExprOp_BitOr:
      return "AstExprOp_BitOr";
    case AstExprOp_BitXor:
      return "AstExprOp_BitXor";
    case AstExprOp_BitShiftLeft:
      return "AstExprOp_BitShiftLeft";
    case AstExprOp_BitShiftRight:
      return "AstExprOp_BitShiftRight";
    case AstExprOp_LogNot:
      return "AstExprOp_LogNot";
    case AstExprOp_BitNot:
      return "AstExprOp_BitNot";
    case AstExprOp_Minus:
      return "AstExprOp_Minus";
    case AstExprOp_Mask:
      return "AstExprOp_Mask";
    default:
      assert(op == AstExprOp_NONE_);
  }
  return "AstExprOp_NONE_";
}

char*
base_type_to_string(enum AstBaseType base_type)
{
  switch (base_type) {
    case AstBaseType_Bool:
      return "AstBaseType_Bool";
    case AstBaseType_Error:
      return "AstBaseType_Error";
    case AstBaseType_Int:
      return "AstBaseType_Int";
    case AstBaseType_Bit:
      return "AstBaseType_Bit";
    case AstBaseType_Varbit:
      return "AstBaseType_Varbit";
    case AstBaseType_String:
      return "AstBaseType_String";
    default:
      assert(base_type == AstBaseType_NONE_);
  }
  return "AstBaseType_NONE_";
}

char*
param_dir_to_string(enum AstParamDirection dir)
{
  switch (dir) {
    case AstParamDir_In:
      return "AstParamDir_In";
    case AstParamDir_Out:
      return "AstParamDir_Out";
    case AstParamDir_InOut:
      return "AstParamDir_InOut";
    default:
      assert(dir == AstParamDir_NONE_);
  }
  return "AstParamDir_NONE_";
}

char*
int_flags_to_string(enum AstIntegerFlags flags)
{
  static char flags_str[64];
      flags_str[0] = '\0';
  char* str = flags_str;
  if ((flags & AstInteger_HasWidth) != 0) {
    str += sprintf(str, "%s ", "AstInteger_HasWidth");
  }
  if ((flags & AstInteger_IsSigned) != 0) {
    str += sprintf(str, "%s ", "AstInteger_IsSigned");
  }
  return flags ? flags_str : "AstInteger_NONE_";
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
ast_start()
{
  printf("{\n");
  tab_level++;
}

internal void
ast_end()
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
  if (type == Value_Integer) {
    int i = va_arg(value, int);
    printf("%d", i);
  } else if (type == Value_String) {
    char* s = va_arg(value, char*);
    printf("%s", s);
  } else if (type == Value_Id) {
    int id = va_arg(value, int);
    printf("$%d", id);
  }
  else assert(0);
}

internal void
print_prop(char* name, enum ValueType type, ...)
{
  indent_right();
  printf("%s: ", name);
  if (type != Value_NONE_) {
    va_list value;
    va_start(value, type);
    if (type == Value_IdList) {
      struct List* list = va_arg(value, struct List*);
      list_open();
      if (list) {
        struct ListLink* link = list->head->next;
        while (link) {
          printf("$%d", ((struct Ast*)link->object)->id);
          if (link->next) {
            printf(", ");
          }
          link = link->next;
        }
      }
      list_close();
    } else {
      print_value(type, value);
    }
    print_nl();
    va_end(value);
  }
}

void
print_ast(struct Ast* ast)
{
  if (!ast) { return; }
  ast_start();
  print_prop("id", Value_Id, ast->id);
  print_prop("kind", Value_String, ast_kind_to_string(ast->kind));
  print_prop("line_nr", Value_Integer, ast->line_nr);
  struct AstAttributeIterator attr_iter = {};
  struct AstAttribute* attr;
  for (attr = ast_attriter_init(&attr_iter, ast); attr; attr = ast_attriter_get_next(&attr_iter)) {
    if (attr->type == AstAttr_Integer) {
      print_prop(attr->name, Value_Integer, *(int*)attr->value);
    } else if (attr->type == AstAttr_String) {
      print_prop(attr->name, Value_String, attr->value);
    } else if (attr->type == AstAttr_Ast) {
      if (attr->value) {
        print_prop(attr->name, Value_Id, ((struct Ast*)attr->value)->id);
      }
    } else if (attr->type == AstAttr_AstList) {
      print_prop(attr->name, Value_IdList, attr->value);
    } else if (attr->type == AstAttr_ExprOperator) {
      print_prop(attr->name, Value_String, expr_operator_to_string(*(enum AstExprOperator*)attr->value));
    } else if (attr->type == AstAttr_BaseType) {
      print_prop(attr->name, Value_String, base_type_to_string(*(enum AstBaseType*)attr->value));
    } else if (attr->type == AstAttr_ParamDir) {
      print_prop(attr->name, Value_String, param_dir_to_string(*(enum AstParamDirection*)attr->value));
    } else if (attr->type == AstAttr_IntFlags) {
      print_prop(attr->name, Value_String, int_flags_to_string(*(enum AstIntegerFlags*)attr->value));
    }
    else assert(0);
  }
  ast_end();
  for (attr = ast_attriter_init(&attr_iter, ast); attr; attr = ast_attriter_get_next(&attr_iter)) {
    if (attr->type == AstAttr_Ast) {
      print_ast(attr->value);
    } else if (attr->type == AstAttr_AstList) {
      struct List* list = attr->value;
      if (list) {
        struct ListLink* link = list->head->next;
        while (link) {
          print_ast(link->object);
          link = link->next;
        }
      }
    }
  }
}

