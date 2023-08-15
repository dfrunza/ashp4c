#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static Arena*   storage;
static Scope*   root_scope;
static Hashmap* type_table;
static Hashmap* scope_map;
static Hashmap* potential_type;
static PassResult_PotentialType pass_result = {};

/** PROGRAM **/

static void visit_p4program(Ast_P4Program* p4program);
static void visit_declarationList(Ast_DeclarationList* decl_list);
static void visit_declaration(Ast_Declaration* decl);
static void visit_name(Ast_Name* name);
static void visit_parameterList(Ast_ParameterList* params);
static void visit_parameter(Ast_Parameter* param);
static void visit_packageTypeDeclaration(Ast_PackageTypeDeclaration* type_decl);
static void visit_instantiation(Ast_Instantiation* inst);

/** PARSER **/

static void visit_parserDeclaration(Ast_ParserDeclaration* parser_decl);
static void visit_parserTypeDeclaration(Ast_ParserTypeDeclaration* type_decl);
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
static void visit_controlTypeDeclaration(Ast_ControlTypeDeclaration* type_decl);
static void visit_controlLocalDeclarations(Ast_ControlLocalDeclarations* local_decls);
static void visit_controlLocalDeclaration(Ast_ControlLocalDeclaration* local_decl);

/** EXTERN **/

static void visit_externDeclaration(Ast_ExternDeclaration* extern_decl);
static void visit_externTypeDeclaration(Ast_ExternTypeDeclaration* type_decl);
static void visit_methodPrototypes(Ast_MethodPrototypes* protos);
static void visit_functionPrototype(Ast_FunctionPrototype* func_proto);

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
static void visit_typeDeclaration(Ast_TypeDeclaration* type_decl);
static void visit_derivedTypeDeclaration(Ast_DerivedTypeDeclaration* type_decl);
static void visit_headerTypeDeclaration(Ast_HeaderTypeDeclaration* header_decl);
static void visit_headerUnionDeclaration(Ast_HeaderUnionDeclaration* union_decl);
static void visit_structTypeDeclaration(Ast_StructTypeDeclaration* struct_decl);
static void visit_structFieldList(Ast_StructFieldList* field_list);
static void visit_structField(Ast_StructField* field);
static void visit_enumDeclaration(Ast_EnumDeclaration* enum_decl);
static void visit_errorDeclaration(Ast_ErrorDeclaration* error_decl);
static void visit_matchKindDeclaration(Ast_MatchKindDeclaration* match_decl);
static void visit_identifierList(Ast_IdentifierList* ident_list);
static void visit_specifiedIdentifierList(Ast_SpecifiedIdentifierList* ident_list);
static void visit_specifiedIdentifier(Ast_SpecifiedIdentifier* ident);
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
static void visit_tablePropertyList(Ast_TablePropertyList* prop_list);
static void visit_tableProperty(Ast_TableProperty* table_prop);
static void visit_keyProperty(Ast_KeyProperty* key_prop);
static void visit_keyElementList(Ast_KeyElementList* element_list);
static void visit_keyElement(Ast_KeyElement* element);
static void visit_actionsProperty(Ast_ActionsProperty* actions_prop);
static void visit_actionList(Ast_ActionList* action_list);
static void visit_actionRef(Ast_ActionRef* action_ref);
static void visit_entriesProperty(Ast_EntriesProperty* entries_prop);
static void visit_entriesList(Ast_EntriesList* entries_list);
static void visit_entry(Ast_Entry* entry);
static void visit_simpleProperty(Ast_SimpleProperty* simple_prop);
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
  visit_declarationList((Ast_DeclarationList*)p4program->decl_list);
}

static void
visit_declarationList(Ast_DeclarationList* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  for (Ast** p = list_cursor_begin(&decl_list->members);
       p != 0; p = list_cursor_next(&decl_list->members)) {
    visit_declaration((Ast_Declaration*)(*p));
  }
}

