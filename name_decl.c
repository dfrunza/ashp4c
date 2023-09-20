#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static Arena*   storage;
static Scope*   current_scope;
static Hashmap  scope_map = {};
static Hashmap  field_map = {};

/** PROGRAM **/

static void visit_p4program(Ast_P4Program* p4program);
static void visit_declarationList(Ast_DeclarationList* decl_list);
static Scope* visit_declaration(Ast_Declaration* decl);
static void visit_name(Ast_Name* name);
static void visit_parameterList(Ast_ParameterList* params);
static void visit_parameter(Ast_Parameter* param);
static void visit_packageTypeDeclaration(Ast_PackageTypeDeclaration* type_decl);
static void visit_instantiation(Ast_Instantiation* inst);

/** PARSER **/

static void visit_parserDeclaration(Ast_ParserDeclaration* parser_decl);
static Scope* visit_parserTypeDeclaration(Ast_ParserTypeDeclaration* type_decl);
static void visit_parserLocalElements(Ast_ParserLocalElements* local_elements);
static void visit_parserLocalElement(Ast_ParserLocalElement* local_element);
static void visit_parserStates(Ast_ParserStates* states);
static void visit_parserState(Ast_ParserState* state);
static void visit_parserStatements(Ast_ParserStatements* stmts);
static void visit_parserStatement(Ast_ParserStatement* stmt);
static void visit_parserBlockStatement(Ast_ParserBlockStatement* block_stmt);
static void visit_transitionStatement(Ast_TransitionStatement* transition_stmt);
static void visit_stateExpression(Ast_StateExpression* state_expr);
static void visit_selectExpression(Ast_SelectExpression* select_expr);
static void visit_selectCaseList(Ast_SelectCaseList* case_list);
static void visit_selectCase(Ast_SelectCase* select_case);
static void visit_keysetExpression(Ast_KeysetExpression* keyset_expr);
static void visit_tupleKeysetExpression(Ast_TupleKeysetExpression* tuple_expr);
static void visit_simpleKeysetExpression(Ast_SimpleKeysetExpression* simple_expr);
static void visit_simpleExpressionList(Ast_SimpleExpressionList* expr_list);

/** CONTROL **/

static void visit_controlDeclaration(Ast_ControlDeclaration* control_decl);
static Scope* visit_controlTypeDeclaration(Ast_ControlTypeDeclaration* type_decl);
static void visit_controlLocalDeclarations(Ast_ControlLocalDeclarations* local_decls);
static void visit_controlLocalDeclaration(Ast_ControlLocalDeclaration* local_decl);

/** EXTERN **/

static Scope* visit_externDeclaration(Ast_ExternDeclaration* extern_decl);
static void visit_externTypeDeclaration(Ast_ExternTypeDeclaration* type_decl);
static void visit_methodPrototypes(Ast_MethodPrototypes* protos);
static Scope* visit_functionPrototype(Ast_FunctionPrototype* func_proto);

/** TYPES **/

static void visit_typeRef(Ast_TypeRef* type_ref);
static void visit_tupleType(Ast_TupleType* type);
static void visit_headerStackType(Ast_HeaderStackType* type_decl);
static void visit_specializedType(Ast_SpecializedType* type_decl);
static void visit_baseTypeBoolean(Ast_BooleanType* bool_type);
static void visit_baseTypeInteger(Ast_IntegerType* int_type);
static void visit_baseTypeBit(Ast_BitType* bit_type);
static void visit_baseTypeVarbit(Ast_VarbitType* varbit_type);
static void visit_baseTypeString(Ast_StringType* str_type);
static void visit_baseTypeVoid(Ast_VoidType* void_type);
static void visit_baseTypeError(Ast_ErrorType* error_type);
static void visit_integerTypeSize(Ast_IntegerTypeSize* type_size);
static void visit_typeParameterList(Ast_TypeParameterList* param_list);
static void visit_realTypeArg(Ast_RealTypeArg* type_arg);
static void visit_typeArg(Ast_TypeArg* type_arg);
static void visit_realTypeArgumentList(Ast_RealTypeArgumentList* arg_list);
static void visit_typeArgumentList(Ast_TypeArgumentList* arg_list);
static Scope* visit_typeDeclaration(Ast_TypeDeclaration* type_decl);
static void visit_derivedTypeDeclaration(Ast_DerivedTypeDeclaration* type_decl);
static void visit_headerTypeDeclaration(Ast_HeaderTypeDeclaration* header_decl);
static void visit_headerUnionDeclaration(Ast_HeaderUnionDeclaration* union_decl);
static void visit_structTypeDeclaration(Ast_StructTypeDeclaration* struct_decl);
static void visit_structFieldList(Ast_StructFieldList* field_list, Scope* field_scope);
static void visit_structField(Ast_StructField* field, Scope* field_scope);
static void visit_enumDeclaration(Ast_EnumDeclaration* enum_decl);
static void visit_errorDeclaration(Ast_ErrorDeclaration* error_decl);
static void visit_matchKindDeclaration(Ast_MatchKindDeclaration* match_decl);
static void visit_identifierList(Ast_IdentifierList* ident_list, Scope* field_scope);
static void visit_specifiedIdentifierList(Ast_SpecifiedIdentifierList* ident_list, Scope* field_scope);
static void visit_specifiedIdentifier(Ast_SpecifiedIdentifier* ident, Scope* field_scope);
static void visit_typedefDeclaration(Ast_TypedefDeclaration* typedef_decl);

/** STATEMENTS **/

