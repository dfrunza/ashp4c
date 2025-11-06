#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

/** PROGRAM **/

static void visit_p4program(BuiltinMethodBuilder* builder, Ast* p4program);
static void visit_declarationList(BuiltinMethodBuilder* builder, Ast* decl_list);
static void visit_declaration(BuiltinMethodBuilder* builder, Ast* decl);
static void visit_name(BuiltinMethodBuilder* builder, Ast* name);
static void visit_parameterList(BuiltinMethodBuilder* builder, Ast* params);
static void visit_parameter(BuiltinMethodBuilder* builder, Ast* param);
static void visit_packageTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl);
static void visit_instantiation(BuiltinMethodBuilder* builder, Ast* inst);

/** PARSER **/

static void visit_parserDeclaration(BuiltinMethodBuilder* builder, Ast* parser_decl);
static void visit_parserTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl);
static void visit_parserLocalElements(BuiltinMethodBuilder* builder, Ast* local_elements);
static void visit_parserLocalElement(BuiltinMethodBuilder* builder, Ast* local_element);
static void visit_parserStates(BuiltinMethodBuilder* builder, Ast* states);
static void visit_parserState(BuiltinMethodBuilder* builder, Ast* state);
static void visit_parserStatements(BuiltinMethodBuilder* builder, Ast* stmts);
static void visit_parserStatement(BuiltinMethodBuilder* builder, Ast* stmt);
static void visit_parserBlockStatement(BuiltinMethodBuilder* builder, Ast* block_stmt);
static void visit_transitionStatement(BuiltinMethodBuilder* builder, Ast* transition_stmt);
static void visit_stateExpression(BuiltinMethodBuilder* builder, Ast* state_expr);
static void visit_selectExpression(BuiltinMethodBuilder* builder, Ast* select_expr);
static void visit_selectCaseList(BuiltinMethodBuilder* builder, Ast* case_list);
static void visit_selectCase(BuiltinMethodBuilder* builder, Ast* select_case);
static void visit_keysetExpression(BuiltinMethodBuilder* builder, Ast* keyset_expr);
static void visit_tupleKeysetExpression(BuiltinMethodBuilder* builder, Ast* tuple_expr);
static void visit_simpleKeysetExpression(BuiltinMethodBuilder* builder, Ast* simple_expr);
static void visit_simpleExpressionList(BuiltinMethodBuilder* builder, Ast* expr_list);

/** CONTROL **/

static void visit_controlDeclaration(BuiltinMethodBuilder* builder, Ast* control_decl);
static void visit_controlTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl);
static void visit_controlLocalDeclarations(BuiltinMethodBuilder* builder, Ast* local_decls);
static void visit_controlLocalDeclaration(BuiltinMethodBuilder* builder, Ast* local_decl);

/** EXTERN **/

static void visit_externDeclaration(BuiltinMethodBuilder* builder, Ast* extern_decl);
static void visit_externTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl);
static void visit_methodPrototypes(BuiltinMethodBuilder* builder, Ast* protos);
static void visit_functionPrototype(BuiltinMethodBuilder* builder, Ast* func_proto);

/** TYPES **/

