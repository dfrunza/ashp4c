#include "arena.h"
#include "ast.h"
#include "hashmap.h"
#include "symtable.h"
#include "build_type.h"
#include <memory.h>  // memset


internal struct Arena *m_type_storage;
internal struct Hashmap* m_nameref_map;
internal struct Hashmap m_type_map = {};

internal void build_type_block_statement(struct Ast* block_stmt);
internal void build_type_statement(struct Ast* decl);
internal void build_type_expression(struct Ast* expr);
internal void build_type_type_ref(struct Ast* type_ref);


#define new_type(type_type, type_ctor) ({ \
  type_type* type = arena_push(m_type_storage, sizeof(type_type)); \
  memset(type, 0, sizeof(type_type)); \
  type->ctor = type_ctor; \
  type; \
})

internal void
build_type_param(struct Ast* ast)
{
  assert(ast->kind == AST_PARAM);
  struct Ast_Param* param = (struct Ast_Param*)ast;
  struct Ast_Name* name = (struct Ast_Name*)param->name;
  build_type_type_ref(param->type);
  struct Type* param_type = type_get_entry(&m_type_map, param->type->id);
  type_add_entry(&m_type_map, param_type, name->id);
  type_add_entry(&m_type_map, param_type, param->id);
}

internal void
build_type_type_param(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* name = (struct Ast_Name*)ast;
  struct NameRef* nameref = nameref_get_entry(m_nameref_map, name->id);
  if (!nameref) {
    struct Type_TypeParam* type = new_type(struct Type_TypeParam, TYPE_TYPEPARAM);
    type->strname = name->strname;
    type_add_entry(&m_type_map, (struct Type*)type, name->id);
  } else {
    build_type_expression(ast);
  }
}

internal void
nameref_context_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL_DECL);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  if (type_decl->type_params) {
    struct ListLink* li = list_first_link(type_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_type_type_param(type_param);
      li = li->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* li = list_first_link(control_decl->ctor_params);
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      li = li->next;
    }
  }
  if (control_decl->local_decls) {
    struct ListLink* li = list_first_link(control_decl->local_decls);
    while (li) {
      struct Ast* decl = li->object;
      build_type_statement(decl);
      li = li->next;
    }
  }
  if (control_decl->apply_stmt) {
    build_type_block_statement(control_decl->apply_stmt);
  }
}

internal void
build_type_struct_field(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  struct Ast_StructField* field = (struct Ast_StructField*)ast;
  build_type_type_ref(field->type);
}

internal void
build_type_header_union_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION_DECL);
  struct Ast_HeaderUnionDecl* header_union_decl = (struct Ast_HeaderUnionDecl*)ast;
  if (header_union_decl->fields) {
    struct ListLink* li = list_first_link(header_union_decl->fields);
    while (li) {
      struct Ast* field = li->object;
      build_type_struct_field(field);
      li = li->next;
    }
  }
}

internal void
build_type_header_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_DECL);
  struct Ast_HeaderDecl* header_decl = (struct Ast_HeaderDecl*)ast;
  if (header_decl->fields) {
    struct ListLink* li = list_first_link(header_decl->fields);
    while (li) {
      struct Ast* field = li->object;
      build_type_struct_field(field);
      li = li->next;
    }
  }
}

internal void
build_type_struct_decl(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_DECL);
  struct Ast_StructDecl* struct_decl = (struct Ast_StructDecl*)ast;
  if (struct_decl->fields) {
    struct ListLink* li = list_first_link(struct_decl->fields);
    while (li) {
      struct Ast* field = li->object;
      build_type_struct_field(field);
      li = li->next;
    }
  }
}

