#include "arena.h"
#include "ast.h"
#include "hashmap.h"
#include "symtable.h"


internal struct Hashmap* m_nameref_map = 0;


internal void resolve_nameref_block_statement(struct Ast* block_stmt);
internal void resolve_nameref_statement(struct Ast* decl);
internal void resolve_nameref_expression(struct Ast* expr);
internal void resolve_nameref_type_ref(struct Ast* type_ref);


internal void
resolve_nameref_param(struct Ast* ast)
{
  assert(ast->kind == AST_PARAM);
  struct Ast_Param* param = (struct Ast_Param*)ast;
  resolve_nameref_type_ref(param->type);
}

internal void
resolve_nameref_type_param(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  resolve_nameref_expression(ast);
}

internal void
nameref_context_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL_DECL);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  if (type_decl->type_params) {
    struct ListLink* link = list_first_link(type_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      resolve_nameref_type_param(type_param);
      link = link->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* link = list_first_link(control_decl->ctor_params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
  if (control_decl->local_decls) {
    struct ListLink* link = list_first_link(control_decl->local_decls);
    while (link) {
      struct Ast* decl = link->object;
      resolve_nameref_statement(decl);
      link = link->next;
    }
  }
  if (control_decl->apply_stmt) {
    resolve_nameref_block_statement(control_decl->apply_stmt);
  }
}

internal void
resolve_nameref_struct_field(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  struct Ast_StructField* field = (struct Ast_StructField*)ast;
  resolve_nameref_type_ref(field->type);
}

internal void
resolve_nameref_header_union_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION_DECL);
  struct Ast_HeaderUnionDecl* header_union_decl = (struct Ast_HeaderUnionDecl*)ast;
  if (header_union_decl->fields) {
    struct ListLink* link = list_first_link(header_union_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      resolve_nameref_struct_field(field);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_header_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_DECL);
  struct Ast_HeaderDecl* header_decl = (struct Ast_HeaderDecl*)ast;
  if (header_decl->fields) {
    struct ListLink* link = list_first_link(header_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      resolve_nameref_struct_field(field);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_struct_decl(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_DECL);
  struct Ast_StructDecl* struct_decl = (struct Ast_StructDecl*)ast;
  if (struct_decl->fields) {
    struct ListLink* link = list_first_link(struct_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      resolve_nameref_struct_field(field);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_type_ref(struct Ast* ast)
{
  if (ast->kind == AST_BASETYPE_BOOL || ast->kind == AST_BASETYPE_ERROR
      || ast->kind == AST_BASETYPE_INT || ast->kind == AST_BASETYPE_BIT
      || ast->kind == AST_BASETYPE_VARBIT || ast->kind == AST_BASETYPE_STRING
      || ast->kind == AST_BASETYPE_VOID) {
    struct Ast_BaseType* base_type = (struct Ast_BaseType*)ast;
    resolve_nameref_type_ref(base_type->name);
  } else if (ast->kind == AST_HEADER_STACK) {
    struct Ast_HeaderStack* type_ref = (struct Ast_HeaderStack*)ast;
    resolve_nameref_type_ref(type_ref->name);
    resolve_nameref_expression(type_ref->stack_expr);
  } else if (ast->kind == AST_NAME) {
    resolve_nameref_expression(ast);
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    struct Ast_SpecializedType* speclzd_type = (struct Ast_SpecializedType*)ast;
    resolve_nameref_type_ref(speclzd_type->name);
    struct ListLink* link = list_first_link(speclzd_type->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      resolve_nameref_type_ref(type_arg);
      link = link->next;
    }
  } else if (ast->kind == AST_TUPLE) {
    struct Ast_Tuple* type_ref = (struct Ast_Tuple*)ast;
    if (type_ref->type_args) {
      struct ListLink* link = list_first_link(type_ref->type_args);
      while (link) {
        struct Ast* type_arg = link->object;
        resolve_nameref_type_ref(type_arg);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_STRUCT_DECL) {
    resolve_nameref_struct_decl(ast);
  } else if (ast->kind == AST_HEADER_DECL) {
    resolve_nameref_header_decl(ast);
  } else if (ast->kind == AST_HEADER_UNION_DECL) {
    resolve_nameref_header_union_decl(ast);
  } else if (ast->kind == AST_DONTCARE) {
    ; // pass
  }
  else assert(0);
}

internal void
resolve_nameref_function_call(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_CALL_EXPR);
  struct Ast_FunctionCallExpr* expr = (struct Ast_FunctionCallExpr*)ast;
  resolve_nameref_expression(expr->callee_expr);
  struct Ast_Expression* callee_expr = (struct Ast_Expression*)(expr->callee_expr);
  if (callee_expr->type_args) {
    struct ListLink* link = list_first_link(callee_expr->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      resolve_nameref_type_ref(type_arg);
      link = link->next;
    }
  }
  if (expr->args) {
    struct ListLink* link = list_first_link(expr->args);
    while (link) {
      struct Ast* arg = link->object;
      resolve_nameref_expression(arg);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_instantiation(struct Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  struct Ast_Instantiation* decl = (struct Ast_Instantiation*)ast;
  resolve_nameref_type_ref(decl->type_ref);
  if (decl->args) {
    struct ListLink* link = list_first_link(decl->args);
    while (link) {
      struct Ast* arg = link->object;
      resolve_nameref_expression(arg);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_switch_label(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    ; // pass
  } else {
    resolve_nameref_expression(ast);
  }
}

internal void
resolve_nameref_switch_case(struct Ast* ast)
{
  assert(ast->kind == AST_SWITCH_CASE);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  resolve_nameref_switch_label(switch_case->label);
  struct Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_BLOCK_STMT) {
    resolve_nameref_block_statement(case_stmt);
  }
}

internal void
resolve_nameref_keyset_expr(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT || ast->kind == AST_DONTCARE) {
    ; // pass
  } else {
    resolve_nameref_expression(ast);
  }
}

internal void
resolve_nameref_select_keyset(struct Ast* ast)
{
  if (ast->kind == AST_TUPLE_KEYSET) {
    struct Ast_TupleKeyset* keyset = (struct Ast_TupleKeyset*)ast;
    struct ListLink* link = list_first_link(keyset->expr_list);
    while (link) {
      struct Ast* expr = link->object;
      resolve_nameref_keyset_expr(expr);
      link = link->next;
    }
  } else {
    resolve_nameref_keyset_expr(ast);
  }
}

internal void
resolve_nameref_action_ref(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_REF);
  struct Ast_ActionRef* action = (struct Ast_ActionRef*)ast;
  resolve_nameref_expression(action->name);
  if (action->args) {
    struct ListLink* link = list_first_link(action->args);
    while (link) {
      struct Ast* arg = link->object;
      resolve_nameref_expression(arg);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_table_keyelem(struct Ast* ast)
{
  assert(ast->kind == AST_KEY_ELEMENT);
  struct Ast_KeyElement* keyelem = (struct Ast_KeyElement*)ast;
  resolve_nameref_expression(keyelem->expr);
  resolve_nameref_expression(keyelem->name);
}

internal void
resolve_nameref_table_entry(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRY);
  struct Ast_TableEntry* entry = (struct Ast_TableEntry*)ast;
  resolve_nameref_select_keyset(entry->keyset);
  resolve_nameref_action_ref(entry->action);
}

internal void
resolve_nameref_table_property(struct Ast* ast)
{
  if (ast->kind == AST_TABLE_ACTIONS) {
    struct Ast_TableActions* prop = (struct Ast_TableActions*)ast;
    if (prop->action_list) {
      struct ListLink* link = list_first_link(prop->action_list);
      while (link) {
        struct Ast* action = link->object;
        resolve_nameref_action_ref(action);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    struct Ast_TableSingleEntry* prop = (struct Ast_TableSingleEntry*)ast;
    if (prop->init_expr) {
      resolve_nameref_expression(prop->init_expr);
    }
  } else if (ast->kind == AST_TABLE_KEY) {
    struct Ast_TableKey* prop = (struct Ast_TableKey*)ast;
    struct ListLink* link = list_first_link(prop->keyelem_list);
    while (link) {
      struct Ast* keyelem = link->object;
      resolve_nameref_table_keyelem(keyelem);
      link = link->next;
    }
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    struct Ast_TableEntries* prop = (struct Ast_TableEntries*)ast;
    struct ListLink* link = list_first_link(prop->entries);
    while (link) {
      struct Ast* entry = link->object;
      resolve_nameref_table_entry(entry);
      link = link->next;
    }
  }
  else assert(0);
}

internal void
resolve_nameref_table_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_DECL);
  struct Ast_TableDecl* decl = (struct Ast_TableDecl*)ast;
  if (decl->prop_list) {
    struct ListLink* link = list_first_link(decl->prop_list);
    while (link) {
      struct Ast* prop = link->object;
      resolve_nameref_table_property(prop);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_action_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_DECL);
  struct Ast_ActionDecl* action_decl = (struct Ast_ActionDecl*)ast;
  struct List* params = action_decl->params;
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* action_body = (struct Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    struct List* stmt_list = action_body->stmt_list;
    if (stmt_list) {
      struct ListLink* link = list_first_link(stmt_list);
      while (link) {
        struct Ast* stmt = link->object;
        resolve_nameref_statement(stmt);
        link = link->next;
      }
    }
  }
}

internal void
resolve_nameref_statement(struct Ast* ast)
{
  if (ast->kind == AST_VAR_DECL) {
    struct Ast_VarDecl* decl = (struct Ast_VarDecl*)ast;
    resolve_nameref_type_ref(decl->type);
    if (decl->init_expr) {
      resolve_nameref_expression(decl->init_expr);
    }
  } else if (ast->kind == AST_ACTION_DECL) {
    resolve_nameref_action_decl(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    resolve_nameref_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    resolve_nameref_instantiation(ast);
  } else if (ast->kind == AST_TABLE_DECL) {
    resolve_nameref_table_decl(ast);
  } else if (ast->kind == AST_IF_STMT) {
    struct Ast_IfStmt* stmt = (struct Ast_IfStmt*)ast;
    struct Ast* if_stmt = stmt->stmt;
    resolve_nameref_expression(stmt->cond_expr);
    resolve_nameref_statement(if_stmt);
    struct Ast* else_stmt = stmt->else_stmt;
    if (else_stmt) {
      resolve_nameref_statement(else_stmt);
    }
  } else if (ast->kind == AST_SWITCH_STMT) {
    struct Ast_SwitchStmt* stmt = (struct Ast_SwitchStmt*)ast;
    resolve_nameref_expression(stmt->expr);
    if (stmt->switch_cases) {
      struct ListLink* link = list_first_link(stmt->switch_cases);
      while (link) {
        struct Ast* switch_case = link->object;
        resolve_nameref_switch_case(switch_case);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_ASSIGNMENT_STMT) {
    struct Ast_AssignmentStmt* stmt = (struct Ast_AssignmentStmt*)ast;
    resolve_nameref_expression(stmt->lvalue);
    struct Ast* assign_expr = stmt->expr;
    resolve_nameref_expression(assign_expr);
  } else if (ast->kind == AST_FUNCTION_CALL_EXPR) {
    resolve_nameref_function_call(ast);
  } else if (ast->kind == AST_RETURN_STMT) {
    struct Ast_ReturnStmt* stmt = (struct Ast_ReturnStmt*)ast;
    if (stmt->expr) {
      resolve_nameref_expression(stmt->expr);
    }
  } else if (ast->kind == AST_EXIT_STMT) {
    ; // pass
  }
  else assert(0);
}

internal void
resolve_nameref_function_return_type(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    struct Ast_Name* return_type = (struct Ast_Name*)ast;
    resolve_nameref_type_param((struct Ast*)return_type);
  } else {
    resolve_nameref_type_ref(ast);
  }
}

void
resolve_nameref_function_proto(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)ast;
  if (function_proto->return_type) {
    resolve_nameref_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* link = list_first_link(function_proto->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      resolve_nameref_type_param(type_param);
      link = link->next;
    }
  }
  if (function_proto->params) {
    struct ListLink* link = list_first_link(function_proto->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_block_statement(struct Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  struct Ast_BlockStmt* block_stmt = (struct Ast_BlockStmt*)ast;
  if (block_stmt->stmt_list) {
    struct ListLink* link = list_first_link(block_stmt->stmt_list);
    while (link) {
      struct Ast* decl = link->object;
      resolve_nameref_statement(decl);
      link = link->next;
    }
  }
}


internal void
resolve_nameref_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL_DECL);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  if (type_decl->type_params) {
    struct ListLink* link = list_first_link(type_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      resolve_nameref_type_param(type_param);
      link = link->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* link = list_first_link(type_decl->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* link = list_first_link(control_decl->ctor_params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
  if (control_decl->local_decls) {
    struct ListLink* link = list_first_link(control_decl->local_decls);
    while (link) {
      struct Ast* decl = link->object;
      resolve_nameref_statement(decl);
      link = link->next;
    }
  }
  if (control_decl->apply_stmt) {
    resolve_nameref_block_statement(control_decl->apply_stmt);
  }
}

internal void
resolve_nameref_extern_decl(struct Ast* ast)
{
  assert(ast->kind == AST_EXTERN_DECL);
  struct Ast_ExternDecl* extern_decl = (struct Ast_ExternDecl*)ast;
  if (extern_decl->type_params) {
    struct ListLink* link = list_first_link(extern_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      resolve_nameref_type_param(type_param);
      link = link->next;
    }
  }
  if (extern_decl->method_protos) {
    struct ListLink* link = list_first_link(extern_decl->method_protos);
    while (link) {
      struct Ast* proto = link->object;
      resolve_nameref_function_proto(proto);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_package_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PACKAGE_DECL);
  struct Ast_PackageDecl* package_decl = (struct Ast_PackageDecl*)ast;
  if (package_decl->params) {
    struct ListLink* link = list_first_link(package_decl->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_transition_select_case(struct Ast* ast)
{
  assert(ast->kind == AST_SELECT_CASE);
  struct Ast_SelectCase* select_case = (struct Ast_SelectCase*)ast;
  resolve_nameref_select_keyset(select_case->keyset);
  resolve_nameref_expression(select_case->name);
}

internal void
resolve_nameref_parser_transition(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    resolve_nameref_expression(ast);
  } else if (ast->kind == AST_SELECT_EXPR) {
    struct Ast_SelectExpr* trans_stmt = (struct Ast_SelectExpr*)ast;
    struct ListLink* link = list_first_link(trans_stmt->expr_list);
    while (link) {
      struct Ast* expr = link->object;
      resolve_nameref_expression(expr);
      link = link->next;
    }
    link = list_first_link(trans_stmt->case_list);
    while (link) {
      struct Ast* select_case = link->object;
      resolve_nameref_transition_select_case(select_case);
      link = link->next;
    }
  }
  else assert(0);
}

internal void
resolve_nameref_parser_state(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  if (state->stmt_list) {
    struct ListLink* link = list_first_link(state->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      resolve_nameref_statement(stmt);
      link = link->next;
    }
  }
  resolve_nameref_parser_transition(state->trans_stmt);
}

void
resolve_nameref_const_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONST_DECL);
  struct Ast_ConstDecl* decl = (struct Ast_ConstDecl*)ast;
  resolve_nameref_type_ref(decl->type_ref);
  resolve_nameref_expression(decl->expr);
}

internal void
resolve_nameref_local_parser_element(struct Ast* ast)
{
  if (ast->kind == AST_CONST_DECL) {
    resolve_nameref_const_decl(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    resolve_nameref_instantiation(ast);
  } else if (ast->kind == AST_VAR_DECL) {
    resolve_nameref_statement(ast);
  } else assert(0);
}

internal void
resolve_nameref_parser_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_DECL);
  struct Ast_ParserDecl* parser_decl = (struct Ast_ParserDecl*)ast;
  struct Ast_ParserProto* type_decl = (struct Ast_ParserProto*)parser_decl->type_decl;
  if (type_decl->type_params) {
    struct ListLink* link = list_first_link(type_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      resolve_nameref_type_param(type_param);
      link = link->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* link = list_first_link(type_decl->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
  if (parser_decl->ctor_params) {
    struct ListLink* link = list_first_link(parser_decl->ctor_params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
  if (parser_decl->local_elements) {
    struct ListLink* link = list_first_link(parser_decl->local_elements);
    while (link) {
      struct Ast* element = link->object;
      resolve_nameref_local_parser_element(element);
      link = link->next;
    }
  }
  if (parser_decl->states) {
    struct ListLink* link = list_first_link(parser_decl->states);
    while (link) {
      struct Ast* state = link->object;
      resolve_nameref_parser_state(state);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_type_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TYPE_DECL);
  struct Ast_TypeDecl* type_decl = (struct Ast_TypeDecl*)ast;
  struct Ast* type_ref = type_decl->type_ref;
  resolve_nameref_type_ref(type_ref);
}

internal void
resolve_nameref_function_decl(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_DECL);
  struct Ast_FunctionDecl* function_decl = (struct Ast_FunctionDecl*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)function_decl->proto;
  if (function_proto->return_type) {
    resolve_nameref_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* link = list_first_link(function_proto->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      resolve_nameref_type_param(type_param);
      link = link->next;
    }
  }
  if (function_proto->params) {
    struct ListLink* link = list_first_link(function_proto->params);
    while (link) {
      struct Ast* param = link->object;
      resolve_nameref_param(param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    if (function_body->stmt_list) {
      struct ListLink* link = list_first_link(function_body->stmt_list);
      while (link) {
        struct Ast* stmt = link->object;
        resolve_nameref_statement(stmt);
        link = link->next;
      }
    }
  }
}

internal void
resolve_nameref_enum_field(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
}

internal void
resolve_nameref_specified_id(struct Ast* ast)
{
  assert(ast->kind == AST_SPECIFIED_IDENT);
  struct Ast_SpecifiedIdent* id = (struct Ast_SpecifiedIdent*)ast;
  struct Ast* init_expr = id->init_expr;
  if (init_expr) {
    resolve_nameref_expression(init_expr);
  }
}

internal void
resolve_nameref_error_decl(struct Ast* ast)
{
  assert (ast->kind == AST_ERROR_DECL);
  struct Ast_ErrorDecl* decl = (struct Ast_ErrorDecl*)ast;
  if (decl->id_list) {
    struct ListLink* link = list_first_link(decl->id_list);
    while (link) {
      struct Ast* id = link->object;
      if (id->kind == AST_NAME) {
        resolve_nameref_enum_field(id);
      }
      else assert(0);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_enum_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ENUM_DECL);
  struct Ast_EnumDecl* enum_decl = (struct Ast_EnumDecl*)ast;
  if (enum_decl->id_list) {
    struct ListLink* link = list_first_link(enum_decl->id_list);
    while (link) {
      struct Ast* id = link->object;
      if (id->kind == AST_SPECIFIED_IDENT) {
        resolve_nameref_specified_id(id);
      }
      else assert(0);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_expression(struct Ast* ast)
{
  if (ast->kind == AST_BINARY_EXPR) {
    struct Ast_BinaryExpr* expr = (struct Ast_BinaryExpr*)ast;
    resolve_nameref_expression(expr->left_operand);
    resolve_nameref_expression(expr->right_operand);
  } else if (ast->kind == AST_UNARY_EXPR) {
    struct Ast_UnaryExpr* expr = (struct Ast_UnaryExpr*)ast;
    resolve_nameref_expression(expr->operand);
  } else if (ast->kind == AST_NAME) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    struct NameRef* nameref = nameref_get_entry(m_nameref_map, name->id);
    if (nameref) {
      struct SymtableEntry* entry = scope_lookup_name(nameref->scope, nameref->strname);
    } // else it's a declaration
  } else if (ast->kind == AST_FUNCTION_CALL_EXPR) {
    resolve_nameref_function_call(ast);
  } else if (ast->kind == AST_MEMBER_SELECT_EXPR) {
    struct Ast_MemberSelectExpr* expr = (struct Ast_MemberSelectExpr*)ast;
    resolve_nameref_expression(expr->lhs_expr);
    struct Ast_Name* name = (struct Ast_Name*)expr->member_name;
  } else if (ast->kind == AST_EXPRLIST_EXPR) {
    struct Ast_ExprListExpr* expr = (struct Ast_ExprListExpr*)ast;
    if (expr->expr_list) {
      struct ListLink* link = list_first_link(expr->expr_list);
      while (link) {
        struct Ast* expr_expr = link->object;
        resolve_nameref_expression(expr_expr);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_CAST_EXPR) {
    struct Ast_CastExpr* expr = (struct Ast_CastExpr*)ast;
    resolve_nameref_type_ref(expr->to_type);
    resolve_nameref_expression(expr->expr);
  } else if (ast->kind == AST_INDEXED_ARRAY_EXPR) {
    struct Ast_IndexedArrayExpr* expr = (struct Ast_IndexedArrayExpr*)ast;
    resolve_nameref_expression(expr->index);
    if (expr->colon_index) {
      resolve_nameref_expression(expr->colon_index);
    }
  } else if (ast->kind == AST_KVPAIR_EXPR) {
    struct Ast_KeyValuePairExpr* expr = (struct Ast_KeyValuePairExpr*)ast;
    resolve_nameref_expression(expr->name);
    resolve_nameref_expression(expr->expr);
  } else if (ast->kind == AST_INT_LITERAL || ast->kind == AST_BOOL_LITERAL || ast->kind == AST_STRING_LITERAL) {
    ; // pass
  }
  else assert(0);
}

internal void
resolve_nameref_match_kind(struct Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND_DECL);
  struct Ast_MatchKindDecl* decl = (struct Ast_MatchKindDecl*)ast;
  if (decl->id_list) {
    struct ListLink* link = list_first_link(decl->id_list);
    while (link) {
      struct Ast* id = link->object;
      if (id->kind == AST_NAME) {
        resolve_nameref_enum_field(id);
      } else if (id->kind == AST_SPECIFIED_IDENT) {
        resolve_nameref_specified_id(id);
      }
      else assert(0);
      link = link->next;
    }
  }
}

internal void
resolve_nameref_p4program(struct Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  struct ListLink* link = list_first_link(program->decl_list);
  while (link) {
    struct Ast* decl = link->object;
    if (decl->kind == AST_CONTROL_DECL) {
      resolve_nameref_control_decl(decl);
    } else if (decl->kind == AST_EXTERN_DECL) {
      resolve_nameref_extern_decl(decl);
    } else if (decl->kind == AST_STRUCT_DECL) {
      resolve_nameref_struct_decl(decl);
    } else if (decl->kind == AST_HEADER_DECL) {
      resolve_nameref_header_decl(decl);
    } else if (decl->kind == AST_HEADER_UNION_DECL) {
      resolve_nameref_header_union_decl(decl);
    } else if (decl->kind == AST_PACKAGE_DECL) {
      resolve_nameref_package_decl(decl);
    } else if (decl->kind == AST_PARSER_DECL) {
      resolve_nameref_parser_decl(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      resolve_nameref_instantiation(decl);
    } else if (decl->kind == AST_TYPE_DECL) {
      resolve_nameref_type_decl(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      resolve_nameref_function_proto(decl);
    } else if (decl->kind == AST_CONST_DECL) {
      resolve_nameref_const_decl(decl);
    } else if (decl->kind == AST_ENUM_DECL) {
      resolve_nameref_enum_decl(decl);
    } else if (decl->kind == AST_FUNCTION_DECL) {
      resolve_nameref_function_decl(decl);
    } else if (decl->kind == AST_ACTION_DECL) {
      resolve_nameref_action_decl(decl);
    } else if (decl->kind == AST_MATCH_KIND_DECL) {
      resolve_nameref_match_kind(decl);
    } else if (decl->kind == AST_ERROR_DECL) {
      resolve_nameref_error_decl(decl);
    }
    else assert(0);
    link = link->next;
  }
}

void
resolve_nameref(struct Ast* p4program, struct Hashmap* nameref_map)
{
  m_nameref_map = nameref_map;
  resolve_nameref_p4program(p4program);
}