static void visit_typeRef(BuiltinMethodBuilder* builder, Ast* type_ref);
static void visit_tupleType(BuiltinMethodBuilder* builder, Ast* type);
static void visit_headerStackType(BuiltinMethodBuilder* builder, Ast* type_decl);
static void visit_baseTypeBoolean(BuiltinMethodBuilder* builder, Ast* bool_type);
static void visit_baseTypeInteger(BuiltinMethodBuilder* builder, Ast* int_type);
static void visit_baseTypeBit(BuiltinMethodBuilder* builder, Ast* bit_type);
static void visit_baseTypeVarbit(BuiltinMethodBuilder* builder, Ast* varbit_type);
static void visit_baseTypeString(BuiltinMethodBuilder* builder, Ast* str_type);
static void visit_baseTypeVoid(BuiltinMethodBuilder* builder, Ast* void_type);
static void visit_baseTypeError(BuiltinMethodBuilder* builder, Ast* error_type);
static void visit_integerTypeSize(BuiltinMethodBuilder* builder, Ast* type_size);
static void visit_realTypeArg(BuiltinMethodBuilder* builder, Ast* type_arg);
static void visit_typeArg(BuiltinMethodBuilder* builder, Ast* type_arg);
static void visit_typeArgumentList(BuiltinMethodBuilder* builder, Ast* arg_list);
static void visit_typeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl);
static void visit_derivedTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl);
static void visit_headerTypeDeclaration(BuiltinMethodBuilder* builder, Ast* header_decl);
static void visit_headerUnionDeclaration(BuiltinMethodBuilder* builder, Ast* union_decl);
static void visit_structTypeDeclaration(BuiltinMethodBuilder* builder, Ast* struct_decl);
static void visit_structFieldList(BuiltinMethodBuilder* builder, Ast* field_list);
static void visit_structField(BuiltinMethodBuilder* builder, Ast* field);
static void visit_enumDeclaration(BuiltinMethodBuilder* builder, Ast* enum_decl);
static void visit_errorDeclaration(BuiltinMethodBuilder* builder, Ast* error_decl);
static void visit_matchKindDeclaration(BuiltinMethodBuilder* builder, Ast* match_decl);
static void visit_identifierList(BuiltinMethodBuilder* builder, Ast* ident_list);
static void visit_specifiedIdentifierList(BuiltinMethodBuilder* builder, Ast* ident_list);
static void visit_specifiedIdentifier(BuiltinMethodBuilder* builder, Ast* ident);
static void visit_typedefDeclaration(BuiltinMethodBuilder* builder, Ast* typedef_decl);

/** STATEMENTS **/

static void visit_assignmentStatement(BuiltinMethodBuilder* builder, Ast* assign_stmt);
static void visit_functionCall(BuiltinMethodBuilder* builder, Ast* func_call);
static void visit_returnStatement(BuiltinMethodBuilder* builder, Ast* return_stmt);
static void visit_exitStatement(BuiltinMethodBuilder* builder, Ast* exit_stmt);
static void visit_conditionalStatement(BuiltinMethodBuilder* builder, Ast* cond_stmt);
static void visit_directApplication(BuiltinMethodBuilder* builder, Ast* applic_stmt);
static void visit_statement(BuiltinMethodBuilder* builder, Ast* stmt);
static void visit_blockStatement(BuiltinMethodBuilder* builder, Ast* block_stmt);
static void visit_statementOrDeclList(BuiltinMethodBuilder* builder, Ast* stmt_list);
static void visit_switchStatement(BuiltinMethodBuilder* builder, Ast* switch_stmt);
static void visit_switchCases(BuiltinMethodBuilder* builder, Ast* switch_cases);
static void visit_switchCase(BuiltinMethodBuilder* builder, Ast* switch_case);
static void visit_switchLabel(BuiltinMethodBuilder* builder, Ast* label);
static void visit_statementOrDeclaration(BuiltinMethodBuilder* builder, Ast* stmt);

/** TABLES **/

