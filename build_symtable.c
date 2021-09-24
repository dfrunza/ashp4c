#include "arena.h"
#include "ast.h"
#include "symtable.h"


internal void build_symtable_block_statement(struct Ast* block_stmt);
internal void build_symtable_statement(struct Ast* decl);


internal void
build_symtable_param(struct Ast* ast)
{
  assert(ast->kind == Ast_Parameter);
  struct Ast_Parameter* param = (struct Ast_Parameter*)ast;
  struct Ast_Name* name = (struct Ast_Name*)param->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_type_param(struct Ast* ast)
{
  assert(ast->kind == Ast_Name);
  struct Ast_Name* type_param = (struct Ast_Name*)ast;
  char* strname = type_param->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, type_param->line_nr);
  };
}

internal void
build_symtable_action_decl(struct Ast* ast) {
  assert(ast->kind == Ast_ActionDecl);
  struct Ast_ActionDecl* action_decl = (struct Ast_ActionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)action_decl->name;
  char* strname = name->strname;
  new_ident(get_current_scope(), strname, ast, name->line_nr);

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
  assert(ast->kind == Ast_Instantiation);
  struct Ast_Instantiation* inst = (struct Ast_Instantiation*)ast;
  struct Ast_Name* name = (struct Ast_Name*)inst->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_table_property(struct Ast* prop)
{
  if (prop->kind == Ast_TableProp_Actions || prop->kind == Ast_TableProp_Entries ||
      prop->kind == Ast_TableProp_SingleEntry || prop->kind == Ast_TableProp_Key) {
    ; // pass
  }
  else assert(0);
}

internal void
build_symtable_table_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_TableDecl);
  struct Ast_TableDecl* decl = (struct Ast_TableDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
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
  assert(ast->kind == Ast_SwitchCase);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  struct Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == Ast_BlockStmt) {
    build_symtable_block_statement(case_stmt);
  }
}

