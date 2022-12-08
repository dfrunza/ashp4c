#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "ast.h"

internal Scope* root_scope;
internal Arena *type_storage;
internal Hashmap* nameref_map;
internal Hashmap type_map = {};

internal void visit_block_statement(Ast* block_stmt);
internal void visit_statement(Ast* decl);
internal void visit_expression(Ast* expr);
internal void visit_type_ref(Ast* type_ref);

Type*
typeset_create(Hashmap* map, uint32_t id)
{
  HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, map->capacity_log2);
  HashmapEntry* he = hashmap_get_or_create_entry(map, &key);
  Type* ty_set = he->object;
  if (!ty_set) {
    ty_set = arena_push_struct(type_storage, Type);
    he->object = ty_set;
  }
  assert((!ty_set->members.next) && (ty_set->members.count == 0));
  return ty_set;
}

Type*
typeset_get(Hashmap* map, uint32_t id)
{
  HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, map->capacity_log2);
  HashmapEntry* he = hashmap_get_or_create_entry(map, &key);
  Type* ty_set = he->object;
  return ty_set;
}

Type*
typeset_add_type(Type* ty_set, Type* type)
{
  DList* li = arena_push_struct(type_storage, DList);
  li->object = type;
  dlist_concat(li, ty_set->members.next);
  ty_set->members.next = li;
  ty_set->members.count += 1;
  return ty_set;
}

Type* typeset_add_set(Type* to_set, Type* from_set)
{
  DList* li = from_set->members.next;
  while (li) {
    typeset_add_type(to_set, li->object);
    li = li->next;
  }
  return to_set;
}

internal void
visit_param(Ast* ast)
{
  assert(ast->kind == AST_PARAM);
  Ast_Param* param = (Ast_Param*)ast;
  visit_type_ref(param->type);
  Type* ty_set = typeset_create(&type_map, param->id);
  typeset_add_set(ty_set, typeset_get(&type_map, param->type->id));
  ty_set->ast = (Ast*)param;
}

internal void
visit_type_param(Ast* ast)
{
  assert(ast->kind == AST_NAME);
  Ast_Name* name = (Ast_Name*)ast;
  NameRef* ref = nameref_get(nameref_map, name->id);
  if (!ref) {
    Type_TypeParam* param_ty = arena_push_struct(type_storage, Type_TypeParam);
    param_ty->ctor = TYPE_TYPEPARAM;
    param_ty->strname = name->strname;
    param_ty->ast = (Ast*)name;
    Type* ty_set = typeset_create(&type_map, name->id);
    typeset_add_type(ty_set, (Type*)param_ty);
    ty_set->ast = (Ast*)name;
  } else {
    visit_expression(ast);
  }
}

internal void
visit_struct_field(Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  Ast_StructField* field = (Ast_StructField*)ast;
  visit_type_ref(field->type);
  Type* ty_set = typeset_create(&type_map, field->id);
  typeset_add_set(ty_set, typeset_get(&type_map, field->type->id));
  ty_set->ast = (Ast*)field;
}

internal void
visit_header_union(Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION);
  Ast_HeaderUnion* header_union_decl = (Ast_HeaderUnion*)ast;
  Ast_NodeList* fields = &header_union_decl->fields;
  DList* li = fields->head.next;
  while (li) {
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
  }
}

internal void
visit_header(Ast* ast)
{
  assert(ast->kind == AST_HEADER);
  Ast_Header* header_decl = (Ast_Header*)ast;
  Ast_Name* name = (Ast_Name*)header_decl->name;
  Ast_NodeList* fields = &header_decl->fields;
  if (fields->head.next) {
    DList* li = fields->head.next;
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
    if (li) {
      Type* fields_ty = typeset_get(&type_map, field->id);
      while (li) {
        Ast* field = li->object;
        visit_struct_field(field);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = fields_ty;
        product_ty->rhs_ty = typeset_get(&type_map, field->id);
        fields_ty = (Type*)product_ty;
        li = li->next;
      }
      Type* ty_set = typeset_create(&type_map, header_decl->id);
      typeset_add_type(ty_set, fields_ty);
      ty_set->ast = (Ast*)header_decl;
    } else {
      Type* ty_set = typeset_create(&type_map, header_decl->id);
      typeset_add_set(ty_set, typeset_get(&type_map, field->id));
      ty_set->ast = (Ast*)header_decl;
    }
  } else {
    Type* ty_set = typeset_create(&type_map, header_decl->id);
    ty_set->ast = (Ast*)header_decl;
  }
}

internal void
visit_struct(Ast* ast)
{
  assert(ast->kind == AST_STRUCT);
  Ast_Struct* struct_decl = (Ast_Struct*)ast;
  Ast_NodeList* fields = &struct_decl->fields;
  if (fields->head.next) {
    DList* li = fields->head.next;
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
    if (li) {
      Type* fields_ty = typeset_get(&type_map, field->id);
      while (li) {
        Ast* field = li->object;
        visit_struct_field(field);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = fields_ty;
        product_ty->rhs_ty = typeset_get(&type_map, field->id);
        fields_ty = (Type*)product_ty;
        li = li->next;
      }
      Type* ty_set = typeset_create(&type_map, field->id);
      typeset_add_type(ty_set, fields_ty);
      ty_set->ast = (Ast*)struct_decl;
    } else {
      Type* ty_set = typeset_create(&type_map, struct_decl->id);
      typeset_add_set(ty_set, typeset_get(&type_map, field->id));
      ty_set->ast = (Ast*)struct_decl;
    }
  } else {
    Type* ty_set = typeset_create(&type_map, struct_decl->id);
    ty_set->ast = (Ast*)struct_decl;
  }
}

