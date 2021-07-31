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
    if (!name_is_declared_in_scope(strname, Symbol_Ident, get_current_scope())) {
      new_ident(strname, decl, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  } else if (decl->kind == Ast_BlockStmt) {
    build_symtable_block_statement(decl);
  } else if (decl->kind == Ast_MethodCallStmt || decl->kind == Ast_AssignmentStmt || decl->kind == Ast_IfStmt ||
             decl->kind == Ast_SwitchStmt || decl->kind == Ast_DirectApplic) {
    ;  // pass
  } else assert(0);
}

internal void
build_symtable_local_control_declarations(struct AstList* local_decls)
{
  push_scope();
  struct AstListLink* link = ast_list_first_link(local_decls);
  while (link) {
    build_symtable_local_control_declaration(link->ast);
    link = link->next;
  }
  pop_scope();
}

internal void
build_symtable_block_statement(struct Ast* block_stmt)
{
  push_scope();
  struct AstList* stmt_list = (struct AstList*)ast_getattr(block_stmt, "stmt_list");
  if (stmt_list) {
    struct AstListLink* link = ast_list_first_link(stmt_list);
    while (link) {
      build_symtable_local_control_declaration(link->ast);
      link = link->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_control(struct Ast* control_decl)
{
  struct Ast* type_decl = (struct Ast*)ast_getattr(control_decl, "type_decl");
  struct Ast* name = (struct Ast*)ast_getattr(type_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Type, get_current_scope())) {
    new_type(strname, type_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  push_scope();
  struct AstList* local_decls = (struct AstList*)ast_getattr(control_decl, "local_decls");
  if (local_decls) {
    build_symtable_local_control_declarations(local_decls);
  }
  struct Ast* apply_stmt = (struct Ast*)ast_getattr(control_decl, "apply_stmt");
  if (apply_stmt) {
    build_symtable_block_statement(apply_stmt);
  }
  pop_scope();
}

internal void
build_symtable_local_parser_element(struct Ast* element)
{
  if (element->kind == Ast_ConstDecl || element->kind == Ast_Instantiation || element->kind == Ast_VarDecl) {
    struct Ast* name = (struct Ast*)ast_getattr(element, "name");
    char* strname = (char*)ast_getattr(name, "name");
    if (!name_is_declared_in_scope(strname, Symbol_Ident, get_current_scope())) {
      new_ident(strname, element, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  } else assert(0);
}

internal void
build_symtable_local_parser_elements(struct AstList* local_elements)
{
  push_scope();
  struct AstListLink* link = ast_list_first_link(local_elements);
  while (link) {
    build_symtable_local_parser_element(link->ast);
    link = link->next;
  }
  pop_scope();
}

internal void
build_symtable_parser(struct Ast* parser_decl)
{
  struct Ast* type_decl = (struct Ast*)ast_getattr(parser_decl, "type_decl");
  struct Ast* name = (struct Ast*)ast_getattr(type_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Type, get_current_scope())) {
    new_type(strname, type_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  push_scope();
  struct AstList* local_elements = (struct AstList*)ast_getattr(parser_decl, "local_elements");
  if (local_elements) {
    build_symtable_local_parser_elements(local_elements);
  }
  pop_scope();
}

void
build_symtable_function_proto(struct Ast* function_proto)
{
  struct Ast* name = (struct Ast*)ast_getattr(function_proto, "name");
  char* strname = (char*)ast_getattr(name, "name");
  new_ident(strname, function_proto, name->line_nr);
}

internal void
build_symtable_extern_method_protos(struct AstList* method_protos)
{
  push_scope();
  struct AstListLink* link = ast_list_first_link(method_protos);
  while (link) {
    build_symtable_function_proto(link->ast);
    link = link->next;
  }
  pop_scope();
}

internal void
build_symtable_extern(struct Ast* extern_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(extern_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Type, get_current_scope())) {
    new_type(strname, extern_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct AstList* method_protos = (struct AstList*)ast_getattr(extern_decl, "method_protos");
  if (method_protos) {
    build_symtable_extern_method_protos(method_protos);
  }
}

internal void
build_symtable_struct_field(struct Ast* field)
{
  struct Ast* name = (struct Ast*)ast_getattr(field, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Ident, get_current_scope())) {
    new_ident(strname, field, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_struct_fields(struct AstList* fields)
{
  push_scope();
  struct AstListLink* link = ast_list_first_link(fields);
  while (link) {
    build_symtable_struct_field(link->ast);
    link = link->next;
  }
  pop_scope();
}

internal void
build_symtable_structlike(struct Ast* struct_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(struct_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Type, get_current_scope())) {
    new_type(strname, struct_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct AstList* fields = (struct AstList*)ast_getattr(struct_decl, "fields");
  if (fields) {
    build_symtable_struct_fields(fields);
  }
}

internal void
build_symtable_enum_id(struct Ast* id)
{
  struct Ast* name = (struct Ast*)ast_getattr(id, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Ident, get_current_scope())) {
    new_ident(strname, id, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_enum_id_list(struct AstList* id_list)
{
  push_scope();
  struct AstListLink* link = ast_list_first_link(id_list);
  while (link) {
    build_symtable_enum_id(link->ast);
    link = link->next;
  }
  pop_scope();
}

internal void
build_symtable_enum(struct Ast* enum_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(enum_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Type, get_current_scope())) {
    new_type(strname, enum_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);

  struct AstList* id_list = (struct AstList*)ast_getattr(enum_decl, "id_list");
  if (id_list) {
    build_symtable_enum_id_list(id_list);
  }
}

internal void
build_symtable_package(struct Ast* package_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(package_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Type, get_current_scope())) {
    new_type(strname, package_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_top_level_instantiation(struct Ast* instantiation)
{
  struct Ast* name = (struct Ast*)ast_getattr(instantiation, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Ident, get_current_scope())) {
    new_ident(strname, instantiation, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

internal void
build_symtable_type_decl(struct Ast* type_decl)
{
  bool is_typedef = *(bool*)ast_getattr(type_decl, "is_typedef");
  if (is_typedef) {
    struct Ast* name = (struct Ast*)ast_getattr(type_decl, "name");
    char* strname = (char*)ast_getattr(name, "name");
    if (!name_is_declared_in_scope(strname, Symbol_Type, get_current_scope())) {
      new_type(strname, type_decl, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
  }
}

void
build_symtable_const_declaration(struct Ast* const_decl)
{
  struct Ast* name = (struct Ast*)ast_getattr(const_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  if (!name_is_declared_in_scope(strname, Symbol_Ident, get_current_scope())) {
    new_ident(strname, const_decl, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, strname);
}

void
build_symtable_program(struct Ast* ast)
{
  if (ast->kind == Ast_P4Program) {
    push_scope();
    struct AstList* decl_list = (struct AstList*)ast_getattr(ast, "decl_list");
    struct AstListLink* link = ast_list_first_link(decl_list);
    while (link) {
      struct Ast* decl = link->ast;
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
  } else assert (0);
}