internal void
build_type_type_ref(struct Ast* ast)
{
  if (ast->kind == AST_BASETYPE_BOOL || ast->kind == AST_BASETYPE_ERROR
      || ast->kind == AST_BASETYPE_INT || ast->kind == AST_BASETYPE_BIT
      || ast->kind == AST_BASETYPE_VARBIT || ast->kind == AST_BASETYPE_STRING
      || ast->kind == AST_BASETYPE_VOID) {
    struct Ast_BaseType* base_type = (struct Ast_BaseType*)ast;
    build_type_type_ref(base_type->name);
    struct Type* type = type_get_entry(&m_type_map, base_type->name->id);
    type_add_entry(&m_type_map, type, base_type->id);
  } else if (ast->kind == AST_HEADER_STACK) {
    struct Ast_HeaderStack* type_ref = (struct Ast_HeaderStack*)ast;
    build_type_type_ref(type_ref->name);
    build_type_expression(type_ref->stack_expr);
  } else if (ast->kind == AST_NAME) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    struct NameRef* nameref = nameref_get_entry(m_nameref_map, name->id);
    struct SymtableEntry* se = scope_lookup_name(nameref->scope, nameref->strname);
    if (se->ns_type) {
      struct Type* type = type_get_entry(&m_type_map, se->ns_type->id);
      type_add_entry(&m_type_map, type, name->id);
    } else error("at line %d: unknown name `%s`.", name->line_no, name->strname);
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    struct Ast_SpecializedType* speclzd_type = (struct Ast_SpecializedType*)ast;
    build_type_type_ref(speclzd_type->name);
    struct ListLink* li = list_first_link(speclzd_type->type_args);
    while (li) {
      struct Ast* type_arg = li->object;
      build_type_type_ref(type_arg);
      li = li->next;
    }
  } else if (ast->kind == AST_TUPLE) {
    struct Ast_Tuple* type_ref = (struct Ast_Tuple*)ast;
    if (type_ref->type_args) {
      struct ListLink* li = list_first_link(type_ref->type_args);
      while (li) {
        struct Ast* type_arg = li->object;
        build_type_type_ref(type_arg);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_STRUCT_DECL) {
    build_type_struct_decl(ast);
  } else if (ast->kind == AST_HEADER_DECL) {
    build_type_header_decl(ast);
  } else if (ast->kind == AST_HEADER_UNION_DECL) {
    build_type_header_union_decl(ast);
  } else if (ast->kind == AST_DONTCARE) {
    ; // pass
  }
  else assert(0);
}

internal void
build_type_function_call(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_CALL_EXPR);
  struct Ast_FunctionCallExpr* function_call = (struct Ast_FunctionCallExpr*)ast;
  build_type_expression(function_call->callee_expr);
  struct Ast_Expression* callee_expr = (struct Ast_Expression*)(function_call->callee_expr);
  if (callee_expr->type_args) {
    struct ListLink* li = list_first_link(callee_expr->type_args);
    while (li) {
      struct Ast* type_arg = li->object;
      build_type_type_ref(type_arg);
      li = li->next;
    }
  }
  struct SymtableEntry* se = scope_lookup_name(get_root_scope(), "void");
  struct Type* args_type = type_get_entry(&m_type_map, se->ns_type->id);
  if (function_call->args) {
    struct ListLink* li = list_first_link(function_call->args);
    struct Ast* arg = li->object;
    build_type_expression(arg);
    args_type = type_get_entry(&m_type_map, arg->id);
    li = li->next;
    while (li) {
      struct Ast* arg = li->object;
      build_type_expression(arg);
      struct Type_Product* product_type = new_type(struct Type_Product, TYPE_PRODUCT);
      product_type->lhs_ty = args_type;
      product_type->rhs_ty = type_get_entry(&m_type_map, arg->id);
      li = li->next;
    }
  }
  struct Type_FunctionCall* call_type = new_type(struct Type_FunctionCall, TYPE_FUNCTION_CALL);
  call_type->function_ty = type_get_entry(&m_type_map, callee_expr->id);
  call_type->args_ty = args_type;
  type_add_entry(&m_type_map, (struct Type*)call_type, function_call->id);
}

internal void
build_type_instantiation(struct Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  struct Ast_Instantiation* decl = (struct Ast_Instantiation*)ast;
  build_type_type_ref(decl->type_ref);
  if (decl->args) {
    struct ListLink* li = list_first_link(decl->args);
    while (li) {
      struct Ast* arg = li->object;
      build_type_expression(arg);
      li = li->next;
    }
  }
}

internal void
build_type_switch_label(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    ; // pass
  } else {
    build_type_expression(ast);
  }
}

internal void
build_type_switch_case(struct Ast* ast)
{
  assert(ast->kind == AST_SWITCH_CASE);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  build_type_switch_label(switch_case->label);
  struct Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_BLOCK_STMT) {
    build_type_block_statement(case_stmt);
  }
}

internal void
build_type_keyset_expr(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT || ast->kind == AST_DONTCARE) {
    ; // pass
  } else {
    build_type_expression(ast);
  }
}