static void
visit_declaration(Ast_Declaration* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_externDeclaration) {
    visit_externDeclaration((Ast_ExternDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration((Ast_ActionDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration((Ast_FunctionDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration((Ast_ParserDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration((Ast_ParserTypeDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration((Ast_ControlDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration((Ast_ControlTypeDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration((Ast_TypeDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration((Ast_ErrorDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration((Ast_MatchKindDeclaration*)decl->decl);
  } else if (decl->decl->kind == AST_instantiation) {
    visit_instantiation((Ast_Instantiation*)decl->decl);
  } else assert(0);
}

static void
visit_name(Ast_Name* name)
{
  assert(name->kind == AST_name);
  Scope** scope = hashmap_lookup(scope_map, HASHMAP_KEY_UINT64, (uint64_t)name);
  NameEntry* ns_entry = scope_lookup_namespace(*scope, name->strname, NS_VAR);
  if (ns_entry) {
    NameDecl* namedecl = ns_entry->ns[NS_VAR];
    Type** type = hashmap_lookup(type_table, HASHMAP_KEY_STRING, namedecl->strname);
    if (type) { ; }
  } else error("At line %d, column %d: unknown name `%s`.",
               name->line_no, name->column_no, name->strname);
}

static void
visit_parameterList(Ast_ParameterList* params)
{
  assert(params->kind == AST_parameterList);
  for (Ast** p = list_cursor_begin(&params->members);
       p != 0; p = list_cursor_next(&params->members)) {
    visit_parameter((Ast_Parameter*)(*p));
  }
}

static void
visit_parameter(Ast_Parameter* param)
{
  assert(param->kind == AST_parameter);
  visit_typeRef((Ast_TypeRef*)param->type);
  /* visit_name((Ast_Name*)param->name); */
  if (param->init_expr) {
    visit_expression((Ast_Expression*)param->init_expr);
  }
}

static void
visit_packageTypeDeclaration(Ast_PackageTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  /* visit_name((Ast_Name*)type_decl->name); */
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
}

static void
visit_instantiation(Ast_Instantiation* inst)
{
  assert(inst->kind == AST_instantiation);
  visit_typeRef((Ast_TypeRef*)inst->type_ref);
  visit_argumentList((Ast_ArgumentList*)inst->args);
  /* visit_name((Ast_Name*)inst->name); */
}

/** PARSER **/

static void
visit_parserDeclaration(Ast_ParserDeclaration* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  visit_typeDeclaration((Ast_TypeDeclaration*)parser_decl->proto);
  if (parser_decl->ctor_params) {
    visit_parameterList((Ast_ParameterList*)parser_decl->ctor_params);
  }
  visit_parserLocalElements((Ast_ParserLocalElements*)parser_decl->local_elements);
  visit_parserStates((Ast_ParserStates*)parser_decl->states);
}

static void
visit_parserTypeDeclaration(Ast_ParserTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  /* visit_name((Ast_Name*)type_decl->name); */
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
}

static void
visit_parserLocalElements(Ast_ParserLocalElements* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  for (Ast** p = list_cursor_begin(&local_elements->members);
       p != 0; p = list_cursor_next(&local_elements->members)) {
    visit_parserLocalElement((Ast_ParserLocalElement*)(*p));
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
  for (Ast** p = list_cursor_begin(&states->members);
       p != 0; p = list_cursor_next(&states->members)) {
    visit_parserState((Ast_ParserState*)(*p));
  }
}

static void
visit_parserState(Ast_ParserState* state)
{
  assert(state->kind == AST_parserState);
  /* visit_name((Ast_Name*)state->name); */
  visit_parserStatements((Ast_ParserStatements*)state->stmt_list);
  visit_transitionStatement((Ast_TransitionStatement*)state->transition_stmt);
}

static void
visit_parserStatements(Ast_ParserStatements* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  for (Ast** p = list_cursor_begin(&stmts->members);
       p != 0; p = list_cursor_next(&stmts->members)) {
    visit_parserStatement((Ast_ParserStatement*)(*p));
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
    visit_parserBlockStatement((Ast_ParserBlockStatement*)stmt->stmt);
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
    /* visit_name((Ast_Name*)state_expr->expr); */
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
  for (Ast** p = list_cursor_begin(&case_list->members);
       p != 0; p = list_cursor_next(&case_list->members)) {
    visit_selectCase((Ast_SelectCase*)(*p));
  }
}

static void
visit_selectCase(Ast_SelectCase* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression((Ast_KeysetExpression*)select_case->keyset_expr);
  /* visit_name((Ast_Name*)select_case->name); */
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
  for (Ast** p = list_cursor_begin(&expr_list->members);
       p != 0; p = list_cursor_next(&expr_list->members)) {
    visit_simpleKeysetExpression((Ast_SimpleKeysetExpression*)(*p));
  }
}

/** CONTROL **/

static void
visit_controlDeclaration(Ast_ControlDeclaration* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  visit_typeDeclaration((Ast_TypeDeclaration*)control_decl->proto);
  if (control_decl->ctor_params) {
    visit_parameterList((Ast_ParameterList*)control_decl->ctor_params);
  }
  visit_controlLocalDeclarations((Ast_ControlLocalDeclarations*)control_decl->local_decls);
  visit_blockStatement((Ast_BlockStatement*)control_decl->apply_stmt);
}

static void
visit_controlTypeDeclaration(Ast_ControlTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  /* visit_name((Ast_Name*)type_decl->name); */
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
}

static void
visit_controlLocalDeclarations(Ast_ControlLocalDeclarations* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  for (Ast** p = list_cursor_begin(&local_decls->members);
       p != 0; p = list_cursor_next(&local_decls->members)) {
    visit_controlLocalDeclaration((Ast_ControlLocalDeclaration*)(*p));
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

static void
visit_externDeclaration(Ast_ExternDeclaration* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration((Ast_ExternTypeDeclaration*)extern_decl->decl);
  } else if (extern_decl->decl->kind == AST_functionPrototype) {
    visit_functionPrototype((Ast_FunctionPrototype*)extern_decl->decl);
  } else assert(0);
}

static void
visit_externTypeDeclaration(Ast_ExternTypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  /* visit_name((Ast_Name*)type_decl->name); */
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_methodPrototypes((Ast_MethodPrototypes*)type_decl->method_protos);
}

static void
visit_methodPrototypes(Ast_MethodPrototypes* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  for (Ast** p = list_cursor_begin(&protos->members);
       p != 0; p = list_cursor_next(&protos->members)) {
    visit_functionPrototype((Ast_FunctionPrototype*)(*p));
  }
}

static void
visit_functionPrototype(Ast_FunctionPrototype* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  if (func_proto->return_type) {
    visit_typeRef((Ast_TypeRef*)func_proto->return_type);
  }
  /* visit_name((Ast_Name*)func_proto->name); */
  if (func_proto->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)func_proto->type_params);
  }
  visit_parameterList((Ast_ParameterList*)func_proto->params);
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
    /* visit_name((Ast_Name*)type_ref->type); */
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
  /* visit_name((Ast_Name*)type_decl->name); */
  visit_typeArgumentList((Ast_TypeArgumentList*)type_decl->type_args);
}

static void
visit_headerStackType(Ast_HeaderStackType* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  /* visit_name((Ast_Name*)type_decl->name); */
  visit_typeRef((Ast_TypeRef*)type_decl->type);
  visit_expression((Ast_Expression*)type_decl->stack_expr);
}

static void
visit_specializedType(Ast_SpecializedType* type_decl)
{
  assert(type_decl->kind == AST_specializedType);
  /* visit_name((Ast_Name*)type_decl->name); */
  visit_typeRef((Ast_TypeRef*)type_decl->type);
  visit_typeArgumentList((Ast_TypeArgumentList*)type_decl->type_args);
}

static void
visit_baseTypeBoolean(Ast_BooleanType* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  /* visit_name((Ast_Name*)bool_type->name); */
}

static void
visit_baseTypeInteger(Ast_IntegerType* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  /* visit_name((Ast_Name*)int_type->name); */
  if (int_type->size) {
    visit_integerTypeSize((Ast_IntegerTypeSize*)int_type->size);
  }
}

static void
visit_baseTypeBit(Ast_BitType* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  /* visit_name((Ast_Name*)bit_type->name); */
  if (bit_type->size) {
    visit_integerTypeSize((Ast_IntegerTypeSize*)bit_type->size);
  }
}

static void
visit_baseTypeVarbit(Ast_VarbitType* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  /* visit_name((Ast_Name*)varbit_type->name); */
  visit_integerTypeSize((Ast_IntegerTypeSize*)varbit_type->size);
}

static void
visit_baseTypeString(Ast_StringType* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  /* visit_name((Ast_Name*)str_type->name); */
}

static void
visit_baseTypeVoid(Ast_VoidType* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  /* visit_name((Ast_Name*)void_type->name); */
}

static void
visit_baseTypeError(Ast_ErrorType* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  /* visit_name((Ast_Name*)error_type->name); */
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
  for (Ast** p = list_cursor_begin(&param_list->members);
       p != 0; p = list_cursor_next(&param_list->members)) {
    /* visit_name((Ast_Name*)ast); */
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
    /* visit_name((Ast_Name*)type_arg->arg); */
  } else if (type_arg->arg->kind == AST_dontcare) {
    visit_dontcare((Ast_Dontcare*)type_arg->arg);
  } else assert(0);
}

static void
visit_realTypeArgumentList(Ast_RealTypeArgumentList* arg_list)
{
  assert(arg_list->kind == AST_realTypeArgumentList);
  for (Ast** p = list_cursor_begin(&arg_list->members);
       p != 0; p = list_cursor_next(&arg_list->members)) {
    visit_realTypeArg((Ast_RealTypeArg*)(*p));
  }
}

static void
visit_typeArgumentList(Ast_TypeArgumentList* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  for (Ast** p = list_cursor_begin(&arg_list->members);
       p != 0; p = list_cursor_next(&arg_list->members)) {
    visit_typeArg((Ast_TypeArg*)(*p));
  }
}

static void
visit_typeDeclaration(Ast_TypeDeclaration* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration((Ast_DerivedTypeDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration((Ast_TypedefDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration((Ast_ParserTypeDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration((Ast_ControlTypeDeclaration*)type_decl->decl);
  } else if (type_decl->decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration((Ast_PackageTypeDeclaration*)type_decl->decl);
  } else assert(0);
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
  /* visit_name((Ast_Name*)header_decl->name); */
  visit_structFieldList((Ast_StructFieldList*)header_decl->fields);
}

static void
visit_headerUnionDeclaration(Ast_HeaderUnionDeclaration* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  /* visit_name((Ast_Name*)union_decl->name); */
  visit_structFieldList((Ast_StructFieldList*)union_decl->fields);
}

static void
visit_structTypeDeclaration(Ast_StructTypeDeclaration* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  /* visit_name((Ast_Name*)struct_decl->name); */
  visit_structFieldList((Ast_StructFieldList*)struct_decl->fields);
}

static void
visit_structFieldList(Ast_StructFieldList* field_list)
{
  assert(field_list->kind == AST_structFieldList);
  for (Ast** p = list_cursor_begin(&field_list->members);
       p != 0; p = list_cursor_next(&field_list->members)) {
    visit_structField((Ast_StructField*)(*p));
  }
}

static void
visit_structField(Ast_StructField* field)
{
  assert(field->kind == AST_structField);
  visit_typeRef((Ast_TypeRef*)field->type);
  /* visit_name((Ast_Name*)field->name); */
}

static void
visit_enumDeclaration(Ast_EnumDeclaration* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  /* visit_name((Ast_Name*)enum_decl->name); */
  visit_specifiedIdentifierList((Ast_SpecifiedIdentifierList*)enum_decl->fields);
}

static void
visit_errorDeclaration(Ast_ErrorDeclaration* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  visit_identifierList((Ast_IdentifierList*)error_decl->fields);
}

static void
visit_matchKindDeclaration(Ast_MatchKindDeclaration* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  visit_identifierList((Ast_IdentifierList*)match_decl->fields);
}

static void
visit_identifierList(Ast_IdentifierList* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  for (Ast** p = list_cursor_begin(&ident_list->members);
       p != 0; p = list_cursor_next(&ident_list->members)) {
    /* visit_name((Ast_Name*)ast); */
  }
}

static void
visit_specifiedIdentifierList(Ast_SpecifiedIdentifierList* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  for (Ast** p = list_cursor_begin(&ident_list->members);
       p != 0; p = list_cursor_next(&ident_list->members)) {
    visit_specifiedIdentifier((Ast_SpecifiedIdentifier*)(*p));
  }
}

static void
visit_specifiedIdentifier(Ast_SpecifiedIdentifier* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  /* visit_name((Ast_Name*)ident->name); */
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
  /* visit_name((Ast_Name*)typedef_decl->name); */
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
    /* visit_name((Ast_Name*)applic_stmt->name); */
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
    visit_blockStatement((Ast_BlockStatement*)stmt->stmt);
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
  for (Ast** p = list_cursor_begin(&stmt_list->members);
       p != 0; p = list_cursor_next(&stmt_list->members)) {
    visit_statementOrDeclaration((Ast_StatementOrDeclaration*)(*p));
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
  for (Ast** p = list_cursor_begin(&switch_cases->members);
       p != 0; p = list_cursor_next(&switch_cases->members)) {
    visit_switchCase((Ast_SwitchCase*)(*p));
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
    /* visit_name((Ast_Name*)label->label); */
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
  /* visit_name((Ast_Name*)table_decl->name); */
  visit_tablePropertyList((Ast_TablePropertyList*)table_decl->prop_list);
}

static void
visit_tablePropertyList(Ast_TablePropertyList* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  for (Ast** p = list_cursor_begin(&prop_list->members);
       p != 0; p = list_cursor_next(&prop_list->members)) {
    visit_tableProperty((Ast_TableProperty*)(*p));
  }
}

static void
visit_tableProperty(Ast_TableProperty* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->prop->kind == AST_keyProperty) {
    visit_keyProperty((Ast_KeyProperty*)table_prop->prop);
  } else if (table_prop->prop->kind == AST_actionsProperty) {
    visit_actionsProperty((Ast_ActionsProperty*)table_prop->prop);
  } else if (table_prop->prop->kind == AST_entriesProperty) {
    visit_entriesProperty((Ast_EntriesProperty*)table_prop->prop);
  } else if (table_prop->prop->kind == AST_simpleProperty) {
    visit_simpleProperty((Ast_SimpleProperty*)table_prop->prop);
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
  for (Ast** p = list_cursor_begin(&element_list->members);
       p != 0; p = list_cursor_next(&element_list->members)) {
    visit_keyElement((Ast_KeyElement*)(*p));
  }
}

static void
visit_keyElement(Ast_KeyElement* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression((Ast_Expression*)element->expr);
  /* visit_name((Ast_Name*)element->match); */
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
  for (Ast** p = list_cursor_begin(&action_list->members);
       p != 0; p = list_cursor_next(&action_list->members)) {
    visit_actionRef((Ast_ActionRef*)(*p));
  }
}

static void
visit_actionRef(Ast_ActionRef* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  /* visit_name((Ast_Name*)action_ref->name); */
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
  for (Ast** p = list_cursor_begin(&entries_list->members);
       p != 0; p = list_cursor_next(&entries_list->members)) {
    visit_entry((Ast_Entry*)(*p));
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
visit_simpleProperty(Ast_SimpleProperty* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  /* visit_name((Ast_Name*)simple_prop->name); */
  visit_expression((Ast_Expression*)simple_prop->init_expr);
}

static void
visit_actionDeclaration(Ast_ActionDeclaration* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  /* visit_name((Ast_Name*)action_decl->name); */
  visit_parameterList((Ast_ParameterList*)action_decl->params);
  visit_blockStatement((Ast_BlockStatement*)action_decl->stmt);
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast_VarDeclaration* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  visit_typeRef((Ast_TypeRef*)var_decl->type);
  /* visit_name((Ast_Name*)var_decl->name); */
  if (var_decl->init_expr) {
    visit_expression((Ast_Expression*)var_decl->init_expr);
  }
}

/** EXPRESSIONS **/

static void
visit_functionDeclaration(Ast_FunctionDeclaration* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visit_functionPrototype((Ast_FunctionPrototype*)func_decl->proto);
  visit_blockStatement((Ast_BlockStatement*)func_decl->stmt);
}

static void
visit_argumentList(Ast_ArgumentList* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  for (Ast** p = list_cursor_begin(&arg_list->members);
       p != 0; p = list_cursor_next(&arg_list->members)) {
    visit_argument((Ast_Argument*)(*p));
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
  for (Ast** p = list_cursor_begin(&expr_list->members);
       p != 0; p = list_cursor_next(&expr_list->members)) {
    visit_expression((Ast_Expression*)(*p));
  }
}

static void
visit_lvalueExpression(Ast_LvalueExpression* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->expr->kind == AST_name) {
    /* visit_name((Ast_Name*)lvalue_expr->expr); */
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
    /* visit_name((Ast_Name*)expr->expr); */
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
  /* visit_name((Ast_Name*)selector->name); */
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
  NameDecl* namedecl = scope_lookup_namespace(root_scope, "int", NS_TYPE)->ns[NS_TYPE];
  Type** type = hashmap_lookup(type_table, HASHMAP_KEY_STRING, namedecl->strname);
  if (type) { ; }
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

PassResult_PotentialType*
pass_potential_type(ParsedProgram* p4program, Arena* _storage,
                    PassResult_NameDecl* namedecl_result, PassResult_TypeDecl* typedecl_result)
{
  storage = _storage;
  root_scope = &p4program->root_scope;
  scope_map = &namedecl_result->scope_map;
  type_table = &typedecl_result->type_table;
  potential_type = &pass_result.potential_type;
  hashmap_create(potential_type, storage, HASHMAP_KEY_UINT64, sizeof(Type*), 7, 1023);
  visit_p4program(p4program->ast);
  return &pass_result;
}
