#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

struct BuiltinName {
  char* strname;
  enum NameSpace ns;
};

struct BuiltinType {
  char* strname;
  enum TypeEnum ty_former;
};

/** PROGRAM **/

static void visit_p4program(NameBinder* name_binder, Ast* p4program);
static void visit_declarationList(NameBinder* name_binder, Ast* decl_list);
static void visit_declaration(NameBinder* name_binder, Ast* decl);
static void visit_name(NameBinder* name_binder, Ast* name);
static void visit_parameterList(NameBinder* name_binder, Ast* params);
static void visit_parameter(NameBinder* name_binder, Ast* param);
static void visit_packageTypeDeclaration(NameBinder* name_binder, Ast* type_decl);
static void visit_instantiation(NameBinder* name_binder, Ast* inst);

/** PARSER **/

static void visit_parserDeclaration(NameBinder* name_binder, Ast* parser_decl);
static void visit_parserTypeDeclaration(NameBinder* name_binder, Ast* type_decl);
static void visit_parserLocalElements(NameBinder* name_binder, Ast* local_elements);
static void visit_parserLocalElement(NameBinder* name_binder, Ast* local_element);
static void visit_parserStates(NameBinder* name_binder, Ast* states);
static void visit_parserState(NameBinder* name_binder, Ast* state);
static void visit_parserStatements(NameBinder* name_binder, Ast* stmts);
static void visit_parserStatement(NameBinder* name_binder, Ast* stmt);
static void visit_parserBlockStatement(NameBinder* name_binder, Ast* block_stmt);
static void visit_transitionStatement(NameBinder* name_binder, Ast* transition_stmt);
static void visit_stateExpression(NameBinder* name_binder, Ast* state_expr);
static void visit_selectExpression(NameBinder* name_binder, Ast* select_expr);
static void visit_selectCaseList(NameBinder* name_binder, Ast* case_list);
static void visit_selectCase(NameBinder* name_binder, Ast* select_case);
static void visit_keysetExpression(NameBinder* name_binder, Ast* keyset_expr);
static void visit_tupleKeysetExpression(NameBinder* name_binder, Ast* tuple_expr);
static void visit_simpleKeysetExpression(NameBinder* name_binder, Ast* simple_expr);
static void visit_simpleExpressionList(NameBinder* name_binder, Ast* expr_list);

/** CONTROL **/

static void visit_controlDeclaration(NameBinder* name_binder, Ast* control_decl);
static void visit_controlTypeDeclaration(NameBinder* name_binder, Ast* type_decl);
static void visit_controlLocalDeclarations(NameBinder* name_binder, Ast* local_decls);
static void visit_controlLocalDeclaration(NameBinder* name_binder, Ast* local_decl);

/** EXTERN **/

static void visit_externDeclaration(NameBinder* name_binder, Ast* extern_decl);
static void visit_externTypeDeclaration(NameBinder* name_binder, Ast* type_decl);
static void visit_methodPrototypes(NameBinder* name_binder, Ast* protos, NameDeclaration* name_decl);
static void visit_functionPrototype(NameBinder* name_binder, Ast* func_proto);

/** TYPES **/

static void visit_typeRef(NameBinder* name_binder, Ast* type_ref);
static void visit_tupleType(NameBinder* name_binder, Ast* type);
static void visit_headerStackType(NameBinder* name_binder, Ast* type_decl);
static void visit_baseTypeBoolean(NameBinder* name_binder, Ast* bool_type);
static void visit_baseTypeInteger(NameBinder* name_binder, Ast* int_type);
static void visit_baseTypeBit(NameBinder* name_binder, Ast* bit_type);
static void visit_baseTypeVarbit(NameBinder* name_binder, Ast* varbit_type);
static void visit_baseTypeString(NameBinder* name_binder, Ast* str_type);
static void visit_baseTypeVoid(NameBinder* name_binder, Ast* void_type);
static void visit_baseTypeError(NameBinder* name_binder, Ast* error_type);
static void visit_integerTypeSize(NameBinder* name_binder, Ast* type_size);
static void visit_realTypeArg(NameBinder* name_binder, Ast* type_arg);
static void visit_typeArg(NameBinder* name_binder, Ast* type_arg);
static void visit_typeArgumentList(NameBinder* name_binder, Ast* arg_list);
static void visit_typeDeclaration(NameBinder* name_binder, Ast* type_decl);
static void visit_derivedTypeDeclaration(NameBinder* name_binder, Ast* type_decl);
static void visit_headerTypeDeclaration(NameBinder* name_binder, Ast* header_decl);
static void visit_headerUnionDeclaration(NameBinder* name_binder, Ast* union_decl);
static void visit_structTypeDeclaration(NameBinder* name_binder, Ast* struct_decl);
static void visit_structFieldList(NameBinder* name_binder, Ast* field_list, NameDeclaration* name_decl);
static void visit_structField(NameBinder* name_binder, Ast* field);
static void visit_enumDeclaration(NameBinder* name_binder, Ast* enum_decl);
static void visit_errorDeclaration(NameBinder* name_binder, Ast* error_decl);
static void visit_matchKindDeclaration(NameBinder* name_binder, Ast* match_decl);
static int  visit_identifierList(NameBinder* name_binder, Ast* ident_list);
static void visit_specifiedIdentifierList(NameBinder* name_binder, Ast* ident_list, NameDeclaration* name_decl);
static void visit_specifiedIdentifier(NameBinder* name_binder, Ast* ident);
static void visit_typedefDeclaration(NameBinder* name_binder, Ast* typedef_decl);