internal void
visit_type_ref(Ast* ast)
{
  if (ast->kind == AST_BOOL_TYPE) {
    NameEntry* ne = scope_lookup_name(root_scope, "bool");
    Ast* bool_decl = ne->ns_type->ast;
    Type* ty_set = typeset_create(&type_map, ast->id);
    typeset_add_set(ty_set, typeset_get(&type_map, bool_decl->id));
    ty_set->ast = ast;
  } else if (ast->kind == AST_INT_TYPE) {
    NameEntry* ne = scope_lookup_name(root_scope, "int");
    Ast* int_decl = ne->ns_type->ast;
    Type* ty_set = typeset_create(&type_map, ast->id);
    typeset_add_set(ty_set, typeset_get(&type_map, int_decl->id));
    ty_set->ast = ast;
  } else if (ast->kind == AST_BIT_TYPE) {
    NameEntry* ne = scope_lookup_name(root_scope, "bit");
    Ast* bit_decl = ne->ns_type->ast;
    Type* ty_set = typeset_create(&type_map, ast->id);
    typeset_add_set(ty_set, typeset_get(&type_map, bit_decl->id));
    ty_set->ast = ast;
  } else if (ast->kind == AST_VARBIT_TYPE) {
    NameEntry* ne = scope_lookup_name(root_scope, "varbit");
    Ast* varbit_decl = ne->ns_type->ast;
    Type* ty_set = typeset_create(&type_map, ast->id);
    typeset_add_set(ty_set, typeset_get(&type_map, varbit_decl->id));
    ty_set->ast = ast;
  } else if (ast->kind == AST_STRING_TYPE) {
    NameEntry* ne = scope_lookup_name(root_scope, "string");
    Ast* string_decl = ne->ns_type->ast;
    Type* ty_set = typeset_create(&type_map, ast->id);
    typeset_add_set(ty_set, typeset_get(&type_map, string_decl->id));
    ty_set->ast = ast;
  } else if (ast->kind == AST_VOID_TYPE) {
    NameEntry* ne = scope_lookup_name(root_scope, "void");
    Ast* void_decl = ne->ns_type->ast;
    Type* ty_set = typeset_create(&type_map, ast->id);
    typeset_add_set(ty_set, typeset_get(&type_map, void_decl->id));
    ty_set->ast = ast;
  } else if (ast->kind == AST_ERROR_TYPE) {
    NameEntry* ne = scope_lookup_name(root_scope, "error");
    Ast* error_decl = ne->ns_type->ast;
    Type* ty_set = typeset_create(&type_map, ast->id);
    typeset_add_set(ty_set, typeset_get(&type_map, error_decl->id));
    ty_set->ast = ast;
  } else if (ast->kind == AST_HEADER_STACK) {
    Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
    visit_expression(type_ref->name);
    visit_expression(type_ref->stack_expr);
    Type* ty_set = typeset_create(&type_map, type_ref->id);
    typeset_add_set(ty_set, typeset_get(&type_map, type_ref->name->id));
    ty_set->ast = (Ast*)type_ref;
  } else if (ast->kind == AST_NAME) {
    visit_expression(ast);
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
    Ast_NodeList* type_args = &speclzd_type->type_args;
    DList* li = type_args->head.next;
    while (li) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  } else if (ast->kind == AST_TUPLE) {
    Ast_Tuple* type_ref = (Ast_Tuple*)ast;
    Ast_NodeList* type_args = &type_ref->type_args;
    DList* li = type_args->head.next;
    while (li) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  } else if (ast->kind == AST_STRUCT) {
    visit_struct(ast);
  } else if (ast->kind == AST_HEADER) {
    visit_header(ast);
  } else if (ast->kind == AST_HEADER_UNION) {
    visit_header_union(ast);
  } else if (ast->kind == AST_DONTCARE) {
    ; // pass
  }
  else assert(0);
}

internal void
visit_function_call(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_CALL);
  Ast_FunctionCall* function_call = (Ast_FunctionCall*)ast;
  visit_expression(function_call->callee_expr);
  Ast_Expression* callee_expr = (Ast_Expression*)(function_call->callee_expr);
  Ast_NodeList* type_args = &callee_expr->type_args;
  DList* li = type_args->head.next;
  while (li) {
    Ast* type_arg = li->object;
    visit_type_ref(type_arg);
    li = li->next;
  }
  NameEntry* ne = scope_lookup_name(root_scope, "void");
  NameDecl* void_decl = ne->ns_type;

  Type* args_type = type_get(&type_map, void_decl->ast->id);
  Ast_NodeList* args = &function_call->args;
  if (args->head.next) {
    DList* li = args->head.next;
    Ast* arg = li->object;
    visit_expression(arg);
    args_type = type_get(&type_map, arg->id);
    li = li->next;
    while (li) {
      Ast* arg = li->object;
      visit_expression(arg);
      Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
      product_ty->ctor = TYPE_PRODUCT;
      product_ty->lhs_ty = args_type;
      product_ty->rhs_ty = type_get(&type_map, arg->id);
      args_type = (Type*)product_ty;
      li = li->next;
    }
  }
  Type_FunctionCall* call_ty = arena_push_struct(type_storage, Type_FunctionCall);
  call_ty->ctor = TYPE_FUNCTION_CALL;
  type_add(&type_map, (Type*)call_ty, function_call->id);
  call_ty->args_ty = args_type;
}