internal void
build_type_select_keyset(struct Ast* ast)
{
  if (ast->kind == AST_TUPLE_KEYSET) {
    struct Ast_TupleKeyset* keyset = (struct Ast_TupleKeyset*)ast;
    struct ListLink* li = list_first_link(keyset->expr_list);
    while (li) {
      struct Ast* expr = li->object;
      build_type_keyset_expr(expr);
      li = li->next;
    }
  } else {
    build_type_keyset_expr(ast);
  }
}

internal void
build_type_action_ref(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_REF);
  struct Ast_ActionRef* action = (struct Ast_ActionRef*)ast;
  build_type_expression(action->name);
  if (action->args) {
    struct ListLink* li = list_first_link(action->args);
    while (li) {
      struct Ast* arg = li->object;
      build_type_expression(arg);
      li = li->next;
    }
  }
}

internal void
build_type_table_keyelem(struct Ast* ast)
{
  assert(ast->kind == AST_KEY_ELEMENT);
  struct Ast_KeyElement* keyelem = (struct Ast_KeyElement*)ast;
  build_type_expression(keyelem->expr);
  build_type_expression(keyelem->name);
}

internal void
build_type_table_entry(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRY);
  struct Ast_TableEntry* entry = (struct Ast_TableEntry*)ast;
  build_type_select_keyset(entry->keyset);
  build_type_action_ref(entry->action);
}

internal void
build_type_table_property(struct Ast* ast)
{
  if (ast->kind == AST_TABLE_ACTIONS) {
    struct Ast_TableActions* prop = (struct Ast_TableActions*)ast;
    if (prop->action_list) {
      struct ListLink* li = list_first_link(prop->action_list);
      while (li) {
        struct Ast* action = li->object;
        build_type_action_ref(action);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    struct Ast_TableSingleEntry* prop = (struct Ast_TableSingleEntry*)ast;
    if (prop->init_expr) {
      build_type_expression(prop->init_expr);
    }
  } else if (ast->kind == AST_TABLE_KEY) {
    struct Ast_TableKey* prop = (struct Ast_TableKey*)ast;
    struct ListLink* li = list_first_link(prop->keyelem_list);
    while (li) {
      struct Ast* keyelem = li->object;
      build_type_table_keyelem(keyelem);
      li = li->next;
    }
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    struct Ast_TableEntries* prop = (struct Ast_TableEntries*)ast;
    struct ListLink* li = list_first_link(prop->entries);
    while (li) {
      struct Ast* entry = li->object;
      build_type_table_entry(entry);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
build_type_table_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_DECL);
  struct Ast_TableDecl* decl = (struct Ast_TableDecl*)ast;
  if (decl->prop_list) {
    struct ListLink* li = list_first_link(decl->prop_list);
    while (li) {
      struct Ast* prop = li->object;
      build_type_table_property(prop);
      li = li->next;
    }
  }
}

internal void
build_type_action_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_DECL);
  struct Ast_ActionDecl* action_decl = (struct Ast_ActionDecl*)ast;
  struct List* params = action_decl->params;
  if (params) {
    struct ListLink* li = list_first_link(params);
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      li = li->next;
    }
  }
  struct Ast_BlockStmt* action_body = (struct Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    struct List* stmt_list = action_body->stmt_list;
    if (stmt_list) {
      struct ListLink* li = list_first_link(stmt_list);
      while (li) {
        struct Ast* stmt = li->object;
        build_type_statement(stmt);
        li = li->next;
      }
    }
  }
}

