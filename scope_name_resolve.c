#include "arena.h"
#include "ast.h"
#include "symtable.h"


#define DEBUG_ENABLED 0


internal void resolve_names_expression(struct Scope* scope, struct Ast* expr);
internal void resolve_names_type_ref(struct Scope* scope, struct Ast* type_ref);
internal void resolve_names_statement(struct Scope* scope, struct Ast* stmt);
  

internal void
resolve_names_instantiation(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_Instantiation);
  struct Ast_Instantiation* decl = (struct Ast_Instantiation*)ast;
  resolve_names_type_ref(scope, decl->type_ref);
  if (decl->args) {
    struct ListLink* link = list_first_link(decl->args);
    while (link) {
      struct Ast* arg = link->object;
      resolve_names_expression(scope, arg);
      link = link->next;
    }
  }
}

internal void
resolve_names_function_call(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_FunctionCallExpr);
  struct Ast_FunctionCallExpr* expr = (struct Ast_FunctionCallExpr*)ast;
  resolve_names_expression(scope, expr->expr);
  if (expr->expr->type_args) {
    struct ListLink* link = list_first_link(expr->expr->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      resolve_names_type_ref(scope, type_arg);
      link = link->next;
    }
  }
  if (expr->args) {
    struct ListLink* link = list_first_link(expr->args);
    while (link) {
      struct Ast* arg = link->object;
      resolve_names_expression(scope, arg);
      link = link->next;
    }
  }
}

