#include "arena.h"
#include "ast.h"
#include "symtable.h"


internal void build_symtable_block_statement(struct Ast* block_stmt);


internal void
build_symtable_param(struct Ast* param)
{
  assert(param->kind == Ast_Parameter);
  struct Ast* name = ast_getattr(param, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, param, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_statement(struct Ast* decl)
{
  if (decl->kind == Ast_ActionDecl || decl->kind == Ast_VarDecl || decl->kind == Ast_TableDecl) {
    struct Ast* name = ast_getattr(decl, "name");
    char* strname = ast_getattr(name, "name");
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
    if (!entry->id_ident) {
      new_ident(get_current_scope(), strname, decl, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  } else if (decl->kind == Ast_BlockStmt) {
    build_symtable_block_statement(decl);
  } else if (decl->kind == Ast_MethodCallStmt || decl->kind == Ast_AssignmentStmt || decl->kind == Ast_IfStmt ||
             decl->kind == Ast_SwitchStmt || decl->kind == Ast_DirectApplication || decl->kind == Ast_ReturnStmt) {
    ; // pass
  }
  else assert(0);
}

internal void
build_symtable_block_statement(struct Ast* block_stmt)
{
  assert(block_stmt->kind == Ast_BlockStmt);
  struct List* stmt_list = ast_getattr(block_stmt, "stmt_list");
  if (stmt_list) {
    block_stmt->scope = push_scope();
    struct ListLink* link = list_first_link(stmt_list);
    while (link) {
      struct Ast* decl = link->object;
      build_symtable_statement(decl);
      link = link->next;
    }
    pop_scope();
  }
}

internal void
build_symtable_control_decl(struct Ast* control_decl)
{
  assert(control_decl->kind == Ast_ControlDecl);
  struct Ast* type_decl = ast_getattr(control_decl, "type_decl");
  struct Ast* name = ast_getattr(type_decl, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, type_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  control_decl->scope = push_scope();
  struct List* params = ast_getattr(type_decl, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }

  struct List* local_decls = ast_getattr(control_decl, "local_decls");
  if (local_decls) {
    struct ListLink* link = list_first_link(local_decls);
    while (link) {
      struct Ast* decl = link->object;
      build_symtable_statement(decl);
      link = link->next;
    }
  }
  struct Ast* apply_stmt = ast_getattr(control_decl, "apply_stmt");
  if (apply_stmt) {
    build_symtable_block_statement(apply_stmt);
  }
  pop_scope();
}

internal void
build_symtable_local_parser_element(struct Ast* element)
{
  if (element->kind == Ast_ConstDecl || element->kind == Ast_Instantiation || element->kind == Ast_VarDecl) {
    struct Ast* name = ast_getattr(element, "name");
    char* strname = ast_getattr(name, "name");
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
    if (!entry->id_ident) {
      new_ident(get_current_scope(), strname, element, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  }
  else assert(0);
}

internal void
build_symtable_parser_state(struct Ast* state)
{
  assert(state->kind == Ast_ParserState);
  struct Ast* name = ast_getattr(state, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, state, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* stmt_list = ast_getattr(state, "stmt_list");
  if (stmt_list) {
    state->scope = push_scope();
    struct ListLink* link = list_first_link(stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      build_symtable_statement(stmt);
      link = link->next;
    }
    pop_scope();
  }
}

internal void
build_symtable_parser_decl(struct Ast* parser_decl)
{
  assert(parser_decl->kind == Ast_ParserDecl);
  struct Ast* type_decl = ast_getattr(parser_decl, "type_decl");
  struct Ast* name = ast_getattr(type_decl, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, type_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  parser_decl->scope = push_scope();
  struct List* params = ast_getattr(type_decl, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct List* local_elements = ast_getattr(parser_decl, "local_elements");
  if (local_elements) {
    struct ListLink* link = list_first_link(local_elements);
    while (link) {
      struct Ast* element = link->object;
      build_symtable_local_parser_element(element);
      link = link->next;
    }
  }
  struct List* parser_states = ast_getattr(parser_decl, "states");
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
build_symtable_function_proto(struct Ast* function_proto)
{
  assert(function_proto->kind == Ast_FunctionProto);
  struct Ast* name = ast_getattr(function_proto, "name");
  char* strname = ast_getattr(name, "name");
  new_ident(get_current_scope(), strname, function_proto, name->line_nr);
  struct List* params = ast_getattr(function_proto, "params");
  if (params) {
    function_proto->scope = push_scope();
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
    pop_scope();
  }
}

internal void
build_symtable_extern_decl(struct Ast* extern_decl)
{
  assert(extern_decl->kind == Ast_ExternDecl);
  struct Ast* name = ast_getattr(extern_decl, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, extern_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* method_protos = ast_getattr(extern_decl, "method_protos");
  if (method_protos) {
    extern_decl->scope = push_scope();
    struct ListLink* link = list_first_link(method_protos);
    while (link) {
      struct Ast* proto = link->object;
      build_symtable_function_proto(proto);
      link = link->next;
    }
    pop_scope();
  }
}

internal void
build_symtable_struct_field(struct Scope* struct_scope, struct Ast* field)
{
  assert(field->kind == Ast_StructField);
  struct Ast* name = ast_getattr(field, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(struct_scope, strname);
  if (!entry->id_ident) {
    new_ident(struct_scope, strname, field, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_structlike_decl(struct Ast* struct_decl)
{
  assert(struct_decl->kind == Ast_StructDecl || struct_decl->kind == Ast_HeaderDecl);
  struct Ast* name = ast_getattr(struct_decl, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, struct_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* fields = ast_getattr(struct_decl, "fields");
  if (fields) {
    struct_decl->scope = new_scope(4);
    struct ListLink* link = list_first_link(fields);
    while (link) {
      struct Ast* field = link->object;
      build_symtable_struct_field(struct_decl->scope, field);
      link = link->next;
    }
  }
}

internal void
build_symtable_enum_id(struct Ast* id)
{
  assert(id->kind == Ast_Name);
  struct Ast* name = ast_getattr(id, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, id, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_enum_id_list(struct List* id_list)
{
  struct ListLink* link = list_first_link(id_list);
  while (link) {
    struct Ast* id = link->object;
    build_symtable_enum_id(id);
    link = link->next;
  }
}

internal void
build_symtable_enum_decl(struct Ast* enum_decl)
{
  assert(enum_decl->kind == Ast_EnumDecl);
  struct Ast* name = ast_getattr(enum_decl, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, enum_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* id_list = ast_getattr(enum_decl, "id_list");
  if (id_list) {
    enum_decl->scope = push_scope();
    build_symtable_enum_id_list(id_list);
    pop_scope();
  }
}

internal void
build_symtable_package(struct Ast* package_decl)
{
  assert(package_decl->kind == Ast_PackageDecl);
  struct Ast* name = ast_getattr(package_decl, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, package_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_instantiation(struct Ast* instantiation)
{
  assert(instantiation->kind == Ast_Instantiation);
  struct Ast* name = ast_getattr(instantiation, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, instantiation, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_type_decl(struct Ast* type_decl)
{
  assert(type_decl->kind == Ast_TypeDecl);
  bool is_typedef = *(bool*)ast_getattr(type_decl, "is_typedef");
  if (is_typedef) {
    struct Ast* name = ast_getattr(type_decl, "name");
    char* strname = ast_getattr(name, "name");
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
    if (!entry->id_type) {
      new_type(get_current_scope(), strname, type_decl, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  }
}

void
build_symtable_const_decl(struct Ast* const_decl)
{
  assert(const_decl->kind == Ast_ConstDecl);
  struct Ast* name = ast_getattr(const_decl, "name");
  char* strname = ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, const_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_function_decl(struct Ast* function_decl)
{
  assert(function_decl->kind == Ast_FunctionDecl);
  struct Ast* function_proto = ast_getattr(function_decl, "proto");
  struct Ast* name = ast_getattr(function_proto, "name");
  char* strname = ast_getattr(name, "name");
  new_ident(get_current_scope(), strname, function_proto, name->line_nr);

  function_decl->scope = push_scope();
  struct List* params = ast_getattr(function_proto, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct Ast* function_body = ast_getattr(function_decl, "stmt");
  struct List* stmt_list = ast_getattr(function_body, "stmt_list");
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

void
build_symtable_program(struct Ast* program)
{
  assert(program->kind == Ast_P4Program);
  program->scope = push_scope();
  struct List* decl_list = ast_getattr(program, "decl_list");
  struct ListLink* link = list_first_link(decl_list);
  while (link) {
    struct Ast* decl = link->object;
    if (decl->kind == Ast_ControlDecl) {
      build_symtable_control_decl(decl);
    } else if (decl->kind == Ast_ExternDecl) {
      build_symtable_extern_decl(decl);
    } else if (decl->kind == Ast_StructDecl || decl->kind == Ast_HeaderDecl) {
      build_symtable_structlike_decl(decl);
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
    }
    else assert(0);
    link = link->next;
  }
  pop_scope();
}