internal void
build_type_statement(struct Ast* ast)
{
  if (ast->kind == AST_VAR_DECL) {
    struct Ast_VarDecl* decl = (struct Ast_VarDecl*)ast;
    build_type_type_ref(decl->type);
    if (decl->init_expr) {
      build_type_expression(decl->init_expr);
    }
  } else if (ast->kind == AST_ACTION_DECL) {
    build_type_action_decl(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    build_type_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    build_type_instantiation(ast);
  } else if (ast->kind == AST_TABLE_DECL) {
    build_type_table_decl(ast);
  } else if (ast->kind == AST_IF_STMT) {
    struct Ast_IfStmt* stmt = (struct Ast_IfStmt*)ast;
    struct Ast* if_stmt = stmt->stmt;
    build_type_expression(stmt->cond_expr);
    build_type_statement(if_stmt);
    struct Ast* else_stmt = stmt->else_stmt;
    if (else_stmt) {
      build_type_statement(else_stmt);
    }
  } else if (ast->kind == AST_SWITCH_STMT) {
    struct Ast_SwitchStmt* stmt = (struct Ast_SwitchStmt*)ast;
    build_type_expression(stmt->expr);
    if (stmt->switch_cases) {
      struct ListLink* li = list_first_link(stmt->switch_cases);
      while (li) {
        struct Ast* switch_case = li->object;
        build_type_switch_case(switch_case);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_ASSIGNMENT_STMT) {
    struct Ast_AssignmentStmt* stmt = (struct Ast_AssignmentStmt*)ast;
    build_type_expression(stmt->lvalue);
    struct Ast* assign_expr = stmt->expr;
    build_type_expression(assign_expr);
  } else if (ast->kind == AST_FUNCTION_CALL_EXPR) {
    build_type_function_call(ast);
  } else if (ast->kind == AST_DIRECT_APPLICATION) {
    struct Ast_DirectApplication* stmt = (struct Ast_DirectApplication*)ast;
    build_type_expression(stmt->name);
  } else if (ast->kind == AST_RETURN_STMT) {
    struct Ast_ReturnStmt* stmt = (struct Ast_ReturnStmt*)ast;
    if (stmt->expr) {
      build_type_expression(stmt->expr);
    }
  } else if (ast->kind == AST_EXIT_STMT) {
    ; // pass
  }
  else assert(0);
}

internal void
build_type_function_return_type(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    struct Ast_Name* return_type = (struct Ast_Name*)ast;
    build_type_type_param((struct Ast*)return_type);
  } else {
    build_type_type_ref(ast);
  }
}

void
build_type_function_proto(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)ast;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  if (function_proto->return_type) {
    build_type_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* li = list_first_link(function_proto->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_type_type_param(type_param);
      li = li->next;
    }
  }
  struct SymtableEntry* se = scope_lookup_name(get_root_scope(), "void");
  struct Type* params_type = type_get_entry(&m_type_map, se->ns_type->id);
  if (function_proto->params) {
    struct ListLink* li = list_first_link(function_proto->params);
    struct Ast* param = li->object;
    build_type_param(param);
    params_type = type_get_entry(&m_type_map, param->id);
    li = li->next;
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      struct Type_Product* product_type = new_type(struct Type_Product, TYPE_PRODUCT); 
      product_type->lhs_ty = params_type;
      product_type->rhs_ty = type_get_entry(&m_type_map, param->id);
      params_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  struct Type_Function* function_type = new_type(struct Type_Function, TYPE_FUNCTION);
  function_type->params_ty = params_type;
  function_type->return_ty = type_get_entry(&m_type_map, function_proto->return_type->id);
  type_add_entry(&m_type_map, (struct Type*)function_type, name->id);
  type_add_entry(&m_type_map, (struct Type*)function_type, function_proto->id);
}

internal void
build_type_block_statement(struct Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  struct Ast_BlockStmt* block_stmt = (struct Ast_BlockStmt*)ast;
  if (block_stmt->stmt_list) {
    struct ListLink* li = list_first_link(block_stmt->stmt_list);
    while (li) {
      struct Ast* decl = li->object;
      build_type_statement(decl);
      li = li->next;
    }
  }
}

internal void
build_type_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL_DECL);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct Type_Name* control_type = new_type(struct Type_Name, TYPE_NAME);
  control_type->strname = name->strname;
  type_add_entry(&m_type_map, (struct Type*)control_type, name->id);
  type_add_entry(&m_type_map, (struct Type*)control_type, control_decl->id);
  if (type_decl->type_params) {
    struct ListLink* li = list_first_link(type_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_type_type_param(type_param);
      li = li->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* li = list_first_link(type_decl->params);
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      li = li->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* li = list_first_link(control_decl->ctor_params);
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      li = li->next;
    }
  }
  if (control_decl->local_decls) {
    struct ListLink* li = list_first_link(control_decl->local_decls);
    while (li) {
      struct Ast* decl = li->object;
      build_type_statement(decl);
      li = li->next;
    }
  }
  if (control_decl->apply_stmt) {
    build_type_block_statement(control_decl->apply_stmt);
  }
}