internal void
visit_instantiation(Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  visit_type_ref(inst_decl->type_ref);
  NameEntry* ne = scope_lookup_name(root_scope, "void");
  NameDecl* void_decl = ne->ns_type;
  Type* args_type = type_get(&type_map, void_decl->ast->id);
  Ast_NodeList* args = &inst_decl->args;
  if (args->head.next) {
    DList* li = args->head.next;
    Ast* arg = li->object;
    visit_expression(arg);
    args_type = type_get(&type_map, arg->id);
    li = li->next;
    while (li) {
      Ast* arg = li->object;
      visit_expression(arg);
      Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
      product_ty->ctor = TYPE_PRODUCT;
      product_ty->lhs_ty = args_type;
      product_ty->rhs_ty = type_get(&type_map, arg->id);
      args_type = (Type*)product_ty;
      li = li->next;
    }
  }
  Type_FunctionCall* inst_ty = arena_push_struct(type_storage, Type_FunctionCall);
  inst_ty->ctor = TYPE_FUNCTION_CALL;
  inst_ty->args_ty = args_type;
  type_add(&type_map, (Type*)inst_ty, inst_decl->id);
}

internal void
visit_switch_label(Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    ; // pass
  } else {
    visit_expression(ast);
  }
}

internal void
visit_switch_case(Ast* ast)
{
  assert(ast->kind == AST_SWITCH_CASE);
  Ast_SwitchCase* switch_case = (Ast_SwitchCase*)ast;
  visit_switch_label(switch_case->label);
  Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_BLOCK_STMT) {
    visit_block_statement(case_stmt);
  }
}

internal void
visit_keyset_expr(Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT || ast->kind == AST_DONTCARE) {
    ; // pass
  } else {
    visit_expression(ast);
  }
}

internal void
visit_select_keyset(Ast* ast)
{
  if (ast->kind == AST_TUPLE_KEYSET) {
    Ast_TupleKeyset* keyset = (Ast_TupleKeyset*)ast;
    Ast_NodeList* expr_list = &keyset->expr_list;
    DList* li = expr_list->head.next;
    while (li) {
      Ast* expr = li->object;
      visit_keyset_expr(expr);
      li = li->next;
    }
  } else {
    visit_keyset_expr(ast);
  }
}

internal void
visit_action_ref(Ast* ast)
{
  assert(ast->kind == AST_ACTION_REF);
  Ast_ActionRef* action = (Ast_ActionRef*)ast;
  visit_expression(action->name);
  Ast_NodeList* args = &action->args;
  DList* li = args->head.next;
  while (li) {
    Ast* arg = li->object;
    visit_expression(arg);
    li = li->next;
  }
}

internal void
visit_table_keyelem(Ast* ast)
{
  assert(ast->kind == AST_KEY_ELEMENT);
  Ast_KeyElement* keyelem = (Ast_KeyElement*)ast;
  visit_expression(keyelem->expr);
}

internal void
visit_table_entry(Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRY);
  Ast_TableEntry* entry = (Ast_TableEntry*)ast;
  visit_select_keyset(entry->keyset);
  visit_action_ref(entry->action);
}

