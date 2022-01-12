#include "arena.h"
#include "ast.h"
#include "symtable.h"


internal void collect_name_ref_expression(struct Ast* expr);
internal void collect_name_ref_type_ref(struct Ast* type_ref);
internal void collect_name_ref_statement(struct Ast* stmt);


internal void
collect_name_ref_instantiation(struct Ast* ast)
{
  assert(ast->kind == Ast_Instantiation);
  struct Ast_Instantiation* decl = (struct Ast_Instantiation*)ast;
  collect_name_ref_type_ref(decl->type_ref);
  if (decl->args) {
    struct ListLink* link = list_first_link(decl->args);
    while (link) {
      struct Ast* arg = link->object;
      collect_name_ref_expression(arg);
      link = link->next;
    }
  }
}


internal void
collect_name_ref_function_call(struct Ast* ast)
{
  assert(ast->kind == Ast_FunctionCallExpr);
  struct Ast_FunctionCallExpr* expr = (struct Ast_FunctionCallExpr*)ast;
  collect_name_ref_expression(expr->expr);
  if (expr->expr->type_args) {
    struct ListLink* link = list_first_link(expr->expr->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      collect_name_ref_type_ref(type_arg);
      link = link->next;
    }
  }
  if (expr->args) {
    struct ListLink* link = list_first_link(expr->args);
    while (link) {
      struct Ast* arg = link->object;
      collect_name_ref_expression(arg);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_expression(struct Ast* ast)
{
  if (ast->kind == Ast_BinaryExpr) {
    struct Ast_BinaryExpr* expr = (struct Ast_BinaryExpr*)ast;
    collect_name_ref_expression(expr->left_operand);
    collect_name_ref_expression(expr->right_operand);
  } else if (ast->kind == Ast_UnaryExpr) {
    struct Ast_UnaryExpr* expr = (struct Ast_UnaryExpr*)ast;
    collect_name_ref_expression(expr->operand);
  } else if (ast->kind == Ast_Name) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    //printf("%s\n", name->strname);
  } else if (ast->kind == Ast_Lvalue) {
    struct Ast_Lvalue* expr = (struct Ast_Lvalue*)ast;
    collect_name_ref_expression(expr->name);
    if (expr->expr) {
      struct ListLink* link = list_first_link(expr->expr);
      while (link) {
        struct Ast* lvalue_expr = link->object;
        if (lvalue_expr->kind == Ast_Name) {
          if (((struct Ast_Name*)lvalue_expr)->is_dotprefixed) {
            link = link->next;
            continue; // Member selection is resolved in a later pass.
          }
        }
        collect_name_ref_expression(lvalue_expr);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_FunctionCallExpr) {
    collect_name_ref_function_call(ast);
  } else if (ast->kind == Ast_MemberSelectExpr) {
    struct Ast_MemberSelectExpr* expr = (struct Ast_MemberSelectExpr*)ast;
    if (expr->expr->kind == Ast_MemberSelectExpr) {
      ; // Member selection is resolved in a later pass.
    } else {
      collect_name_ref_expression(expr->expr);
    }
    // 'member_name' is resolved in a later pass.
  } else if (ast->kind == Ast_SpecializedType) {
    struct Ast_SpecializedType* expr = (struct Ast_SpecializedType*)ast;
    collect_name_ref_expression(expr->name);
  } else if (ast->kind == Ast_ExpressionListExpr) {
    struct Ast_ExpressionListExpr* expr = (struct Ast_ExpressionListExpr*)ast;
    if (expr->expr_list) {
      struct ListLink* link = list_first_link(expr->expr_list);
      while (link) {
        struct Ast* expr_expr = link->object;
        collect_name_ref_expression(expr_expr);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_CastExpr) {
    struct Ast_CastExpr* expr = (struct Ast_CastExpr*)ast;
    collect_name_ref_type_ref(expr->to_type);
    collect_name_ref_expression(expr->expr);
  } else if (ast->kind == Ast_IndexedArrayExpr) {
    struct Ast_IndexedArrayExpr* expr = (struct Ast_IndexedArrayExpr*)ast;
    collect_name_ref_expression(expr->index);
    if (expr->colon_index) {
      collect_name_ref_expression(expr->colon_index);
    }
  } else if (ast->kind == Ast_KvPair) {
    struct Ast_KvPair* expr = (struct Ast_KvPair*)ast;
    collect_name_ref_expression(expr->expr);
  } else if (ast->kind == Ast_IntLiteral || ast->kind == Ast_BoolLiteral || ast->kind == Ast_StringLiteral) {
    ; // pass
  }
  else assert(0);
}

internal void
collect_name_ref_method_call(struct Ast* ast)
{
  assert(ast->kind == Ast_MethodCallStmt);
  struct Ast_MethodCallStmt* stmt = (struct Ast_MethodCallStmt*)ast;
  collect_name_ref_expression(stmt->lvalue);
  if (stmt->type_args) {
    struct ListLink* link = list_first_link(stmt->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      collect_name_ref_type_ref(type_arg);
      link = link->next;
    }
  }
  if (stmt->args) {
    struct ListLink* link = list_first_link(stmt->args);
    while (link) {
      struct Ast* arg = link->object;
      collect_name_ref_expression(arg);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_function_param(struct Ast* ast)
{
  assert(ast->kind == Ast_Parameter);
  struct Ast_Parameter* param = (struct Ast_Parameter*)ast;
  struct Ast* type_name = param->type;
  collect_name_ref_type_ref(type_name);
}

internal void
collect_name_ref_action_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_ActionDecl);
  struct Ast_ActionDecl* decl = (struct Ast_ActionDecl*)ast;
  if (decl->params) {
    struct ListLink* link = list_first_link(decl->params);
    while (link) {
      struct Ast* param = link->object;
      collect_name_ref_function_param(param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* action_body = (struct Ast_BlockStmt*)decl->stmt;
  if (action_body->stmt_list) {
    struct ListLink* link = list_first_link(action_body->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      collect_name_ref_statement(stmt);
      link = link->next;
    }
  }
}


internal void
collect_name_ref_action_ref(struct Ast* ast)
{
  assert(ast->kind == Ast_ActionRef);
  struct Ast_ActionRef* action = (struct Ast_ActionRef*)ast;
  collect_name_ref_expression(action->name);
  if (action->args) {
    struct ListLink* link = list_first_link(action->args);
    while (link) {
      struct Ast* arg = link->object;
      collect_name_ref_expression(arg);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_keyset_expr(struct Ast* expr)
{
  if (expr->kind == Ast_Default || expr->kind == Ast_Dontcare) {
    ; // pass
  } else {
    collect_name_ref_expression(expr);
  }
}

internal void
collect_name_ref_select_keyset(struct Ast* ast)
{
  if (ast->kind == Ast_TupleKeyset) {
    struct Ast_TupleKeyset* keyset = (struct Ast_TupleKeyset*)ast;
    struct ListLink* link = list_first_link(keyset->expr_list);
    while (link) {
      struct Ast* expr = link->object;
      collect_name_ref_keyset_expr(expr);
      link = link->next;
    }
  } else {
    collect_name_ref_keyset_expr(ast);
  }
}

internal void
collect_name_ref_table_keyelem(struct Ast* ast)
{
  assert(ast->kind == Ast_KeyElement);
  struct Ast_KeyElement* keyelem = (struct Ast_KeyElement*)ast;
  collect_name_ref_expression(keyelem->expr);
  collect_name_ref_expression(keyelem->name);
}

internal void
collect_name_ref_table_entry(struct Ast* ast)
{
  assert(ast->kind == Ast_TableEntry);
  struct Ast_TableEntry* entry = (struct Ast_TableEntry*)ast;
  collect_name_ref_select_keyset(entry->keyset);
  collect_name_ref_action_ref(entry->action);
}

internal void
collect_name_ref_table_property(struct Ast* ast)
{
  if (ast->kind == Ast_TableProp_Actions) {
    struct Ast_TableProp_Actions* prop = (struct Ast_TableProp_Actions*)ast;
    if (prop->action_list) {
      struct ListLink* link = list_first_link(prop->action_list);
      while (link) {
        struct Ast* action = link->object;
        collect_name_ref_action_ref(action);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_TableProp_SingleEntry) {
    struct Ast_TableProp_SingleEntry* prop = (struct Ast_TableProp_SingleEntry*)ast;
    if (prop->init_expr) {
      collect_name_ref_expression(prop->init_expr);
    }
  } else if (ast->kind == Ast_TableProp_Key) {
    struct Ast_TableProp_Key* prop = (struct Ast_TableProp_Key*)ast;
    struct ListLink* link = list_first_link(prop->keyelem_list);
    while (link) {
      struct Ast* keyelem = link->object;
      collect_name_ref_table_keyelem(keyelem);
      link = link->next;
    }
  } else if (ast->kind == Ast_TableProp_Entries) {
    struct Ast_TableProp_Entries* prop = (struct Ast_TableProp_Entries*)ast;
    struct ListLink* link = list_first_link(prop->entries);
    while (link) {
      struct Ast* entry = link->object;
      collect_name_ref_table_entry(entry);
      link = link->next;
    }
  }
  else assert(0);
}

internal void
collect_name_ref_table_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_TableDecl);
  struct Ast_TableDecl* decl = (struct Ast_TableDecl*)ast;
  if (decl->prop_list) {
    struct ListLink* link = list_first_link(decl->prop_list);
    while (link) {
      struct Ast* prop = link->object;
      collect_name_ref_table_property(prop);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_switch_case(struct Ast* ast)
{
  assert(ast->kind == Ast_SwitchCase);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  if (switch_case->stmt) {
    collect_name_ref_statement(switch_case->stmt);
  }
}

internal void
collect_name_ref_statement(struct Ast* ast)
{
  if (ast->kind == Ast_IfStmt) {
    struct Ast_IfStmt* stmt = (struct Ast_IfStmt*)ast;
    collect_name_ref_expression(stmt->cond_expr);
    collect_name_ref_statement(stmt->stmt);
    if (stmt->else_stmt) {
      collect_name_ref_statement(stmt->else_stmt);
    }
  } else if (ast->kind == Ast_BlockStmt) {
    struct Ast_BlockStmt* stmt = (struct Ast_BlockStmt*)ast;
    if (stmt->stmt_list) {
      struct ListLink* link = list_first_link(stmt->stmt_list);
      while (link) {
        struct Ast* block_stmt = link->object;
        collect_name_ref_statement(block_stmt);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_AssignmentStmt) {
    struct Ast_AssignmentStmt* stmt = (struct Ast_AssignmentStmt*)ast;
    collect_name_ref_expression(stmt->lvalue);
    struct Ast* assign_expr = stmt->expr;
    collect_name_ref_expression(assign_expr);
  } else if (ast->kind == Ast_MethodCallStmt) {
    collect_name_ref_method_call(ast);
  } else if (ast->kind == Ast_DirectApplication) {
    struct Ast_DirectApplication* stmt = (struct Ast_DirectApplication*)ast;
    collect_name_ref_expression(stmt->name);
  } else if (ast->kind == Ast_ReturnStmt) {
    struct Ast_ReturnStmt* stmt = (struct Ast_ReturnStmt*)ast;
    if (stmt->expr) {
      collect_name_ref_expression(stmt->expr);
    }
  } else if (ast->kind == Ast_VarDecl) {
    struct Ast_VarDecl* stmt = (struct Ast_VarDecl*)ast;
    collect_name_ref_type_ref(stmt->type);
    if (stmt->init_expr) {
      collect_name_ref_expression(stmt->init_expr);
    }
  } else if (ast->kind == Ast_ActionDecl) {
    collect_name_ref_action_decl(ast);
  } else if (ast->kind == Ast_Instantiation) {
    collect_name_ref_instantiation(ast);
  } else if (ast->kind == Ast_TableDecl) {
    collect_name_ref_table_decl(ast);
  } else if (ast->kind == Ast_SwitchStmt) {
    struct Ast_SwitchStmt* stmt = (struct Ast_SwitchStmt*)ast;
    collect_name_ref_expression(stmt->expr);
    if (stmt->switch_cases) {
      struct ListLink* link = list_first_link(stmt->switch_cases);
      while (link) {
        struct Ast* switch_case = link->object;
        collect_name_ref_switch_case(switch_case);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_ExitStmt) {
      ; // pass
  }
  else assert(0);
}

internal void
collect_name_ref_control_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_ControlDecl);
  struct Ast_ControlDecl* decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlType* type_decl = (struct Ast_ControlType*)decl->type_decl;
  if (decl->local_decls) {
    struct ListLink* link = list_first_link(decl->local_decls);
    while (link) {
      struct Ast* stmt = link->object;
      collect_name_ref_statement(stmt);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* apply_stmt = (struct Ast_BlockStmt*)decl->apply_stmt;
  if (apply_stmt) {
    if (apply_stmt->stmt_list) {
      struct ListLink* link = list_first_link(apply_stmt->stmt_list);
      while (link) {
        struct Ast* stmt = link->object;
        collect_name_ref_statement(stmt);
        link = link->next;
      }
    }
  }
}

internal void
collect_name_ref_type_ref(struct Ast* ast)
{
  if (ast->kind == Ast_BaseType_Bool || ast->kind == Ast_BaseType_Error
      || ast->kind == Ast_BaseType_Int || ast->kind == Ast_BaseType_Bit
      || ast->kind == Ast_BaseType_Bit || ast->kind == Ast_BaseType_Varbit
      || ast->kind == Ast_BaseType_String || ast->kind == Ast_BaseType_Void) {
    collect_name_ref_expression(ast->name);
  } else if (ast->kind == Ast_HeaderStack) {
    struct Ast_HeaderStack* type_ref = (struct Ast_HeaderStack*)ast;
    collect_name_ref_expression(type_ref->name);
    struct Ast* stack_expr = type_ref->stack_expr;
    collect_name_ref_expression(stack_expr);
  } else if (ast->kind == Ast_Name || ast->kind == Ast_SpecializedType) {
    collect_name_ref_expression(ast);
  } else if (ast->kind == Ast_Tuple) {
    struct Ast_Tuple* type_ref = (struct Ast_Tuple*)ast;
    if (type_ref->type_args) {
      struct ListLink* link = list_first_link(type_ref->type_args);
      while (link) {
        struct Ast* type_arg = link->object;
        collect_name_ref_type_ref(type_arg);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_StructDecl || ast->kind == Ast_HeaderDecl || ast->kind == Ast_HeaderUnionDecl) {
    ; // pass
  }
  else assert(0);
}

internal void
collect_name_ref_const_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_ConstDecl);
  struct Ast_ConstDecl* decl = (struct Ast_ConstDecl*)ast;
  collect_name_ref_type_ref(decl->type_ref);
  collect_name_ref_expression(decl->expr);
}

internal void
collect_name_ref_function_proto(struct Ast* ast)
{
  assert(ast->kind == Ast_FunctionProto);
  struct Ast_FunctionProto* decl = (struct Ast_FunctionProto*)ast;
  if (decl->return_type) {
    collect_name_ref_type_ref(decl->return_type);
  }
  if (decl->params) {
    struct ListLink* link = list_first_link(decl->params);
    while (link) {
      struct Ast* param = link->object;
      collect_name_ref_function_param(param);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_package(struct Ast* ast)
{
  assert(ast->kind == Ast_PackageDecl);
  struct Ast_PackageDecl* decl = (struct Ast_PackageDecl*)ast;
  if (decl->params) {
    struct ListLink* link = list_first_link(decl->params);
    while (link) {
      struct Ast* param = link->object;
      collect_name_ref_function_param(param);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_transition_select_case(struct Ast* ast)
{
  assert(ast->kind == Ast_SelectCase);
  struct Ast_SelectCase* select_case = (struct Ast_SelectCase*)ast;
  collect_name_ref_select_keyset(select_case->keyset);
  collect_name_ref_expression(select_case->name);
}

internal void
collect_name_ref_parser_transition(struct Ast* ast)
{
  if (ast->kind == Ast_Name) {
    collect_name_ref_expression(ast);
  } else if (ast->kind == Ast_SelectExpr) {
    struct Ast_SelectExpr* trans_stmt = (struct Ast_SelectExpr*)ast;
    struct ListLink* link = list_first_link(trans_stmt->expr_list);
    while (link) {
      struct Ast* expr = link->object;
      collect_name_ref_expression(expr);
      link = link->next;
    }
    link = list_first_link(trans_stmt->case_list);
    while (link) {
      struct Ast* select_case = link->object;
      collect_name_ref_transition_select_case(select_case);
      link = link->next;
    }
  }
  else assert(0);
}

internal void
collect_name_ref_parser_state(struct Ast* ast)
{
  assert(ast->kind == Ast_ParserState);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  if (state->stmt_list) {
    struct ListLink* link = list_first_link(state->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      collect_name_ref_statement(stmt);
      link = link->next;
    }
  }
  collect_name_ref_parser_transition(state->trans_stmt);
}

internal void
collect_name_ref_parser_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_ParserDecl);
  struct Ast_ParserDecl* decl = (struct Ast_ParserDecl*)ast;
  struct Ast_ParserType* type_decl = (struct Ast_ParserType*)decl->type_decl;
  if (decl->states) {
    struct ListLink* link = list_first_link(decl->states);
    while (link) {
      struct Ast* state = link->object;
      collect_name_ref_parser_state(state);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_function_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_FunctionDecl);
  struct Ast_FunctionDecl* decl = (struct Ast_FunctionDecl*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)decl->proto;
  if (function_proto->params) {
    struct ListLink* link = list_first_link(function_proto->params);
    while (link) {
      struct Ast* param = link->object;
      collect_name_ref_function_param(param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)decl->stmt;
  if (function_body->stmt_list) {
    struct ListLink* link = list_first_link(function_body->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      collect_name_ref_statement(stmt);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_extern_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_ExternDecl);
  struct Ast_ExternDecl* decl = (struct Ast_ExternDecl*)ast;
  if (decl->method_protos) {
    struct ListLink* link = list_first_link(decl->method_protos);
    while (link) {
      struct Ast* method = link->object;
      collect_name_ref_function_proto(method);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_enum_specified_id(struct Ast* ast)
{
  assert(ast->kind == Ast_SpecifiedIdent);
  struct Ast_SpecifiedIdent* id = (struct Ast_SpecifiedIdent*)ast;
  if (id->init_expr) {
    collect_name_ref_expression(id->init_expr);
  }
}

internal void
collect_name_ref_enum_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_EnumDecl);
  struct Ast_EnumDecl* decl = (struct Ast_EnumDecl*)ast;
  if (decl->id_list) {
    struct ListLink* link = list_first_link(decl->id_list);
    while (link) {
      struct Ast* id = link->object;
      if (id->kind == Ast_SpecifiedIdent) {
        collect_name_ref_enum_specified_id(id);
      }
      link = link->next;
    }
  }
}

internal void
collect_name_ref_type_decl(struct Ast* ast)
{
  assert (ast->kind == Ast_TypeDecl);
  struct Ast_TypeDecl* decl = (struct Ast_TypeDecl*)ast;
  collect_name_ref_type_ref(decl->type_ref);
}

internal void
collect_name_ref_struct_decl(struct Ast* ast)
{
  assert (ast->kind == Ast_StructDecl);
  struct Ast_StructDecl* decl = (struct Ast_StructDecl*)ast;
  if (decl->fields) {
    struct ListLink* link = list_first_link(decl->fields);
    while (link) {
      struct Ast_StructField* field = link->object;
      collect_name_ref_type_ref(field->type);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_header_decl(struct Ast* ast)
{
  assert (ast->kind == Ast_HeaderDecl);
  struct Ast_HeaderDecl* decl = (struct Ast_HeaderDecl*)ast;
  if (decl->fields) {
    struct ListLink* link = list_first_link(decl->fields);
    while (link) {
      struct Ast_StructField* field = link->object;
      collect_name_ref_type_ref(field->type);
      link = link->next;
    }
  }
}

internal void
collect_name_ref_header_union_decl(struct Ast* ast)
{
  assert (ast->kind == Ast_HeaderUnionDecl);
  struct Ast_HeaderUnionDecl* decl = (struct Ast_HeaderUnionDecl*)ast;
  if (decl->fields) {
    struct ListLink* link = list_first_link(decl->fields);
    while (link) {
      struct Ast_StructField* field = link->object;
      collect_name_ref_type_ref(field->type);
      link = link->next;
    }
  }
}

void
collect_name_ref_program(struct Ast* ast)
{
  assert(ast->kind == Ast_P4Program);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  struct ListLink* link = list_first_link(program->decl_list);
  while (link) {
    struct Ast* decl = link->object;
    if (decl->kind == Ast_ControlDecl) {
      collect_name_ref_control_decl(decl);
    } else if (decl->kind == Ast_ConstDecl) {
      collect_name_ref_const_decl(decl);
    } else if (decl->kind == Ast_FunctionProto) {
      collect_name_ref_function_proto(decl);
    } else if (decl->kind == Ast_PackageDecl) {
      collect_name_ref_package(decl);
    } else if (decl->kind == Ast_Instantiation) {
      collect_name_ref_instantiation(decl);
    } else if (decl->kind == Ast_ParserDecl) {
      collect_name_ref_parser_decl(decl);
    } else if (decl->kind == Ast_FunctionDecl) {
      collect_name_ref_function_decl(decl);
    } else if (decl->kind == Ast_ExternDecl) {
      collect_name_ref_extern_decl(decl);
    } else if (decl->kind == Ast_ActionDecl) {
      collect_name_ref_action_decl(decl);
    } else if (decl->kind == Ast_EnumDecl) {
      collect_name_ref_enum_decl(decl);
    } else if (decl->kind == Ast_TypeDecl) {
      collect_name_ref_type_decl(decl);
    } else if (decl->kind == Ast_StructDecl) {
      collect_name_ref_struct_decl(decl);
    } else if (decl->kind == Ast_HeaderDecl) {
      collect_name_ref_header_decl(decl);
    } else if (decl->kind == Ast_HeaderUnionDecl) {
      collect_name_ref_header_union_decl(decl);
    } else if (decl->kind == Ast_MatchKindDecl || decl->kind == Ast_ErrorDecl) {
      ; // pass
    }
    else assert(0);
    link = link->next;
  }
}
