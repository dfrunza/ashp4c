#include "arena.h"
#include "ast.h"
#include "symtable.h"
#include <memory.h>  // memset


internal struct Arena* symtable_storage;


internal void build_symtable_block_statement(struct Ast* block_stmt);
internal void build_symtable_statement(struct Ast* decl);


internal struct ObjectDescriptor*
new_object_descriptor(char* name, enum ObjectKind object_kind)
{
  struct ObjectDescriptor* descriptor = arena_push(symtable_storage, sizeof(*descriptor));
  memset(descriptor, 0, sizeof(*descriptor));
  descriptor->name = name;
  descriptor->object_kind = object_kind;
  return descriptor;
}

internal void
build_symtable_param(struct Ast* ast)
{
  assert(ast->kind == AST_PARAM);
  struct Ast_Param* param = (struct Ast_Param*)ast;
  struct Ast_Name* name = (struct Ast_Name*)param->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_PARAM);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_type_param(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* type_param = (struct Ast_Name*)ast;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), type_param->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(type_param->strname, OBJECT_TYPE_PARAM);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, type_param->line_nr);
  }
}

internal void
build_symtable_action_decl(struct Ast* ast) {
  assert(ast->kind == AST_ACTION_DECL);
  struct Ast_ActionDecl* action_decl = (struct Ast_ActionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)action_decl->name;
  struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_ACTION);
  descriptor->ast = ast;
  declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  action_decl->scope = push_scope();
  struct List* params = action_decl->params;
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
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
        build_symtable_statement(stmt);
        link = link->next;
      }
    }
  }
  pop_scope();
}

internal void
build_symtable_instantiation(struct Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  struct Ast_Instantiation* inst = (struct Ast_Instantiation*)ast;
  struct Ast_Name* name = (struct Ast_Name*)inst->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_INSTANTIATION);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_table_property(struct Ast* prop)
{
  if (prop->kind == AST_TABLE_ACTIONS || prop->kind == AST_TABLE_ENTRIES ||
      prop->kind == AST_TABLE_SINGLE_ENTRY || prop->kind == AST_TABLE_KEY) {
    ; // pass
  }
  else assert(0);
}

internal void
build_symtable_table_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_DECL);
  struct Ast_TableDecl* decl = (struct Ast_TableDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_TABLE);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  struct List* prop_list = decl->prop_list;
  if (prop_list) {
    struct ListLink* link = list_first_link(prop_list);
    while (link) {
      struct Ast* prop = link->object;
      build_symtable_table_property(prop);
      link = link->next;
    }
  }
}

internal void
build_symtable_switch_case(struct Ast* ast)
{
  assert(ast->kind == AST_SWITCH_CASE);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  struct Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_BLOCK_STMT) {
    build_symtable_block_statement(case_stmt);
  }
}

