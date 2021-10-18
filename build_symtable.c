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

internal struct ObjectDescriptor*
new_type_descriptor(char* name, enum ObjectKind type_kind)
{
  struct ObjectDescriptor* descriptor = arena_push(symtable_storage, sizeof(*descriptor));
  memset(descriptor, 0, sizeof(*descriptor));
  descriptor->name = name;
  descriptor->type_kind = type_kind;
  return descriptor;
}

internal void
build_symtable_param(struct Ast* ast)
{
  assert(ast->kind == Ast_Parameter);
  struct Ast_Parameter* param = (struct Ast_Parameter*)ast;
  struct Ast_Name* name = (struct Ast_Name*)param->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_ident) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, Object_Variable);
    descriptor->ast = ast;
    register_identifier(get_current_scope(), descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_type_param(struct Ast* ast)
{
  assert(ast->kind == Ast_Name);
  struct Ast_Name* type_param = (struct Ast_Name*)ast;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), type_param->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(type_param->strname, Type_TypeParam);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, type_param->line_nr);
  }
}

internal void
build_symtable_action_decl(struct Ast* ast) {
  assert(ast->kind == Ast_ActionDecl);
  struct Ast_ActionDecl* action_decl = (struct Ast_ActionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)action_decl->name;
  struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, Object_Action);
  descriptor->ast = ast;
  register_identifier(get_current_scope(), descriptor, name->line_nr);
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
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_ident) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, Object_Instantiation);
    descriptor->ast = ast;
    register_identifier(get_current_scope(), descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
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
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_ident) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, Object_Table);
    descriptor->ast = ast;
    register_identifier(get_current_scope(), descriptor, name->line_nr);
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
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
    if (!entry->id_ident) {
      struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, Object_Variable);
      descriptor->ast = ast;
      register_identifier(get_current_scope(), descriptor, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
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
    if (decl->switch_cases) {
      struct ListLink* link = list_first_link(decl->switch_cases);
      while (link) {
        struct Ast* switch_case = link->object;
        build_symtable_switch_case(switch_case);
        link = link->next;
      }
    }
  } else if (ast->kind == Ast_MethodCallStmt || ast->kind == Ast_AssignmentStmt ||
             ast->kind == Ast_DirectApplication || ast->kind == Ast_ReturnStmt ||
             ast->kind == Ast_IntLiteral || ast->kind == Ast_BoolLiteral || ast->kind == Ast_StringLiteral ||
             ast->kind == Ast_ExitStmt) {
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
  assert(ast->kind == Ast_ControlDecl);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlType* type_decl = (struct Ast_ControlType*)control_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Control);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
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
  if (ast->kind == Ast_ConstDecl || ast->kind == Ast_Instantiation || ast->kind == Ast_VarDecl) {
    struct Ast_Name* name = (struct Ast_Name*)ast->name;
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
    if (!entry->id_ident) {
      struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, Object_NONE);
      if (ast->kind == Ast_ConstDecl) {
        descriptor->object_kind = Object_Constant;
      } else if (ast->kind == Ast_VarDecl) {
        descriptor->object_kind = Object_Variable;
      } else if (ast->kind == Ast_Instantiation) {
        descriptor->object_kind = Object_Instantiation;
      } else assert(0);
      descriptor->ast = ast;
      register_identifier(get_current_scope(), descriptor, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  }
  else assert(0);
}