internal void
visit_table_property(Ast* ast)
{
  if (ast->kind == AST_TABLE_ACTIONS) {
    Ast_TableActions* prop = (Ast_TableActions*)ast;
    Ast_NodeList* action_list = &prop->action_list;
    DList* li = action_list->head.next;
    while (li) {
      Ast* action = li->object;
      visit_action_ref(action);
      li = li->next;
    }
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    Ast_TableSingleEntry* prop = (Ast_TableSingleEntry*)ast;
    if (prop->init_expr) {
      visit_expression(prop->init_expr);
    }
  } else if (ast->kind == AST_TABLE_KEY) {
    Ast_TableKey* prop = (Ast_TableKey*)ast;
    Ast_NodeList* keyelem_list = &prop->keyelem_list;
    DList* li = keyelem_list->head.next;
    while (li) {
      Ast* keyelem = li->object;
      visit_table_keyelem(keyelem);
      li = li->next;
    }
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    Ast_TableEntries* prop = (Ast_TableEntries*)ast;
    Ast_NodeList* entries = &prop->entries;
    DList* li = entries->head.next;
    while (li) {
      Ast* entry = li->object;
      visit_table_entry(entry);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
visit_table(Ast* ast)
{
  assert(ast->kind == AST_TABLE);
  Ast_Table* decl = (Ast_Table*)ast;
  Ast_NodeList* prop_list = &decl->prop_list;
  DList* li = prop_list->head.next;
  while (li) {
    Ast* prop = li->object;
    visit_table_property(prop);
    li = li->next;
  }
}

internal void
visit_action(Ast* ast)
{
  assert(ast->kind == AST_ACTION);
  Ast_Action* action_decl = (Ast_Action*)ast;
  Ast_NodeList* params = &action_decl->params;
  DList* li = params->head.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_BlockStmt* action_body = (Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    Ast_NodeList* stmt_list = &action_body->stmt_list;
    DList* li = stmt_list->head.next;
    while (li) {
      Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
}

internal void
visit_var(Ast* ast)
{
  assert(ast->kind == AST_VAR);
  Ast_Var* var_decl = (Ast_Var*)ast;
  visit_type_ref(var_decl->type);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr);
  }
  Type_TypeRef* var_ty = arena_push_struct(type_storage, Type_TypeRef);
  type_add(&type_map, (Type*)var_ty, var_decl->id);
  var_ty->ctor = TYPE_TYPEREF;
  var_ty->ref = type_get(&type_map, var_decl->type->id);
  var_ty->ast = (Ast*)var_decl;
}

internal void
visit_if_stmt(Ast* ast)
{
  assert(ast->kind == AST_IF_STMT);
  Ast_IfStmt* stmt = (Ast_IfStmt*)ast;
  visit_expression(stmt->cond_expr);
  visit_statement(stmt->stmt);
  if (stmt->else_stmt) {
    visit_statement(stmt->else_stmt);
  }
}

internal void
visit_switch_stmt(Ast* ast)
{
  assert(ast->kind == AST_SWITCH_STMT);
  Ast_SwitchStmt* stmt = (Ast_SwitchStmt*)ast;
  visit_expression(stmt->expr);
  Ast_NodeList* switch_cases = &stmt->switch_cases;
  DList* li = switch_cases->head.next;
  while (li) {
    Ast* switch_case = li->object;
    visit_switch_case(switch_case);
    li = li->next;
  }
}

internal void
visit_assignment_stmt(Ast* ast)
{
  assert(ast->kind == AST_ASSIGNMENT_STMT);
  Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
  visit_expression(stmt->lvalue);
  visit_expression(stmt->expr);
  Type_Product* args_ty = arena_push_struct(type_storage, Type_Product);
  args_ty->ctor = TYPE_PRODUCT;
  args_ty->lhs_ty = type_get(&type_map, stmt->lvalue->id);
  args_ty->rhs_ty = type_get(&type_map, stmt->expr->id);
  Type_FunctionCall* stmt_ty = arena_push_struct(type_storage, Type_FunctionCall);
  type_add(&type_map, (Type*)stmt_ty, stmt->id);
  stmt_ty->ctor = TYPE_FUNCTION_CALL;
  stmt_ty->args_ty = (Type*)args_ty;
  stmt_ty->ast = (Ast*)stmt;
}

internal void
visit_return_stmt(Ast* ast)
{
  assert(ast->kind == AST_RETURN_STMT);
  Ast_ReturnStmt* stmt = (Ast_ReturnStmt*)ast;
  if (stmt->expr) {
    visit_expression(stmt->expr);
  }
  NameEntry* ne = scope_lookup_name(root_scope, "void");
  NameDecl* void_decl = ne->ns_type;
  Type* return_type = type_get(&type_map, void_decl->ast->id);
  if (stmt->expr) {
    return_type = type_get(&type_map, stmt->expr->id);
  }
  type_add(&type_map, return_type, stmt->id);
}

internal void
visit_statement(Ast* ast)
{
  if (ast->kind == AST_VAR) {
    visit_var(ast);
  } else if (ast->kind == AST_ACTION) {
    visit_action(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    visit_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_TABLE) {
    visit_table(ast);
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
  } else if (ast->kind == AST_EXIT_STMT || ast->kind == AST_EMPTY_ELEMENT) {
    ; // pass
  }
  else assert(0);
}

internal void
visit_function_return_type(Ast* ast)
{
  if (ast->kind == AST_NAME) {
    Ast_Name* return_type = (Ast_Name*)ast;
    visit_type_param((Ast*)return_type);
  } else {
    visit_type_ref(ast);
  }
}

internal void
visit_function_proto(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  Ast_FunctionProto* proto = (Ast_FunctionProto*)ast;
  if (proto->return_type) {
    visit_function_return_type(proto->return_type);
  }
  Ast_NodeList* type_params = &proto->type_params;
  DList* li = type_params->head.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto->params;
  if (params->head.next) {
    DList* li = params->head.next;
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
    if (li) {
      Type* params_ty = type_get(&type_map, param->id);
      while (li) {
        Ast* param = li->object;
        visit_param(param);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product); 
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = params_ty;
        product_ty->rhs_ty = type_get(&type_map, param->id);
        params_ty = (Type*)product_ty;
        li = li->next;
      }
      type_add(&type_map, params_ty, params->id);
    } else {
      Type_TypeRef* params_ty = arena_push_struct(type_storage, Type_TypeRef);
      type_add(&type_map, (Type*)params_ty, params->id);
      params_ty->ctor = TYPE_TYPEREF;
      params_ty->ref = type_get(&type_map, param->id);
      params_ty->ast = (Ast*)params;
    }
  }
  Type_Function* function_ty = arena_push_struct(type_storage, Type_Function);
  type_add(&type_map, (Type*)function_ty, proto->id);
  function_ty->ctor = TYPE_FUNCTION;
  function_ty->params_ty = type_get(&type_map, params->id);
  if (proto->return_type) {
    function_ty->return_ty = type_get(&type_map, proto->return_type->id);
  }
  function_ty->ast = (Ast*)proto;
}

internal void
visit_block_statement(Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
  Ast_NodeList* stmt_list = &block_stmt->stmt_list;
  DList* li = stmt_list->head.next;
  while (li) {
    Ast* decl = li->object;
    visit_statement(decl);
    li = li->next;
  }
}

internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_CONTROL_PROTO);
  Ast_ControlProto* proto = (Ast_ControlProto*)ast;
  Ast_NodeList* type_params = &proto->type_params;
  DList* li;
  li = type_params->head.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto->params;
  if (params->head.next) {
    DList* li = params->head.next;
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
    if (li) {
      Type* params_ty = type_get(&type_map, param->id);
      while (li) {
        Ast* param = li->object;
        visit_param(param);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product); 
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = params_ty;
        product_ty->rhs_ty = type_get(&type_map, param->id);
        params_ty = (Type*)product_ty;
        li = li->next;
      }
      type_add(&type_map, params_ty, params->id);
    } else {
      Type_TypeRef* params_ty = arena_push_struct(type_storage, Type_TypeRef);
      type_add(&type_map, (Type*)params_ty, params->id);
      params_ty->ctor = TYPE_TYPEREF;
      params_ty->ref = type_get(&type_map, param->id);
      params_ty->ast = (Ast*)params;
    }
  }
  Type_Function* control_ty = arena_push_struct(type_storage, Type_Function);
  type_add(&type_map, (Type*)control_ty, proto->id);
  control_ty->ctor = TYPE_FUNCTION;
  control_ty->params_ty = type_get(&type_map, params->id);
  control_ty->ast = (Ast*)proto;
}

