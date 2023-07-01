#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal Arena* name_storage;
internal Scope* root_scope;
internal Scope* current_scope;

#if 0
internal void
visit_name_identifier(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  name->scope = current_scope;
}
#endif

#if 0
internal void
visit_type_param(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  NamespaceEntry* ne = scope_lookup_name(current_scope, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)name);
  } else {
    visit_type_ref((Ast*)name);
  }
}
#endif

#if 0
internal void
visit_name_type(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  NamespaceEntry* ne = scope_lookup_name(current_scope, name->strname);
  if (!ne->ns_type) {
    /* Declaration of a type parameter. */
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)name);
  } else {
    visit_expression(ast);
  }
}
#endif

#if 0
internal void
visit_enum_field(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)name);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
}
#endif

internal void
walk_in(Ast* ast)
{
  if (ast->kind == AST_p4program) {
    current_scope = push_scope();
  } else if (ast->kind == AST_errorDeclaration) {
    current_scope = push_scope();
  } else if (ast->kind == AST_matchKindDeclaration) {
    current_scope = push_scope();
  } else if (ast->kind == AST_actionDeclaration) {
    Ast_ActionDeclaration* action_decl = (Ast_ActionDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)action_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_var) {
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)action_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
    current_scope = push_scope();
  } else if (ast->kind == AST_enumDeclaration) {
    Ast_EnumDeclaration* enum_decl = (Ast_EnumDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)enum_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_type) {
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)enum_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
    current_scope = push_scope();
  } else if (ast->kind == AST_constantDeclaration) {
    Ast_ConstDeclaration* const_decl = (Ast_ConstDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)const_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_var) {
      declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)const_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
  } else if (ast->kind == AST_functionPrototype) {
    Ast_FunctionPrototype* func_proto = (Ast_FunctionPrototype*)ast;
    Ast_Name* name = (Ast_Name*)func_proto->name;
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)func_proto);
    current_scope = push_scope();
  } else if (ast->kind == AST_functionDeclaration) {
    Ast_FunctionDeclaration* func_decl = (Ast_FunctionDeclaration*)ast;
    Ast_FunctionPrototype* func_proto = (Ast_FunctionPrototype*)func_decl->proto;
    Ast_Name* name = (Ast_Name*)func_proto->name;
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)func_decl);
    current_scope = push_scope();
  } else if (ast->kind == AST_instantiation) {
    Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
    Ast_Name* name = (Ast_Name*)inst_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_var) {
      declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)inst_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
  } else if (ast->kind == AST_parserDeclaration) {
    Ast_ParserDeclaration* parser_decl = (Ast_ParserDeclaration*)ast;
    Ast_ParserPrototype* parser_proto = (Ast_ParserPrototype*)parser_decl->proto;
    Ast_Name* name = (Ast_Name*)parser_proto->name;
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)parser_decl);
    current_scope = push_scope();
  } else if (ast->kind == AST_packageTypeDeclaration) {
    Ast_PackageTypeDeclaration* package_decl = (Ast_PackageTypeDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)package_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_type) {
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)package_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
    current_scope = push_scope();
  } else if (ast->kind == AST_headerUnionDeclaration) {
    Ast_HeaderUnionDeclaration* union_decl = (Ast_HeaderUnionDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)union_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_type) {
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)union_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
    current_scope = push_scope();
  } else if (ast->kind == AST_headerTypeDeclaration) {
    Ast_HeaderTypeDeclaration* header_decl = (Ast_HeaderTypeDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)header_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_type) {
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)header_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
    current_scope = push_scope();
  } else if (ast->kind == AST_structTypeDeclaration) {
    Ast_StructTypeDeclaration* struct_decl = (Ast_StructTypeDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)struct_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_type) {
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)struct_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
    current_scope = push_scope();
  } else if (ast->kind == AST_externDeclaration) {
    Ast_ExternType* extern_decl = (Ast_ExternType*)ast;
    Ast_Name* name = (Ast_Name*)extern_decl->name;
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)extern_decl);
    current_scope = push_scope();
  } else if (ast->kind == AST_controlDeclaration) {
    Ast_ControlDeclaration* control_decl = (Ast_ControlDeclaration*)ast;
    Ast_ControlPrototype* control_proto = (Ast_ControlPrototype*)control_decl->proto;
    Ast_Name* name = (Ast_Name*)control_proto->name;
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)control_proto);
    current_scope = push_scope();
  } else if (ast->kind == AST_structField) {
    Ast_StructField* field = (Ast_StructField*)ast;
    Ast_Name* name = (Ast_Name*)field->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_var) {
      declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)field);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
  } else if (ast->kind == AST_parserState) {
    Ast_ParserState* state = (Ast_ParserState*)ast;
    Ast_Name* name = (Ast_Name*)state->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_var) {
      declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)state);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
    current_scope = push_scope();
  } else if (ast->kind == AST_tableDeclaration) {
    Ast_TableDeclaration* table_decl = (Ast_TableDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)table_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_var) {
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)table_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
  } else if (ast->kind == AST_variableDeclaration) {
    Ast_VarDeclaration* var_decl = (Ast_VarDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)var_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_var) {
      declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)var_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
  } else if (ast->kind == AST_blockStatement) {
    current_scope = push_scope();
  } else if (ast->kind == AST_parameter) {
    Ast_Parameter* param = (Ast_Parameter*)ast;
    Ast_Name* name = (Ast_Name*)param->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_var) {
      declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)param);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
  } else if (ast->kind == AST_typedefDeclaration) {
    Ast_TypedefDeclaration* type_decl = (Ast_TypedefDeclaration*)ast;
    Ast_Name* name = (Ast_Name*)type_decl->name;
    HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
    NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
    ne->strname = name->strname;
    name_he->object = ne;
    if (!ne->ns_type) {
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)type_decl);
    } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
  }
}