internal void
build_symtable_statement(struct Ast* ast)
{
  if (ast->kind == Ast_VarDecl) {
    struct Ast_VarDecl* decl = (struct Ast_VarDecl*)ast;
    struct Ast_Name* name = (struct Ast_Name*)decl->name;
    char* strname = name->strname;
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
    if (!entry->id_ident) {
      new_ident(get_current_scope(), strname, ast, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  } else if (ast->kind == Ast_ActionDecl) {
    build_symtable_action_decl(ast);
  } else if (ast->kind == Ast_BlockStmt) {
    build_symtable_block_statement(ast);
  } else if (ast->kind == Ast_Instantiation) {
    build_symtable_instantiation(ast);
  } else if (ast->kind == Ast_TableDecl) {
    build_symtable_table_decl(ast);
  } else if (ast->kind == Ast_IfStmt) {
    struct Ast_IfStmt* decl = (struct Ast_IfStmt*)ast;
    struct Ast* if_stmt = decl->stmt;
    build_symtable_statement(if_stmt);
    struct Ast* else_stmt = decl->else_stmt;
    if (else_stmt) {
      build_symtable_statement(else_stmt);
    }
  } else if (ast->kind == Ast_SwitchStmt) {
    struct Ast_SwitchStmt* decl = (struct Ast_SwitchStmt*)ast;
    struct List* switch_cases = decl->switch_cases;
    if (switch_cases) {
      struct ListLink* link = list_first_link(switch_cases);
      while (link) {
        struct Ast* switch_case = link->object;
        build_symtable_switch_case(switch_case);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_MethodCallStmt || ast->kind == Ast_AssignmentStmt ||
             ast->kind == Ast_DirectApplication || ast->kind == Ast_ReturnStmt ||
             ast->kind == Ast_IntLiteral || ast->kind == Ast_BoolLiteral || ast->kind == Ast_StringLiteral) {
    ; // pass
  }
  else assert(0);
}

internal void
build_symtable_block_statement(struct Ast* ast)
{
  assert(ast->kind == Ast_BlockStmt);
  struct Ast_BlockStmt* block_stmt = (struct Ast_BlockStmt*)ast;
  block_stmt->scope = push_scope();
  struct List* stmt_list = block_stmt->stmt_list;
  if (stmt_list) {
    struct ListLink* link = list_first_link(stmt_list);
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
  assert(ast->kind == Ast_ControlDecl);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlType* type_decl = (struct Ast_ControlType*)control_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  struct List* type_params = type_decl->type_params;
  if (type_params) {
    struct ListLink* link = list_first_link(type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  control_decl->scope = push_scope();
  struct List* params = type_decl->params;
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct List* ctor_params = control_decl->ctor_params;
  if (ctor_params) {
    struct ListLink* link = list_first_link(ctor_params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct List* local_decls = control_decl->local_decls;
  if (local_decls) {
    struct ListLink* link = list_first_link(local_decls);
    while (link) {
      struct Ast* decl = link->object;
      build_symtable_statement(decl);
      link = link->next;
    }
  }
  struct Ast* apply_stmt = control_decl->apply_stmt;
  if (apply_stmt) {
    build_symtable_block_statement(apply_stmt);
  }
  pop_scope();
}

internal void
build_symtable_local_parser_element(struct Ast* ast)
{
  if (ast->kind == Ast_ConstDecl || ast->kind == Ast_Instantiation || ast->kind == Ast_VarDecl) {
    struct Ast_Name* name = (struct Ast_Name*)ast->name;
    char* strname = name->strname;
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
    if (!entry->id_ident) {
      new_ident(get_current_scope(), strname, ast, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  }
  else assert(0);
}

internal void
build_symtable_parser_state(struct Ast* ast)
{
  assert(ast->kind == Ast_ParserState);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  struct Ast_Name* name = (struct Ast_Name*)state->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  state->scope = push_scope();
  struct List* stmt_list = state->stmt_list;
  if (stmt_list) {
    struct ListLink* link = list_first_link(stmt_list);
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
  assert(ast->kind == Ast_ParserDecl);
  struct Ast_ParserDecl* parser_decl = (struct Ast_ParserDecl*)ast;
  struct Ast_ParserType* type_decl = (struct Ast_ParserType*)parser_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  struct List* type_params = type_decl->type_params;
  if (type_params) {
    struct ListLink* link = list_first_link(type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  parser_decl->scope = push_scope();
  struct List* params = type_decl->params;
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct List* ctor_params = parser_decl->ctor_params;
  if (ctor_params) {
    struct ListLink* link = list_first_link(ctor_params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct List* local_elements = parser_decl->local_elements;
  if (local_elements) {
    struct ListLink* link = list_first_link(local_elements);
    while (link) {
      struct Ast* element = link->object;
      build_symtable_local_parser_element(element);
      link = link->next;
    }
  }
  struct List* parser_states = parser_decl->states;
  if (parser_states) {
    struct ListLink* link = list_first_link(parser_states);
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
  assert(ast->kind == Ast_FunctionProto);
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)ast;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  char* strname = name->strname;
  new_ident(get_current_scope(), strname, ast, name->line_nr);
  struct List* type_params = function_proto->type_params;
  if (type_params) {
    struct ListLink* link = list_first_link(type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  function_proto->scope = push_scope();
  struct List* params = function_proto->params;
  if (params) {
    struct ListLink* link = list_first_link(params);
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
  assert(ast->kind == Ast_ExternDecl);
  struct Ast_ExternDecl* extern_decl = (struct Ast_ExternDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)extern_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  struct List* type_params = extern_decl->type_params;
  if (type_params) {
    struct ListLink* link = list_first_link(type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  extern_decl->scope = push_scope();
  struct List* method_protos = extern_decl->method_protos;
  if (method_protos) {
    struct ListLink* link = list_first_link(method_protos);
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
  assert(ast->kind == Ast_StructField);
  struct Ast_StructField* field = (struct Ast_StructField*)ast;
  struct Ast_Name* name = (struct Ast_Name*)field->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(struct_scope, strname);
  if (!entry->id_ident) {
    new_ident(struct_scope, strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_struct_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_StructDecl);
  struct Ast_StructDecl* struct_decl = (struct Ast_StructDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)struct_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct_decl->scope = push_scope();
  struct List* fields = struct_decl->fields;
  if (fields) {
    struct ListLink* link = list_first_link(fields);
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
  assert(ast->kind == Ast_HeaderDecl);
  struct Ast_HeaderDecl* header_decl = (struct Ast_HeaderDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  header_decl->scope = push_scope();
  struct List* fields = header_decl->fields;
  if (fields) {
    struct ListLink* link = list_first_link(fields);
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
  assert(ast->kind == Ast_HeaderUnionDecl);
  struct Ast_HeaderUnionDecl* header_union_decl = (struct Ast_HeaderUnionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_union_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  header_union_decl->scope = push_scope();
  struct List* fields = header_union_decl->fields;
  if (fields) {
    struct ListLink* link = list_first_link(fields);
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
  assert(ast->kind == Ast_Name);
  struct Ast_Name* field = (struct Ast_Name*)ast;
  char* strname = field->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, ast, field->line_nr);
  } else error("at line %d: name `%s` redeclared.", field->line_nr, strname);
}

internal void
build_symtable_specified_id(struct Ast* ast)
{
  assert(ast->kind == Ast_SpecifiedIdent);
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
    if (id->kind == Ast_Name) {
      build_symtable_enum_field(id);
    } else if (id->kind == Ast_SpecifiedIdent) {
      build_symtable_specified_id(id);
    }
    else assert(0);
    link = link->next;
  }
}

internal void
build_symtable_enum_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_EnumDecl);
  struct Ast_EnumDecl* enum_decl = (struct Ast_EnumDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)enum_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  enum_decl->scope = push_scope();
  struct List* id_list = enum_decl->id_list;
  if (id_list) {
    build_symtable_enum_id_list(id_list);
  }
  pop_scope();
}

internal void
build_symtable_package(struct Ast* ast)
{
  assert(ast->kind == Ast_PackageDecl);
  struct Ast_PackageDecl* package_decl = (struct Ast_PackageDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)package_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_type_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_TypeDecl);
  struct Ast_TypeDecl* type_decl = (struct Ast_TypeDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  struct Ast* type_ref = type_decl->type_ref;
  if (type_ref->kind == Ast_StructDecl) {
    build_symtable_struct_decl(type_ref);
  } else if (type_ref->kind == Ast_HeaderDecl) {
    build_symtable_header_decl(type_ref);
  } else if (type_ref->kind == Ast_BaseType || type_ref->kind == Ast_Name) {
    ; // pass
  }
}

void
build_symtable_const_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_ConstDecl);
  struct Ast_ConstDecl* const_decl = (struct Ast_ConstDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)const_decl->name;
  char* strname = name->strname;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, ast, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_function_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_FunctionDecl);
  struct Ast_FunctionDecl* function_decl = (struct Ast_FunctionDecl*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)function_decl->proto;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  char* strname = name->strname;
  new_ident(get_current_scope(), strname, ast, name->line_nr);

  function_decl->scope = push_scope();
  struct List* params = function_proto->params;
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    struct List* stmt_list = function_body->stmt_list;
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
build_symtable_match_kind(struct Ast* ast)
{
  assert(ast->kind == Ast_MatchKindDecl);
  struct Ast_MatchKindDecl* decl = (struct Ast_MatchKindDecl*)ast;
  struct List* id_list = decl->id_list;
  if (id_list) {
    build_symtable_enum_id_list(id_list);
  }
}

internal void
build_symtable_error_decl(struct Ast* decl)
{
  ; // TODO
}

void
build_symtable_program(struct Ast* ast)
{
  assert(ast->kind == Ast_P4Program);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  program->scope = push_scope();
  struct List* decl_list = program->decl_list;
  struct ListLink* link = list_first_link(decl_list);
  while (link) {
    struct Ast* decl = link->object;
    if (decl->kind == Ast_ControlDecl) {
      build_symtable_control_decl(decl);
    } else if (decl->kind == Ast_ExternDecl) {
      build_symtable_extern_decl(decl);
    } else if (decl->kind == Ast_StructDecl) {
      build_symtable_struct_decl(decl);
    } else if (decl->kind == Ast_HeaderDecl) {
      build_symtable_header_decl(decl);
    } else if (decl->kind == Ast_HeaderUnionDecl) {
      build_symtable_header_union(decl);
    } else if (decl->kind == Ast_PackageDecl) {
      build_symtable_package(decl);
    } else if (decl->kind == Ast_ParserDecl) {
      build_symtable_parser_decl(decl);
    } else if (decl->kind == Ast_Instantiation) {
      build_symtable_instantiation(decl);
    } else if (decl->kind == Ast_TypeDecl) {
      build_symtable_type_decl(decl);
    } else if (decl->kind == Ast_FunctionProto) {
      build_symtable_function_proto(decl);
    } else if (decl->kind == Ast_ConstDecl) {
      build_symtable_const_decl(decl);
    } else if (decl->kind == Ast_EnumDecl) {
      build_symtable_enum_decl(decl);
    } else if (decl->kind == Ast_FunctionDecl) {
      build_symtable_function_decl(decl);
    } else if (decl->kind == Ast_ActionDecl) {
      build_symtable_action_decl(decl);
    } else if (decl->kind == Ast_MatchKindDecl) {
      build_symtable_match_kind(decl);
    } else if (decl->kind == Ast_ErrorDecl) {
      build_symtable_error_decl(decl);
    }
    else assert(0);
    link = link->next;
  }
  pop_scope();
}