internal void
resolve_names_expression(struct Scope* scope, struct Ast* ast)
{
  if (ast->kind == Ast_BinaryExpr) {
    struct Ast_BinaryExpr* expr = (struct Ast_BinaryExpr*)ast;
    resolve_names_expression(scope, expr->left_operand);
    resolve_names_expression(scope, expr->right_operand);
  } else if (ast->kind == Ast_UnaryExpr) {
    struct Ast_UnaryExpr* expr = (struct Ast_UnaryExpr*)ast;
    resolve_names_expression(scope, expr->operand);
  } else if (ast->kind == Ast_Name) {
    struct Ast_Name* expr = (struct Ast_Name*)ast;
    struct SymtableEntry* entry = scope_resolve_name(scope, expr->strname);
    if (!(entry->id_kw || entry->id_type || entry->id_ident)) {
      error("at line %d: unknown identifier `%s`.", expr->line_nr, expr->strname);
    } else if (DEBUG_ENABLED) {
      printf("at line %d: identifier `%s` has been resolved.\n", expr->line_nr, expr->strname);
    }
    expr->symtable_entry = entry;
  } else if (ast->kind == Ast_Lvalue) {
    struct Ast_Lvalue* expr = (struct Ast_Lvalue*)ast;
    resolve_names_expression(scope, expr->name);
    if (expr->expr) {
      struct ListLink* link = list_first_link(expr->expr);
      while (link) {
        struct Ast* lvalue_expr = link->object;
        if (lvalue_expr->kind == Ast_Name) {
          if (((struct Ast_Name*)lvalue_expr)->is_dotprefixed) {
            link = link->next;
            continue; // Member selection is checked in a later pass.
          }
        }
        resolve_names_expression(scope, lvalue_expr);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_FunctionCallExpr) {
    resolve_names_function_call(scope, ast);
  } else if (ast->kind == Ast_MemberSelectExpr) {
    struct Ast_MemberSelectExpr* expr = (struct Ast_MemberSelectExpr*)ast;
    if (expr->expr->kind == Ast_MemberSelectExpr) {
      ; // Member selection is checked in a later pass.
    } else {
      resolve_names_expression(scope, expr->expr);
    }
    // 'member_name' is checked in a later pass.
  } else if (ast->kind == Ast_SpecializedType) {
    struct Ast_SpecializedType* expr = (struct Ast_SpecializedType*)ast;
    resolve_names_expression(scope, expr->name);
  } else if (ast->kind == Ast_ExpressionListExpr) {
    struct Ast_ExpressionListExpr* expr = (struct Ast_ExpressionListExpr*)ast;
    if (expr->expr_list) {
      struct ListLink* link = list_first_link(expr->expr_list);
      while (link) {
        struct Ast* expr_expr = link->object;
        resolve_names_expression(scope, expr_expr);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_CastExpr) {
    struct Ast_CastExpr* expr = (struct Ast_CastExpr*)ast;
    resolve_names_type_ref(scope, expr->to_type);
    resolve_names_expression(scope, expr->expr);
  } else if (ast->kind == Ast_IndexedArrayExpr) {
    struct Ast_IndexedArrayExpr* expr = (struct Ast_IndexedArrayExpr*)ast;
    resolve_names_expression(scope, expr->index);
    if (expr->colon_index) {
      resolve_names_expression(scope, expr->colon_index);
    }
  } else if (ast->kind == Ast_KvPair) {
    struct Ast_KvPair* expr = (struct Ast_KvPair*)ast;
    resolve_names_expression(scope, expr->expr);
  } else if (ast->kind == Ast_IntLiteral || ast->kind == Ast_BoolLiteral || ast->kind == Ast_StringLiteral) {
    ; // pass
  }
  else assert(0);
}

internal void
resolve_names_function_param(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_Parameter);
  struct Ast_Parameter* param = (struct Ast_Parameter*)ast;
  struct Ast* type_name = param->type;
  resolve_names_type_ref(scope, type_name);
}

internal void
resolve_names_action_decl(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_ActionDecl);
  struct Ast_ActionDecl* decl = (struct Ast_ActionDecl*)ast;
  if (decl->params) {
    struct ListLink* link = list_first_link(decl->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_names_function_param(scope, param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* action_body = (struct Ast_BlockStmt*)decl->stmt;
  if (action_body->stmt_list) {
    struct ListLink* link = list_first_link(action_body->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      resolve_names_statement(decl->scope, stmt);
      link = link->next;
    }
  }
}

internal void
resolve_names_keyset_expr(struct Scope* scope, struct Ast* expr)
{
  if (expr->kind == Ast_Default || expr->kind == Ast_Dontcare) {
    ; // pass
  } else {
    resolve_names_expression(scope, expr);
  }
}

internal void
resolve_names_select_keyset(struct Scope* scope, struct Ast* ast)
{
  if (ast->kind == Ast_TupleKeyset) {
    struct Ast_TupleKeyset* keyset = (struct Ast_TupleKeyset*)ast;
    struct ListLink* link = list_first_link(keyset->expr_list);
    while (link) {
      struct Ast* expr = link->object;
      resolve_names_keyset_expr(scope, expr);
      link = link->next;
    }
  } else {
    resolve_names_keyset_expr(scope, ast);
  }
}

internal void
resolve_names_action_ref(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_ActionRef);
  struct Ast_ActionRef* action = (struct Ast_ActionRef*)ast;
  resolve_names_expression(scope, action->name);
  if (action->args) {
    struct ListLink* link = list_first_link(action->args);
    while (link) {
      struct Ast* arg = link->object;
      resolve_names_expression(scope, arg);
      link = link->next;
    }
  }
}

internal void
resolve_names_table_keyelem(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_KeyElement);
  struct Ast_KeyElement* keyelem = (struct Ast_KeyElement*)ast;
  resolve_names_expression(scope, keyelem->expr);
  resolve_names_expression(scope, keyelem->name);
}

internal void
resolve_names_table_entry(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_TableEntry);
  struct Ast_TableEntry* entry = (struct Ast_TableEntry*)ast;
  resolve_names_select_keyset(scope, entry->keyset);
  resolve_names_action_ref(scope, entry->action);
}

internal void
resolve_names_table_property(struct Scope* scope, struct Ast* ast)
{
  if (ast->kind == Ast_TableProp_Actions) {
    struct Ast_TableProp_Actions* prop = (struct Ast_TableProp_Actions*)ast;
    if (prop->action_list) {
      struct ListLink* link = list_first_link(prop->action_list);
      while (link) {
        struct Ast* action = link->object;
        resolve_names_action_ref(scope, action);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_TableProp_SingleEntry) {
    struct Ast_TableProp_SingleEntry* prop = (struct Ast_TableProp_SingleEntry*)ast;
    if (prop->init_expr) {
      resolve_names_expression(scope, prop->init_expr);
    }
  } else if (ast->kind == Ast_TableProp_Key) {
    struct Ast_TableProp_Key* prop = (struct Ast_TableProp_Key*)ast;
    struct ListLink* link = list_first_link(prop->keyelem_list);
    while (link) {
      struct Ast* keyelem = link->object;
      resolve_names_table_keyelem(scope, keyelem);
      link = link->next;
    }
  } else if (ast->kind == Ast_TableProp_Entries) {
    struct Ast_TableProp_Entries* prop = (struct Ast_TableProp_Entries*)ast;
    struct ListLink* link = list_first_link(prop->entries);
    while (link) {
      struct Ast* entry = link->object;
      resolve_names_table_entry(scope, entry);
      link = link->next;
    }
  }
  else assert(0);
}

internal void
resolve_names_table_decl(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_TableDecl);
  struct Ast_TableDecl* decl = (struct Ast_TableDecl*)ast;
  if (decl->prop_list) {
    struct ListLink* link = list_first_link(decl->prop_list);
    while (link) {
      struct Ast* prop = link->object;
      resolve_names_table_property(scope, prop);
      link = link->next;
    }
  }
}

internal void
resolve_names_method_call(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_MethodCallStmt);
  struct Ast_MethodCallStmt* stmt = (struct Ast_MethodCallStmt*)ast;
  resolve_names_expression(scope, stmt->lvalue);
  if (stmt->type_args) {
    struct ListLink* link = list_first_link(stmt->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      resolve_names_type_ref(scope, type_arg);
      link = link->next;
    }
  }
  if (stmt->args) {
    struct ListLink* link = list_first_link(stmt->args);
    while (link) {
      struct Ast* arg = link->object;
      resolve_names_expression(scope, arg);
      link = link->next;
    }
  }
}

internal void
resolve_names_switch_case(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_SwitchCase);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  if (switch_case->stmt) {
    resolve_names_statement(scope, switch_case->stmt);
  }
}

internal void
resolve_names_statement(struct Scope* scope, struct Ast* ast)
{
  if (ast->kind == Ast_IfStmt) {
    struct Ast_IfStmt* stmt = (struct Ast_IfStmt*)ast;
    resolve_names_expression(scope, stmt->cond_expr);
    resolve_names_statement(stmt->stmt->scope ? stmt->stmt->scope : scope, stmt->stmt);
    if (stmt->else_stmt) {
      resolve_names_statement(stmt->else_stmt->scope ? stmt->else_stmt->scope : scope, stmt->else_stmt);
    }
  } else if (ast->kind == Ast_BlockStmt) {
    struct Ast_BlockStmt* stmt = (struct Ast_BlockStmt*)ast;
    if (stmt->stmt_list) {
      struct ListLink* link = list_first_link(stmt->stmt_list);
      while (link) {
        struct Ast* block_stmt = link->object;
        resolve_names_statement(stmt->scope, block_stmt);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_AssignmentStmt) {
    struct Ast_AssignmentStmt* stmt = (struct Ast_AssignmentStmt*)ast;
    resolve_names_expression(scope, stmt->lvalue);
    struct Ast* assign_expr = stmt->expr;
    resolve_names_expression(scope, assign_expr);
  } else if (ast->kind == Ast_MethodCallStmt) {
    resolve_names_method_call(scope, ast);
  } else if (ast->kind == Ast_DirectApplication) {
    struct Ast_DirectApplication* stmt = (struct Ast_DirectApplication*)ast;
    resolve_names_expression(scope, stmt->name);
  } else if (ast->kind == Ast_ReturnStmt) {
    struct Ast_ReturnStmt* stmt = (struct Ast_ReturnStmt*)ast;
    if (stmt->expr) {
      resolve_names_expression(scope, stmt->expr);
    }
  } else if (ast->kind == Ast_VarDecl) {
    struct Ast_VarDecl* stmt = (struct Ast_VarDecl*)ast;
    resolve_names_type_ref(scope, stmt->type);
    if (stmt->init_expr) {
      resolve_names_expression(scope, stmt->init_expr);
    }
  } else if (ast->kind == Ast_ActionDecl) {
    resolve_names_action_decl(scope, ast);
  } else if (ast->kind == Ast_Instantiation) {
    resolve_names_instantiation(scope, ast);
  } else if (ast->kind == Ast_TableDecl) {
    resolve_names_table_decl(scope, ast);
  } else if (ast->kind == Ast_SwitchStmt) {
    struct Ast_SwitchStmt* stmt = (struct Ast_SwitchStmt*)ast;
    resolve_names_expression(scope, stmt->expr);
    if (stmt->switch_cases) {
      struct ListLink* link = list_first_link(stmt->switch_cases);
      while (link) {
        struct Ast* switch_case = link->object;
        resolve_names_switch_case(scope, switch_case);
        link = link->next;
      }
    }
  }
  else assert(0);
}

internal void
resolve_names_type_ref(struct Scope* scope, struct Ast* ast)
{
  if (ast->kind == Ast_BaseType) {
    struct Ast_BaseType* type_ref = (struct Ast_BaseType*)ast;
    resolve_names_expression(scope, type_ref->type_name);
  } else if (ast->kind == Ast_HeaderStack) {
    struct Ast_HeaderStack* type_ref = (struct Ast_HeaderStack*)ast;
    resolve_names_expression(scope, type_ref->name);
    struct Ast* stack_expr = type_ref->stack_expr;
    resolve_names_expression(scope, stack_expr);
  } else if (ast->kind == Ast_Name || ast->kind == Ast_SpecializedType) {
    resolve_names_expression(scope, ast);
  } else if (ast->kind == Ast_Tuple) {
    struct Ast_Tuple* type_ref = (struct Ast_Tuple*)ast;
    if (type_ref->type_args) {
      struct ListLink* link = list_first_link(type_ref->type_args);
      while (link) {
        struct Ast* type_arg = link->object;
        resolve_names_type_ref(scope, type_arg);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_StructDecl || ast->kind == Ast_HeaderDecl || ast->kind == Ast_HeaderUnionDecl) {
    ; // pass
  }
  else assert(0);
}

internal void
resolve_names_transition_select_case(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_SelectCase);
  struct Ast_SelectCase* select_case = (struct Ast_SelectCase*)ast;
  resolve_names_select_keyset(scope, select_case->keyset);
  resolve_names_expression(scope, select_case->name);
}

internal void
resolve_names_parser_transition(struct Scope* scope, struct Ast* ast)
{
  if (ast->kind == Ast_Name) {
    resolve_names_expression(scope, ast);
  } else if (ast->kind == Ast_SelectExpr) {
    struct Ast_SelectExpr* trans_stmt = (struct Ast_SelectExpr*)ast;
    struct ListLink* link = list_first_link(trans_stmt->expr_list);
    while (link) {
      struct Ast* expr = link->object;
      resolve_names_expression(scope, expr);
      link = link->next;
    }
    link = list_first_link(trans_stmt->case_list);
    while (link) {
      struct Ast* select_case = link->object;
      resolve_names_transition_select_case(scope, select_case);
      link = link->next;
    }
  }
  else assert(0);
}

internal void
resolve_names_parser_state(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_ParserState);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  if (state->stmt_list) {
    struct ListLink* link = list_first_link(state->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      resolve_names_statement(state->scope, stmt);
      link = link->next;
    }
  }
  resolve_names_parser_transition(state->scope, state->trans_stmt);
}

internal void
resolve_names_control_decl(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_ControlDecl);
  struct Ast_ControlDecl* decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlType* type_decl = (struct Ast_ControlType*)decl->type_decl;
  if (type_decl->params) {
    struct ListLink* link = list_first_link(type_decl->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_names_function_param(decl->scope, param);
      link = link->next;
    }
  }
  if (decl->local_decls) {
    struct ListLink* link = list_first_link(decl->local_decls);
    while (link) {
      struct Ast* stmt = link->object;
      resolve_names_statement(decl->scope, stmt);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* apply_stmt = (struct Ast_BlockStmt*)decl->apply_stmt;
  if (apply_stmt) {
    if (apply_stmt->stmt_list) {
      struct ListLink* link = list_first_link(apply_stmt->stmt_list);
      while (link) {
        struct Ast* stmt = link->object;
        resolve_names_statement(apply_stmt->scope, stmt);
        link = link->next;
      }
    }
  }
}

internal void
resolve_names_const_decl(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_ConstDecl);
  struct Ast_ConstDecl* decl = (struct Ast_ConstDecl*)ast;
  resolve_names_type_ref(scope, decl->type_ref);
  resolve_names_expression(scope, decl->expr);
}

internal void
resolve_names_function_proto(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_FunctionProto);
  struct Ast_FunctionProto* decl = (struct Ast_FunctionProto*)ast;
  if (decl->return_type) {
    resolve_names_type_ref(scope, decl->return_type);
  }
  if (decl->params) {
    struct ListLink* link = list_first_link(decl->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_names_function_param(decl->scope, param);
      link = link->next;
    }
  }
}

internal void
resolve_names_package(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_PackageDecl);
  struct Ast_PackageDecl* decl = (struct Ast_PackageDecl*)ast;
  if (decl->params) {
    struct ListLink* link = list_first_link(decl->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_names_function_param(scope, param);
      link = link->next;
    }
  }
}

internal void
resolve_names_parser_decl(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_ParserDecl);
  struct Ast_ParserDecl* decl = (struct Ast_ParserDecl*)ast;
  struct Ast_ParserType* type_decl = (struct Ast_ParserType*)decl->type_decl;
  if (type_decl->params) {
    struct ListLink* link = list_first_link(type_decl->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_names_function_param(decl->scope, param);
      link = link->next;
    }
  }
  if (decl->states) {
    struct ListLink* link = list_first_link(decl->states);
    while (link) {
      struct Ast* state = link->object;
      resolve_names_parser_state(state->scope, state);
      link = link->next;
    }
  }
}