internal void
build_type_extern_decl(struct Ast* ast)
{
  assert(ast->kind == AST_EXTERN_DECL);
  struct Ast_ExternDecl* extern_decl = (struct Ast_ExternDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)extern_decl->name;
  struct Type_Name* extern_type = new_type(struct Type_Name, TYPE_NAME);
  extern_type->strname = name->strname;
  type_add_entry(&m_type_map, (struct Type*)extern_type, name->id);
  type_add_entry(&m_type_map, (struct Type*)extern_decl, extern_decl->id);
  if (extern_decl->type_params) {
    struct ListLink* li = list_first_link(extern_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_type_type_param(type_param);
      li = li->next;
    }
  }
  if (extern_decl->method_protos) {
    struct ListLink* li = list_first_link(extern_decl->method_protos);
    while (li) {
      struct Ast* proto = li->object;
      build_type_function_proto(proto);
      li = li->next;
    }
  }
}

internal void
build_type_package(struct Ast* ast)
{
  assert(ast->kind == AST_PACKAGE_DECL);
  struct Ast_PackageDecl* package_decl = (struct Ast_PackageDecl*)ast;
  if (package_decl->params) {
    struct ListLink* li = list_first_link(package_decl->params);
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      li = li->next;
    }
  }
}

internal void
build_type_transition_select_case(struct Ast* ast)
{
  assert(ast->kind == AST_SELECT_CASE);
  struct Ast_SelectCase* select_case = (struct Ast_SelectCase*)ast;
  build_type_select_keyset(select_case->keyset);
  build_type_expression(select_case->name);
}

internal void
build_type_parser_transition(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    build_type_expression(ast);
  } else if (ast->kind == AST_SELECT_EXPR) {
    struct Ast_SelectExpr* trans_stmt = (struct Ast_SelectExpr*)ast;
    struct ListLink* li = list_first_link(trans_stmt->expr_list);
    while (li) {
      struct Ast* expr = li->object;
      build_type_expression(expr);
      li = li->next;
    }
    li = list_first_link(trans_stmt->case_list);
    while (li) {
      struct Ast* select_case = li->object;
      build_type_transition_select_case(select_case);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
build_type_parser_state(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  if (state->stmt_list) {
    struct ListLink* li = list_first_link(state->stmt_list);
    while (li) {
      struct Ast* stmt = li->object;
      build_type_statement(stmt);
      li = li->next;
    }
  }
  build_type_parser_transition(state->trans_stmt);
}

void
build_type_const_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONST_DECL);
  struct Ast_ConstDecl* decl = (struct Ast_ConstDecl*)ast;
  build_type_type_ref(decl->type_ref);
  struct Type* decl_type = type_get_entry(&m_type_map, decl->type_ref->id);
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  type_add_entry(&m_type_map, decl_type, name->id);
  type_add_entry(&m_type_map, decl_type, decl->id);
  build_type_expression(decl->expr);
}

internal void
build_type_local_parser_element(struct Ast* ast)
{
  if (ast->kind == AST_CONST_DECL) {
    build_type_const_decl(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    build_type_instantiation(ast);
  } else if (ast->kind == AST_VAR_DECL) {
    build_type_statement(ast);
  } else assert(0);
}

internal void
build_type_parser_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_DECL);
  struct Ast_ParserDecl* parser_decl = (struct Ast_ParserDecl*)ast;
  struct Ast_ParserProto* type_decl = (struct Ast_ParserProto*)parser_decl->type_decl;
  if (type_decl->type_params) {
    struct ListLink* li = list_first_link(type_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_type_type_param(type_param);
      li = li->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* li = list_first_link(type_decl->params);
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      li = li->next;
    }
  }
  if (parser_decl->ctor_params) {
    struct ListLink* li = list_first_link(parser_decl->ctor_params);
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      li = li->next;
    }
  }
  if (parser_decl->local_elements) {
    struct ListLink* li = list_first_link(parser_decl->local_elements);
    while (li) {
      struct Ast* element = li->object;
      build_type_local_parser_element(element);
      li = li->next;
    }
  }
  if (parser_decl->states) {
    struct ListLink* li = list_first_link(parser_decl->states);
    while (li) {
      struct Ast* state = li->object;
      build_type_parser_state(state);
      li = li->next;
    }
  }
}

internal void
build_type_type_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TYPE_DECL);
  struct Ast_TypeDecl* type_decl = (struct Ast_TypeDecl*)ast;
  struct Ast* type_ref = type_decl->type_ref;
  build_type_type_ref(type_ref);
}