internal void
visit_control(Ast* ast)
{
  assert(ast->kind == AST_CONTROL);
  Ast_Control* control_decl = (Ast_Control*)ast;
  visit_control_proto(control_decl->proto);
  Type_TypeRef* decl_ty = arena_push_struct(type_storage, Type_TypeRef);
  type_add(&type_map, (Type*)decl_ty, control_decl->id);
  decl_ty->ctor = TYPE_TYPEREF;
  decl_ty->ref = type_get(&type_map, control_decl->proto->id);
  decl_ty->ast = (Ast*)control_decl;

  Ast_NodeList* ctor_params = &control_decl->ctor_params;
  DList* li;
  li = ctor_params->head.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_NodeList* local_decls = &control_decl->local_decls;
  li = local_decls->head.next;
  while (li) {
    Ast* decl = li->object;
    visit_statement(decl);
    li = li->next;
  }
  if (control_decl->apply_stmt) {
    visit_block_statement(control_decl->apply_stmt);
  }
}

internal void
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_EXTERN);
  Ast_Extern* extern_decl = (Ast_Extern*)ast;
  Ast_Name* name = (Ast_Name*)extern_decl->name;
  Type_TypeName* extern_ty = arena_push_struct(type_storage, Type_TypeName);
  extern_ty->ctor = TYPE_NAME;
  extern_ty->strname = name->strname;
  type_add(&type_map, (Type*)extern_decl, extern_decl->id);
  Ast_NodeList* type_params = &extern_decl->type_params;
  DList* li;
  li = type_params->head.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* method_protos = &extern_decl->method_protos;
  li = method_protos->head.next;
  while (li) {
    Ast* proto = li->object;
    visit_function_proto(proto);
    li = li->next;
  }
}

internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_PACKAGE);
  Ast_Package* package_decl = (Ast_Package*)ast;
  Ast_Name* name = (Ast_Name*)package_decl->name;
  Type_TypeName* package_ty = arena_push_struct(type_storage, Type_TypeName);
  package_ty->ctor = TYPE_NAME;
  package_ty->strname = name->strname;
  type_add(&type_map, (Type*)package_ty, package_decl->id);
  Ast_NodeList* params = &package_decl->params;
  DList* li = params->head.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
}

internal void
visit_transition_select_case(Ast* ast)
{
  assert(ast->kind == AST_SELECT_CASE);
  Ast_SelectCase* select_case = (Ast_SelectCase*)ast;
  visit_select_keyset(select_case->keyset);
}