static void visit_tableDeclaration(BuiltinMethodBuilder* builder, Ast* table_decl);
static void visit_tablePropertyList(BuiltinMethodBuilder* builder, Ast* prop_list);
static void visit_tableProperty(BuiltinMethodBuilder* builder, Ast* table_prop);
static void visit_keyProperty(BuiltinMethodBuilder* builder, Ast* key_prop);
static void visit_keyElementList(BuiltinMethodBuilder* builder, Ast* element_list);
static void visit_keyElement(BuiltinMethodBuilder* builder, Ast* element);
static void visit_actionsProperty(BuiltinMethodBuilder* builder, Ast* actions_prop);
static void visit_actionList(BuiltinMethodBuilder* builder, Ast* action_list);
static void visit_actionRef(BuiltinMethodBuilder* builder, Ast* action_ref);
static void visit_entriesProperty(BuiltinMethodBuilder* builder, Ast* entries_prop);
static void visit_entriesList(BuiltinMethodBuilder* builder, Ast* entries_list);
static void visit_entry(BuiltinMethodBuilder* builder, Ast* entry);
static void visit_simpleProperty(BuiltinMethodBuilder* builder, Ast* simple_prop);
static void visit_actionDeclaration(BuiltinMethodBuilder* builder, Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(BuiltinMethodBuilder* builder, Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(BuiltinMethodBuilder* builder, Ast* func_decl);
static void visit_argumentList(BuiltinMethodBuilder* builder, Ast* arg_list);
static void visit_argument(BuiltinMethodBuilder* builder, Ast* arg);
static void visit_expressionList(BuiltinMethodBuilder* builder, Ast* expr_list);
static void visit_lvalueExpression(BuiltinMethodBuilder* builder, Ast* lvalue_expr);
static void visit_expression(BuiltinMethodBuilder* builder, Ast* expr);
static void visit_castExpression(BuiltinMethodBuilder* builder, Ast* cast_expr);
static void visit_unaryExpression(BuiltinMethodBuilder* builder, Ast* unary_expr);
static void visit_binaryExpression(BuiltinMethodBuilder* builder, Ast* binary_expr);
static void visit_memberSelector(BuiltinMethodBuilder* builder, Ast* selector);
static void visit_arraySubscript(BuiltinMethodBuilder* builder, Ast* subscript);
static void visit_indexExpression(BuiltinMethodBuilder* builder, Ast* index_expr);
static void visit_booleanLiteral(BuiltinMethodBuilder* builder, Ast* bool_literal);
static void visit_integerLiteral(BuiltinMethodBuilder* builder, Ast* int_literal);
static void visit_stringLiteral(BuiltinMethodBuilder* builder, Ast* str_literal);
static void visit_default(BuiltinMethodBuilder* builder, Ast* default_);
static void visit_dontcare(BuiltinMethodBuilder* builder, Ast* dontcare);

void builtin_methods(BuiltinMethodBuilder* builder, Ast* ast)
{
  visit_p4program(builder, ast);
}

/** PROGRAM **/

static void visit_p4program(BuiltinMethodBuilder* builder, Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  visit_declarationList(builder, p4program->p4program.decl_list);
}

static void visit_declarationList(BuiltinMethodBuilder* builder, Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(builder, container_of(ast, Ast, tree));
  }
}

static void visit_declaration(BuiltinMethodBuilder* builder, Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    visit_externDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration(builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    visit_instantiation(builder, decl->declaration.decl);
  } else assert(0);
}

static void visit_name(BuiltinMethodBuilder* builder, Ast* name)
{
  assert(name->kind == AST_name);
}

static void visit_parameterList(BuiltinMethodBuilder* builder, Ast* params)
{
  assert(params->kind == AST_parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(builder, container_of(ast, Ast, tree));
  }
}

static void visit_parameter(BuiltinMethodBuilder* builder, Ast* param)
{
  assert(param->kind == AST_parameter);
  visit_typeRef(builder, param->parameter.type);
  visit_name(builder, param->parameter.name);
  if (param->parameter.init_expr) {
    visit_expression(builder, param->parameter.init_expr);
  }
}

static void visit_packageTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  visit_name(builder, type_decl->packageTypeDeclaration.name);
  visit_parameterList(builder, type_decl->packageTypeDeclaration.params);
}

static void visit_instantiation(BuiltinMethodBuilder* builder, Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  visit_typeRef(builder, inst->instantiation.type);
  visit_argumentList(builder, inst->instantiation.args);
  visit_name(builder, inst->instantiation.name);
}

/** PARSER **/