internal void
build_symtable_parser_state(struct Ast* ast)
{
  assert(ast->kind == Ast_ParserState);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  struct Ast_Name* name = (struct Ast_Name*)state->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Object_ParserState);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
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
  assert(ast->kind == Ast_ParserDecl);
  struct Ast_ParserDecl* parser_decl = (struct Ast_ParserDecl*)ast;
  struct Ast_ParserType* type_decl = (struct Ast_ParserType*)parser_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Parser);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
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
  assert(ast->kind == Ast_FunctionProto);
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)ast;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Function);
  descriptor->ast = ast;
  register_type(get_current_scope(), descriptor, name->line_nr);
  function_proto->scope = push_scope();
  if (function_proto->return_type) {
    if (function_proto->return_type->kind == Ast_Name) {
      struct Ast_Name* return_type = (struct Ast_Name*)function_proto->return_type;
      struct SymtableEntry* entry = scope_resolve_name(get_current_scope(), return_type->strname);
      if (!entry->id_type) {
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
  assert(ast->kind == Ast_ExternDecl);
  struct Ast_ExternDecl* extern_decl = (struct Ast_ExternDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)extern_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Extern);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
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
  assert(ast->kind == Ast_StructField);
  struct Ast_StructField* field = (struct Ast_StructField*)ast;
  struct Ast_Name* name = (struct Ast_Name*)field->name;
  struct SymtableEntry* entry = get_symtable_entry(struct_scope, name->strname);
  if (!entry->id_ident) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, Object_Variable);
    descriptor->ast = ast;
    register_identifier(struct_scope, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_struct_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_StructDecl);
  struct Ast_StructDecl* struct_decl = (struct Ast_StructDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)struct_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Struct);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
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
  assert(ast->kind == Ast_HeaderDecl);
  struct Ast_HeaderDecl* header_decl = (struct Ast_HeaderDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Header);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
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
  assert(ast->kind == Ast_HeaderUnionDecl);
  struct Ast_HeaderUnionDecl* header_union_decl = (struct Ast_HeaderUnionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_union_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_HeaderUnion);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
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
  assert(ast->kind == Ast_Name);
  struct Ast_Name* field = (struct Ast_Name*)ast;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), field->strname);
  if (!entry->id_ident) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(field->strname, Object_Variable);
    descriptor->ast = ast;
    register_identifier(get_current_scope(), descriptor, field->line_nr);
  } else error("at line %d: name `%s` redeclared.", field->line_nr, field->strname);
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
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Enum);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
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
  assert(ast->kind == Ast_PackageDecl);
  struct Ast_PackageDecl* package_decl = (struct Ast_PackageDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)package_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Package);
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_type_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_TypeDecl);
  struct Ast_TypeDecl* type_decl = (struct Ast_TypeDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_type) {
    struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_NONE);
    if (type_decl->is_typedef) {
      descriptor->object_kind = Type_Typedef;
    } else {
      descriptor->object_kind = Type_Type;
    }
    descriptor->ast = ast;
    register_type(get_current_scope(), descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  struct Ast* type_ref = type_decl->type_ref;
  if (type_ref->kind == Ast_StructDecl) {
    build_symtable_struct_decl(type_ref);
  } else if (type_ref->kind == Ast_HeaderDecl) {
    build_symtable_header_decl(type_ref);
  } else if (type_ref->kind == Ast_Name || type_ref->kind == Ast_BaseType_Bool
             || Ast_BaseType_Error || Ast_BaseType_Int || Ast_BaseType_Bit
             || Ast_BaseType_Varbit || Ast_BaseType_String || Ast_BaseType_Void) {
    ; // pass
  }
}

void
build_symtable_const_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_ConstDecl);
  struct Ast_ConstDecl* const_decl = (struct Ast_ConstDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)const_decl->name;
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), name->strname);
  if (!entry->id_ident) {
    struct ObjectDescriptor* descriptor = new_object_descriptor(name->strname, Object_Constant);
    descriptor->ast = ast;
    register_identifier(get_current_scope(), descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
}

internal void
build_symtable_function_decl(struct Ast* ast)
{
  assert(ast->kind == Ast_FunctionDecl);
  struct Ast_FunctionDecl* function_decl = (struct Ast_FunctionDecl*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)function_decl->proto;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  struct ObjectDescriptor* descriptor = new_type_descriptor(name->strname, Type_Function);
  descriptor->ast = ast;
  register_type(get_current_scope(), descriptor, name->line_nr);
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
  assert(ast->kind == Ast_MatchKindDecl);
  struct Ast_MatchKindDecl* decl = (struct Ast_MatchKindDecl*)ast;
  struct SymtableEntry* entry = get_symtable_entry(get_root_scope(), "match_kind");
  assert (entry->id_ident && entry->id_type);
  if (decl->id_list) {
    build_symtable_enum_id_list(decl->id_list);
  }
}

internal void
build_symtable_error_decl(struct Ast* ast)
{
  assert (ast->kind == Ast_ErrorDecl);
  struct SymtableEntry* entry = get_symtable_entry(get_root_scope(), "error");
  assert (entry->id_ident && entry->id_type);
}

void
build_symtable_program(struct Ast* ast, struct Arena* symtable_storage_)
{
  assert(ast->kind == Ast_P4Program);
  symtable_storage = symtable_storage_;
  
  register_type(get_root_scope(), new_object_descriptor("void", Type_Void), 0);
  register_type(get_root_scope(), new_object_descriptor("bool", Type_Bool), 0);
  register_type(get_root_scope(), new_object_descriptor("int", Type_Int), 0);
  register_type(get_root_scope(), new_object_descriptor("bit", Type_Bit), 0);
  register_type(get_root_scope(), new_object_descriptor("varbit", Type_Varbit), 0);
  register_type(get_root_scope(), new_object_descriptor("string", Type_String), 0);
  register_type(get_root_scope(), new_object_descriptor("error", Type_Error), 0);
  register_type(get_root_scope(), new_object_descriptor("match_kind", Type_MatchKind), 0);

  register_identifier(get_root_scope(), new_object_descriptor("accept", Object_ParserState), 0);
  register_identifier(get_root_scope(), new_object_descriptor("reject", Object_ParserState), 0);
  register_identifier(get_root_scope(), new_object_descriptor("error", Object_Error), 0);
  register_identifier(get_root_scope(), new_object_descriptor("match_kind", Object_MatchKind), 0);

  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  program->scope = push_scope();
  struct ListLink* link = list_first_link(program->decl_list);
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