internal void
visit_parser_transition(Ast* ast)
{
  if (ast->kind == AST_NAME) {
    visit_expression(ast);
  } else if (ast->kind == AST_SELECT_EXPR) {
    Ast_SelectExpr* trans_stmt = (Ast_SelectExpr*)ast;
    Ast_NodeList* expr_list = &trans_stmt->expr_list;
    DList* li;
    li = expr_list->head.next;
    while (li) {
      Ast* expr = li->object;
      visit_expression(expr);
      li = li->next;
    }
    Ast_NodeList* case_list = &trans_stmt->case_list;
    li = case_list->head.next;
    while (li) {
      Ast* select_case = li->object;
      visit_transition_select_case(select_case);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
visit_parser_state(Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  Ast_ParserState* state = (Ast_ParserState*)ast;
  Ast_NodeList* stmt_list = &state->stmt_list;
  DList* li = stmt_list->head.next;
  while (li) {
    Ast* stmt = li->object;
    visit_statement(stmt);
    li = li->next;
  }
  visit_parser_transition(state->trans_stmt);
}

internal void
visit_const(Ast* ast)
{
  assert(ast->kind == AST_CONST);
  Ast_Const* decl = (Ast_Const*)ast;
  visit_type_ref(decl->type_ref);
  Type* decl_type = type_get(&type_map, decl->type_ref->id);
  type_add(&type_map, decl_type, decl->id);
  visit_expression(decl->expr);
}

internal void
visit_local_parser_element(Ast* ast)
{
  if (ast->kind == AST_CONST) {
    visit_const(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_VAR) {
    visit_statement(ast);
  } else assert(0);
}

internal void
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_PARSER_PROTO);
  Ast_ParserProto* proto = (Ast_ParserProto*)ast;
  Ast_NodeList* type_params = &proto->type_params;
  DList* li;
  li = type_params->head.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto->params;
  if (params->head.next) {
    DList* li = params->head.next;
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
    if (li) {
      Type* params_ty = type_get(&type_map, param->id);
      while (li) {
        Ast* param = li->object;
        visit_param(param);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product); 
        product_ty->ctor = TYPE_PRODUCT; 
        product_ty->lhs_ty = params_ty;
        product_ty->rhs_ty = type_get(&type_map, param->id);
        params_ty = (Type*)product_ty;
        li = li->next;
      }
      type_add(&type_map, params_ty, params->id);
    } else {
      Type_TypeRef* params_ty = arena_push_struct(type_storage, Type_TypeRef);
      type_add(&type_map, (Type*)params_ty, params->id);
      params_ty->ctor = TYPE_TYPEREF;
      params_ty->ref = type_get(&type_map, param->id);
      params_ty->ast = (Ast*)params;
    }
  }
  Type_Function* parser_ty = arena_push_struct(type_storage, Type_Function);
  type_add(&type_map, (Type*)parser_ty, proto->id);
  parser_ty->ctor = TYPE_FUNCTION;
  parser_ty->params_ty = type_get(&type_map, params->id);
  parser_ty->ast = (Ast*)proto;
}

internal void
visit_parser(Ast* ast)
{
  assert(ast->kind == AST_PARSER);
  Ast_Parser* parser_decl = (Ast_Parser*)ast;
  visit_parser_proto(parser_decl->proto);
  Type_TypeRef* decl_ty = arena_push_struct(type_storage, Type_TypeRef);
  type_add(&type_map, (Type*)decl_ty, parser_decl->id);
  decl_ty->ctor = TYPE_TYPEREF;
  decl_ty->ref = type_get(&type_map, parser_decl->proto->id);
  decl_ty->ast = (Ast*)parser_decl;

  Ast_NodeList* ctor_params = &parser_decl->ctor_params;
  DList* li;
  li = ctor_params->head.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_NodeList* local_elements = &parser_decl->local_elements;
  li = local_elements->head.next;
  while (li) {
    Ast* element = li->object;
    visit_local_parser_element(element);
    li = li->next;
  }
  Ast_NodeList* states = &parser_decl->states;
  li = states->head.next;
  while (li) {
    Ast* state = li->object;
    visit_parser_state(state);
    li = li->next;
  }
}

internal void
visit_type(Ast* ast)
{
  assert(ast->kind == AST_TYPE);
  Ast_Type* type_decl = (Ast_Type*)ast;
  visit_type_ref(type_decl->type_ref);
}

internal void
visit_function(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION);
  Ast_Function* function_decl = (Ast_Function*)ast;
  visit_function_proto(function_decl->proto);
  Type_TypeRef* decl_ty = arena_push_struct(type_storage, Type_TypeRef);
  type_add(&type_map, (Type*)decl_ty, function_decl->id);
  decl_ty->ctor = TYPE_TYPEREF;
  decl_ty->ref = type_get(&type_map, function_decl->proto->id);
  decl_ty->ast = (Ast*)function_decl;

  Ast_BlockStmt* function_body = (Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    Ast_NodeList* stmt_list = &function_body->stmt_list;
    DList* li = stmt_list->head.next;
    while (li) {
      Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
}

internal void
visit_enum_field(Ast* ast)
{
  assert(ast->kind == AST_NAME);
}

internal void
visit_specified_id(Ast* ast)
{
  // pass
}

internal void
visit_error(Ast* ast)
{
  assert (ast->kind == AST_ERROR);
  Ast_Error* decl = (Ast_Error*)ast;
  Ast_NodeList* id_list = &decl->id_list;
  DList* li = id_list->head.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_NAME) {
      visit_enum_field(id);
    }
    else assert(0);
    li = li->next;
  }
}

internal void
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_ENUM);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_Name* name = (Ast_Name*)enum_decl->name;
  Type_TypeName* enum_ty = arena_push_struct(type_storage, Type_TypeName);
  enum_ty->ctor = TYPE_NAME;
  enum_ty->strname = name->strname;
  type_add(&type_map, (Type*)enum_ty, enum_decl->id);
}