void
walk_out(Ast* ast)
{
  if (ast->kind == AST_p4program) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_errorDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_matchKindDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_actionDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_enumDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_constantDeclaration) {
  } else if (ast->kind == AST_functionPrototype) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_functionDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_instantiation) {
  } else if (ast->kind == AST_parserDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_packageTypeDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_headerUnionDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_headerTypeDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_structTypeDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_externDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_controlDeclaration) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_structField) {
  } else if (ast->kind == AST_parserState) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_tableDeclaration) {
  } else if (ast->kind == AST_variableDeclaration) {
  } else if (ast->kind == AST_blockStatement) {
    current_scope = pop_scope();
  } else if (ast->kind == AST_parameter) {
  } else if (ast->kind == AST_typedefDeclaration) {
  }
}

Scope*
build_name_decl(Ast_P4Program* p4program, Arena* decl_storage_)
{
  name_storage = decl_storage_;
  scope_reset(name_storage);
  root_scope = current_scope = push_scope();

  {
    Ast_Name* void_type = arena_push_struct(name_storage, Ast_Name);
    void_type->kind = AST_name;
    void_type->strname = "void";
    void_type->id = ++p4program->last_node_id;
    declare_type_name(root_scope, void_type->strname, 0, 0, (Ast*)void_type);
  }
  {
    Ast_Name* bool_type = arena_push_struct(name_storage, Ast_Name);
    bool_type->kind = AST_name;
    bool_type->id = ++p4program->last_node_id;
    bool_type->strname = "bool";
    declare_type_name(root_scope, bool_type->strname, 0, 0, (Ast*)bool_type);
  }
  {
    Ast_Name* int_type = arena_push_struct(name_storage, Ast_Name);
    int_type->kind = AST_name;
    int_type->id = ++p4program->last_node_id;
    int_type->strname = "int";
    declare_type_name(root_scope, int_type->strname, 0, 0, (Ast*)int_type);
  }
  {
    Ast_Name* bit_type = arena_push_struct(name_storage, Ast_Name);
    bit_type->kind = AST_name;
    bit_type->id = ++p4program->last_node_id;
    bit_type->strname = "bit";
    declare_type_name(root_scope, bit_type->strname, 0, 0, (Ast*)bit_type);
  }
  {
    Ast_Name* varbit_type = arena_push_struct(name_storage, Ast_Name);
    varbit_type->kind = AST_name;
    varbit_type->id = ++p4program->last_node_id;
    varbit_type->strname = "varbit";
    declare_type_name(root_scope, varbit_type->strname, 0, 0, (Ast*)varbit_type);
  }
  {
    Ast_Name* string_type = arena_push_struct(name_storage, Ast_Name);
    string_type->kind = AST_name;
    string_type->id = ++p4program->last_node_id;
    string_type->strname = "string";
    declare_type_name(root_scope, string_type->strname, 0, 0, (Ast*)string_type);
  }
  {
    Ast_Name* error_type = arena_push_struct(name_storage, Ast_Name);
    error_type->kind = AST_name;
    error_type->id = ++p4program->last_node_id;
    error_type->strname = "error";
    declare_type_name(root_scope, error_type->strname, 0, 0, (Ast*)error_type);
  }
  {
    Ast_Name* match_type = arena_push_struct(name_storage, Ast_Name);
    match_type->kind = AST_name;
    match_type->id = ++p4program->last_node_id;
    match_type->strname = "match_kind";
    declare_type_name(root_scope, match_type->strname, 0, 0, (Ast*)match_type);
  }
  {
    Ast_Name* accept_state = arena_push_struct(name_storage, Ast_Name);
    accept_state->kind = AST_name;
    accept_state->id = ++p4program->last_node_id;
    accept_state->strname = "accept";
    declare_var_name(root_scope, accept_state->strname, 0, 0, (Ast*)accept_state);
  }
  {
    Ast_Name* reject_state = arena_push_struct(name_storage, Ast_Name);
    reject_state->kind = AST_name;
    reject_state->id = ++p4program->last_node_id;
    reject_state->strname = "reject";
    declare_var_name(root_scope, reject_state->strname, 0, 0, (Ast*)reject_state);
  }

  traverse_p4program(p4program, walk_in, walk_out);

  current_scope = pop_scope();
  assert(current_scope == 0);
  return root_scope;
}