internal void
resolve_names_function_decl(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_FunctionDecl);
  struct Ast_FunctionDecl* decl = (struct Ast_FunctionDecl*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)decl->proto;
  if (function_proto->params) {
    struct ListLink* link = list_first_link(function_proto->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_names_function_param(scope, param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)decl->stmt;
  if (function_body->stmt_list) {
    struct ListLink* link = list_first_link(function_body->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      resolve_names_statement(decl->scope, stmt);
      link = link->next;
    }
  }
}

internal void
resolve_names_extern_decl(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_ExternDecl);
  struct Ast_ExternDecl* decl = (struct Ast_ExternDecl*)ast;
  if (decl->method_protos) {
    struct ListLink* link = list_first_link(decl->method_protos);
    while (link) {
      struct Ast* method = link->object;
      resolve_names_function_proto(scope, method);
      link = link->next;
    }
  }
}

internal void
resolve_names_enum_specified_id(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_SpecifiedIdent);
  struct Ast_SpecifiedIdent* id = (struct Ast_SpecifiedIdent*)ast;
  if (id->init_expr) {
    resolve_names_expression(scope, id->init_expr);
  }
}

internal void
resolve_names_enum_decl(struct Scope* scope, struct Ast* ast)
{
  assert(ast->kind == Ast_EnumDecl);
  struct Ast_EnumDecl* decl = (struct Ast_EnumDecl*)ast;
  if (decl->id_list) {
    struct ListLink* link = list_first_link(decl->id_list);
    while (link) {
      struct Ast* id = link->object;
      if (id->kind == Ast_SpecifiedIdent) {
        resolve_names_enum_specified_id(decl->scope, id);
      }
      link = link->next;
    }
  }
}