internal void
visit_binary_expr(Ast* ast)
{
  assert(ast->kind == AST_BINARY_EXPR);
  Ast_BinaryExpr* expr = (Ast_BinaryExpr*)ast;
  visit_expression(expr->left_operand);
  visit_expression(expr->right_operand);
  Type_Product* args_ty = arena_push_struct(type_storage, Type_Product);
  args_ty->ctor = TYPE_PRODUCT;
  args_ty->lhs_ty = type_get(&type_map, expr->left_operand->id);
  args_ty->rhs_ty = type_get(&type_map, expr->right_operand->id);
  Type_FunctionCall* expr_ty = arena_push_struct(type_storage, Type_FunctionCall);
  type_add(&type_map, (Type*)expr_ty, expr->id);
  expr_ty->ctor = TYPE_FUNCTION_CALL;
  expr_ty->args_ty = (Type*)args_ty;
  expr_ty->ast = (Ast*)expr;
}

internal void
visit_unary_expr(Ast* ast)
{
  assert(ast->kind == AST_UNARY_EXPR);
  Ast_UnaryExpr* expr = (Ast_UnaryExpr*)ast;
  visit_expression(expr->operand);
  Type* args_ty = type_get(&type_map, expr->operand->id);
  Type_FunctionCall* expr_ty = arena_push_struct(type_storage, Type_FunctionCall);
  expr_ty->ctor = TYPE_FUNCTION_CALL;
  expr_ty->args_ty = (Type*)args_ty;
  type_add(&type_map, (Type*)expr_ty, expr->id);
}

internal void
visit_member_select(Ast* ast)
{
  assert(ast->kind == AST_MEMBER_SELECT);
  Ast_MemberSelect* expr = (Ast_MemberSelect*)ast;
  visit_expression(expr->lhs_expr);
  Type_TypeVar* member_ty = arena_push_struct(type_storage, Type_TypeVar);
  member_ty->ctor = TYPE_TYPEVAR;
  type_add(&type_map, (Type*)member_ty, expr->id);
}

internal void
visit_expression(Ast* ast)
{
  if (ast->kind == AST_BINARY_EXPR) {
    visit_binary_expr(ast);
  } else if (ast->kind == AST_UNARY_EXPR) {
    visit_unary_expr(ast);
  } else if (ast->kind == AST_NAME) {
    Ast_Name* name = (Ast_Name*)ast;
    NameRef* ref = nameref_get(nameref_map, name->id);
    NameEntry* ne = scope_lookup_name(ref->scope, ref->strname);
    if (ne->ns_type) {
      NameDecl* decl = ne->ns_type;
      while (decl) {
        Type* type = type_get(&type_map, decl->ast->id);
        type_add(&type_map, type, ast->id);
        decl = decl->nextdecl_in_scope;
      }
    } else if (ne->ns_var) {
      NameDecl* decl = ne->ns_var;
      assert(!decl->nextdecl_in_scope);
      Type* type = type_get(&type_map, decl->ast->id);
      type_add(&type_map, type, ast->id);
    } else error("at line %d: unresolved name `%s`.", ref->line_no, ref->strname);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_MEMBER_SELECT) {
    visit_member_select(ast);
  } else if (ast->kind == AST_EXPRLIST) {
    Ast_ExprList* expr = (Ast_ExprList*)ast;
    Ast_NodeList* expr_list = &expr->expr_list;
    DList* li = expr_list->head.next;
    while (li) {
      Ast* expr_expr = li->object;
      visit_expression(expr_expr);
      li = li->next;
    }
  } else if (ast->kind == AST_CAST_EXPR) {
    Ast_CastExpr* expr = (Ast_CastExpr*)ast;
    visit_type_ref(expr->to_type);
    visit_expression(expr->expr);
  } else if (ast->kind == AST_SUBSCRIPT) {
    Ast_Subscript* expr = (Ast_Subscript*)ast;
    visit_expression(expr->index);
    if (expr->end_index) {
      visit_expression(expr->end_index);
    }
  } else if (ast->kind == AST_KVPAIR) {
    Ast_KVPair* expr = (Ast_KVPair*)ast;
    visit_expression(expr->expr);
  } else if (ast->kind == AST_INT_LITERAL || ast->kind == AST_BOOL_LITERAL) {
    NameEntry* ne = scope_lookup_name(root_scope, "int");
    NameDecl* decl = ne->ns_type;
    Type* int_type = type_get(&type_map, decl->ast->id);
    type_add(&type_map, int_type, ast->id);
  } else if (ast->kind == AST_STRING_LITERAL) {
    NameEntry* ne = scope_lookup_name(root_scope, "string");
    NameDecl* decl = ne->ns_type;
    Type* string_type = type_get(&type_map, decl->ast->id);
    type_add(&type_map, string_type, ast->id);
  }
  else assert(0);
}

internal void
visit_match_kind(Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND);
  Ast_MatchKind* decl = (Ast_MatchKind*)ast;
  Ast_NodeList* id_list = &decl->id_list;
  DList* li = id_list->head.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_NAME) {
      visit_enum_field(id);
    } else if (id->kind == AST_SPECIFIED_IDENT) {
      visit_specified_id(id);
    }
    else assert(0);
    li = li->next;
  }
}