internal void
build_symtable_statement(struct Ast* ast)
{
  if (ast->kind == AST_VAR_DECL) {
    struct Ast_VarDecl* decl = (struct Ast_VarDecl*)ast;
    struct Ast_Name* name = (struct Ast_Name*)decl->name;
    struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
    if (!entry->ns_general) {
      struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_VAR);
      descriptor->ast = ast;
      declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  } else if (ast->kind == AST_ACTION_DECL) {
    build_symtable_action_decl(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    build_symtable_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    build_symtable_instantiation(ast);
  } else if (ast->kind == AST_TABLE_DECL) {
    build_symtable_table_decl(ast);
  } else if (ast->kind == AST_IF_STMT) {
    struct Ast_IfStmt* decl = (struct Ast_IfStmt*)ast;
    struct Ast* if_stmt = decl->stmt;
    build_symtable_statement(if_stmt);
    struct Ast* else_stmt = decl->else_stmt;
    if (else_stmt) {
      build_symtable_statement(else_stmt);
    }
  } else if (ast->kind == AST_SWITCH_STMT) {
    struct Ast_SwitchStmt* decl = (struct Ast_SwitchStmt*)ast;
    if (decl->switch_cases) {
      struct ListLink* link = list_first_link(decl->switch_cases);
      while (link) {
        struct Ast* switch_case = link->object;
        build_symtable_switch_case(switch_case);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_METHODCALL_STMT || ast->kind == AST_ASSIGNMENT_STMT ||
             ast->kind == AST_DIRECT_APPLICATION || ast->kind == AST_RETURN_STMT ||
             ast->kind == AST_INT_LITERAL || ast->kind == AST_BOOL_LITERAL || ast->kind == AST_STRING_LITERAL ||
             ast->kind == AST_EXIT_STMT) {
    ; // pass
  }
  else assert(0);
}

internal void
build_symtable_block_statement(struct Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  struct Ast_BlockStmt* block_stmt = (struct Ast_BlockStmt*)ast;
  block_stmt->scope = push_scope();
  if (block_stmt->stmt_list) {
    struct ListLink* link = list_first_link(block_stmt->stmt_list);
    while (link) {
      struct Ast* decl = link->object;
      build_symtable_statement(decl);
      link = link->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL_DECL);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_CONTROL_PROTO);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  control_decl->scope = push_scope();
  if (type_decl->type_params) {
    struct ListLink* link = list_first_link(type_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* link = list_first_link(type_decl->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* link = list_first_link(control_decl->ctor_params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  if (control_decl->local_decls) {
    struct ListLink* link = list_first_link(control_decl->local_decls);
    while (link) {
      struct Ast* decl = link->object;
      build_symtable_statement(decl);
      link = link->next;
    }
  }
  if (control_decl->apply_stmt) {
    build_symtable_block_statement(control_decl->apply_stmt);
  }
  pop_scope();
}

internal void
build_symtable_local_parser_element(struct Ast* ast)
{
  if (ast->kind == AST_CONST_DECL || ast->kind == AST_INSTANTIATION || ast->kind == AST_VAR_DECL) {
    struct Ast_Name* name = (struct Ast_Name*)ast->name;
    struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
    if (!entry->ns_general) {
      struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_NONE);
      if (ast->kind == AST_CONST_DECL) {
        descriptor->object_kind = OBJECT_CONST;
      } else if (ast->kind == AST_VAR_DECL) {
        descriptor->object_kind = OBJECT_VAR;
      } else if (ast->kind == AST_INSTANTIATION) {
        descriptor->object_kind = OBJECT_INSTANTIATION;
      } else assert(0);
      descriptor->ast = ast;
      declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  }
  else assert(0);
}

internal void
build_symtable_parser_state(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  struct Ast_Name* name = (struct Ast_Name*)state->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_PARSER_STATE);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  state->scope = push_scope();
  if (state->stmt_list) {
    struct ListLink* link = list_first_link(state->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      build_symtable_statement(stmt);
      link = link->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_parser_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_DECL);
  struct Ast_ParserDecl* parser_decl = (struct Ast_ParserDecl*)ast;
  struct Ast_ParserProto* type_decl = (struct Ast_ParserProto*)parser_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_PARSER_PROTO);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  parser_decl->scope = push_scope();
  if (type_decl->type_params) {
    struct ListLink* link = list_first_link(type_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* link = list_first_link(type_decl->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  if (parser_decl->ctor_params) {
    struct ListLink* link = list_first_link(parser_decl->ctor_params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  if (parser_decl->local_elements) {
    struct ListLink* link = list_first_link(parser_decl->local_elements);
    while (link) {
      struct Ast* element = link->object;
      build_symtable_local_parser_element(element);
      link = link->next;
    }
  }
  if (parser_decl->states) {
    struct ListLink* link = list_first_link(parser_decl->states);
    while (link) {
      struct Ast* state = link->object;
      build_symtable_parser_state(state);
      link = link->next;
    }
  }
  pop_scope();
}

void
build_symtable_function_proto(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)ast;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_FUNCTION_PROTO);
  descriptor->ast = ast;
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  function_proto->scope = push_scope();
  if (function_proto->return_type) {
    if (function_proto->return_type->kind == AST_NAME) {
      struct Ast_Name* return_type = (struct Ast_Name*)function_proto->return_type;
      struct SymtableEntry* entry = scope_resolve_name(get_current_scope(), return_type->strname);
      if (!entry->ns_type) {
        build_symtable_type_param(function_proto->return_type);
      }
    }
  }
  if (function_proto->type_params) {
    struct ListLink* link = list_first_link(function_proto->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (function_proto->params) {
    struct ListLink* link = list_first_link(function_proto->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_extern_decl(struct Ast* ast)
{
  assert(ast->kind == AST_EXTERN_DECL);
  struct Ast_ExternDecl* extern_decl = (struct Ast_ExternDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)extern_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_EXTERN);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  extern_decl->scope = push_scope();
  if (extern_decl->type_params) {
    struct ListLink* link = list_first_link(extern_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (extern_decl->method_protos) {
    struct ListLink* link = list_first_link(extern_decl->method_protos);
    while (link) {
      struct Ast* proto = link->object;
      build_symtable_function_proto(proto);
      link = link->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_struct_field(struct Scope* struct_scope, struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  struct Ast_StructField* field = (struct Ast_StructField*)ast;
  struct Ast_Name* name = (struct Ast_Name*)field->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(struct_scope, name->strname);
  if (!entry->ns_general) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_STRUCT_FIELD);
    descriptor->ast = ast;
    declare_object_in_scope(struct_scope, NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_struct_decl(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_DECL);
  struct Ast_StructDecl* struct_decl = (struct Ast_StructDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)struct_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_STRUCT);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  struct_decl->scope = push_scope();
  if (struct_decl->fields) {
    struct ListLink* link = list_first_link(struct_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      build_symtable_struct_field(struct_decl->scope, field);
      link = link->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_header_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_DECL);
  struct Ast_HeaderDecl* header_decl = (struct Ast_HeaderDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_HEADER);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  header_decl->scope = push_scope();
  if (header_decl->fields) {
    struct ListLink* link = list_first_link(header_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      build_symtable_struct_field(header_decl->scope, field);
      link = link->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_header_union(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION_DECL);
  struct Ast_HeaderUnionDecl* header_union_decl = (struct Ast_HeaderUnionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_union_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_HEADER_UNION);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  header_union_decl->scope = push_scope();
  if (header_union_decl->fields) {
    struct ListLink* link = list_first_link(header_union_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      build_symtable_struct_field(header_union_decl->scope, field);
      link = link->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_enum_field(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* field = (struct Ast_Name*)ast;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), field->strname);
  if (!entry->ns_general) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(field->strname, OBJECT_ENUM_FIELD);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, field->line_nr);
  } else error("at line %d: name `%s` redeclared.", field->line_nr, field->strname);
}

internal void
build_symtable_specified_id(struct Ast* ast)
{
  assert(ast->kind == AST_SPECIFIED_IDENT);
  struct Ast_SpecifiedIdent* id = (struct Ast_SpecifiedIdent*)ast;
  struct Ast_Name* name = (struct Ast_Name*)id->name;
  build_symtable_enum_field((struct Ast*)name);
  struct Ast* init_expr = id->init_expr;
  if (init_expr) {
    build_symtable_statement(init_expr);
  }
}

internal void
build_symtable_enum_id_list(struct List* id_list)
{
  struct ListLink* link = list_first_link(id_list);
  while (link) {
    struct Ast* id = link->object;
    if (id->kind == AST_NAME) {
      build_symtable_enum_field(id);
    } else if (id->kind == AST_SPECIFIED_IDENT) {
      build_symtable_specified_id(id);
    }
    else assert(0);
    link = link->next;
  }
}

internal void
build_symtable_enum_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ENUM_DECL);
  struct Ast_EnumDecl* enum_decl = (struct Ast_EnumDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)enum_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_ENUM);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  enum_decl->scope = push_scope();
  if (enum_decl->id_list) {
    build_symtable_enum_id_list(enum_decl->id_list);
  }
  pop_scope();
}

internal void
build_symtable_package(struct Ast* ast)
{
  assert(ast->kind == AST_PACKAGE_DECL);
  struct Ast_PackageDecl* package_decl = (struct Ast_PackageDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)package_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_PACKAGE);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_type_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TYPE_DECL);
  struct Ast_TypeDecl* type_decl = (struct Ast_TypeDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_NONE);
    if (type_decl->is_typedef) {
      descriptor->object_kind = OBJECT_TYPEDEF;
    } else {
      descriptor->object_kind = OBJECT_TYPE;
    }
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  struct Ast* type_ref = type_decl->type_ref;
  if (type_ref->kind == AST_STRUCT_DECL) {
    build_symtable_struct_decl(type_ref);
  } else if (type_ref->kind == AST_HEADER_DECL) {
    build_symtable_header_decl(type_ref);
  } else if (type_ref->kind == AST_NAME || type_ref->kind == AST_BASETYPE_BOOL
             || AST_BASETYPE_ERROR || AST_BASETYPE_INT || AST_BASETYPE_BIT
             || AST_BASETYPE_VARBIT || AST_BASETYPE_STRING || AST_BASETYPE_VOID) {
    ; // pass
  }
}

void
build_symtable_const_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONST_DECL);
  struct Ast_ConstDecl* const_decl = (struct Ast_ConstDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)const_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_CONST);
    descriptor->ast = ast;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_function_decl(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_DECL);
  struct Ast_FunctionDecl* function_decl = (struct Ast_FunctionDecl*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)function_decl->proto;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, OBJECT_FUNCTION);
  descriptor->ast = ast;
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  function_decl->scope = push_scope();
  if (function_proto->params) {
    struct ListLink* link = list_first_link(function_proto->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    if (function_body->stmt_list) {
      struct ListLink* link = list_first_link(function_body->stmt_list);
      while (link) {
        struct Ast* stmt = link->object;
        build_symtable_statement(stmt);
        link = link->next;
      }
    }
  }
  pop_scope();
}

internal void
build_symtable_match_kind(struct Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND_DECL);
  struct Ast_MatchKindDecl* decl = (struct Ast_MatchKindDecl*)ast;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_root_scope(), "match_kind");
  assert(entry->ns_type);
  if (decl->id_list) {
    build_symtable_enum_id_list(decl->id_list);
  }
}

internal void
build_symtable_error_decl(struct Ast* ast)
{
  assert (ast->kind == AST_ERROR_DECL);
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_root_scope(), "error");
  assert(entry->ns_type);
}

void
build_symtable_program(struct Ast* ast, struct Arena* symtable_storage_)
{
  assert(ast->kind == AST_P4PROGRAM);
  symtable_storage = symtable_storage_;
  
  symtable_begin_build(symtable_storage);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor("void", OBJECT_VOID), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor("bool", OBJECT_BOOL), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor("int", OBJECT_INT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor("bit", OBJECT_BIT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor("varbit", OBJECT_VARBIT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor("string", OBJECT_STRING), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor("error", OBJECT_ERROR), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor("match_kind", OBJECT_MATCH_KIND), 0);

  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  program->scope = push_scope();
  struct ListLink* link = list_first_link(program->decl_list);
  while (link) {
    struct Ast* decl = link->object;
    if (decl->kind == AST_CONTROL_DECL) {
      build_symtable_control_decl(decl);
    } else if (decl->kind == AST_EXTERN_DECL) {
      build_symtable_extern_decl(decl);
    } else if (decl->kind == AST_STRUCT_DECL) {
      build_symtable_struct_decl(decl);
    } else if (decl->kind == AST_HEADER_DECL) {
      build_symtable_header_decl(decl);
    } else if (decl->kind == AST_HEADER_UNION_DECL) {
      build_symtable_header_union(decl);
    } else if (decl->kind == AST_PACKAGE_DECL) {
      build_symtable_package(decl);
    } else if (decl->kind == AST_PARSER_DECL) {
      build_symtable_parser_decl(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      build_symtable_instantiation(decl);
    } else if (decl->kind == AST_TYPE_DECL) {
      build_symtable_type_decl(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      build_symtable_function_proto(decl);
    } else if (decl->kind == AST_CONST_DECL) {
      build_symtable_const_decl(decl);
    } else if (decl->kind == AST_ENUM_DECL) {
      build_symtable_enum_decl(decl);
    } else if (decl->kind == AST_FUNCTION_DECL) {
      build_symtable_function_decl(decl);
    } else if (decl->kind == AST_ACTION_DECL) {
      build_symtable_action_decl(decl);
    } else if (decl->kind == AST_MATCH_KIND_DECL) {
      build_symtable_match_kind(decl);
    } else if (decl->kind == AST_ERROR_DECL) {
      build_symtable_error_decl(decl);
    }
    else assert(0);
    link = link->next;
  }
  pop_scope();
  symtable_end_build();
}