static void visit_assignmentStatement(Ast_AssignmentStatement* assign_stmt);
static void visit_functionCall(Ast_FunctionCall* func_call);
static void visit_returnStatement(Ast_ReturnStatement* return_stmt);
static void visit_exitStatement(Ast_ExitStatement* exit_stmt);
static void visit_conditionalStatement(Ast_ConditionalStatement* cond_stmt);
static void visit_directApplication(Ast_DirectApplication* applic_stmt);
static void visit_statement(Ast_Statement* stmt);
static void visit_blockStatement(Ast_BlockStatement* block_stmt);
static void visit_statementOrDeclList(Ast_StatementOrDeclList* stmt_list);
static void visit_switchStatement(Ast_SwitchStatement* switch_stmt);
static void visit_switchCases(Ast_SwitchCases* switch_cases);
static void visit_switchCase(Ast_SwitchCase* switch_case);
static void visit_switchLabel(Ast_SwitchLabel* label);
static void visit_statementOrDeclaration(Ast_StatementOrDeclaration* stmt);

/** TABLES **/

static void visit_tableDeclaration(Ast_TableDeclaration* table_decl);
static void visit_tablePropertyList(Ast_TablePropertyList* prop_list, Scope* prop_scope);
static void visit_tableProperty(Ast_TableProperty* table_prop, Scope* prop_scope);
static void visit_keyProperty(Ast_KeyProperty* key_prop);
static void visit_keyElementList(Ast_KeyElementList* element_list);
static void visit_keyElement(Ast_KeyElement* element);
static void visit_actionsProperty(Ast_ActionsProperty* actions_prop);
static void visit_actionList(Ast_ActionList* action_list);
static void visit_actionRef(Ast_ActionRef* action_ref);
static void visit_entriesProperty(Ast_EntriesProperty* entries_prop);
static void visit_entriesList(Ast_EntriesList* entries_list);
static void visit_entry(Ast_Entry* entry);
static void visit_simpleProperty(Ast_SimpleProperty* simple_prop, Scope* prop_scope);
static void visit_actionDeclaration(Ast_ActionDeclaration* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(Ast_VarDeclaration* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(Ast_FunctionDeclaration* func_decl);
static void visit_argumentList(Ast_ArgumentList* arg_list);
static void visit_argument(Ast_Argument* arg);
static void visit_expressionList(Ast_ExpressionList* expr_list);
static void visit_lvalueExpression(Ast_LvalueExpression* lvalue_expr);
static void visit_expression(Ast_Expression* expr);
static void visit_castExpression(Ast_CastExpression* cast_expr);
static void visit_unaryExpression(Ast_UnaryExpression* unary_expr);
static void visit_binaryExpression(Ast_BinaryExpression* binary_expr);
static void visit_memberSelector(Ast_MemberSelector* selector);
static void visit_arraySubscript(Ast_ArraySubscript* subscript);
static void visit_indexExpression(Ast_IndexExpression* index_expr);
static void visit_booleanLiteral(Ast_BooleanLiteral* bool_literal);
static void visit_integerLiteral(Ast_IntegerLiteral* int_literal);
static void visit_stringLiteral(Ast_StringLiteral* str_literal);
static void visit_default(Ast_Default* default_);
static void visit_dontcare(Ast_Dontcare* dontcare_);

/** PROGRAM **/

static void
visit_p4program(Ast_P4Program* p4program)
{
  assert(p4program->kind == AST_p4program);
  Scope* scope = arena_malloc(storage, sizeof(*scope));
  hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  current_scope = scope_push(scope, current_scope);
  visit_declarationList((Ast_DeclarationList*)p4program->decl_list);
  current_scope = scope_pop(current_scope);
}

static void
visit_declarationList(Ast_DeclarationList* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  for (Ast* ast = decl_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration((Ast_Declaration*)ast);
  }
}

static Scope*
visit_declaration(Ast_Declaration* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_externDeclaration) {
    return visit_externDeclaration((Ast_ExternDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration((Ast_ActionDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration((Ast_FunctionDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration((Ast_ParserDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_parserTypeDeclaration) {
    return visit_parserTypeDeclaration((Ast_ParserTypeDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration((Ast_ControlDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_controlTypeDeclaration) {
    return visit_controlTypeDeclaration((Ast_ControlTypeDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_typeDeclaration) {
    return visit_typeDeclaration((Ast_TypeDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration((Ast_ErrorDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration((Ast_MatchKindDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_instantiation) {
    visit_instantiation((Ast_Instantiation*)decl->decl);
  } else assert(0);
  return current_scope;
}

static void
visit_name(Ast_Name* name)
{
  assert(name->kind == AST_name);
  hashmap_set(&scope_map, storage, &current_scope, HKEY_UINT64, (uint64_t)name);
}

static void
visit_parameterList(Ast_ParameterList* params)
{
  assert(params->kind == AST_parameterList);
  for (Ast* ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter((Ast_Parameter*)ast);
  }
}

static void
visit_parameter(Ast_Parameter* param)
{
  assert(param->kind == AST_parameter);
  visit_typeRef((Ast_TypeRef*)param->type);
  Ast_Name* name = (Ast_Name*)param->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)param;
  scope_push_decl(current_scope, storage, namedecl, NS_VAR);
  if (param->init_expr) {
    visit_expression((Ast_Expression*)param->init_expr);
  }
}

static void
visit_packageTypeDeclaration(Ast_PackageTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)type_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* scope = arena_malloc(storage, sizeof(*scope));
  hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  current_scope = scope_push(scope, current_scope);
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
  current_scope = scope_pop(current_scope);
}

static void
visit_instantiation(Ast_Instantiation* inst)
{
  assert(inst->kind == AST_instantiation);
  visit_typeRef((Ast_TypeRef*)inst->type_ref);
  visit_argumentList((Ast_ArgumentList*)inst->args);
  Ast_Name* name = (Ast_Name*)inst->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)inst;
  scope_push_decl(current_scope, storage, namedecl, NS_VAR);
}

/** PARSER **/

static void
visit_parserDeclaration(Ast_ParserDeclaration* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  Scope* parser_scope = visit_typeDeclaration((Ast_TypeDeclaration*)parser_decl->proto);
  Scope* outer_scope = current_scope;
  current_scope = parser_scope;
  if (parser_decl->ctor_params) {
    visit_parameterList((Ast_ParameterList*)parser_decl->ctor_params);
  }
  visit_parserLocalElements((Ast_ParserLocalElements*)parser_decl->local_elements);
  visit_parserStates((Ast_ParserStates*)parser_decl->states);
  current_scope = outer_scope;
}

static Scope*
visit_parserTypeDeclaration(Ast_ParserTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)type_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* scope = arena_malloc(storage, sizeof(*scope));
  hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  current_scope = scope_push(scope, current_scope);
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
  Scope* parser_scope = current_scope;
  current_scope = scope_pop(current_scope);
  return parser_scope;
}

static void
visit_parserLocalElements(Ast_ParserLocalElements* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  for (Ast* ast = local_elements->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement((Ast_ParserLocalElement*)ast);
  }
}

static void
visit_parserLocalElement(Ast_ParserLocalElement* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->element->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)local_element->element);
  } else if (local_element->element->kind == AST_instantiation) {
    visit_instantiation((Ast_Instantiation*)local_element->element);
  } else assert(0);
}

static void
visit_parserStates(Ast_ParserStates* states)
{
  assert(states->kind == AST_parserStates);
  for (Ast* ast = states->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState((Ast_ParserState*)ast);
  }
}

static void
visit_parserState(Ast_ParserState* state)
{
  assert(state->kind == AST_parserState);
  Ast_Name* name = (Ast_Name*)state->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)state;
  scope_push_decl(current_scope, storage, namedecl, NS_VAR);
  Scope* scope = arena_malloc(storage, sizeof(*scope));
  hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  current_scope = scope_push(scope, current_scope);
  visit_parserStatements((Ast_ParserStatements*)state->stmt_list);
  visit_transitionStatement((Ast_TransitionStatement*)state->transition_stmt);
  current_scope = scope_pop(current_scope);
}

static void
visit_parserStatements(Ast_ParserStatements* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  for (Ast* ast = stmts->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement((Ast_ParserStatement*)ast);
  }
}

static void
visit_parserStatement(Ast_ParserStatement* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement((Ast_AssignmentStatement*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_functionCall) {
    visit_functionCall((Ast_FunctionCall*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_directApplication) {
    visit_directApplication((Ast_DirectApplication*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_parserBlockStatement) {
    Scope* scope = arena_malloc(storage, sizeof(*scope));
    hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
    current_scope = scope_push(scope, current_scope);
    visit_parserBlockStatement((Ast_ParserBlockStatement*)stmt->stmt);
    current_scope = scope_pop(current_scope);
  } else if (stmt->stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)stmt->stmt);
  } else assert(0);
}

static void
visit_parserBlockStatement(Ast_ParserBlockStatement* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  visit_parserStatements((Ast_ParserStatements*)block_stmt->stmt_list);
}

static void
visit_transitionStatement(Ast_TransitionStatement* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression((Ast_StateExpression*)transition_stmt->stmt);
}

static void
visit_stateExpression(Ast_StateExpression* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->expr->kind == AST_name) {
    visit_name((Ast_Name*)state_expr->expr);
  } else if (state_expr->expr->kind == AST_selectExpression) {
    visit_selectExpression((Ast_SelectExpression*)state_expr->expr);
  } else assert(0);
}

static void
visit_selectExpression(Ast_SelectExpression* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList((Ast_ExpressionList*)select_expr->expr_list);
  visit_selectCaseList((Ast_SelectCaseList*)select_expr->case_list);
}

static void
visit_selectCaseList(Ast_SelectCaseList* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  for (Ast* ast = case_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase((Ast_SelectCase*)ast);
  }
}

static void
visit_selectCase(Ast_SelectCase* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression((Ast_KeysetExpression*)select_case->keyset_expr);
  visit_name((Ast_Name*)select_case->name);
}

static void
visit_keysetExpression(Ast_KeysetExpression* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression((Ast_TupleKeysetExpression*)keyset_expr->expr);
  } else if (keyset_expr->expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression((Ast_SimpleKeysetExpression*)keyset_expr->expr);
  } else assert(0);
}

static void
visit_tupleKeysetExpression(Ast_TupleKeysetExpression* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visit_simpleExpressionList((Ast_SimpleExpressionList*)tuple_expr->expr_list);
}

static void
visit_simpleKeysetExpression(Ast_SimpleKeysetExpression* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->expr->kind == AST_expression) {
    visit_expression((Ast_Expression*)simple_expr->expr);
  } else if (simple_expr->expr->kind == AST_default) {
    visit_default((Ast_Default*)simple_expr->expr);
  } else if (simple_expr->expr->kind == AST_dontcare) {
    visit_dontcare((Ast_Dontcare*)simple_expr->expr);
  } else assert(0);
}

static void
visit_simpleExpressionList(Ast_SimpleExpressionList* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  for (Ast* ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression((Ast_SimpleKeysetExpression*)ast);
  }
}

/** CONTROL **/

static void
visit_controlDeclaration(Ast_ControlDeclaration* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  Scope* control_scope = visit_typeDeclaration((Ast_TypeDeclaration*)control_decl->proto);
  Scope* outer_scope = current_scope;
  current_scope = control_scope;
  if (control_decl->ctor_params) {
    visit_parameterList((Ast_ParameterList*)control_decl->ctor_params);
  }
  visit_controlLocalDeclarations((Ast_ControlLocalDeclarations*)control_decl->local_decls);
  visit_blockStatement((Ast_BlockStatement*)control_decl->apply_stmt);
  current_scope = outer_scope;
}

static Scope*
visit_controlTypeDeclaration(Ast_ControlTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)type_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* scope = arena_malloc(storage, sizeof(*scope));
  hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  current_scope = scope_push(scope, current_scope);
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
  Scope* control_scope = current_scope;
  current_scope = scope_pop(current_scope);
  return control_scope;
}

static void
visit_controlLocalDeclarations(Ast_ControlLocalDeclarations* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  for (Ast* ast = local_decls->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration((Ast_ControlLocalDeclaration*)ast);
  }
}

static void
visit_controlLocalDeclaration(Ast_ControlLocalDeclaration* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)local_decl->decl);
  } else if (local_decl->decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration((Ast_ActionDeclaration*)local_decl->decl);
  } else if (local_decl->decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration((Ast_TableDeclaration*)local_decl->decl);
  } else if (local_decl->decl->kind == AST_instantiation) {
    visit_instantiation((Ast_Instantiation*)local_decl->decl);
  } else assert(0);
}

/** EXTERN **/

static Scope*
visit_externDeclaration(Ast_ExternDeclaration* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration((Ast_ExternTypeDeclaration*)extern_decl->decl);
  } else if (extern_decl->decl->kind == AST_functionPrototype) {
    return visit_functionPrototype((Ast_FunctionPrototype*)extern_decl->decl);
  } else assert(0);
  return current_scope;
}

static void
visit_externTypeDeclaration(Ast_ExternTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)type_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* scope = arena_malloc(storage, sizeof(*scope));
  hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  current_scope = scope_push(scope, current_scope);
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_methodPrototypes((Ast_MethodPrototypes*)type_decl->method_protos);
  current_scope = scope_pop(current_scope);
}

static void
visit_methodPrototypes(Ast_MethodPrototypes* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  for (Ast* ast = protos->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype((Ast_FunctionPrototype*)ast);
  }
}

static Scope*
visit_functionPrototype(Ast_FunctionPrototype* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  if (func_proto->return_type) {
    visit_typeRef((Ast_TypeRef*)func_proto->return_type);
  }
  Ast_Name* name = (Ast_Name*)func_proto->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)func_proto;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* scope = arena_malloc(storage, sizeof(*scope));
  hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  current_scope = scope_push(scope, current_scope);
  if (func_proto->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)func_proto->type_params);
  }
  visit_parameterList((Ast_ParameterList*)func_proto->params);
  Scope* func_scope = current_scope;
  current_scope = scope_pop(current_scope);
  return func_scope;
}

/** TYPES **/

static void
visit_typeRef(Ast_TypeRef* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  if (type_ref->type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean((Ast_BooleanType*)type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger((Ast_IntegerType*)type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeBit) {
    visit_baseTypeBit((Ast_BitType*)type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit((Ast_VarbitType*)type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeString) {
    visit_baseTypeString((Ast_StringType*)type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid((Ast_VoidType*)type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeError) {
    visit_baseTypeError((Ast_ErrorType*)type_ref->type);
  } else if (type_ref->type->kind == AST_name) {
    visit_name((Ast_Name*)type_ref->type);
  } else if (type_ref->type->kind == AST_specializedType) {
    visit_specializedType((Ast_SpecializedType*)type_ref->type);
  } else if (type_ref->type->kind == AST_headerStackType) {
    visit_headerStackType((Ast_HeaderStackType*)type_ref->type);
  } else if (type_ref->type->kind == AST_tupleType) {
    visit_tupleType((Ast_TupleType*)type_ref->type);
  } else assert(0);
}

static void
visit_tupleType(Ast_TupleType* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)type_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  visit_typeArgumentList((Ast_TypeArgumentList*)type_decl->type_args);
}

static void
visit_headerStackType(Ast_HeaderStackType* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)type_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  visit_typeRef((Ast_TypeRef*)type_decl->type);
  visit_expression((Ast_Expression*)type_decl->stack_expr);
}

static void
visit_specializedType(Ast_SpecializedType* type_decl)
{
  assert(type_decl->kind == AST_specializedType);
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)type_decl;
  visit_typeRef((Ast_TypeRef*)type_decl->type);
  visit_typeArgumentList((Ast_TypeArgumentList*)type_decl->type_args);
}

static void
visit_baseTypeBoolean(Ast_BooleanType* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  visit_name((Ast_Name*)bool_type->name);
}

static void
visit_baseTypeInteger(Ast_IntegerType* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  visit_name((Ast_Name*)int_type->name);
  if (int_type->size) {
    visit_integerTypeSize((Ast_IntegerTypeSize*)int_type->size);
  }
}

static void
visit_baseTypeBit(Ast_BitType* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  visit_name((Ast_Name*)bit_type->name);
  if (bit_type->size) {
    visit_integerTypeSize((Ast_IntegerTypeSize*)bit_type->size);
  }
}

static void
visit_baseTypeVarbit(Ast_VarbitType* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  visit_name((Ast_Name*)varbit_type->name);
  visit_integerTypeSize((Ast_IntegerTypeSize*)varbit_type->size);
}

static void
visit_baseTypeString(Ast_StringType* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  visit_name((Ast_Name*)str_type->name);
}

static void
visit_baseTypeVoid(Ast_VoidType* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  visit_name((Ast_Name*)void_type->name);
}

static void
visit_baseTypeError(Ast_ErrorType* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  visit_name((Ast_Name*)error_type->name);
}

static void
visit_integerTypeSize(Ast_IntegerTypeSize* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void
visit_typeParameterList(Ast_TypeParameterList* param_list)
{
  assert(param_list->kind == AST_typeParameterList);
  for (Ast* ast = param_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast_Name* name = (Ast_Name*)ast;
    NameEntry* name_entry = scope_lookup_any(current_scope, name->strname);
    if (name_entry && name_entry->ns[NS_TYPE]) {
      visit_name(name);
    } else {
      NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
      namedecl->strname = name->strname;
      namedecl->ast = (Ast*)param_list;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
    }
  }
}

static void
visit_realTypeArg(Ast_RealTypeArg* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->arg->kind == AST_typeRef) {
    visit_typeRef((Ast_TypeRef*)type_arg->arg);
  } else if (type_arg->arg->kind == AST_dontcare) {
    visit_dontcare((Ast_Dontcare*)type_arg->arg);
  } else assert(0);
}

static void
visit_typeArg(Ast_TypeArg* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->arg->kind == AST_typeRef) {
    visit_typeRef((Ast_TypeRef*)type_arg->arg);
  } else if (type_arg->arg->kind == AST_name) {
    visit_name((Ast_Name*)type_arg->arg);
  } else if (type_arg->arg->kind == AST_dontcare) {
    visit_dontcare((Ast_Dontcare*)type_arg->arg);
  } else assert(0);
}

static void
visit_realTypeArgumentList(Ast_RealTypeArgumentList* arg_list)
{
  assert(arg_list->kind == AST_realTypeArgumentList);
  for (Ast* ast = arg_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_realTypeArg((Ast_RealTypeArg*)ast);
  }
}

static void
visit_typeArgumentList(Ast_TypeArgumentList* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  for (Ast* ast = arg_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg((Ast_TypeArg*)ast);
  }
}

static Scope*
visit_typeDeclaration(Ast_TypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration((Ast_DerivedTypeDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration((Ast_TypedefDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_parserTypeDeclaration) {
    return visit_parserTypeDeclaration((Ast_ParserTypeDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_controlTypeDeclaration) {
    return visit_controlTypeDeclaration((Ast_ControlTypeDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration((Ast_PackageTypeDeclaration*)type_decl->decl);
  } else assert(0);
  return current_scope;
}

static void
visit_derivedTypeDeclaration(Ast_DerivedTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  if (type_decl->decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration((Ast_HeaderTypeDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration((Ast_HeaderUnionDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration((Ast_StructTypeDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration((Ast_EnumDeclaration*)type_decl->decl);
  } else assert(0);
}

static void
visit_headerTypeDeclaration(Ast_HeaderTypeDeclaration* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  Ast_Name* name = (Ast_Name*)header_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)header_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* field_scope = arena_malloc(storage, sizeof(*field_scope));
  hashmap_create(&field_scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  hashmap_set(&field_map, storage, &field_scope, HKEY_UINT64, (uint64_t)header_decl);
  visit_structFieldList((Ast_StructFieldList*)header_decl->fields, field_scope);
}

static void
visit_headerUnionDeclaration(Ast_HeaderUnionDeclaration* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  Ast_Name* name = (Ast_Name*)union_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)union_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* field_scope = arena_malloc(storage, sizeof(*field_scope));
  hashmap_create(&field_scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  hashmap_set(&field_map, storage, &field_scope, HKEY_UINT64, (uint64_t)union_decl);
  visit_structFieldList((Ast_StructFieldList*)union_decl->fields, field_scope);
}

static void
visit_structTypeDeclaration(Ast_StructTypeDeclaration* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  Ast_Name* name = (Ast_Name*)struct_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)struct_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* field_scope = arena_malloc(storage, sizeof(*field_scope));
  hashmap_create(&field_scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  hashmap_set(&field_map, storage, &field_scope, HKEY_UINT64, (uint64_t)struct_decl);
  visit_structFieldList((Ast_StructFieldList*)struct_decl->fields, field_scope);
}

static void
visit_structFieldList(Ast_StructFieldList* field_list, Scope* field_scope)
{
  assert(field_list->kind == AST_structFieldList);
  for (Ast* ast = field_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField((Ast_StructField*)ast, field_scope);
  }
}

static void
visit_structField(Ast_StructField* field, Scope* field_scope)
{
  assert(field->kind == AST_structField);
  visit_typeRef((Ast_TypeRef*)field->type);
  Ast_Name* name = (Ast_Name*)field->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)field;
  scope_push_decl(field_scope, storage, namedecl, NS_VAR);
}

static void
visit_enumDeclaration(Ast_EnumDeclaration* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  Ast_Name* name = (Ast_Name*)enum_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)enum_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* field_scope = arena_malloc(storage, sizeof(*field_scope));
  hashmap_create(&field_scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 7, 511);
  hashmap_set(&field_map, storage, &field_scope, HKEY_UINT64, (uint64_t)enum_decl);
  visit_specifiedIdentifierList((Ast_SpecifiedIdentifierList*)enum_decl->fields, field_scope);
}

static void
visit_errorDeclaration(Ast_ErrorDeclaration* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  Scope* field_scope = arena_malloc(storage, sizeof(*field_scope));
  hashmap_create(&field_scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  hashmap_set(&field_map, storage, &field_scope, HKEY_UINT64, (uint64_t)error_decl);
  visit_identifierList((Ast_IdentifierList*)error_decl->fields, field_scope);
}

static void
visit_matchKindDeclaration(Ast_MatchKindDeclaration* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  Scope* field_scope = arena_malloc(storage, sizeof(*field_scope));
  hashmap_create(&field_scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  hashmap_set(&field_map, storage, &field_scope, HKEY_UINT64, (uint64_t)match_decl);
  visit_identifierList((Ast_IdentifierList*)match_decl->fields, field_scope);
}

static void
visit_identifierList(Ast_IdentifierList* ident_list, Scope* field_scope)
{
  assert(ident_list->kind == AST_identifierList);
  for (Ast* ast = ident_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast_Name* name = (Ast_Name*)ast;
    NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
    namedecl->strname = name->strname;
    namedecl->ast = (Ast*)ident_list;
    scope_push_decl(field_scope, storage, namedecl, NS_VAR);
  }
}

static void
visit_specifiedIdentifierList(Ast_SpecifiedIdentifierList* ident_list, Scope* field_scope)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  for (Ast* ast = ident_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier((Ast_SpecifiedIdentifier*)ast, field_scope);
  }
}

static void
visit_specifiedIdentifier(Ast_SpecifiedIdentifier* ident, Scope* field_scope)
{
  assert(ident->kind == AST_specifiedIdentifier);
  Ast_Name* name = (Ast_Name*)ident->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)ident;
  scope_push_decl(field_scope, storage, namedecl, NS_VAR);
  if (ident->init_expr) {
    visit_expression((Ast_Expression*)ident->init_expr);
  }
}

static void
visit_typedefDeclaration(Ast_TypedefDeclaration* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->type_ref->kind == AST_typeRef) {
    visit_typeRef((Ast_TypeRef*)typedef_decl->type_ref);
  } else if (typedef_decl->type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration((Ast_DerivedTypeDeclaration*)typedef_decl->type_ref);
  } else assert(0);
  Ast_Name* name = (Ast_Name*)typedef_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)typedef_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
}

/** STATEMENTS **/

static void
visit_assignmentStatement(Ast_AssignmentStatement* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->lhs_expr->kind == AST_expression) {
    visit_expression((Ast_Expression*)assign_stmt->lhs_expr);
  } else if (assign_stmt->lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression((Ast_LvalueExpression*)assign_stmt->lhs_expr);
  } else assert(0);
  visit_expression((Ast_Expression*)assign_stmt->rhs_expr);
}

static void
visit_functionCall(Ast_FunctionCall* func_call)
{
  assert(func_call->kind == AST_functionCall);
  Ast* lhs_expr = func_call->lhs_expr;
  if (lhs_expr->kind == AST_expression) {
    visit_expression((Ast_Expression*)lhs_expr);
  } else if (lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression((Ast_LvalueExpression*)lhs_expr);
  } else assert(0);
  visit_argumentList((Ast_ArgumentList*)func_call->args);
}

static void
visit_returnStatement(Ast_ReturnStatement* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->expr) {
    visit_expression((Ast_Expression*)return_stmt->expr);
  }
}

static void
visit_exitStatement(Ast_ExitStatement* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
}

static void
visit_conditionalStatement(Ast_ConditionalStatement* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visit_expression((Ast_Expression*)cond_stmt->cond_expr);
  visit_statement((Ast_Statement*)cond_stmt->stmt);
  if (cond_stmt->else_stmt) {
    visit_statement((Ast_Statement*)cond_stmt->else_stmt);
  }
}

static void
visit_directApplication(Ast_DirectApplication* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->name->kind == AST_name) {
    visit_name((Ast_Name*)applic_stmt->name);
  } else if (applic_stmt->name->kind == AST_typeRef) {
    visit_typeRef((Ast_TypeRef*)applic_stmt->name);
  } else assert(0);
  visit_argumentList((Ast_ArgumentList*)applic_stmt->args);
}

static void
visit_statement(Ast_Statement* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement((Ast_AssignmentStatement*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_functionCall) {
    visit_functionCall((Ast_FunctionCall*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_directApplication) {
    visit_directApplication((Ast_DirectApplication*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement((Ast_ConditionalStatement*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_emptyStatement) {
  } else if (stmt->stmt->kind == AST_blockStatement) {
    Scope* scope = arena_malloc(storage, sizeof(*scope));
    hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
    current_scope = scope_push(scope, current_scope);
    visit_blockStatement((Ast_BlockStatement*)stmt->stmt);
    current_scope = scope_pop(current_scope);
  } else if (stmt->stmt->kind == AST_exitStatement) {
    visit_exitStatement((Ast_ExitStatement*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_returnStatement) {
    visit_returnStatement((Ast_ReturnStatement*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_switchStatement) {
    visit_switchStatement((Ast_SwitchStatement*)stmt->stmt);
  } else assert(0);
}

static void
visit_blockStatement(Ast_BlockStatement* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList((Ast_StatementOrDeclList*)block_stmt->stmt_list);
}

static void
visit_statementOrDeclList(Ast_StatementOrDeclList* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  for (Ast* ast = stmt_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration((Ast_StatementOrDeclaration*)ast);
  }
}

static void
visit_switchStatement(Ast_SwitchStatement* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression((Ast_Expression*)switch_stmt->expr);
  visit_switchCases((Ast_SwitchCases*)switch_stmt->switch_cases);
}

static void
visit_switchCases(Ast_SwitchCases* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  for (Ast* ast = switch_cases->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase((Ast_SwitchCase*)ast);
  }
}

static void
visit_switchCase(Ast_SwitchCase* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel((Ast_SwitchLabel*)switch_case->label);
  if (switch_case->stmt) {
    visit_blockStatement((Ast_BlockStatement*)switch_case->stmt);
  }
}

static void
visit_switchLabel(Ast_SwitchLabel* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->label->kind == AST_name) {
    visit_name((Ast_Name*)label->label);
  } else if (label->label->kind == AST_default) {
    visit_default((Ast_Default*)label->label);
  } else assert(0);
}

static void
visit_statementOrDeclaration(Ast_StatementOrDeclaration* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_statement) {
    visit_statement((Ast_Statement*)stmt->stmt);
  } else if (stmt->stmt->kind == AST_instantiation) {
    visit_instantiation((Ast_Instantiation*)stmt->stmt);
  } else assert(0);
}

/** TABLES **/

static void
visit_tableDeclaration(Ast_TableDeclaration* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  Ast_Name* name = (Ast_Name*)table_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)table_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_VAR);
  Scope* prop_scope = arena_malloc(storage, sizeof(*prop_scope));
  hashmap_create(&prop_scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  hashmap_set(&field_map, storage, &prop_scope, HKEY_UINT64, (uint64_t)table_decl);
  visit_tablePropertyList((Ast_TablePropertyList*)table_decl->prop_list, prop_scope);
}

static void
visit_tablePropertyList(Ast_TablePropertyList* prop_list, Scope* prop_scope)
{
  assert(prop_list->kind == AST_tablePropertyList);
  for (Ast* ast = prop_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty((Ast_TableProperty*)ast, prop_scope);
  }
}

static void
visit_tableProperty(Ast_TableProperty* table_prop, Scope* prop_scope)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->prop->kind == AST_keyProperty) {
    visit_keyProperty((Ast_KeyProperty*)table_prop->prop);
  } else if (table_prop->prop->kind == AST_actionsProperty) {
    visit_actionsProperty((Ast_ActionsProperty*)table_prop->prop);
  } else if (table_prop->prop->kind == AST_entriesProperty) {
    visit_entriesProperty((Ast_EntriesProperty*)table_prop->prop);
  } else if (table_prop->prop->kind == AST_simpleProperty) {
    visit_simpleProperty((Ast_SimpleProperty*)table_prop->prop, prop_scope);
  } else assert(0);
}

static void
visit_keyProperty(Ast_KeyProperty* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList((Ast_KeyElementList*)key_prop->keyelem_list);
}

static void
visit_keyElementList(Ast_KeyElementList* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  for (Ast* ast = element_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement((Ast_KeyElement*)ast);
  }
}

static void
visit_keyElement(Ast_KeyElement* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression((Ast_Expression*)element->expr);
  visit_name((Ast_Name*)element->match);
}

static void
visit_actionsProperty(Ast_ActionsProperty* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList((Ast_ActionList*)actions_prop->action_list);
}

static void
visit_actionList(Ast_ActionList* action_list)
{
  assert(action_list->kind == AST_actionList);
  for (Ast* ast = action_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef((Ast_ActionRef*)ast);
  }
}

static void
visit_actionRef(Ast_ActionRef* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name((Ast_Name*)action_ref->name);
  if (action_ref->args) {
    visit_argumentList((Ast_ArgumentList*)action_ref->args);
  }
}

static void
visit_entriesProperty(Ast_EntriesProperty* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList((Ast_EntriesList*)entries_prop->entries_list);
}

static void
visit_entriesList(Ast_EntriesList* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  for (Ast* ast = entries_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry((Ast_Entry*)ast);
  }
}

static void
visit_entry(Ast_Entry* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression((Ast_KeysetExpression*)entry->keyset);
  visit_actionRef((Ast_ActionRef*)entry->action);
}

static void
visit_simpleProperty(Ast_SimpleProperty* simple_prop, Scope* prop_scope)
{
  assert(simple_prop->kind == AST_simpleProperty);
  Ast_Name* name = (Ast_Name*)simple_prop->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)simple_prop;
  scope_push_decl(prop_scope, storage, namedecl, NS_VAR);
  visit_expression((Ast_Expression*)simple_prop->init_expr);
}

static void
visit_actionDeclaration(Ast_ActionDeclaration* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  Ast_Name* name = (Ast_Name*)action_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)action_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
  Scope* scope = arena_malloc(storage, sizeof(*scope));
  hashmap_create(&scope->name_table, storage, HKEY_STRING, sizeof(NameEntry), 15, 511);
  current_scope = scope_push(scope, current_scope);
  visit_parameterList((Ast_ParameterList*)action_decl->params);
  visit_blockStatement((Ast_BlockStatement*)action_decl->stmt);
  current_scope = scope_pop(current_scope);
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast_VarDeclaration* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  visit_typeRef((Ast_TypeRef*)var_decl->type);
  Ast_Name* name = (Ast_Name*)var_decl->name;
  NameDecl* namedecl = arena_malloc(storage, sizeof(*namedecl));
  namedecl->strname = name->strname;
  namedecl->ast = (Ast*)var_decl;
  scope_push_decl(current_scope, storage, namedecl, NS_VAR);
  if (var_decl->init_expr) {
    visit_expression((Ast_Expression*)var_decl->init_expr);
  }
}

/** EXPRESSIONS **/

static void
visit_functionDeclaration(Ast_FunctionDeclaration* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  Scope* func_scope = visit_functionPrototype((Ast_FunctionPrototype*)func_decl->proto);
  Scope* outer_scope = current_scope;
  current_scope = func_scope;
  visit_blockStatement((Ast_BlockStatement*)func_decl->stmt);
  current_scope = outer_scope;
}

static void
visit_argumentList(Ast_ArgumentList* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  for (Ast* ast = arg_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument((Ast_Argument*)ast);
  }
}

static void
visit_argument(Ast_Argument* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->arg->kind == AST_expression) {
    visit_expression((Ast_Expression*)arg->arg);
  } else if (arg->arg->kind == AST_dontcare) {
    visit_dontcare((Ast_Dontcare*)arg->arg);
  } else assert(0);
}

static void
visit_expressionList(Ast_ExpressionList* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  for (Ast* ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression((Ast_Expression*)ast);
  }
}

static void
visit_lvalueExpression(Ast_LvalueExpression* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->expr->kind == AST_name) {
    visit_name((Ast_Name*)lvalue_expr->expr);
  } else if (lvalue_expr->expr->kind == AST_memberSelector) {
    visit_memberSelector((Ast_MemberSelector*)lvalue_expr->expr);
  } else if (lvalue_expr->expr->kind == AST_arraySubscript) {
    visit_arraySubscript((Ast_ArraySubscript*)lvalue_expr->expr);
  } else assert(0);
}

static void
visit_expression(Ast_Expression* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expr->kind == AST_expression) {
    visit_expression((Ast_Expression*)expr->expr);
  } else if (expr->expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral((Ast_BooleanLiteral*)expr->expr);
  } else if (expr->expr->kind == AST_integerLiteral) {
    visit_integerLiteral((Ast_IntegerLiteral*)expr->expr);
  } else if (expr->expr->kind == AST_stringLiteral) {
    visit_stringLiteral((Ast_StringLiteral*)expr->expr);
  } else if (expr->expr->kind == AST_name) {
    visit_name((Ast_Name*)expr->expr);
  } else if (expr->expr->kind == AST_specializedType) {
    visit_specializedType((Ast_SpecializedType*)expr->expr);
  } else if (expr->expr->kind == AST_headerStackType) {
    visit_headerStackType((Ast_HeaderStackType*)expr->expr);
  } else if (expr->expr->kind == AST_expressionList) {
    visit_expressionList((Ast_ExpressionList*)expr->expr);
  } else if (expr->expr->kind == AST_castExpression) {
    visit_castExpression((Ast_CastExpression*)expr->expr);
  } else if (expr->expr->kind == AST_unaryExpression) {
    visit_unaryExpression((Ast_UnaryExpression*)expr->expr);
  } else if (expr->expr->kind == AST_binaryExpression) {
    visit_binaryExpression((Ast_BinaryExpression*)expr->expr);
  } else if (expr->expr->kind == AST_memberSelector) {
    visit_memberSelector((Ast_MemberSelector*)expr->expr);
  } else if (expr->expr->kind == AST_arraySubscript) {
    visit_arraySubscript((Ast_ArraySubscript*)expr->expr);
  } else if (expr->expr->kind == AST_functionCall) {
    visit_functionCall((Ast_FunctionCall*)expr->expr);
  } else if (expr->expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement((Ast_AssignmentStatement*)expr->expr);
  } else assert(0);
  if (expr->type_args) {
    visit_realTypeArgumentList((Ast_RealTypeArgumentList*)expr->type_args);
  }
}

static void
visit_castExpression(Ast_CastExpression* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef((Ast_TypeRef*)cast_expr->type);
  visit_expression((Ast_Expression*)cast_expr->expr);
}

static void
visit_unaryExpression(Ast_UnaryExpression* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression((Ast_Expression*)unary_expr->operand);
}

static void
visit_binaryExpression(Ast_BinaryExpression* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression((Ast_Expression*)binary_expr->left_operand);
  visit_expression((Ast_Expression*)binary_expr->right_operand);
}

static void
visit_memberSelector(Ast_MemberSelector* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->lhs_expr->kind == AST_expression) {
    visit_expression((Ast_Expression*)selector->lhs_expr);
  } else if (selector->lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression((Ast_LvalueExpression*)selector->lhs_expr);
  } else assert(0);
  visit_name((Ast_Name*)selector->name);
}

static void
visit_arraySubscript(Ast_ArraySubscript* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->lhs_expr->kind == AST_expression) {
    visit_expression((Ast_Expression*)subscript->lhs_expr);
  } else if (subscript->lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression((Ast_LvalueExpression*)subscript->lhs_expr);
  } else assert(0);
  visit_indexExpression((Ast_IndexExpression*)subscript->index_expr);
}

static void
visit_indexExpression(Ast_IndexExpression* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression((Ast_Expression*)index_expr->start_index);
  if (index_expr->end_index) {
    visit_expression((Ast_Expression*)index_expr->end_index);
  }
}

static void
visit_booleanLiteral(Ast_BooleanLiteral* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
}

static void
visit_integerLiteral(Ast_IntegerLiteral* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
}

static void
visit_stringLiteral(Ast_StringLiteral* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
}

static void
visit_default(Ast_Default* default_)
{
  assert(default_->kind == AST_default);
}

static void
visit_dontcare(Ast_Dontcare* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
}

void
name_decl(Ast_P4Program* ast, Scope* root_scope,
      Hashmap** _scope_map, Hashmap** _field_map, Arena* _storage)
{
  storage = _storage;
  hashmap_create(&scope_map, storage, HKEY_UINT64, sizeof(Scope*), 15, 1023);
  hashmap_create(&field_map, storage, HKEY_UINT64, sizeof(Scope*), 15, 1023);
  current_scope = root_scope;
  visit_p4program(ast);
  assert(current_scope == root_scope);
  *_scope_map = &scope_map;
  *_field_map = &field_map;
}