internal void
build_type_function_decl(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_DECL);
  struct Ast_FunctionDecl* function_decl = (struct Ast_FunctionDecl*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)function_decl->proto;
  if (function_proto->return_type) {
    build_type_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* li = list_first_link(function_proto->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_type_type_param(type_param);
      li = li->next;
    }
  }
  if (function_proto->params) {
    struct ListLink* li = list_first_link(function_proto->params);
    while (li) {
      struct Ast* param = li->object;
      build_type_param(param);
      li = li->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    if (function_body->stmt_list) {
      struct ListLink* li = list_first_link(function_body->stmt_list);
      while (li) {
        struct Ast* stmt = li->object;
        build_type_statement(stmt);
        li = li->next;
      }
    }
  }
}

internal void
build_type_enum_field(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
}

internal void
build_type_specified_id(struct Ast* ast)
{
  // pass
}

internal void
build_type_error_decl(struct Ast* ast)
{
  assert (ast->kind == AST_ERROR_DECL);
  struct Ast_ErrorDecl* decl = (struct Ast_ErrorDecl*)ast;
  if (decl->id_list) {
    struct ListLink* li = list_first_link(decl->id_list);
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_NAME) {
        build_type_enum_field(id);
      }
      else assert(0);
      li = li->next;
    }
  }
}

internal void
build_type_enum_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ENUM_DECL);
  struct Ast_EnumDecl* enum_decl = (struct Ast_EnumDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)enum_decl->name;
  struct Type_Name* enum_type = new_type(struct Type_Name, TYPE_NAME);
  enum_type->strname = name->strname;
  type_add_entry(&m_type_map, (struct Type*)enum_type, name->id);
  type_add_entry(&m_type_map, (struct Type*)enum_type, enum_decl->id);
}

internal void
build_type_expression(struct Ast* ast)
{
  if (ast->kind == AST_BINARY_EXPR) {
    struct Ast_BinaryExpr* expr = (struct Ast_BinaryExpr*)ast;
    build_type_expression(expr->left_operand);
    build_type_expression(expr->right_operand);
  } else if (ast->kind == AST_UNARY_EXPR) {
    struct Ast_UnaryExpr* expr = (struct Ast_UnaryExpr*)ast;
    build_type_expression(expr->operand);
  } else if (ast->kind == AST_NAME) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    struct NameRef* nameref = nameref_get_entry(m_nameref_map, name->id);
    struct SymtableEntry* se = scope_lookup_name(nameref->scope, nameref->strname);
    if (se->ns_type || se->ns_var) {
      if (se->ns_type && se->ns_var) {
        struct Type_Typevar* type = new_type(struct Type_Typevar, TYPE_TYPEVAR);
        type_add_entry(&m_type_map, (struct Type*)type, name->id);
      } else {
        struct NameDecl* decl = se->ns_type ? se->ns_type : se->ns_var;
        struct Type* type = type_get_entry(&m_type_map, decl->id);
        type_add_entry(&m_type_map, type, name->id);
      }
    } else error("at line %d: unknown name `%s`.", name->line_no, name->strname);
  } else if (ast->kind == AST_FUNCTION_CALL_EXPR) {
    build_type_function_call(ast);
  } else if (ast->kind == AST_MEMBER_SELECT_EXPR) {
    struct Ast_MemberSelectExpr* expr = (struct Ast_MemberSelectExpr*)ast;
    build_type_expression(expr->lhs_expr);
    struct Ast_Name* name = (struct Ast_Name*)expr->member_name;
    struct Type_Typevar* member_type = new_type(struct Type_Typevar, TYPE_TYPEVAR);
    type_add_entry(&m_type_map, (struct Type*)member_type, name->id);
    type_add_entry(&m_type_map, (struct Type*)member_type, expr->id);
  } else if (ast->kind == AST_EXPRLIST_EXPR) {
    struct Ast_ExprListExpr* expr = (struct Ast_ExprListExpr*)ast;
    if (expr->expr_list) {
      struct ListLink* li = list_first_link(expr->expr_list);
      while (li) {
        struct Ast* expr_expr = li->object;
        build_type_expression(expr_expr);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_CAST_EXPR) {
    struct Ast_CastExpr* expr = (struct Ast_CastExpr*)ast;
    build_type_type_ref(expr->to_type);
    build_type_expression(expr->expr);
  } else if (ast->kind == AST_INDEXED_ARRAY_EXPR) {
    struct Ast_IndexedArrayExpr* expr = (struct Ast_IndexedArrayExpr*)ast;
    build_type_expression(expr->index);
    if (expr->colon_index) {
      build_type_expression(expr->colon_index);
    }
  } else if (ast->kind == AST_KVPAIR_EXPR) {
    struct Ast_KeyValuePairExpr* expr = (struct Ast_KeyValuePairExpr*)ast;
    build_type_expression(expr->name);
    build_type_expression(expr->expr);
  } else if (ast->kind == AST_INT_LITERAL || ast->kind == AST_BOOL_LITERAL) {
    struct SymtableEntry* se = scope_lookup_name(get_root_scope(), "int");
    struct Type* int_type = type_get_entry(&m_type_map, se->ns_type->id);
    type_add_entry(&m_type_map, int_type, ast->id);
  } else if (ast->kind == AST_STRING_LITERAL) {
    struct SymtableEntry* se = scope_lookup_name(get_root_scope(), "string");
    struct Type* str_type = type_get_entry(&m_type_map, se->ns_type->id);
    type_add_entry(&m_type_map, str_type, ast->id);
  }
  else assert(0);
}

internal void
build_type_match_kind(struct Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND_DECL);
  struct Ast_MatchKindDecl* decl = (struct Ast_MatchKindDecl*)ast;
  if (decl->id_list) {
    struct ListLink* li = list_first_link(decl->id_list);
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_NAME) {
        build_type_enum_field(id);
      } else if (id->kind == AST_SPECIFIED_IDENT) {
        build_type_specified_id(id);
      }
      else assert(0);
      li = li->next;
    }
  }
}

