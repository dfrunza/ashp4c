#include "arena.h"
#include "ast.h"
#include "symtable.h"


internal void build_symtable_block_statement(struct Ast* block_stmt);


internal void
build_symtable_local_control_declaration(struct Ast* decl)
{
  if (decl->kind == Ast_Instantiation || decl->kind == Ast_ActionDecl || decl->kind == Ast_VarDecl ||
      decl->kind == Ast_TableDecl) {
    struct Ast* name = (struct Ast*)ast_getattr(decl, "name");
    char* strname = (char*)ast_getattr(name, "name");
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
    if (!entry->id_ident) {
      new_ident(get_current_scope(), strname, decl, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  } else if (decl->kind == Ast_BlockStmt) {
    build_symtable_block_statement(decl);
  } else if (decl->kind == Ast_MethodCallStmt || decl->kind == Ast_AssignmentStmt || decl->kind == Ast_IfStmt ||
             decl->kind == Ast_SwitchStmt || decl->kind == Ast_DirectApplic) {
    ;  // pass
  } else assert(0);
}

internal void
build_symtable_local_control_declarations(struct List* local_decls)
{
  struct ListLink* link = list_first_link(local_decls);
  while (link) {
    build_symtable_local_control_declaration(link->object);
    link = link->next;
  }
}

internal void
build_symtable_block_statement(struct Ast* block_stmt)
{
  struct List* stmt_list = (struct List*)ast_getattr(block_stmt, "stmt_list");
  if (stmt_list) {
    struct Scope* stmt_scope = push_scope();
    ast_setattr(block_stmt, "stmt_scope", stmt_scope, AstAttr_Scope);
    struct ListLink* link = list_first_link(stmt_list);
    while (link) {
      build_symtable_local_control_declaration(link->object);
      link = link->next;
    }
    pop_scope();
  }
}

internal void
build_symtable_control(struct Ast* control_decl)
{
  struct Ast* type_decl = (struct Ast*)ast_getattr(control_decl, "type_decl");
  struct Ast* name = (struct Ast*)ast_getattr(type_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, type_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* local_decls = (struct List*)ast_getattr(control_decl, "local_decls");
  if (local_decls) {
    struct Scope* local_decls_scope = push_scope();
    ast_setattr(control_decl, "local_decls_scope", local_decls_scope, AstAttr_Scope);
    build_symtable_local_control_declarations(local_decls);
    pop_scope();
  }
  struct Ast* apply_stmt = (struct Ast*)ast_getattr(control_decl, "apply_stmt");
  if (apply_stmt) {
    build_symtable_block_statement(apply_stmt);
  }
}

internal void
build_symtable_local_parser_element(struct Ast* element)
{
  if (element->kind == Ast_ConstDecl || element->kind == Ast_Instantiation || element->kind == Ast_VarDecl) {
    struct Ast* name = (struct Ast*)ast_getattr(element, "name");
    char* strname = (char*)ast_getattr(name, "name");
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
    if (!entry->id_ident) {
      new_ident(get_current_scope(), strname, element, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  } else assert(0);
}

internal void
build_symtable_local_parser_elements(struct List* local_elements)
{
  struct ListLink* link = list_first_link(local_elements);
  while (link) {
    build_symtable_local_parser_element(link->object);
    link = link->next;
  }
}

internal void
build_symtable_parser(struct Ast* parser_decl)
{
  struct Ast* type_decl = (struct Ast*)ast_getattr(parser_decl, "type_decl");
  struct Ast* name = (struct Ast*)ast_getattr(type_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, type_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* local_elements = (struct List*)ast_getattr(parser_decl, "local_elements");
  if (local_elements) {
    struct Scope* local_elements_scope = push_scope();
    ast_setattr(parser_decl, "local_elements_scope", local_elements_scope, AstAttr_Scope);
    build_symtable_local_parser_elements(local_elements);
    pop_scope();
  }
}

void
build_symtable_function_proto(struct Ast* function_proto)
{
  struct Ast* name = (struct Ast*)ast_getattr(function_proto, "name");
  char* strname = (char*)ast_getattr(name, "name");
  new_ident(get_current_scope(), strname, function_proto, name->line_nr);
}

internal void
build_symtable_extern_method_protos(struct List* method_protos)
{
  struct ListLink* link = list_first_link(method_protos);
  while (link) {
    build_symtable_function_proto(link->object);
    link = link->next;
  }
}

internal void
build_symtable_extern(struct Ast* extern_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(extern_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, extern_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* method_protos = (struct List*)ast_getattr(extern_decl, "method_protos");
  if (method_protos) {
    struct Scope* methods_scope = push_scope();
    ast_setattr(extern_decl, "methods_scope", methods_scope, AstAttr_Scope);
    build_symtable_extern_method_protos(method_protos);
    pop_scope();
  }
}

internal void
build_symtable_struct_field(struct Ast* field)
{
  struct Ast* name = (struct Ast*)ast_getattr(field, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, field, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_struct_fields(struct List* fields)
{
  struct ListLink* link = list_first_link(fields);
  while (link) {
    build_symtable_struct_field(link->object);
    link = link->next;
  }
}

internal void
build_symtable_structlike(struct Ast* struct_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(struct_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, struct_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* fields = (struct List*)ast_getattr(struct_decl, "fields");
  if (fields) {
    struct Scope* fields_scope = push_scope();
    ast_setattr(struct_decl, "fields_scope", fields_scope, AstAttr_Scope);
    build_symtable_struct_fields(fields);
    pop_scope();
  }
}

internal void
build_symtable_enum_id(struct Ast* id)
{
  struct Ast* name = (struct Ast*)ast_getattr(id, "name");
  char* strname = (char*)ast_getattr(name, "name");
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
    build_symtable_enum_id(link->object);
    link = link->next;
  }
}

internal void
build_symtable_enum(struct Ast* enum_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(enum_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, enum_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct List* id_list = (struct List*)ast_getattr(enum_decl, "id_list");
  if (id_list) {
    struct Scope* id_scope = push_scope();
    ast_setattr(enum_decl, "id_scope", id_scope, AstAttr_Scope);
    build_symtable_enum_id_list(id_list);
    pop_scope();
  }
}

internal void
build_symtable_package(struct Ast* package_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(package_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_type) {
    new_type(get_current_scope(), strname, package_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_top_level_instantiation(struct Ast* instantiation)
{
  struct Ast* name = (struct Ast*)ast_getattr(instantiation, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, instantiation, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_type_decl(struct Ast* type_decl)
{
  bool is_typedef = *(bool*)ast_getattr(type_decl, "is_typedef");
  if (is_typedef) {
    struct Ast* name = (struct Ast*)ast_getattr(type_decl, "name");
    char* strname = (char*)ast_getattr(name, "name");
    struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
    if (!entry->id_type) {
      new_type(get_current_scope(), strname, type_decl, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  }
}

void
build_symtable_const_declaration(struct Ast* const_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(const_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  struct SymtableEntry* entry = get_symtable_entry(get_current_scope(), strname);
  if (!entry->id_ident) {
    new_ident(get_current_scope(), strname, const_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

void
build_symtable_program(struct Ast* program)
{
  if (program->kind == Ast_P4Program) {
    struct Scope* scope = push_scope();
    ast_setattr(program, "scope", scope, AstAttr_Scope);
    struct List* decl_list = (struct List*)ast_getattr(program, "decl_list");
    struct ListLink* link = list_first_link(decl_list);
    while (link) {
      struct Ast* decl = link->object;
      if (decl->kind == Ast_Control) {
        build_symtable_control(decl);
      } else if (decl->kind == Ast_ExternDecl) {
        build_symtable_extern(decl);
      } else if (decl->kind == Ast_StructDecl || decl->kind == Ast_HeaderDecl) {
        build_symtable_structlike(decl);
      } else if (decl->kind == Ast_Package) {
        build_symtable_package(decl);
      } else if (decl->kind == Ast_Parser) {
        build_symtable_parser(decl);
      } else if (decl->kind == Ast_Instantiation) {
        build_symtable_top_level_instantiation(decl);
      } else if (decl->kind == Ast_TypeDecl) {
        build_symtable_type_decl(decl);
      } else if (decl->kind == Ast_FunctionProto) {
        build_symtable_function_proto(decl);
      } else if (decl->kind == Ast_ConstDecl) {
        build_symtable_const_declaration(decl);
      } else if (decl->kind == Ast_EnumDecl) {
        build_symtable_enum(decl);
      }
      else assert(0);
      link = link->next;
    }
    pop_scope();
  }
  else assert (0);
}