static void
visit_parserDeclaration(BuiltinMethodBuilder* builder, Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  visit_typeDeclaration(builder, parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(builder, parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(builder, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(builder, parser_decl->parserDeclaration.states);
}

static void visit_parserTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  Ast* type_ref, *return_type, *method, *name;
  Ast* method_protos;
  AstTreeCtor tree_ctor = {0};

  return_type = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  return_type->kind = AST_baseTypeVoid;
  return_type->name.strname = "void";
  type_ref = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  type_ref->kind = AST_typeRef;
  type_ref->typeRef.type = return_type;
  method = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  method->kind = AST_functionPrototype;
  method->line_no = type_decl->line_no;
  method->column_no = type_decl->column_no;
  method->functionPrototype.return_type = type_ref;
  method->functionPrototype.params = clone_ast(builder->storage, type_decl->parserTypeDeclaration.params);
  name = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  name->kind = AST_name;
  name->name.strname = "apply";
  method->functionPrototype.name = name;
  method_protos = type_decl->parserTypeDeclaration.method_protos;
  ast_tree_append_node(&method_protos->tree, &tree_ctor, &method->tree);
}

static void visit_parserLocalElements(BuiltinMethodBuilder* builder, Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(builder, container_of(ast, Ast, tree));
  }
}

static void visit_parserLocalElement(BuiltinMethodBuilder* builder, Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    visit_variableDeclaration(builder, local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    visit_instantiation(builder, local_element->parserLocalElement.element);
  } else assert(0);
}

static void visit_parserStates(BuiltinMethodBuilder* builder, Ast* states)
{
  assert(states->kind == AST_parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(builder, container_of(ast, Ast, tree));
  }
}

static void visit_parserState(BuiltinMethodBuilder* builder, Ast* state)
{
  assert(state->kind == AST_parserState);
  visit_name(builder, state->parserState.name);
  visit_parserStatements(builder, state->parserState.stmt_list);
  visit_transitionStatement(builder, state->parserState.transition_stmt);
}

static void visit_parserStatements(BuiltinMethodBuilder* builder, Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(builder, container_of(ast, Ast, tree));
  }
}

static void visit_parserStatement(BuiltinMethodBuilder* builder, Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visit_functionCall(builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
}

static void visit_parserBlockStatement(BuiltinMethodBuilder* builder, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  visit_parserStatements(builder, block_stmt->parserBlockStatement.stmt_list);
}

static void visit_transitionStatement(BuiltinMethodBuilder* builder, Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression(builder, transition_stmt->transitionStatement.stmt);
}

static void visit_stateExpression(BuiltinMethodBuilder* builder, Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    visit_name(builder, state_expr->stateExpression.expr);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(builder, state_expr->stateExpression.expr);
  } else assert(0);
}

static void visit_selectExpression(BuiltinMethodBuilder* builder, Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(builder, select_expr->selectExpression.expr_list);
  visit_selectCaseList(builder, select_expr->selectExpression.case_list);
}

static void visit_selectCaseList(BuiltinMethodBuilder* builder, Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(builder, container_of(ast, Ast, tree));
  }
}

static void visit_selectCase(BuiltinMethodBuilder* builder, Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(builder, select_case->selectCase.keyset_expr);
  visit_name(builder, select_case->selectCase.name);
}

static void visit_keysetExpression(BuiltinMethodBuilder* builder, Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(builder, keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(builder, keyset_expr->keysetExpression.expr);
  } else assert(0);
}

static void visit_tupleKeysetExpression(BuiltinMethodBuilder* builder, Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visit_simpleExpressionList(builder, tuple_expr->tupleKeysetExpression.expr_list);
}