void
resolve_names_type_decl(struct Scope* scope, struct Ast* ast)
{
  assert (ast->kind == Ast_TypeDecl);
  struct Ast_TypeDecl* decl = (struct Ast_TypeDecl*)ast;
  resolve_names_type_ref(scope, decl->type_ref);
}

internal void
resolve_names_struct_decl(struct Scope* scope, struct Ast* ast)
{
  assert (ast->kind == Ast_StructDecl);
  struct Ast_StructDecl* decl = (struct Ast_StructDecl*)ast;
  if (decl->fields) {
    struct ListLink* link = list_first_link(decl->fields);
    while (link) {
      struct Ast_StructField* field = link->object;
      resolve_names_type_ref(scope, field->type);
      link = link->next;
    }
  }
}

internal void
resolve_names_header_decl(struct Scope* scope, struct Ast* ast)
{
  assert (ast->kind == Ast_HeaderDecl);
  struct Ast_HeaderDecl* decl = (struct Ast_HeaderDecl*)ast;
  if (decl->fields) {
    struct ListLink* link = list_first_link(decl->fields);
    while (link) {
      struct Ast_StructField* field = link->object;
      resolve_names_type_ref(scope, field->type);
      link = link->next;
    }
  }
}

internal void
resolve_names_header_union_decl(struct Scope* scope, struct Ast* ast)
{
  assert (ast->kind == Ast_HeaderUnionDecl);
  struct Ast_HeaderUnionDecl* decl = (struct Ast_HeaderUnionDecl*)ast;
  if (decl->fields) {
    struct ListLink* link = list_first_link(decl->fields);
    while (link) {
      struct Ast_StructField* field = link->object;
      resolve_names_type_ref(scope, field->type);
      link = link->next;
    }
  }
}