internal void
visit_p4program(Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  Ast_P4Program* program = (Ast_P4Program*)ast;
  Ast_NodeList* decl_list = &program->decl_list;
  DList* li = decl_list->head.next;
  while (li) {
    Ast* decl = li->object;
    if (decl->kind == AST_CONTROL) {
      visit_control(decl);
    } else if (decl->kind == AST_CONTROL_PROTO) {
      visit_control_proto(decl);
    } else if (decl->kind == AST_EXTERN) {
      visit_extern(decl);
    } else if (decl->kind == AST_STRUCT) {
      visit_struct(decl);
    } else if (decl->kind == AST_HEADER) {
      visit_header(decl);
    } else if (decl->kind == AST_HEADER_UNION) {
      visit_header_union(decl);
    } else if (decl->kind == AST_PACKAGE) {
      visit_package(decl);
    } else if (decl->kind == AST_PARSER) {
      visit_parser(decl);
    } else if (decl->kind == AST_PARSER_PROTO) {
      visit_parser_proto(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      visit_instantiation(decl);
    } else if (decl->kind == AST_TYPE) {
      visit_type(decl);
    } else if (decl->kind == AST_CONST) {
      visit_const(decl);
    } else if (decl->kind == AST_FUNCTION) {
      visit_function(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      visit_function_proto(decl);
    } else if (decl->kind == AST_ACTION) {
      visit_action(decl);
    } else if (decl->kind == AST_ENUM) {
      visit_enum(decl);
    } else if (decl->kind == AST_MATCH_KIND) {
      visit_match_kind(decl);
    } else if (decl->kind == AST_ERROR) {
      visit_error(decl);
    } else assert(0);
    li = li->next;
  }
}

Hashmap*
build_type(Ast_P4Program* p4program, Scope* root_scope_,
           Hashmap* nameref_map_, Arena* type_storage_)
{
  root_scope = root_scope_;
  nameref_map = nameref_map_;
  type_storage = type_storage_;
  hashmap_init(&type_map, HASHMAP_KEY_UINT32, 8, type_storage);

  {
    NameEntry* ne = scope_lookup_name(root_scope, "void");
    Ast* void_decl = ne->ns_type->ast;
    Type* void_type = arena_push_struct(type_storage, Type);
    void_type->ctor = TYPE_VOID;
    void_type->ast = void_decl;
    Type* ty_set = typeset_create(&type_map, void_decl->id);
    typeset_add_type(ty_set, void_type);
    ty_set->ast = void_decl;
  }
  {
    NameEntry* ne = scope_lookup_name(root_scope, "bool");
    Ast* bool_decl = ne->ns_type->ast;
    Type* bool_type = arena_push_struct(type_storage, Type);
    bool_type->ctor = TYPE_BOOL;
    bool_type->ast = bool_decl;
    Type* ty_set = typeset_create(&type_map, bool_decl->id);
    typeset_add_type(ty_set, bool_type);
    ty_set->ast = bool_decl;
  }
  {
    NameEntry* ne = scope_lookup_name(root_scope, "int");
    Ast* int_decl = ne->ns_type->ast;
    Type* int_type = arena_push_struct(type_storage, Type);
    int_type->ctor = TYPE_INT;
    int_type->ast = int_decl;
    Type* ty_set = typeset_create(&type_map, int_decl->id);
    typeset_add_type(ty_set, int_type);
    ty_set->ast = int_decl;
  }
  {
    NameEntry* ne = scope_lookup_name(root_scope, "bit");
    Ast* bit_decl = ne->ns_type->ast;
    Type* bit_type = arena_push_struct(type_storage, Type);
    bit_type->ctor = TYPE_BIT;
    bit_type->ast = bit_decl;
    Type* ty_set = typeset_create(&type_map, bit_decl->id);
    typeset_add_type(ty_set, bit_type);
    ty_set->ast = bit_decl;
  }
  {
    NameEntry* ne = scope_lookup_name(root_scope, "varbit");
    Ast* varbit_decl = ne->ns_type->ast;
    Type* varbit_type = arena_push_struct(type_storage, Type);
    varbit_type->ctor = TYPE_VARBIT;
    varbit_type->ast = varbit_decl;
    Type* ty_set = typeset_create(&type_map, varbit_decl->id);
    typeset_add_type(ty_set, varbit_type);
    ty_set->ast = varbit_decl;
  }
  {
    NameEntry* ne = scope_lookup_name(root_scope, "string");
    Ast* string_decl = ne->ns_type->ast;
    Type* string_type = arena_push_struct(type_storage, Type);
    string_type->ctor = TYPE_STRING;
    string_type->ast = string_decl;
    Type* ty_set = typeset_create(&type_map, string_decl->id);
    typeset_add_type(ty_set, string_type);
    ty_set->ast = string_decl;
  }
  {
    NameEntry* ne = scope_lookup_name(root_scope, "error");
    Ast* error_decl = ne->ns_type->ast;
    Type* error_type = arena_push_struct(type_storage, Type);
    error_type->ctor = TYPE_ERROR;
    error_type->ast = error_decl;
    Type* ty_set = typeset_create(&type_map, error_decl->id);
    typeset_add_type(ty_set, error_type);
    ty_set->ast = error_decl;
  }

  visit_p4program((Ast*)p4program);
  return &type_map;
}
