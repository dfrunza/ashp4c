#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "ast.h"

internal struct Scope* root_scope;
internal struct Arena *type_storage;
internal struct Hashmap* nameref_map;
internal struct Hashmap type_map = {};

internal void visit_block_statement(struct Ast* block_stmt);
internal void visit_statement(struct Ast* decl);
internal void visit_expression(struct Ast* expr);
internal void visit_type_ref(struct Ast* type_ref);

struct Type*
type_get(struct Hashmap* map, uint32_t id)
{
  struct HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_INT, &key, map->capacity_log2);
  struct HashmapEntry* he = hashmap_get_entry(map, &key);
  struct Type* type = 0;
  if (he) {
    type = he->object;
  }
  return type;
}

void
type_add(struct Hashmap* map, struct Type* type, uint32_t id)
{
  struct HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_INT, &key, map->capacity_log2);
  struct HashmapEntry* he = hashmap_get_or_create_entry(map, &key);
  assert(!he->object);
  he->object = type;
}

internal void
visit_param(struct Ast* ast)
{
  assert(ast->kind == AST_PARAM);
  struct Ast_Param* param = (struct Ast_Param*)ast;
  visit_type_ref(param->type);
  struct Type* param_type = type_get(&type_map, param->type->id);
  type_add(&type_map, param_type, param->id);
}

internal void
visit_type_param(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* name = (struct Ast_Name*)ast;
  struct NameRef* ref = nameref_get(nameref_map, name->id);
  if (!ref) {
    struct Type_TypeParam* type = arena_push_struct(type_storage, struct Type_TypeParam);
    type->ctor = TYPE_TYPEPARAM;
    type->strname = name->strname;
    type_add(&type_map, (struct Type*)type, ast->id);
  } else {
    visit_expression(ast);
  }
}

internal void
visit_struct_field(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  struct Ast_StructField* field = (struct Ast_StructField*)ast;
  visit_type_ref(field->type);
  struct Type* field_type = type_get(&type_map, field->type->id);
  type_add(&type_map, field_type, field->id);
}

internal void
visit_header_union_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION);
  struct Ast_HeaderUnion* header_union_decl = (struct Ast_HeaderUnion*)ast;
  if (header_union_decl->fields) {
    struct ListLink* li = list_first_link(header_union_decl->fields);
    while (li) {
      struct Ast* field = li->object;
      visit_struct_field(field);
      li = li->next;
    }
  }
}