void
resolve_names_program(struct Ast* ast)
{
  assert(ast->kind == Ast_P4Program);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  struct ListLink* link = list_first_link(program->decl_list);
  while (link) {
    struct Ast* decl = link->object;
    if (decl->kind == Ast_ControlDecl) {
      resolve_names_control_decl(program->scope, decl);
    } else if (decl->kind == Ast_ConstDecl) {
      resolve_names_const_decl(program->scope, decl);
    } else if (decl->kind == Ast_FunctionProto) {
      resolve_names_function_proto(program->scope, decl);
    } else if (decl->kind == Ast_PackageDecl) {
      resolve_names_package(program->scope, decl);
    } else if (decl->kind == Ast_Instantiation) {
      resolve_names_instantiation(program->scope, decl);
    } else if (decl->kind == Ast_ParserDecl) {
      resolve_names_parser_decl(program->scope, decl);
    } else if (decl->kind == Ast_FunctionDecl) {
      resolve_names_function_decl(program->scope, decl);
    } else if (decl->kind == Ast_ExternDecl) {
      resolve_names_extern_decl(program->scope, decl);
    } else if (decl->kind == Ast_ActionDecl) {
      resolve_names_action_decl(program->scope, decl);
    } else if (decl->kind == Ast_EnumDecl) {
      resolve_names_enum_decl(program->scope, decl);
    } else if (decl->kind == Ast_TypeDecl) {
      resolve_names_type_decl(program->scope, decl);
    } else if (decl->kind == Ast_StructDecl) {
      resolve_names_struct_decl(program->scope, decl);
    } else if (decl->kind == Ast_HeaderDecl) {
      resolve_names_header_decl(program->scope, decl);
    } else if (decl->kind == Ast_HeaderUnionDecl) {
      resolve_names_header_union_decl(program->scope, decl);
    } else if (decl->kind == Ast_MatchKindDecl || decl->kind == Ast_ErrorDecl) {
      ; // pass
    }
    else assert(0);
    link = link->next;
  }
}