static void visit_simpleKeysetExpression(BuiltinMethodBuilder* builder, Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visit_expression(builder, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visit_default(builder, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visit_dontcare(builder, simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

static void visit_simpleExpressionList(BuiltinMethodBuilder* builder, Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(builder, container_of(ast, Ast, tree));
  }
}

/** CONTROL **/

static void visit_controlDeclaration(BuiltinMethodBuilder* builder, Ast* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  visit_typeDeclaration(builder, control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(builder, control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(builder, control_decl->controlDeclaration.local_decls);
  visit_blockStatement(builder, control_decl->controlDeclaration.apply_stmt);
}

static void visit_controlTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  Ast* type_ref, *return_type, *method, *name;
  Ast* method_protos;
  AstTreeCtor tree_ctor = {0};

  return_type = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  return_type->kind = AST_baseTypeVoid;
  return_type->name.strname = "void";
  type_ref = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  type_ref->kind = AST_typeRef;
  type_ref->typeRef.type = return_type;
  method = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  method->kind = AST_functionPrototype;
  method->line_no = type_decl->line_no;
  method->column_no = type_decl->column_no;
  method->functionPrototype.return_type = type_ref;
  method->functionPrototype.params = clone_ast(builder->storage, type_decl->controlTypeDeclaration.params);
  name = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  name->kind = AST_name;
  name->name.strname = "apply";
  method->functionPrototype.name = name;
  method_protos = type_decl->controlTypeDeclaration.method_protos;
  ast_tree_append_node(&method_protos->tree, &tree_ctor, &method->tree);
}

static void visit_controlLocalDeclarations(BuiltinMethodBuilder* builder, Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(builder, container_of(ast, Ast, tree));
  }
}

static void visit_controlLocalDeclaration(BuiltinMethodBuilder* builder, Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(builder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(builder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration(builder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    visit_instantiation(builder, local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void visit_externDeclaration(BuiltinMethodBuilder* builder, Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration(builder, extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    visit_functionPrototype(builder, extern_decl->externDeclaration.decl);
  } else assert(0);
}

static void visit_externTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  visit_name(builder, type_decl->externTypeDeclaration.name);
  visit_methodPrototypes(builder, type_decl->externTypeDeclaration.method_protos);
}

static void visit_methodPrototypes(BuiltinMethodBuilder* builder, Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(builder, container_of(ast, Ast, tree));
  }
}

static void visit_functionPrototype(BuiltinMethodBuilder* builder, Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(builder, func_proto->functionPrototype.return_type);
  }
  visit_name(builder, func_proto->functionPrototype.name);
  visit_parameterList(builder, func_proto->functionPrototype.params);
}

/** TYPES **/

static void visit_typeRef(BuiltinMethodBuilder* builder, Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    visit_baseTypeBit(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    visit_baseTypeString(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    visit_baseTypeError(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    visit_name(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(builder, type_ref->typeRef.type);
  } else assert(0);
}

static void visit_tupleType(BuiltinMethodBuilder* builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  visit_typeArgumentList(builder, type_decl->tupleType.type_args);
}

static void visit_headerStackType(BuiltinMethodBuilder* builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  visit_typeRef(builder, type_decl->headerStackType.type);
  visit_expression(builder, type_decl->headerStackType.stack_expr);
}

static void visit_baseTypeBoolean(BuiltinMethodBuilder* builder, Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  visit_name(builder, bool_type->baseTypeBoolean.name);
}

static void visit_baseTypeInteger(BuiltinMethodBuilder* builder, Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  visit_name(builder, int_type->baseTypeInteger.name);
  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(builder, int_type->baseTypeInteger.size);
  }
}

static void visit_baseTypeBit(BuiltinMethodBuilder* builder, Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  visit_name(builder, bit_type->baseTypeBit.name);
  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(builder, bit_type->baseTypeBit.size);
  }
}

static void visit_baseTypeVarbit(BuiltinMethodBuilder* builder, Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  visit_name(builder, varbit_type->baseTypeVarbit.name);
  visit_integerTypeSize(builder, varbit_type->baseTypeVarbit.size);
}

static void visit_baseTypeString(BuiltinMethodBuilder* builder, Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  visit_name(builder, str_type->baseTypeString.name);
}

static void visit_baseTypeVoid(BuiltinMethodBuilder* builder, Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  visit_name(builder, void_type->baseTypeVoid.name);
}

static void visit_baseTypeError(BuiltinMethodBuilder* builder, Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  visit_name(builder, error_type->baseTypeError.name);
}

static void visit_integerTypeSize(BuiltinMethodBuilder* builder, Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void visit_realTypeArg(BuiltinMethodBuilder* builder, Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visit_typeRef(builder, type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(builder, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(BuiltinMethodBuilder* builder, Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(builder, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(builder, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(builder, type_arg->typeArg.arg);
  } else assert(0);
}

static void visit_typeArgumentList(BuiltinMethodBuilder* builder, Ast* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(builder, container_of(ast, Ast, tree));
  }
}

static void visit_typeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(builder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(builder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(builder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(builder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration(builder, type_decl->typeDeclaration.decl);
  } else assert(0);
}

static void visit_derivedTypeDeclaration(BuiltinMethodBuilder* builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(builder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(builder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(builder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(builder, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
}

static void visit_headerTypeDeclaration(BuiltinMethodBuilder* builder, Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  visit_name(builder, header_decl->headerTypeDeclaration.name);
  visit_structFieldList(builder, header_decl->headerTypeDeclaration.fields);
}

static void visit_headerUnionDeclaration(BuiltinMethodBuilder* builder, Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  visit_name(builder, union_decl->headerUnionDeclaration.name);
  visit_structFieldList(builder, union_decl->headerUnionDeclaration.fields);
}

static void visit_structTypeDeclaration(BuiltinMethodBuilder* builder, Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  visit_name(builder, struct_decl->structTypeDeclaration.name);
  visit_structFieldList(builder, struct_decl->structTypeDeclaration.fields);
}

static void visit_structFieldList(BuiltinMethodBuilder* builder, Ast* field_list)
{
  assert(field_list->kind == AST_structFieldList);
  AstTree* ast;

  for (ast = field_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(builder, container_of(ast, Ast, tree));
  }
}

static void visit_structField(BuiltinMethodBuilder* builder, Ast* field)
{
  assert(field->kind == AST_structField);
  visit_typeRef(builder, field->structField.type);
  visit_name(builder, field->structField.name);
}

static void visit_enumDeclaration(BuiltinMethodBuilder* builder, Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  visit_name(builder, enum_decl->enumDeclaration.name);
  visit_specifiedIdentifierList(builder, enum_decl->enumDeclaration.fields);
}

static void visit_errorDeclaration(BuiltinMethodBuilder* builder, Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  visit_identifierList(builder, error_decl->errorDeclaration.fields);
}

static void visit_matchKindDeclaration(BuiltinMethodBuilder* builder, Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  visit_identifierList(builder, match_decl->matchKindDeclaration.fields);
}

static void visit_identifierList(BuiltinMethodBuilder* builder, Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_name(builder, container_of(ast, Ast, tree));
  }
}

static void visit_specifiedIdentifierList(BuiltinMethodBuilder* builder, Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(builder, container_of(ast, Ast, tree));
  }
}

static void visit_specifiedIdentifier(BuiltinMethodBuilder* builder, Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  visit_name(builder, ident->specifiedIdentifier.name);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(builder, ident->specifiedIdentifier.init_expr);
  }
}

static void visit_typedefDeclaration(BuiltinMethodBuilder* builder, Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(builder, typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(builder, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  visit_name(builder, typedef_decl->typedefDeclaration.name);
}

/** STATEMENTS **/

static void visit_assignmentStatement(BuiltinMethodBuilder* builder, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(builder, assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(builder, assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(builder, assign_stmt->assignmentStatement.rhs_expr);
}

static void visit_functionCall(BuiltinMethodBuilder* builder, Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(builder, func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(builder, func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(builder, func_call->functionCall.args);
}

static void visit_returnStatement(BuiltinMethodBuilder* builder, Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(builder, return_stmt->returnStatement.expr);
  }
}

static void visit_exitStatement(BuiltinMethodBuilder* builder, Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
}

static void visit_conditionalStatement(BuiltinMethodBuilder* builder, Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visit_expression(builder, cond_stmt->conditionalStatement.cond_expr);
  visit_statement(builder, cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(builder, cond_stmt->conditionalStatement.else_stmt);
  }
}

static void visit_directApplication(BuiltinMethodBuilder* builder, Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visit_name(builder, applic_stmt->directApplication.name);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(builder, applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(builder, applic_stmt->directApplication.args);
}

static void visit_statement(BuiltinMethodBuilder* builder, Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    visit_blockStatement(builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    visit_switchStatement(builder, stmt->statement.stmt);
  } else assert(0);
}

static void visit_blockStatement(BuiltinMethodBuilder* builder, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList(builder, block_stmt->blockStatement.stmt_list);
}

static void visit_statementOrDeclList(BuiltinMethodBuilder* builder, Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(builder, container_of(ast, Ast, tree));
  }
}

static void visit_switchStatement(BuiltinMethodBuilder* builder, Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(builder, switch_stmt->switchStatement.expr);
  visit_switchCases(builder, switch_stmt->switchStatement.switch_cases);
}

static void visit_switchCases(BuiltinMethodBuilder* builder, Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(builder, container_of(ast, Ast, tree));
  }
}

static void visit_switchCase(BuiltinMethodBuilder* builder, Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel(builder, switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(builder, switch_case->switchCase.stmt);
  }
}

static void visit_switchLabel(BuiltinMethodBuilder* builder, Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    visit_name(builder, label->switchLabel.label);
  } else if (label->switchLabel.label->kind == AST_default) {
    visit_default(builder, label->switchLabel.label);
  } else assert(0);
}

static void visit_statementOrDeclaration(BuiltinMethodBuilder* builder, Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(builder, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    visit_statement(builder, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    visit_instantiation(builder, stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void visit_tableDeclaration(BuiltinMethodBuilder* builder, Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  Ast* type_ref, *return_type, *method, *name;
  Ast* method_protos, *params;
  AstTreeCtor tree_ctor = {0};

  return_type = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  return_type->kind = AST_baseTypeVoid;
  return_type->name.strname = "void";
  type_ref = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  type_ref->kind = AST_typeRef;
  type_ref->typeRef.type = return_type;
  method = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  method->kind = AST_functionPrototype;
  method->line_no = table_decl->line_no;
  method->column_no = table_decl->column_no;
  method->functionPrototype.return_type = type_ref;
  params = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  params->kind = AST_parameterList;
  params->line_no = table_decl->line_no;
  params->column_no = table_decl->column_no;
  method->functionPrototype.params = params;
  name = (Ast*)arena_malloc(builder->storage, sizeof(Ast));
  name->kind = AST_name;
  name->name.strname = "apply";
  method->functionPrototype.name = name;
  method_protos = table_decl->tableDeclaration.method_protos;
  ast_tree_append_node(&method_protos->tree, &tree_ctor, &method->tree);
}

static void visit_tablePropertyList(BuiltinMethodBuilder* builder, Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(builder, container_of(ast, Ast, tree));
  }
}

static void visit_tableProperty(BuiltinMethodBuilder* builder, Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    visit_keyProperty(builder, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    visit_actionsProperty(builder, table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    visit_entriesProperty(builder, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    visit_simpleProperty(builder, table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
}

static void visit_keyProperty(BuiltinMethodBuilder* builder, Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList(builder, key_prop->keyProperty.keyelem_list);
}

static void visit_keyElementList(BuiltinMethodBuilder* builder, Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(builder, container_of(ast, Ast, tree));
  }
}

static void visit_keyElement(BuiltinMethodBuilder* builder, Ast* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression(builder, element->keyElement.expr);
  visit_name(builder, element->keyElement.match);
}

static void visit_actionsProperty(BuiltinMethodBuilder* builder, Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(builder, actions_prop->actionsProperty.action_list);
}

static void visit_actionList(BuiltinMethodBuilder* builder, Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(builder, container_of(ast, Ast, tree));
  }
}

static void visit_actionRef(BuiltinMethodBuilder* builder, Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name(builder, action_ref->actionRef.name);
  if (action_ref->actionRef.args) {
    visit_argumentList(builder, action_ref->actionRef.args);
  }
}

#if 0
static void visit_entriesProperty(BuiltinMethodBuilder* builder, Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList(builder, entries_prop->entriesProperty.entries_list);
}

static void visit_entriesList(BuiltinMethodBuilder* builder, Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(builder, container_of(ast, Ast, tree));
  }
}

static void visit_entry(BuiltinMethodBuilder* builder, Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(builder, entry->entry.keyset);
  visit_actionRef(builder, entry->entry.action);
}

static void visit_simpleProperty(BuiltinMethodBuilder* builder, Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  visit_name(builder, simple_prop->simpleProperty.name);
  visit_expression(builder, simple_prop->simpleProperty.init_expr);
}
#endif

static void visit_actionDeclaration(BuiltinMethodBuilder* builder, Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  visit_name(builder, action_decl->actionDeclaration.name);
  visit_parameterList(builder, action_decl->actionDeclaration.params);
  visit_blockStatement(builder, action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

static void visit_variableDeclaration(BuiltinMethodBuilder* builder, Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  visit_typeRef(builder, var_decl->variableDeclaration.type);
  visit_name(builder, var_decl->variableDeclaration.name);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(builder, var_decl->variableDeclaration.init_expr);
  }
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(BuiltinMethodBuilder* builder, Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visit_functionPrototype(builder, func_decl->functionDeclaration.proto);
  visit_blockStatement(builder, func_decl->functionDeclaration.stmt);
}

static void visit_argumentList(BuiltinMethodBuilder* builder, Ast* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(builder, container_of(ast, Ast, tree));
  }
}

static void visit_argument(BuiltinMethodBuilder* builder, Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(builder, arg->argument.arg);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(builder, arg->argument.arg);
  } else assert(0);
}

static void visit_expressionList(BuiltinMethodBuilder* builder, Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(builder, container_of(ast, Ast, tree));
  }
}

static void visit_lvalueExpression(BuiltinMethodBuilder* builder, Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visit_name(builder, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(builder, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(builder, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

static void visit_expression(BuiltinMethodBuilder* builder, Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    visit_name(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(builder, expr->expression.expr);
  } else assert(0);
}

static void visit_castExpression(BuiltinMethodBuilder* builder, Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef(builder, cast_expr->castExpression.type);
  visit_expression(builder, cast_expr->castExpression.expr);
}

static void visit_unaryExpression(BuiltinMethodBuilder* builder, Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(builder, unary_expr->unaryExpression.operand);
}

static void visit_binaryExpression(BuiltinMethodBuilder* builder, Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression(builder, binary_expr->binaryExpression.left_operand);
  visit_expression(builder, binary_expr->binaryExpression.right_operand);
}

static void visit_memberSelector(BuiltinMethodBuilder* builder, Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(builder, selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(builder, selector->memberSelector.lhs_expr);
  } else assert(0);
  visit_name(builder, selector->memberSelector.name);
}

static void visit_arraySubscript(BuiltinMethodBuilder* builder, Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(builder, subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(builder, subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(builder, subscript->arraySubscript.index_expr);
}

static void visit_indexExpression(BuiltinMethodBuilder* builder, Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression(builder, index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(builder, index_expr->indexExpression.end_index);
  }
}

static void visit_booleanLiteral(BuiltinMethodBuilder* builder, Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
}

static void visit_integerLiteral(BuiltinMethodBuilder* builder, Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
}

static void visit_stringLiteral(BuiltinMethodBuilder* builder, Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
}

static void visit_default(BuiltinMethodBuilder* builder, Ast* default_)
{
  assert(default_->kind == AST_default);
}

static void visit_dontcare(BuiltinMethodBuilder* builder, Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
}