internal void
build_type_p4program(struct Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  struct ListLink* li = list_first_link(program->decl_list);
  while (li) {
    struct Ast* decl = li->object;
    if (decl->kind == AST_CONTROL_DECL) {
      build_type_control_decl(decl);
    } else if (decl->kind == AST_EXTERN_DECL) {
      build_type_extern_decl(decl);
    } else if (decl->kind == AST_STRUCT_DECL) {
      build_type_struct_decl(decl);
    } else if (decl->kind == AST_HEADER_DECL) {
      build_type_header_decl(decl);
    } else if (decl->kind == AST_HEADER_UNION_DECL) {
      build_type_header_union_decl(decl);
    } else if (decl->kind == AST_PACKAGE_DECL) {
      build_type_package(decl);
    } else if (decl->kind == AST_PARSER_DECL) {
      build_type_parser_decl(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      build_type_instantiation(decl);
    } else if (decl->kind == AST_TYPE_DECL) {
      build_type_type_decl(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      build_type_function_proto(decl);
    } else if (decl->kind == AST_CONST_DECL) {
      build_type_const_decl(decl);
    } else if (decl->kind == AST_FUNCTION_DECL) {
      build_type_function_decl(decl);
    } else if (decl->kind == AST_ACTION_DECL) {
      build_type_action_decl(decl);
    } else if (decl->kind == AST_ENUM_DECL) {
      build_type_enum_decl(decl);
    } else if (decl->kind == AST_MATCH_KIND_DECL) {
      build_type_match_kind(decl);
    } else if (decl->kind == AST_ERROR_DECL) {
      build_type_error_decl(decl);
    }
    else assert(0);
    li = li->next;
  }
}

struct Hashmap*
build_type(struct Ast* p4program, struct Hashmap* nameref_map, struct Arena* type_storage)
{
  void add_basic_type(char* strname, enum BasicType basic_ty) {
    struct Type_Basic* type = new_type(struct Type_Basic, TYPE_BASIC);
    type->basic_ty = basic_ty;
    type->strname = strname;
    struct SymtableEntry* se = scope_lookup_name(get_root_scope(), strname);
    type_add_entry(&m_type_map, (struct Type*)type, se->ns_type->id);
  }

  m_type_storage = type_storage;
  m_nameref_map = nameref_map;
  hashmap_init(&m_type_map, HASHMAP_KEY_INT, 8, m_type_storage);

  add_basic_type("void", TYPE_INT);
  add_basic_type("bool", TYPE_INT);
  add_basic_type("int", TYPE_INT);
  add_basic_type("bit", TYPE_INT);
  add_basic_type("varbit", TYPE_INT);
  add_basic_type("string", TYPE_STRING);

  build_type_p4program(p4program);
  return &m_type_map;
}