internal void
visit_header_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER);
  struct Ast_Header* header_decl = (struct Ast_Header*)ast;
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* struct_type = type_get(&type_map, void_decl->ast->id);
  if (header_decl->fields) {
    struct ListLink* li = list_first_link(header_decl->fields);
    struct Ast* field = li->object;
    visit_struct_field(field);
    struct_type = type_get(&type_map, field->id);
    li = li->next;
    while (li) {
      struct Ast* field = li->object;
      visit_struct_field(field);
      struct Type_Product* product_type = arena_push_struct(type_storage, struct Type_Product);
      product_type->ctor = TYPE_PRODUCT;
      product_type->lhs_ty = struct_type;
      product_type->rhs_ty = type_get(&type_map, field->id);
      struct_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  type_add(&type_map, struct_type, header_decl->id);
}

internal void
visit_struct_decl(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT);
  struct Ast_Struct* struct_decl = (struct Ast_Struct*)ast;
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* struct_type = type_get(&type_map, void_decl->ast->id);
  if (struct_decl->fields) {
    struct ListLink* li = list_first_link(struct_decl->fields);
    struct Ast* field = li->object;
    visit_struct_field(field);
    struct_type = type_get(&type_map, field->id);
    li = li->next;
    while (li) {
      struct Ast* field = li->object;
      visit_struct_field(field);
      struct Type_Product* product_type = arena_push_struct(type_storage, struct Type_Product);
      product_type->ctor = TYPE_PRODUCT;
      product_type->lhs_ty = struct_type;
      product_type->rhs_ty = type_get(&type_map, field->id);
      struct_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  type_add(&type_map, struct_type, struct_decl->id);
}

internal void
visit_type_ref(struct Ast* ast)
{
  if (ast->kind == AST_BOOL_TYPE || ast->kind == AST_ERROR_TYPE
      || ast->kind == AST_INT_TYPE || ast->kind == AST_BIT_TYPE
      || ast->kind == AST_VARBIT_TYPE || ast->kind == AST_STRING_TYPE
      || ast->kind == AST_VOID_TYPE) {
    struct Ast_BasicType* base_type = (struct Ast_BasicType*)ast;
    struct Ast_Name* name = (struct Ast_Name*)base_type->name;
    struct NameEntry* ne = scope_lookup_name(root_scope, name->strname);
    struct NameDecl* decl = ne->ns_type;
    struct Type* type = type_get(&type_map, decl->ast->id);
    if (ast->kind == AST_INT_TYPE) {
      struct Ast_IntType* int_type = (struct Ast_IntType*)ast;
      if (int_type->size) {
        struct NameEntry* ne = scope_lookup_name(root_scope, "int");
        struct NameDecl* decl = ne->ns_type;
        type->type_params = type_get(&type_map, decl->ast->id);
      }
    } else if (ast->kind == AST_BIT_TYPE) {
      struct Ast_BitType* bit_type = (struct Ast_BitType*)ast;
      if (bit_type->size) {
        struct NameEntry* ne = scope_lookup_name(root_scope, "int");
        struct NameDecl* decl = ne->ns_type;
        type->type_params = type_get(&type_map, decl->ast->id);
      }
    } else if (ast->kind == AST_VARBIT_TYPE) {
      struct Ast_VarbitType* varbit_type = (struct Ast_VarbitType*)ast;
      if (varbit_type->size) {
        struct NameEntry* ne = scope_lookup_name(root_scope, "int");
        struct NameDecl* decl = ne->ns_type;
        type->type_params = type_get(&type_map, decl->ast->id);
      }
    }
    type_add(&type_map, type, base_type->id);
  } else if (ast->kind == AST_HEADER_STACK) {
    struct Ast_HeaderStack* type_ref = (struct Ast_HeaderStack*)ast;
    visit_expression(type_ref->stack_expr);
  } else if (ast->kind == AST_NAME) {
    visit_expression(ast);
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    struct Ast_SpecializedType* speclzd_type = (struct Ast_SpecializedType*)ast;
    struct ListLink* li = list_first_link(speclzd_type->type_args);
    while (li) {
      struct Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  } else if (ast->kind == AST_TUPLE) {
    struct Ast_Tuple* type_ref = (struct Ast_Tuple*)ast;
    if (type_ref->type_args) {
      struct ListLink* li = list_first_link(type_ref->type_args);
      while (li) {
        struct Ast* type_arg = li->object;
        visit_type_ref(type_arg);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_STRUCT) {
    visit_struct_decl(ast);
  } else if (ast->kind == AST_HEADER) {
    visit_header_decl(ast);
  } else if (ast->kind == AST_HEADER_UNION) {
    visit_header_union_decl(ast);
  } else if (ast->kind == AST_DONTCARE) {
    ; // pass
  }
  else assert(0);
}

internal void
visit_function_call(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_CALL);
  struct Ast_FunctionCall* function_call = (struct Ast_FunctionCall*)ast;
  visit_expression(function_call->callee_expr);
  struct Ast_Expression* callee_expr = (struct Ast_Expression*)(function_call->callee_expr);
  if (callee_expr->type_args) {
    struct ListLink* li = list_first_link(callee_expr->type_args);
    while (li) {
      struct Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  }
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* args_type = type_get(&type_map, void_decl->ast->id);
  if (function_call->args) {
    struct ListLink* li = list_first_link(function_call->args);
    struct Ast* arg = li->object;
    visit_expression(arg);
    args_type = type_get(&type_map, arg->id);
    li = li->next;
    while (li) {
      struct Ast* arg = li->object;
      visit_expression(arg);
      struct Type_Product* product_type = arena_push_struct(type_storage, struct Type_Product);
      product_type->ctor = TYPE_PRODUCT;
      product_type->lhs_ty = args_type;
      product_type->rhs_ty = type_get(&type_map, arg->id);
      args_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  struct Type_FunctionCall* call_type = arena_push_struct(type_storage, struct Type_FunctionCall);
  call_type->ctor = TYPE_FUNCTION_CALL;
  call_type->args_ty = args_type;
  type_add(&type_map, (struct Type*)call_type, function_call->id);
}

internal void
visit_instantiation(struct Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  struct Ast_Instantiation* inst_decl = (struct Ast_Instantiation*)ast;
  visit_type_ref(inst_decl->type_ref);
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* args_type = type_get(&type_map, void_decl->ast->id);
  if (inst_decl->args) {
    struct ListLink* li = list_first_link(inst_decl->args);
    struct Ast* arg = li->object;
    visit_expression(arg);
    args_type = type_get(&type_map, arg->id);
    li = li->next;
    while (li) {
      struct Ast* arg = li->object;
      visit_expression(arg);
      struct Type_Product* product_type = arena_push_struct(type_storage, struct Type_Product);
      product_type->ctor = TYPE_PRODUCT;
      product_type->lhs_ty = args_type;
      product_type->rhs_ty = type_get(&type_map, arg->id);
      args_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  struct Type_FunctionCall* inst_type = arena_push_struct(type_storage, struct Type_FunctionCall);
  inst_type->ctor = TYPE_FUNCTION_CALL;
  inst_type->args_ty = args_type;
  type_add(&type_map, (struct Type*)inst_type, inst_decl->id);
}

internal void
visit_switch_label(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    ; // pass
  } else {
    visit_expression(ast);
  }
}

internal void
visit_switch_case(struct Ast* ast)
{
  assert(ast->kind == AST_SWITCH_CASE);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  visit_switch_label(switch_case->label);
  struct Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_BLOCK_STMT) {
    visit_block_statement(case_stmt);
  }
}

internal void
visit_keyset_expr(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT || ast->kind == AST_DONTCARE) {
    ; // pass
  } else {
    visit_expression(ast);
  }
}

internal void
visit_select_keyset(struct Ast* ast)
{
  if (ast->kind == AST_TUPLE_KEYSET) {
    struct Ast_TupleKeyset* keyset = (struct Ast_TupleKeyset*)ast;
    struct ListLink* li = list_first_link(keyset->expr_list);
    while (li) {
      struct Ast* expr = li->object;
      visit_keyset_expr(expr);
      li = li->next;
    }
  } else {
    visit_keyset_expr(ast);
  }
}

internal void
visit_action_ref(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_REF);
  struct Ast_ActionRef* action = (struct Ast_ActionRef*)ast;
  visit_expression(action->name);
  if (action->args) {
    struct ListLink* li = list_first_link(action->args);
    while (li) {
      struct Ast* arg = li->object;
      visit_expression(arg);
      li = li->next;
    }
  }
}

internal void
visit_table_keyelem(struct Ast* ast)
{
  assert(ast->kind == AST_KEY_ELEMENT);
  struct Ast_KeyElement* keyelem = (struct Ast_KeyElement*)ast;
  visit_expression(keyelem->name);
  visit_expression(keyelem->expr);
}

internal void
visit_table_entry(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRY);
  struct Ast_TableEntry* entry = (struct Ast_TableEntry*)ast;
  visit_select_keyset(entry->keyset);
  visit_action_ref(entry->action);
}

internal void
visit_table_property(struct Ast* ast)
{
  if (ast->kind == AST_TABLE_ACTIONS) {
    struct Ast_TableActions* prop = (struct Ast_TableActions*)ast;
    if (prop->action_list) {
      struct ListLink* li = list_first_link(prop->action_list);
      while (li) {
        struct Ast* action = li->object;
        visit_action_ref(action);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    struct Ast_TableSingleEntry* prop = (struct Ast_TableSingleEntry*)ast;
    if (prop->init_expr) {
      visit_expression(prop->init_expr);
    }
  } else if (ast->kind == AST_TABLE_KEY) {
    struct Ast_TableKey* prop = (struct Ast_TableKey*)ast;
    struct ListLink* li = list_first_link(prop->keyelem_list);
    while (li) {
      struct Ast* keyelem = li->object;
      visit_table_keyelem(keyelem);
      li = li->next;
    }
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    struct Ast_TableEntries* prop = (struct Ast_TableEntries*)ast;
    struct ListLink* li = list_first_link(prop->entries);
    while (li) {
      struct Ast* entry = li->object;
      visit_table_entry(entry);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
visit_table_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE);
  struct Ast_Table* decl = (struct Ast_Table*)ast;
  if (decl->prop_list) {
    struct ListLink* li = list_first_link(decl->prop_list);
    while (li) {
      struct Ast* prop = li->object;
      visit_table_property(prop);
      li = li->next;
    }
  }
}

internal void
visit_action_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION);
  struct Ast_Action* action_decl = (struct Ast_Action*)ast;
  struct List* params = action_decl->params;
  if (params) {
    struct ListLink* li = list_first_link(params);
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
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
        visit_statement(stmt);
        li = li->next;
      }
    }
  }
}

internal void
visit_var_decl(struct Ast* ast)
{
  assert(ast->kind == AST_VAR);
  struct Ast_Var* decl = (struct Ast_Var*)ast;
  visit_type_ref(decl->type);
  if (decl->init_expr) {
    visit_expression(decl->init_expr);
  }
  struct Type* decl_type = type_get(&type_map, decl->type->id);
  type_add(&type_map, decl_type, decl->id);
}

internal void
visit_if_stmt(struct Ast* ast)
{
  assert(ast->kind == AST_IF_STMT);
  struct Ast_IfStmt* stmt = (struct Ast_IfStmt*)ast;
  struct Ast* if_stmt = stmt->stmt;
  visit_expression(stmt->cond_expr);
  visit_statement(if_stmt);
  struct Ast* else_stmt = stmt->else_stmt;
  if (else_stmt) {
    visit_statement(else_stmt);
  }
}

internal void
visit_switch_stmt(struct Ast* ast)
{
  assert(ast->kind == AST_SWITCH_STMT);
  struct Ast_SwitchStmt* stmt = (struct Ast_SwitchStmt*)ast;
  visit_expression(stmt->expr);
  if (stmt->switch_cases) {
    struct ListLink* li = list_first_link(stmt->switch_cases);
    while (li) {
      struct Ast* switch_case = li->object;
      visit_switch_case(switch_case);
      li = li->next;
    }
  }
}

internal void
visit_assignment_stmt(struct Ast* ast)
{
  assert(ast->kind == AST_ASSIGNMENT_STMT);
  struct Ast_AssignmentStmt* stmt = (struct Ast_AssignmentStmt*)ast;
  visit_expression(stmt->lvalue);
  visit_expression(stmt->expr);
  struct Type_Product* args_type = arena_push_struct(type_storage, struct Type_Product);
  args_type->ctor = TYPE_PRODUCT;
  args_type->lhs_ty = type_get(&type_map, stmt->lvalue->id);
  args_type->rhs_ty = type_get(&type_map, stmt->expr->id);
  struct Type_FunctionCall* stmt_type = arena_push_struct(type_storage, struct Type_FunctionCall);
  stmt_type->ctor = TYPE_FUNCTION_CALL;
  stmt_type->args_ty = (struct Type*)args_type;
  type_add(&type_map, (struct Type*)stmt_type, stmt->id);
}

internal void
visit_return_stmt(struct Ast* ast)
{
  assert(ast->kind == AST_RETURN_STMT);
  struct Ast_ReturnStmt* stmt = (struct Ast_ReturnStmt*)ast;
  if (stmt->expr) {
    visit_expression(stmt->expr);
  }
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* return_type = type_get(&type_map, void_decl->ast->id);
  if (stmt->expr) {
    return_type = type_get(&type_map, stmt->expr->id);
  }
  type_add(&type_map, return_type, stmt->id);
}

internal void
visit_statement(struct Ast* ast)
{
  if (ast->kind == AST_VAR) {
    visit_var_decl(ast);
  } else if (ast->kind == AST_ACTION) {
    visit_action_decl(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    visit_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_TABLE) {
    visit_table_decl(ast);
  } else if (ast->kind == AST_IF_STMT) {
    visit_if_stmt(ast);
  } else if (ast->kind == AST_SWITCH_STMT) {
    visit_switch_stmt(ast);
  } else if (ast->kind == AST_ASSIGNMENT_STMT) {
    visit_assignment_stmt(ast);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_RETURN_STMT) {
    visit_return_stmt(ast);
  } else if (ast->kind == AST_EXIT_STMT || ast->kind == AST_EMPTY_STMT) {
    ; // pass
  }
  else assert(0);
}

internal void
visit_function_return_type(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    struct Ast_Name* return_type = (struct Ast_Name*)ast;
    visit_type_param((struct Ast*)return_type);
  } else {
    visit_type_ref(ast);
  }
}

internal void
visit_function_proto(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)ast;
  if (function_proto->return_type) {
    visit_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* li = list_first_link(function_proto->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* params_type = type_get(&type_map, void_decl->ast->id);
  if (function_proto->params) {
    struct ListLink* li = list_first_link(function_proto->params);
    struct Ast* param = li->object;
    visit_param(param);
    params_type = type_get(&type_map, param->id);
    li = li->next;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      struct Type_Product* product_type = arena_push_struct(type_storage, struct Type_Product); 
      product_type->ctor = TYPE_PRODUCT;
      product_type->lhs_ty = params_type;
      product_type->rhs_ty = type_get(&type_map, param->id);
      params_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  struct Type_Function* function_type = arena_push_struct(type_storage, struct Type_Function);
  function_type->ctor = TYPE_FUNCTION;
  function_type->params_ty = params_type;
  if (function_proto->return_type) {
    function_type->return_ty = type_get(&type_map, function_proto->return_type->id);
  }
  type_add(&type_map, (struct Type*)function_type, function_proto->id);
}

internal void
visit_block_statement(struct Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  struct Ast_BlockStmt* block_stmt = (struct Ast_BlockStmt*)ast;
  if (block_stmt->stmt_list) {
    struct ListLink* li = list_first_link(block_stmt->stmt_list);
    while (li) {
      struct Ast* decl = li->object;
      visit_statement(decl);
      li = li->next;
    }
  }
}

internal void
visit_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL);
  struct Ast_Control* control_decl = (struct Ast_Control*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  if (type_decl->type_params) {
    struct ListLink* li = list_first_link(type_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* params_type = type_get(&type_map, void_decl->ast->id);
  if (type_decl->params) {
    struct ListLink* li = list_first_link(type_decl->params);
    struct Ast* param = li->object;
    visit_param(param);
    params_type = type_get(&type_map, param->id);
    li = li->next;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      struct Type_Product* product_type = arena_push_struct(type_storage, struct Type_Product); 
      product_type->ctor = TYPE_PRODUCT;
      product_type->lhs_ty = params_type;
      product_type->rhs_ty = type_get(&type_map, param->id);
      params_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* li = list_first_link(control_decl->ctor_params);
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  struct Type_Function* function_type = arena_push_struct(type_storage, struct Type_Function);
  function_type->ctor = TYPE_FUNCTION;
  function_type->params_ty = params_type;
  function_type->return_ty = type_get(&type_map, void_decl->ast->id);
  type_add(&type_map, (struct Type*)function_type, control_decl->id);
  if (control_decl->local_decls) {
    struct ListLink* li = list_first_link(control_decl->local_decls);
    while (li) {
      struct Ast* decl = li->object;
      visit_statement(decl);
      li = li->next;
    }
  }
  if (control_decl->apply_stmt) {
    visit_block_statement(control_decl->apply_stmt);
  }
}

internal void
visit_extern_decl(struct Ast* ast)
{
  assert(ast->kind == AST_EXTERN);
  struct Ast_Extern* extern_decl = (struct Ast_Extern*)ast;
  struct Ast_Name* name = (struct Ast_Name*)extern_decl->name;
  struct Type_Name* extern_type = arena_push_struct(type_storage, struct Type_Name);
  extern_type->ctor = TYPE_NAME;
  extern_type->strname = name->strname;
  type_add(&type_map, (struct Type*)extern_decl, extern_decl->id);
  if (extern_decl->type_params) {
    struct ListLink* li = list_first_link(extern_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  if (extern_decl->method_protos) {
    struct ListLink* li = list_first_link(extern_decl->method_protos);
    while (li) {
      struct Ast* proto = li->object;
      visit_function_proto(proto);
      li = li->next;
    }
  }
}

internal void
visit_package_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PACKAGE);
  struct Ast_Package* package_decl = (struct Ast_Package*)ast;
  struct Ast_Name* name = (struct Ast_Name*)package_decl->name;
  struct Type_Name* package_type = arena_push_struct(type_storage, struct Type_Name);
  package_type->ctor = TYPE_NAME;
  package_type->strname = name->strname;
  type_add(&type_map, (struct Type*)package_type, package_decl->id);
  if (package_decl->params) {
    struct ListLink* li = list_first_link(package_decl->params);
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
}

internal void
visit_transition_select_case(struct Ast* ast)
{
  assert(ast->kind == AST_SELECT_CASE);
  struct Ast_SelectCase* select_case = (struct Ast_SelectCase*)ast;
  visit_select_keyset(select_case->keyset);
}

internal void
visit_parser_transition(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    visit_expression(ast);
  } else if (ast->kind == AST_SELECT_EXPR) {
    struct Ast_SelectExpr* trans_stmt = (struct Ast_SelectExpr*)ast;
    struct ListLink* li = list_first_link(trans_stmt->expr_list);
    while (li) {
      struct Ast* expr = li->object;
      visit_expression(expr);
      li = li->next;
    }
    li = list_first_link(trans_stmt->case_list);
    while (li) {
      struct Ast* select_case = li->object;
      visit_transition_select_case(select_case);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
visit_parser_state(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  if (state->stmt_list) {
    struct ListLink* li = list_first_link(state->stmt_list);
    while (li) {
      struct Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
  visit_parser_transition(state->trans_stmt);
}

internal void
visit_const_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONST);
  struct Ast_Const* decl = (struct Ast_Const*)ast;
  visit_type_ref(decl->type_ref);
  struct Type* decl_type = type_get(&type_map, decl->type_ref->id);
  type_add(&type_map, decl_type, decl->id);
  visit_expression(decl->expr);
}

internal void
visit_local_parser_element(struct Ast* ast)
{
  if (ast->kind == AST_CONST) {
    visit_const_decl(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_VAR) {
    visit_statement(ast);
  } else assert(0);
}

internal void
visit_parser_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER);
  struct Ast_Parser* parser_decl = (struct Ast_Parser*)ast;
  struct Ast_ParserProto* type_decl = (struct Ast_ParserProto*)parser_decl->type_decl;
  if (type_decl->type_params) {
    struct ListLink* li = list_first_link(type_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* params_type = type_get(&type_map, void_decl->ast->id);
  if (type_decl->params) {
    struct ListLink* li = list_first_link(type_decl->params);
    struct Ast* param = li->object;
    visit_param(param);
    params_type = type_get(&type_map, param->id);
    li = li->next;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      struct Type_Product* product_type = arena_push_struct(type_storage, struct Type_Product); 
      product_type->ctor = TYPE_PRODUCT; 
      product_type->lhs_ty = params_type;
      product_type->rhs_ty = type_get(&type_map, param->id);
      params_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  if (parser_decl->ctor_params) {
    struct ListLink* li = list_first_link(parser_decl->ctor_params);
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  struct Type_Function* function_type = arena_push_struct(type_storage, struct Type_Function);
  function_type->ctor = TYPE_FUNCTION;
  function_type->params_ty = params_type;
  function_type->return_ty = type_get(&type_map, void_decl->ast->id);
  type_add(&type_map, (struct Type*)function_type, parser_decl->id);
  if (parser_decl->local_elements) {
    struct ListLink* li = list_first_link(parser_decl->local_elements);
    while (li) {
      struct Ast* element = li->object;
      visit_local_parser_element(element);
      li = li->next;
    }
  }
  if (parser_decl->states) {
    struct ListLink* li = list_first_link(parser_decl->states);
    while (li) {
      struct Ast* state = li->object;
      visit_parser_state(state);
      li = li->next;
    }
  }
}

internal void
visit_type_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TYPE);
  struct Ast_Type* type_decl = (struct Ast_Type*)ast;
  visit_type_ref(type_decl->type_ref);
}

internal void
visit_function_decl(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION);
  struct Ast_Function* function_decl = (struct Ast_Function*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)function_decl->proto;
  if (function_proto->return_type) {
    visit_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* li = list_first_link(function_proto->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  struct NameEntry* ne = scope_lookup_name(root_scope, "void");
  struct NameDecl* void_decl = ne->ns_type;
  struct Type* params_type = type_get(&type_map, void_decl->ast->id);
  if (function_proto->params) {
    struct ListLink* li = list_first_link(function_proto->params);
    struct Ast* param = li->object;
    visit_param(param);
    params_type = type_get(&type_map, param->id);
    li = li->next;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      struct Type_Product* product_type = arena_push_struct(type_storage, struct Type_Product); 
      product_type->ctor = TYPE_PRODUCT;
      product_type->lhs_ty = params_type;
      product_type->rhs_ty = type_get(&type_map, param->id);
      params_type = (struct Type*)product_type;
      li = li->next;
    }
  }
  struct Type_Function* function_type = arena_push_struct(type_storage, struct Type_Function);
  function_type->ctor = TYPE_FUNCTION;
  function_type->params_ty = params_type;
  function_type->return_ty = type_get(&type_map, void_decl->ast->id);
  if (function_proto->return_type) {
    function_type->return_ty = type_get(&type_map, function_proto->return_type->id);
  }
  type_add(&type_map, (struct Type*)function_type, function_proto->id);
  type_add(&type_map, (struct Type*)function_type, function_decl->id);
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    if (function_body->stmt_list) {
      struct ListLink* li = list_first_link(function_body->stmt_list);
      while (li) {
        struct Ast* stmt = li->object;
        visit_statement(stmt);
        li = li->next;
      }
    }
  }
}

internal void
visit_enum_field(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
}

internal void
visit_specified_id(struct Ast* ast)
{
  // pass
}

internal void
visit_error_decl(struct Ast* ast)
{
  assert (ast->kind == AST_ERROR);
  struct Ast_Error* decl = (struct Ast_Error*)ast;
  if (decl->id_list) {
    struct ListLink* li = list_first_link(decl->id_list);
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_NAME) {
        visit_enum_field(id);
      }
      else assert(0);
      li = li->next;
    }
  }
}

internal void
visit_enum_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ENUM);
  struct Ast_Enum* enum_decl = (struct Ast_Enum*)ast;
  struct Ast_Name* name = (struct Ast_Name*)enum_decl->name;
  struct Type_Name* enum_type = arena_push_struct(type_storage, struct Type_Name);
  enum_type->ctor = TYPE_NAME;
  enum_type->strname = name->strname;
  type_add(&type_map, (struct Type*)enum_type, enum_decl->id);
}

internal void
visit_binary_expr(struct Ast* ast)
{
  assert(ast->kind == AST_BINARY_EXPR);
  struct Ast_BinaryExpr* expr = (struct Ast_BinaryExpr*)ast;
  visit_expression(expr->left_operand);
  visit_expression(expr->right_operand);
  struct Type_Product* args_type = arena_push_struct(type_storage, struct Type_Product);
  args_type->ctor = TYPE_PRODUCT;
  args_type->lhs_ty = type_get(&type_map, expr->left_operand->id);
  args_type->rhs_ty = type_get(&type_map, expr->right_operand->id);
  struct Type_FunctionCall* expr_type = arena_push_struct(type_storage, struct Type_FunctionCall);
  expr_type->ctor = TYPE_FUNCTION_CALL;
  expr_type->args_ty = (struct Type*)args_type;
  type_add(&type_map, (struct Type*)expr_type, expr->id);
}

internal void
visit_unary_expr(struct Ast* ast)
{
  assert(ast->kind == AST_UNARY_EXPR);
  struct Ast_UnaryExpr* expr = (struct Ast_UnaryExpr*)ast;
  visit_expression(expr->operand);
  struct Type* args_type = type_get(&type_map, expr->operand->id);
  struct Type_FunctionCall* expr_type = arena_push_struct(type_storage, struct Type_FunctionCall);
  expr_type->ctor = TYPE_FUNCTION_CALL;
  expr_type->args_ty = (struct Type*)args_type;
  type_add(&type_map, (struct Type*)expr_type, expr->id);
}

internal void
visit_member_select(struct Ast* ast)
{
  assert(ast->kind == AST_MEMBER_SELECT);
  struct Ast_MemberSelect* expr = (struct Ast_MemberSelect*)ast;
  visit_expression(expr->lhs_expr);
  struct Type_Typevar* member_type = arena_push_struct(type_storage, struct Type_Typevar);
  member_type->ctor = TYPE_TYPEVAR;
  type_add(&type_map, (struct Type*)member_type, expr->id);
}

internal void
visit_expression(struct Ast* ast)
{
  if (ast->kind == AST_BINARY_EXPR) {
    visit_binary_expr(ast);
  } else if (ast->kind == AST_UNARY_EXPR) {
    visit_unary_expr(ast);
  } else if (ast->kind == AST_NAME) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    struct Type_Typevar* type = arena_push_struct(type_storage, struct Type_Typevar);
    type->ctor = TYPE_TYPEVAR;
    type_add(&type_map, (struct Type*)type, name->id);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_MEMBER_SELECT) {
    visit_member_select(ast);
  } else if (ast->kind == AST_EXPRLIST) {
    struct Ast_ExprList* expr = (struct Ast_ExprList*)ast;
    if (expr->expr_list) {
      struct ListLink* li = list_first_link(expr->expr_list);
      while (li) {
        struct Ast* expr_expr = li->object;
        visit_expression(expr_expr);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_CAST_EXPR) {
    struct Ast_CastExpr* expr = (struct Ast_CastExpr*)ast;
    visit_type_ref(expr->to_type);
    visit_expression(expr->expr);
  } else if (ast->kind == AST_SUBSCRIPT) {
    struct Ast_Subscript* expr = (struct Ast_Subscript*)ast;
    visit_expression(expr->index);
    if (expr->colon_index) {
      visit_expression(expr->colon_index);
    }
  } else if (ast->kind == AST_KVPAIR) {
    struct Ast_KVPair* expr = (struct Ast_KVPair*)ast;
    visit_expression(expr->expr);
  } else if (ast->kind == AST_INT_LITERAL || ast->kind == AST_BOOL_LITERAL) {
    struct NameEntry* ne = scope_lookup_name(root_scope, "int");
    struct NameDecl* decl = ne->ns_type;
    struct Type* int_type = type_get(&type_map, decl->ast->id);
    type_add(&type_map, int_type, ast->id);
  } else if (ast->kind == AST_STRING_LITERAL) {
    struct NameEntry* ne = scope_lookup_name(root_scope, "string");
    struct NameDecl* decl = ne->ns_type;
    struct Type* string_type = type_get(&type_map, decl->ast->id);
    type_add(&type_map, string_type, ast->id);
  }
  else assert(0);
}

internal void
visit_match_kind(struct Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND);
  struct Ast_MatchKind* decl = (struct Ast_MatchKind*)ast;
  if (decl->id_list) {
    struct ListLink* li = list_first_link(decl->id_list);
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_NAME) {
        visit_enum_field(id);
      } else if (id->kind == AST_SPECIFIED_IDENT) {
        visit_specified_id(id);
      }
      else assert(0);
      li = li->next;
    }
  }
}

internal void
visit_p4program(struct Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  struct ListLink* li = list_first_link(program->decl_list);
  while (li) {
    struct Ast* decl = li->object;
    if (decl->kind == AST_CONTROL) {
      visit_control_decl(decl);
    } else if (decl->kind == AST_EXTERN) {
      visit_extern_decl(decl);
    } else if (decl->kind == AST_STRUCT) {
      visit_struct_decl(decl);
    } else if (decl->kind == AST_HEADER) {
      visit_header_decl(decl);
    } else if (decl->kind == AST_HEADER_UNION) {
      visit_header_union_decl(decl);
    } else if (decl->kind == AST_PACKAGE) {
      visit_package_decl(decl);
    } else if (decl->kind == AST_PARSER) {
      visit_parser_decl(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      visit_instantiation(decl);
    } else if (decl->kind == AST_TYPE) {
      visit_type_decl(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      visit_function_proto(decl);
    } else if (decl->kind == AST_CONST) {
      visit_const_decl(decl);
    } else if (decl->kind == AST_FUNCTION) {
      visit_function_decl(decl);
    } else if (decl->kind == AST_ACTION) {
      visit_action_decl(decl);
    } else if (decl->kind == AST_ENUM) {
      visit_enum_decl(decl);
    } else if (decl->kind == AST_MATCH_KIND) {
      visit_match_kind(decl);
    } else if (decl->kind == AST_ERROR) {
      visit_error_decl(decl);
    }
    else assert(0);
    li = li->next;
  }
}

struct Hashmap*
build_type(struct Ast_P4Program* p4program, struct Scope* root_scope_,
           struct Hashmap* nameref_map_, struct Arena* type_storage_)
{
  struct Type_Basic*
  add_basic_type(char* strname, enum BasicType basic_ty)
  {
    struct Type_Basic* type = arena_push_struct(type_storage, struct Type_Basic);
    type->ctor = TYPE_BASIC;
    type->basic_ty = basic_ty;
    type->strname = strname;
    struct NameEntry* ne = scope_lookup_name(root_scope, strname);
    struct NameDecl* decl = ne->ns_type;
    type_add(&type_map, (struct Type*)type, decl->ast->id);
    return type;
  }

  root_scope = root_scope_;
  nameref_map = nameref_map_;
  type_storage = type_storage_;
  hashmap_init(&type_map, HASHMAP_KEY_INT, 8, type_storage);

  add_basic_type("void", TYPE_VOID);
  add_basic_type("bool", TYPE_INT);
  add_basic_type("int", TYPE_INT);
  add_basic_type("bit", TYPE_INT);
  add_basic_type("varbit", TYPE_INT);
  add_basic_type("string", TYPE_STRING);

  visit_p4program((struct Ast*)p4program);
  return &type_map;
}