/** STATEMENTS **/

static void visit_assignmentStatement(NameBinder* name_binder, Ast* assign_stmt);
static void visit_functionCall(NameBinder* name_binder, Ast* func_call);
static void visit_returnStatement(NameBinder* name_binder, Ast* return_stmt);
static void visit_exitStatement(NameBinder* name_binder, Ast* exit_stmt);
static void visit_conditionalStatement(NameBinder* name_binder, Ast* cond_stmt);
static void visit_directApplication(NameBinder* name_binder, Ast* applic_stmt);
static void visit_statement(NameBinder* name_binder, Ast* stmt);
static void visit_blockStatement(NameBinder* name_binder, Ast* block_stmt);
static void visit_statementOrDeclList(NameBinder* name_binder, Ast* stmt_list);
static void visit_switchStatement(NameBinder* name_binder, Ast* switch_stmt);
static void visit_switchCases(NameBinder* name_binder, Ast* switch_cases);
static void visit_switchCase(NameBinder* name_binder, Ast* switch_case);
static void visit_switchLabel(NameBinder* name_binder, Ast* label);
static void visit_statementOrDeclaration(NameBinder* name_binder, Ast* stmt);

/** TABLES **/

static void visit_tableDeclaration(NameBinder* name_binder, Ast* table_decl);
static void visit_tablePropertyList(NameBinder* name_binder, Ast* prop_list);
static void visit_tableProperty(NameBinder* name_binder, Ast* table_prop);
static void visit_keyProperty(NameBinder* name_binder, Ast* key_prop);
static void visit_keyElementList(NameBinder* name_binder, Ast* element_list);
static void visit_keyElement(NameBinder* name_binder, Ast* element);
static void visit_actionsProperty(NameBinder* name_binder, Ast* actions_prop);
static void visit_actionList(NameBinder* name_binder, Ast* action_list);
static void visit_actionRef(NameBinder* name_binder, Ast* action_ref);
static void visit_entriesProperty(NameBinder* name_binder, Ast* entries_prop);
static void visit_entriesList(NameBinder* name_binder, Ast* entries_list);
static void visit_entry(NameBinder* name_binder, Ast* entry);
static void visit_simpleProperty(NameBinder* name_binder, Ast* simple_prop);
static void visit_actionDeclaration(NameBinder* name_binder, Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(NameBinder* name_binder, Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(NameBinder* name_binder, Ast* func_decl);
static void visit_argumentList(NameBinder* name_binder, Ast* arg_list);
static void visit_argument(NameBinder* name_binder, Ast* arg);
static void visit_expressionList(NameBinder* name_binder, Ast* expr_list);
static void visit_lvalueExpression(NameBinder* name_binder, Ast* lvalue_expr);
static void visit_expression(NameBinder* name_binder, Ast* expr);
static void visit_castExpression(NameBinder* name_binder, Ast* cast_expr);
static void visit_unaryExpression(NameBinder* name_binder, Ast* unary_expr);
static void visit_binaryExpression(NameBinder* name_binder, Ast* binary_expr);
static void visit_memberSelector(NameBinder* name_binder, Ast* selector);
static void visit_arraySubscript(NameBinder* name_binder, Ast* subscript);
static void visit_indexExpression(NameBinder* name_binder, Ast* index_expr);
static void visit_booleanLiteral(NameBinder* name_binder, Ast* bool_literal);
static void visit_integerLiteral(NameBinder* name_binder, Ast* int_literal);
static void visit_stringLiteral(NameBinder* name_binder, Ast* str_literal);
static void visit_default(NameBinder* name_binder, Ast* default_);
static void visit_dontcare(NameBinder* name_binder, Ast* dontcare);

static void define_builtin_names(NameBinder* name_binder)
{
  struct BuiltinName builtin_names[] = {
    {"void",   NameSpace::TYPE},
    {"bool",   NameSpace::TYPE},
    {"int",    NameSpace::TYPE},
    {"bit",    NameSpace::TYPE},
    {"varbit", NameSpace::TYPE},
    {"string", NameSpace::TYPE},
    {"error",  NameSpace::TYPE},
    {"match_kind", NameSpace::TYPE},
    {"_",      NameSpace::TYPE},
    {"accept", NameSpace::VAR},
    {"reject", NameSpace::VAR},
  };
  struct BuiltinType builtin_types[] = {
    {"void",       TypeEnum::VOID},
    {"bool",       TypeEnum::BOOL},
    {"int",        TypeEnum::INT},
    {"bit",        TypeEnum::BIT},
    {"varbit",     TypeEnum::VARBIT},
    {"string",     TypeEnum::STRING},
    {"error",      TypeEnum::ERROR},
    {"match_kind", TypeEnum::MATCH_KIND},
    {"_",          TypeEnum::ANY},
  };
  Ast* name;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  Type* ty;

  for (int i = 0; i < sizeof(builtin_names)/sizeof(builtin_names[0]); i++) {
    name = (Ast*)arena_malloc(name_binder->storage, sizeof(Ast));
    name->kind = AST_name;
    name->name.strname = builtin_names[i].strname;
    name_decl = name_binder->root_scope->bind(name_binder->storage, name->name.strname, builtin_names[i].ns);
    name_decl->ast = name;
  }
  for (int i = 0; i < sizeof(builtin_types)/sizeof(builtin_types[0]); i++) {
    name_entry = name_binder->root_scope->lookup(builtin_types[i].strname, NameSpace::TYPE);
    name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
    ty = (Type*)array_append(name_binder->type_array, sizeof(Type));
    ty->ty_former = builtin_types[i].ty_former;
    ty->strname = name_decl->strname;
    ty->ast = name_decl->ast;
    name_decl->type = ty;
  }

  ty = scope_builtin_lookup(name_binder->root_scope, "error", NameSpace::TYPE)->type;
  ty->enum_.fields = (Type*)array_append(name_binder->type_array, sizeof(Type));
  ty->enum_.fields->ty_former = TypeEnum::PRODUCT;

  ty = scope_builtin_lookup(name_binder->root_scope, "match_kind", NameSpace::TYPE)->type;
  ty->enum_.fields = (Type*)array_append(name_binder->type_array, sizeof(Type));
  ty->enum_.fields->ty_former = TypeEnum::PRODUCT;
}

void Debug_scope_decls(Scope* scope)
{
  NameEntry* name_entry;
  NameDeclaration* decl;
  int count = 0;
  StrmapCursor it = {0};
  StrmapEntry* he;
  enum NameSpace ns[] = {NameSpace::VAR, NameSpace::TYPE, NameSpace::KEYWORD};

  it.begin(&scope->name_table);
  printf("Names in scope 0x%x\n\n", scope);
  he = it.next();
  while (he) {
    name_entry = (NameEntry*)he->value;
    for (int i = 0; i < sizeof(ns)/sizeof(ns[0]); i++) {
      decl = name_entry->ns[(int)ns[i] >> 1];
      while (decl) {
        if (ns[i] == NameSpace::KEYWORD) {
          printf("%s, %s\n", decl->strname, NameSpace_to_string(ns[i]));
        } else {
          Ast* ast = decl->ast;
          printf("%s  ...  at %d:%d, %s\n", decl->strname, ast->line_no, ast->column_no, NameSpace_to_string(ns[i]));
        }
        decl = decl->next_in_scope;
        count += 1;
      }
    }
    he = it.next();
  }
  printf("\nTotal names: %d\n", count);
}

char* NameSpace_to_string(enum NameSpace ns)
{
  switch (ns) {
    case NameSpace::VAR: return "VAR";
    case NameSpace::TYPE: return "TYPE";
    case NameSpace::KEYWORD: return "KEYWORD";

    default: return "?";
  }
  assert(0);
  return 0;
}

void name_bind(NameBinder* name_binder)
{
  name_binder->current_scope = name_binder->root_scope;
  name_binder->decl_map = (Map*)arena_malloc(name_binder->storage, sizeof(Map));
  name_binder->decl_map->storage = name_binder->storage;
  name_binder->type_array = (Array*)array_create(name_binder->storage, sizeof(Type), 5);
  define_builtin_names(name_binder);
  visit_p4program(name_binder, name_binder->p4program);
  assert(name_binder->current_scope == name_binder->root_scope);
}

/** PROGRAM **/

static void visit_p4program(NameBinder* name_binder, Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  Scope* prev_scope;

  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, p4program, 0);
  visit_declarationList(name_binder, p4program->p4program.decl_list);
  name_binder->current_scope = prev_scope;
}

static void
visit_declarationList(NameBinder* name_binder, Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_declaration(NameBinder* name_binder, Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    visit_externDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration(name_binder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    visit_instantiation(name_binder, decl->declaration.decl);
  } else assert(0);
}

static void visit_name(NameBinder* name_binder, Ast* name)
{
  MapEntry* m;
  assert(name->kind == AST_name);
  m = map_insert(name_binder->scope_map, name, name_binder->current_scope, 0);
  assert(m);
}

static void visit_parameterList(NameBinder* name_binder, Ast* params)
{
  assert(params->kind == AST_parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_parameter(NameBinder* name_binder, Ast* param)
{
  assert(param->kind == AST_parameter);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(name_binder, param->parameter.type);
  visit_name(name_binder, param->parameter.name);
  name = param->parameter.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::VAR);
  name_decl->ast = param;
  map_insert(name_binder->decl_map, param, name_decl, 0);
  if (param->parameter.init_expr) {
    visit_expression(name_binder, param->parameter.init_expr);
  }
}

static void visit_packageTypeDeclaration(NameBinder* name_binder, Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(name_binder, type_decl->packageTypeDeclaration.name);
  name = type_decl->packageTypeDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = type_decl;
  map_insert(name_binder->decl_map, type_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, type_decl, 0);
  visit_parameterList(name_binder, type_decl->packageTypeDeclaration.params);
  name_binder->current_scope = prev_scope;
}

static void visit_instantiation(NameBinder* name_binder, Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(name_binder, inst->instantiation.type);
  visit_argumentList(name_binder, inst->instantiation.args);
  name = inst->instantiation.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::VAR);
  name_decl->ast = inst;
  map_insert(name_binder->decl_map, inst, name_decl, 0);
}

/** PARSER **/

static void visit_parserDeclaration(NameBinder* name_binder, Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  Scope* prev_scope;

  visit_typeDeclaration(name_binder, parser_decl->parserDeclaration.proto);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, parser_decl, 0);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(name_binder, parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(name_binder, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(name_binder, parser_decl->parserDeclaration.states);
  name_binder->current_scope = prev_scope;
}

static void visit_parserTypeDeclaration(NameBinder* name_binder, Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(name_binder, type_decl->parserTypeDeclaration.name);
  name = type_decl->parserTypeDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = type_decl;
  map_insert(name_binder->decl_map, type_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, type_decl, 0);
  visit_parameterList(name_binder, type_decl->parserTypeDeclaration.params);
  visit_methodPrototypes(name_binder, type_decl->parserTypeDeclaration.method_protos, name_decl);
  name_binder->current_scope = prev_scope;
}

static void visit_parserLocalElements(NameBinder* name_binder, Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_parserLocalElement(NameBinder* name_binder, Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    visit_variableDeclaration(name_binder, local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    visit_instantiation(name_binder, local_element->parserLocalElement.element);
  } else assert(0);
}

static void visit_parserStates(NameBinder* name_binder, Ast* states)
{
  assert(states->kind == AST_parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_parserState(NameBinder* name_binder, Ast* state)
{
  assert(state->kind == AST_parserState);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = state->parserState.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::VAR);
  name_decl->ast = state;
  map_insert(name_binder->decl_map, state, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, state, 0);
  visit_parserStatements(name_binder, state->parserState.stmt_list);
  visit_transitionStatement(name_binder, state->parserState.transition_stmt);
  name_binder->current_scope = prev_scope;
}

static void visit_parserStatements(NameBinder* name_binder, Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_parserStatement(NameBinder* name_binder, Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);

  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(name_binder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visit_functionCall(name_binder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(name_binder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(name_binder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(name_binder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
}

static void visit_parserBlockStatement(NameBinder* name_binder, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  Scope* prev_scope;

  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, block_stmt, 0);
  visit_parserStatements(name_binder, block_stmt->parserBlockStatement.stmt_list);
  name_binder->current_scope = prev_scope;
}

static void visit_transitionStatement(NameBinder* name_binder, Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression(name_binder, transition_stmt->transitionStatement.stmt);
}

static void visit_stateExpression(NameBinder* name_binder, Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    visit_name(name_binder, state_expr->stateExpression.expr);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(name_binder, state_expr->stateExpression.expr);
  } else assert(0);
}

static void visit_selectExpression(NameBinder* name_binder, Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(name_binder, select_expr->selectExpression.expr_list);
  visit_selectCaseList(name_binder, select_expr->selectExpression.case_list);
}

static void visit_selectCaseList(NameBinder* name_binder, Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_selectCase(NameBinder* name_binder, Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(name_binder, select_case->selectCase.keyset_expr);
  visit_name(name_binder, select_case->selectCase.name);
}

static void visit_keysetExpression(NameBinder* name_binder, Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(name_binder, keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(name_binder, keyset_expr->keysetExpression.expr);
  } else assert(0);
}

static void visit_tupleKeysetExpression(NameBinder* name_binder, Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visit_simpleExpressionList(name_binder, tuple_expr->tupleKeysetExpression.expr_list);
}

static void visit_simpleKeysetExpression(NameBinder* name_binder, Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visit_expression(name_binder, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visit_default(name_binder, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visit_dontcare(name_binder, simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

static void visit_simpleExpressionList(NameBinder* name_binder, Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(name_binder, container_of(ast, Ast, tree));
  }
}

/** CONTROL **/

static void visit_controlDeclaration(NameBinder* name_binder, Ast* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  Scope* prev_scope;

  visit_typeDeclaration(name_binder, control_decl->controlDeclaration.proto);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, control_decl, 0);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(name_binder, control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(name_binder, control_decl->controlDeclaration.local_decls);
  visit_blockStatement(name_binder, control_decl->controlDeclaration.apply_stmt);
  name_binder->current_scope = prev_scope;
}

static void visit_controlTypeDeclaration(NameBinder* name_binder, Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(name_binder, type_decl->controlTypeDeclaration.name);
  name = type_decl->controlTypeDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = type_decl;
  map_insert(name_binder->decl_map, type_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, type_decl, 0);
  visit_parameterList(name_binder, type_decl->controlTypeDeclaration.params);
  visit_methodPrototypes(name_binder, type_decl->controlTypeDeclaration.method_protos, name_decl);
  name_binder->current_scope = prev_scope;
}

static void visit_controlLocalDeclarations(NameBinder* name_binder, Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_controlLocalDeclaration(NameBinder* name_binder, Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(name_binder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(name_binder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration(name_binder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    visit_instantiation(name_binder, local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void visit_externDeclaration(NameBinder* name_binder, Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration(name_binder, extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    visit_functionPrototype(name_binder, extern_decl->externDeclaration.decl);
  } else assert(0);
}

static void visit_externTypeDeclaration(NameBinder* name_binder, Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = type_decl->externTypeDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = type_decl;
  map_insert(name_binder->decl_map, type_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, type_decl, 0);
  visit_methodPrototypes(name_binder, type_decl->externTypeDeclaration.method_protos, name_decl);
  name_binder->current_scope = prev_scope;
}

static void visit_methodPrototypes(NameBinder* name_binder, Ast* protos, NameDeclaration* name_decl)
{
  assert(protos->kind == AST_methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_functionPrototype(NameBinder* name_binder, Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(name_binder, func_proto->functionPrototype.name);
  name = func_proto->functionPrototype.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = func_proto;
  map_insert(name_binder->decl_map, func_proto, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, func_proto, 0);
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(name_binder, func_proto->functionPrototype.return_type);
  }
  visit_parameterList(name_binder, func_proto->functionPrototype.params);
  name_binder->current_scope = prev_scope;
}

/** TYPES **/

static void visit_typeRef(NameBinder* name_binder, Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    visit_baseTypeBit(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    visit_baseTypeString(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    visit_baseTypeError(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    visit_name(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(name_binder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(name_binder, type_ref->typeRef.type);
  } else assert(0);
}

static void visit_tupleType(NameBinder* name_binder, Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  visit_typeArgumentList(name_binder, type_decl->tupleType.type_args);
}

static void visit_headerStackType(NameBinder* name_binder, Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  visit_typeRef(name_binder, type_decl->headerStackType.type);
  visit_expression(name_binder, type_decl->headerStackType.stack_expr);
}

static void visit_baseTypeBoolean(NameBinder* name_binder, Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = name_binder->root_scope->lookup("bool", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  map_insert(name_binder->decl_map, bool_type, name_decl, 0);
}

static void visit_baseTypeInteger(NameBinder* name_binder, Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(name_binder, int_type->baseTypeInteger.size);
  }
  name_entry = name_binder->root_scope->lookup("int", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  map_insert(name_binder->decl_map, int_type, name_decl, 0);
}

static void visit_baseTypeBit(NameBinder* name_binder, Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(name_binder, bit_type->baseTypeBit.size);
  }
  name_entry = name_binder->root_scope->lookup("bit", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  map_insert(name_binder->decl_map, bit_type, name_decl, 0);
}

static void visit_baseTypeVarbit(NameBinder* name_binder, Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  visit_integerTypeSize(name_binder, varbit_type->baseTypeVarbit.size);
  name_entry = name_binder->root_scope->lookup("varbit", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  map_insert(name_binder->decl_map, varbit_type, name_decl, 0);
}

static void visit_baseTypeString(NameBinder* name_binder, Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = name_binder->root_scope->lookup("string", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  map_insert(name_binder->decl_map, str_type, name_decl, 0);
}

static void visit_baseTypeVoid(NameBinder* name_binder, Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = name_binder->root_scope->lookup("void", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  map_insert(name_binder->decl_map, void_type, name_decl, 0);
}

static void visit_baseTypeError(NameBinder* name_binder, Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = name_binder->root_scope->lookup("error", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  map_insert(name_binder->decl_map, error_type, name_decl, 0);
}

static void visit_integerTypeSize(NameBinder* name_binder, Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void visit_realTypeArg(NameBinder* name_binder, Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visit_typeRef(name_binder, type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(name_binder, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(NameBinder* name_binder, Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(name_binder, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(name_binder, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(name_binder, type_arg->typeArg.arg);
  } else assert(0);
}

static void visit_typeArgumentList(NameBinder* name_binder, Ast* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_typeDeclaration(NameBinder* name_binder, Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(name_binder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(name_binder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(name_binder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(name_binder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration(name_binder, type_decl->typeDeclaration.decl);
  } else assert(0);
}

static void visit_derivedTypeDeclaration(NameBinder* name_binder, Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(name_binder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(name_binder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(name_binder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(name_binder, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
}

static void visit_headerTypeDeclaration(NameBinder* name_binder, Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = header_decl->headerTypeDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = header_decl;
  map_insert(name_binder->decl_map, header_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, header_decl, 0);
  visit_structFieldList(name_binder, header_decl->headerTypeDeclaration.fields, name_decl);
  name_binder->current_scope = prev_scope;
}

static void visit_headerUnionDeclaration(NameBinder* name_binder, Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = union_decl->headerUnionDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = union_decl;
  map_insert(name_binder->decl_map, union_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, union_decl, 0);
  visit_structFieldList(name_binder, union_decl->headerUnionDeclaration.fields, name_decl);
  name_binder->current_scope = prev_scope;
}

static void visit_structTypeDeclaration(NameBinder* name_binder, Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = struct_decl->structTypeDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = struct_decl;
  map_insert(name_binder->decl_map, struct_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, struct_decl, 0);
  visit_structFieldList(name_binder, struct_decl->structTypeDeclaration.fields, name_decl);
  name_binder->current_scope = prev_scope;
}

static void visit_structFieldList(NameBinder* name_binder, Ast* field_list, NameDeclaration* name_decl)
{
  assert(field_list->kind == AST_structFieldList);
  AstTree* ast;

  for (ast = field_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_structField(NameBinder* name_binder, Ast* field)
{
  assert(field->kind == AST_structField);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(name_binder, field->structField.type);
  name = field->structField.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = field;
  map_insert(name_binder->decl_map, field, name_decl, 0);
}

static void visit_enumDeclaration(NameBinder* name_binder, Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = enum_decl->enumDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = enum_decl;
  map_insert(name_binder->decl_map, enum_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, enum_decl, 0);
  visit_specifiedIdentifierList(name_binder, enum_decl->enumDeclaration.fields, name_decl);
  name_binder->current_scope = prev_scope;
}

static void visit_errorDeclaration(NameBinder* name_binder, Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  Scope* prev_scope;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  Type* error_ty;

  name_entry = name_binder->root_scope->lookup("error", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  error_ty = name_decl->type;
  map_insert(name_binder->decl_map, error_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, error_decl, 0);
  error_ty->enum_.field_count += visit_identifierList(name_binder, error_decl->errorDeclaration.fields);
  name_binder->current_scope = prev_scope;
}

static void visit_matchKindDeclaration(NameBinder* name_binder, Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  Scope* prev_scope;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  Type* match_kind_ty;

  name_entry = name_binder->root_scope->lookup("match_kind", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  match_kind_ty = name_decl->type;
  map_insert(name_binder->decl_map, match_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, match_decl, 0);
  match_kind_ty->enum_.field_count += visit_identifierList(name_binder, match_decl->matchKindDeclaration.fields);
  name_binder->current_scope = prev_scope;
}

static int visit_identifierList(NameBinder* name_binder, Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  Ast* name;
  AstTree* ast;
  NameDeclaration* name_decl;
  int count = 0;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    name = container_of(ast, Ast, tree);
    name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
    name_decl->ast = name;
    map_insert(name_binder->decl_map, name, name_decl, 0);
    count += 1;
  }
  return count;
}

static void visit_specifiedIdentifierList(NameBinder* name_binder, Ast* ident_list, NameDeclaration* name_decl)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_specifiedIdentifier(NameBinder* name_binder, Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  Ast* name;
  NameDeclaration* name_decl;

  name = ident->specifiedIdentifier.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = ident;
  map_insert(name_binder->decl_map, ident, name_decl, 0);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(name_binder, ident->specifiedIdentifier.init_expr);
  }
}

static void visit_typedefDeclaration(NameBinder* name_binder, Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  Ast* name;
  NameDeclaration* name_decl;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(name_binder, typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(name_binder, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  name = typedef_decl->typedefDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = typedef_decl;
  map_insert(name_binder->decl_map, typedef_decl, name_decl, 0);
}

/** STATEMENTS **/

static void visit_assignmentStatement(NameBinder* name_binder, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(name_binder, assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(name_binder, assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(name_binder, assign_stmt->assignmentStatement.rhs_expr);
}

static void visit_functionCall(NameBinder* name_binder, Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(name_binder, func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(name_binder, func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(name_binder, func_call->functionCall.args);
}

static void visit_returnStatement(NameBinder* name_binder, Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(name_binder, return_stmt->returnStatement.expr);
  }
}

static void visit_exitStatement(NameBinder* name_binder, Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
}

static void visit_conditionalStatement(NameBinder* name_binder, Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visit_expression(name_binder, cond_stmt->conditionalStatement.cond_expr);
  visit_statement(name_binder, cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(name_binder, cond_stmt->conditionalStatement.else_stmt);
  }
}

static void visit_directApplication(NameBinder* name_binder, Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visit_name(name_binder, applic_stmt->directApplication.name);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(name_binder, applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(name_binder, applic_stmt->directApplication.args);
}

static void visit_statement(NameBinder* name_binder, Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  Scope* prev_scope;

  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(name_binder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(name_binder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(name_binder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(name_binder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    prev_scope = name_binder->current_scope;
    name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, stmt, 0);
    visit_blockStatement(name_binder, stmt->statement.stmt);
    name_binder->current_scope = prev_scope;
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(name_binder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(name_binder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    visit_switchStatement(name_binder, stmt->statement.stmt);
  } else assert(0);
}

static void visit_blockStatement(NameBinder* name_binder, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList(name_binder, block_stmt->blockStatement.stmt_list);
}

static void visit_statementOrDeclList(NameBinder* name_binder, Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_switchStatement(NameBinder* name_binder, Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(name_binder, switch_stmt->switchStatement.expr);
  visit_switchCases(name_binder, switch_stmt->switchStatement.switch_cases);
}

static void visit_switchCases(NameBinder* name_binder, Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_switchCase(NameBinder* name_binder, Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel(name_binder, switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(name_binder, switch_case->switchCase.stmt);
  }
}

static void visit_switchLabel(NameBinder* name_binder, Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    visit_name(name_binder, label->switchLabel.label);
  } else if (label->switchLabel.label->kind == AST_default) {
    visit_default(name_binder, label->switchLabel.label);
  } else assert(0);
}

static void visit_statementOrDeclaration(NameBinder* name_binder, Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(name_binder, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    visit_statement(name_binder, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    visit_instantiation(name_binder, stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void visit_tableDeclaration(NameBinder* name_binder, Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = table_decl->tableDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = table_decl;
  map_insert(name_binder->decl_map, table_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, table_decl, 0);
  visit_tablePropertyList(name_binder, table_decl->tableDeclaration.prop_list);
  visit_methodPrototypes(name_binder, table_decl->tableDeclaration.method_protos, name_decl);
  name_binder->current_scope = prev_scope;
}

static void visit_tablePropertyList(NameBinder* name_binder, Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_tableProperty(NameBinder* name_binder, Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    visit_keyProperty(name_binder, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    visit_actionsProperty(name_binder, table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    visit_entriesProperty(name_binder, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    visit_simpleProperty(name_binder, table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
}

static void visit_keyProperty(NameBinder* name_binder, Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList(name_binder, key_prop->keyProperty.keyelem_list);
}

static void visit_keyElementList(NameBinder* name_binder, Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_keyElement(NameBinder* name_binder, Ast* element)
{
  assert(element->kind == AST_keyElement);
  Scope* prev_scope;

  visit_expression(name_binder, element->keyElement.expr);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = name_binder->root_scope;
  visit_name(name_binder, element->keyElement.match);
  name_binder->current_scope = prev_scope;
}

static void visit_actionsProperty(NameBinder* name_binder, Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(name_binder, actions_prop->actionsProperty.action_list);
}

static void visit_actionList(NameBinder* name_binder, Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_actionRef(NameBinder* name_binder, Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name(name_binder, action_ref->actionRef.name);
  if (action_ref->actionRef.args) {
    visit_argumentList(name_binder, action_ref->actionRef.args);
  }
}

#if 0
static void visit_entriesProperty(NameBinder* name_binder, Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList(name_binder, entries_prop->entriesProperty.entries_list);
}

static void visit_entriesList(NameBinder* name_binder, Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_entry(NameBinder* name_binder, Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(name_binder, entry->entry.keyset);
  visit_actionRef(name_binder, entry->entry.action);
}

static void visit_simpleProperty(NameBinder* name_binder, Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  Ast* name;
  NameDeclaration* name_decl;

  name = simple_prop->simpleProperty.name;
  name_decl = bind(name_binder->storage, name_binder->current_scope, name->name.strname, TYPE);
  name_decl->ast = simple_prop;
  map_insert(name_binder->decl_map, simple_prop, name_decl, 0);
  visit_expression(name_binder, simple_prop->simpleProperty.init_expr);
}
#endif

static void visit_actionDeclaration(NameBinder* name_binder, Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = action_decl->actionDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = action_decl;
  map_insert(name_binder->decl_map, action_decl, name_decl, 0);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, action_decl, 0);
  visit_parameterList(name_binder, action_decl->actionDeclaration.params);
  visit_blockStatement(name_binder, action_decl->actionDeclaration.stmt);
  name_binder->current_scope = prev_scope;
}

/** VARIABLES **/

static void visit_variableDeclaration(NameBinder* name_binder, Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(name_binder, var_decl->variableDeclaration.type);
  visit_name(name_binder, var_decl->variableDeclaration.name);
  name = var_decl->variableDeclaration.name;
  name_decl = name_binder->current_scope->bind(name_binder->storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = var_decl;
  map_insert(name_binder->decl_map, var_decl, name_decl, 0);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(name_binder, var_decl->variableDeclaration.init_expr);
  }
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(NameBinder* name_binder, Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  Scope* prev_scope;

  visit_functionPrototype(name_binder, func_decl->functionDeclaration.proto);
  prev_scope = name_binder->current_scope;
  name_binder->current_scope = (Scope*)map_lookup(name_binder->scope_map, func_decl, 0);
  visit_blockStatement(name_binder, func_decl->functionDeclaration.stmt);
  name_binder->current_scope = prev_scope;
}

static void visit_argumentList(NameBinder* name_binder, Ast* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_argument(NameBinder* name_binder, Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(name_binder, arg->argument.arg);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(name_binder, arg->argument.arg);
  } else assert(0);
}

static void visit_expressionList(NameBinder* name_binder, Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(name_binder, container_of(ast, Ast, tree));
  }
}

static void visit_lvalueExpression(NameBinder* name_binder, Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visit_name(name_binder, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(name_binder, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(name_binder, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

static void visit_expression(NameBinder* name_binder, Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    visit_name(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(name_binder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(name_binder, expr->expression.expr);
  } else assert(0);
}

static void visit_castExpression(NameBinder* name_binder, Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef(name_binder, cast_expr->castExpression.type);
  visit_expression(name_binder, cast_expr->castExpression.expr);
}

static void visit_unaryExpression(NameBinder* name_binder, Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(name_binder, unary_expr->unaryExpression.operand);
}

static void visit_binaryExpression(NameBinder* name_binder, Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression(name_binder, binary_expr->binaryExpression.left_operand);
  visit_expression(name_binder, binary_expr->binaryExpression.right_operand);
}

static void visit_memberSelector(NameBinder* name_binder, Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(name_binder, selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(name_binder, selector->memberSelector.lhs_expr);
  } else assert(0);
  visit_name(name_binder, selector->memberSelector.name);
}

static void visit_arraySubscript(NameBinder* name_binder, Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(name_binder, subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(name_binder, subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(name_binder, subscript->arraySubscript.index_expr);
}

static void visit_indexExpression(NameBinder* name_binder, Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression(name_binder, index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(name_binder, index_expr->indexExpression.end_index);
  }
}

static void visit_booleanLiteral(NameBinder* name_binder, Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
}

static void visit_integerLiteral(NameBinder* name_binder, Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
}

static void visit_stringLiteral(NameBinder* name_binder, Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
}

static void visit_default(NameBinder* name_binder, Ast* default_)
{
  assert(default_->kind == AST_default);
}

static void visit_dontcare(NameBinder* name_binder, Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
}
