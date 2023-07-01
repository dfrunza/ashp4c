#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal int node_id = 0;

/** PROGRAM **/

/* 1 */ internal void visit_p4program(Ast_P4Program* p4program);
/* 2 */ internal void visit_declarationList(Ast_DeclarationList* decl_list);
/* 3 */ internal void visit_declaration(Ast_Declaration* decl);
/* 4 */ internal void visit_name(Ast_Name* name);
/* 5 */ internal void visit_parameterList(Ast_ParameterList* params);
/* 6 */ internal void visit_parameter(Ast_Parameter* param);
/* 7 */ internal void visit_packageTypeDeclaration(Ast_PackageTypeDeclaration* type_decl);
/* 8 */ internal void visit_instantiation(Ast_Instantiation* inst);

/** PARSER **/

/* 1 */ internal void visit_parserDeclaration(Ast_ParserDeclaration* parser_decl);
/* 2 */ internal void visit_parserTypeDeclaration(Ast_ParserTypeDeclaration* type_decl);
/* 3 */ internal void visit_parserLocalElements(Ast_ParserLocalElements* local_elements);
/* 4 */ internal void visit_parserLocalElement(Ast_ParserLocalElement* local_element);
/* 5 */ internal void visit_parserStates(Ast_ParserStates* states);
/* 6 */ internal void visit_parserState(Ast_ParserState* state);
/* 7 */ internal void visit_parserStatements(Ast_ParserStatements* stmts);
/* 8 */ internal void visit_parserStatement(Ast_ParserStatement* stmt);
/* 9 */ internal void visit_parserBlockStatement(Ast_ParserBlockStatement* block_stmt);
/* 10 */ internal void visit_transitionStatement(Ast_TransitionStatement* transition_stmt);
/* 11 */ internal void visit_stateExpression(Ast_StateExpression* state_expr);
/* 12 */ internal void visit_selectExpression(Ast_SelectExpression* select_expr);
/* 13 */ internal void visit_selectCaseList(Ast_SelectCaseList* case_list);
/* 14 */ internal void visit_selectCase(Ast_SelectCase* select_case);
/* 15 */ internal void visit_keysetExpression(Ast_KeysetExpression* keyset_expr);
/* 16 */ internal void visit_tupleKeysetExpression(Ast_TupleKeysetExpression* tuple_expr);
/* 17 */ internal void visit_keysetExpressionList(Ast_KeysetExpressionList* keyset_list);

/** CONTROL **/

/* 1 */ internal void visit_controlDeclaration(Ast_ControlDeclaration* control_decl);
/* 2 */ internal void visit_controlTypeDeclaration(Ast_ControlTypeDeclaration* type_decl);
/* 3 */ internal void visit_controlLocalDeclarations(Ast_ControlLocalDeclarations* local_decls);
/* 4 */ internal void visit_controlLocalDeclaration(Ast_ControlLocalDeclaration* local_decl);

/** EXTERN **/

/* 1 */ internal void visit_externDeclaration(Ast_ExternDeclaration* extern_decl);
/* 2 */ internal void visit_externTypeDeclaration(Ast_ExternTypeDeclaration* type_decl);
/* 3 */ internal void visit_methodPrototypes(Ast_MethodPrototypes* prototypes);
/* 4 */ internal void visit_functionPrototype(Ast_FunctionPrototype* func_decl);

/** TYPES **/

/* 1 */ internal void visit_typeRef(Ast_TypeRef* type_ref);
/* 2 */ internal void visit_namedType(Ast_NamedType* type);
/* 3 */ internal void visit_tupleType(Ast_TupleType* type);
/* 4 */ internal void visit_headerStackType(Ast_HeaderTypeDeclaration* type_decl);
/* 5 */ internal void visit_specializedType(Ast_SpecializedType* type_decl);
/* 6 */ internal void visit_baseTypeBool(Ast_BoolType* bool_type);
/* 7 */ internal void visit_baseTypeInteger(Ast_IntegerType* int_type);
/* 8 */ internal void visit_baseTypeBit(Ast_BitType* bit_type);
/* 9 */ internal void visit_baseTypeVarbit(Ast_VarbitType* varbit_type);
/* 10 */ internal void visit_baseTypeString(Ast_StringType* str_type);
/* 11 */ internal void visit_baseTypeVoid(Ast_VoidType* void_type);
/* 12 */ internal void visit_baseTypeError(Ast_ErrorType* error_type);
/* 13 */ internal void visit_integerTypeSize(Ast_IntegerTypeSize* type_size);
/* 14 */ internal void visit_typeParameterList(Ast_TypeParameterList* param_list);
/* 15 */ internal void visit_realTypeArg(Ast_RealTypeArg* type_arg);
/* 16 */ internal void visit_typeArg(Ast_TypeArg* type_arg);
/* 17 */ internal void visit_realTypeArgumentList(Ast_RealTypeArgumentList* arg_list);
/* 18 */ internal void visit_typeArgumentList(Ast_TypeArgumentList* arg_list);
/* 19 */ internal void visit_typeDeclaration(Ast_TypeDeclaration* type_decl);
/* 20 */ internal void visit_derivedTypeDeclaration(Ast_DerivedTypeDeclaration* type_decl);
/* 21 */ internal void visit_headerTypeDeclaration(Ast_HeaderTypeDeclaration* header_decl);
/* 22 */ internal void visit_headerUnionDeclaration(Ast_HeaderUnionDeclaration* union_decl);
/* 23 */ internal void visit_structTypeDeclaration(Ast_StructTypeDeclaration* struct_decl);
/* 24 */ internal void visit_structFieldList(Ast_StructFieldList* field_list);
/* 25 */ internal void visit_structField(Ast_StructField* field);
/* 26 */ internal void visit_enumDeclaration(Ast_EnumDeclaration* enum_decl);
/* 27 */ internal void visit_errorDeclaration(Ast_ErrorDeclaration* error_decl);
/* 28 */ internal void visit_matchKindDeclaration(Ast_MatchKindDeclaration* match_decl);
/* 29 */ internal void visit_identifierList(Ast_IdentifierList* ident_list);
/* 30 */ internal void visit_specifiedIdentifierList(Ast_SpecifiedIdentifierList* ident_list);
/* 31 */ internal void visit_specifiedIdentifier(Ast_SpecifiedIdentifier* ident);
/* 32 */ internal void visit_typedefDeclaration(Ast_TypedefDeclaration* typedef_decl);

/** STATEMENTS **/

/* 1 */ internal void visit_assignmentStatement(Ast_AssignmentStatement* assgn_stmt);
/* 2 */ internal void visit_functionCall(Ast_FunctionCall* func_call);
/* 3 */ internal void visit_returnStatement(Ast_ReturnStatement* return_stmt);
/* 4 */ internal void visit_exitStatement(Ast_ExitStatement* exit_stmt);
/* 5 */ internal void visit_conditionalStatement(Ast_ConditionalStatement* cond_stmt);
/* 6 */ internal void visit_directApplication(Ast_DirectApplication* applic_stmt);
/* 7 */ internal void visit_statement(Ast_Statement* stmt);
/* 8 */ internal void visit_blockStatement(Ast_BlockStatement* block_stmt);
/* 9 */ internal void visit_statementOrDeclList(Ast_StatementOrDeclList* stmt_list);
/* 10 */ internal void visit_switchStatement(Ast_SwitchStatement* switch_stmt);
/* 11 */ internal void visit_switchCases(Ast_SwitchCases* switch_cases);
/* 12 */ internal void visit_switchCase(Ast_SwitchCase* switch_case);
/* 13 */ internal void visit_switchLabel(Ast_SwitchLabel* label);
/* 14 */ internal void visit_statementOrDeclaration(Ast_StatementOrDeclaration* stmt);

/** TABLES **/

/* 1 */ internal void visit_tableDeclaration(Ast_TableDeclaration* table_decl);
/* 2 */ internal void visit_tablePropertyList(Ast_TablePropertyList* prop_list);
/* 3 */ internal void visit_tableProperty(Ast_TableProperty* table_prop);
/* 4 */ internal void visit_keyProperty(Ast_KeyProperty* key_prop);
/* 5 */ internal void visit_keyElementList(Ast_KeyElementList* element_list);
/* 6 */ internal void visit_keyElement(Ast_KeyElement* element);
/* 7 */ internal void visit_actionsProperty(Ast_ActionsProperty* actions_prop);
/* 8 */ internal void visit_actionList(Ast_ActionList* action_list);
/* 9 */ internal void visit_actionRef(Ast_ActionRef* action_ref);
/* 10 */ internal void visit_entriesProperty(Ast_EntriesProperty* entries_prop);
/* 11 */ internal void visit_entriesList(Ast_EntriesList* entries_list);
/* 12 */ internal void visit_entry(Ast_Entry* entry);
/* 13 */ internal void visit_simpleProperty(Ast_SimpleProperty* simple_prop);
/* 14 */ internal void visit_actionDeclaration(Ast_ActionDeclaration* action_decl);

/** VARIABLES **/

/* 1 */ internal void visit_variableDeclaration(Ast_VarDeclaration* var_decl);

/** EXPRESSIONS **/

/* 1 */ internal void visit_functionDeclaration(Ast_FunctionDeclaration* func_decl);
/* 2 */ internal void visit_argumentList(Ast_ArgumentList* arg_list);
/* 3 */ internal void visit_argument(Ast_Argument* arg);
/* 4 */ internal void visit_kvPair(Ast_KVPair* pair);
/* 5 */ internal void visit_expressionList(Ast_ExpressionList* expr_list);
/* 6 */ internal void visit_lvalueExpression(Ast_LvalueExpression* lvalue_expr);
/* 7 */ internal void visit_expression(Ast_Expression* expr);
/* 8 */ internal void visit_castExpression(Ast_CastExpression* cast_expr);
/* 9 */ internal void visit_unaryExpression(Ast_UnaryExpression* unary_expr);
/* 10 */ internal void visit_binaryExpression(Ast_BinaryExpression* binary_expr);
/* 11 */ internal void visit_memberSelector(Ast_MemberSelector* selector);
/* 12 */ internal void visit_arraySubscript(Ast_ArraySubscript* subscript);
/* 13 */ internal void visit_indexExpression(Ast_IndexExpression* index_expr);
/* 14 */ internal void visit_integerLiteral(Ast_IntegerLiteral* int_literal);
/* 15 */ internal void visit_booleanLiteral(Ast_BooleanLiteral* bool_literal);
/* 16 */ internal void visit_stringLiteral(Ast_StringLiteral* str_literal);

/** PROGRAM **/

internal void
/* 1 */ visit_p4program(Ast_P4Program* p4program)
{
  p4program->id = ++node_id;
  visit_declarationList((Ast_DeclarationList*)p4program->decl_list);
}

/* 2 */ internal void
visit_declarationList(Ast_DeclarationList* decl_list)
{
  decl_list->id = ++node_id;
  for (ListItem* li = decl_list->members.sentinel.next;
        li != 0; li = li->next) {
    visit_declaration((Ast_Declaration*)li->object);
  }
}

/* 3 */ internal void
visit_declaration(Ast_Declaration* decl)
{
  decl->id = ++node_id;
  Ast* inner_decl = decl->decl;
  if (inner_decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_externDeclaration) {
    visit_externDeclaration((Ast_ExternDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration((Ast_ActionDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration((Ast_FunctionDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration((Ast_ParserDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration((Ast_ParserTypeDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration((Ast_ControlDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration((Ast_ControlTypeDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration((Ast_TypeDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration((Ast_ErrorDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration((Ast_MatchKindDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_instantiation) {
    visit_instantiation((Ast_Instantiation*)inner_decl);
  }
  else assert(0);
}

/* 4 */ internal void visit_name(Ast_Name* name)
{
  name->id = ++node_id;
}

/* 5 */ internal void visit_parameterList(Ast_ParameterList* params)
{
  params->id = ++node_id;
  for (ListItem* li = params->members.sentinel.next;
        li != 0; li = li->next) {
    visit_parameter((Ast_Parameter*)li->object);
  }
}

/* 6 */ internal void visit_parameter(Ast_Parameter* param)
{
  param->id = ++node_id;
  visit_typeRef((Ast_TypeRef*)param->type);
  visit_name((Ast_Name*)param->name);
  if (param->init_expr) {
    visit_expression((Ast_Expression*)param->init_expr);
  }
}

/* 7 */ internal void visit_packageTypeDeclaration(Ast_PackageTypeDeclaration* type_decl)
{
  type_decl->id = ++node_id;
  visit_name((Ast_Name*)type_decl->name);
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
}

/* 8 */ internal void visit_instantiation(Ast_Instantiation* inst)
{
  inst->id = ++node_id;
  visit_typeRef((Ast_TypeRef*)inst->type_ref);
  visit_argumentList((Ast_ArgumentList*)inst->args);
  visit_name((Ast_Name*)inst->name);
}

/** PARSER **/
/* 1 */ internal void visit_parserDeclaration(Ast_ParserDeclaration* parser_decl)
{
  parser_decl->id = ++node_id;
  visit_typeDeclaration((Ast_TypeDeclaration*)parser_decl->proto);
  if (parser_decl->ctor_params) {
    visit_parameterList((Ast_ParameterList*)parser_decl->ctor_params);
  }
  visit_parserLocalElements((Ast_ParserLocalElements*)parser_decl->local_elements);
  visit_parserStates((Ast_ParserStates*)parser_decl->states);
}

/* 2 */ internal void visit_parserTypeDeclaration(Ast_ParserTypeDeclaration* type_decl)
{
  type_decl->id = ++node_id;
  visit_name((Ast_Name*)type_decl->name);
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
}

/* 3 */ internal void visit_parserLocalElements(Ast_ParserLocalElements* local_elements)
{
  local_elements->id = ++node_id;
  for (ListItem* li = local_elements->members.sentinel.next;
        li != 0; li = li->next) {
    visit_parserLocalElement((Ast_ParserLocalElement*)li->object);
  }
}

/* 4 */ internal void visit_parserLocalElement(Ast_ParserLocalElement* local_element)
{
  local_element->id = ++node_id;
  Ast* inner_element = local_element->element;
  if (inner_element->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)inner_element);
  } else if (inner_element->kind == AST_instantiation) {
    visit_instantiation((Ast_Instantiation*)inner_element);
  }
  else assert(0);
}

/* 5 */ internal void visit_parserStates(Ast_ParserStates* states)
{
  states->id = ++node_id;
  for (ListItem* li = states->members.sentinel.next;
        li != 0; li = li->next) {
    visit_parserState((Ast_ParserState*)li->object);
  }
}

/* 6 */ internal void visit_parserState(Ast_ParserState* state)
{
  state->id = ++node_id;
  visit_name((Ast_Name*)state->name);
  visit_parserStatements((Ast_ParserStatements*)state->stmt_list);
  visit_transitionStatement((Ast_TransitionStatement*)state->transition_stmt);
}

/* 7 */ internal void visit_parserStatements(Ast_ParserStatements* stmts)
{
  assert(0);
}
/* 8 */ internal void visit_parserStatement(Ast_ParserStatement* stmt)
{
  assert(0);
}
/* 9 */ internal void visit_parserBlockStatement(Ast_ParserBlockStatement* block_stmt)
{
  assert(0);
}
/* 10 */ internal void visit_transitionStatement(Ast_TransitionStatement* transition_stmt)
{
  assert(0);
}
/* 11 */ internal void visit_stateExpression(Ast_StateExpression* state_expr)
{
  assert(0);
}
/* 12 */ internal void visit_selectExpression(Ast_SelectExpression* select_expr)
{
  assert(0);
}
/* 13 */ internal void visit_selectCaseList(Ast_SelectCaseList* case_list)
{
  assert(0);
}
/* 14 */ internal void visit_selectCase(Ast_SelectCase* select_case)
{
  assert(0);
}
/* 15 */ internal void visit_keysetExpression(Ast_KeysetExpression* keyset_expr)
{
  assert(0);
}
/* 16 */ internal void visit_tupleKeysetExpression(Ast_TupleKeysetExpression* tuple_expr)
{
  assert(0);
}
/* 17 */ internal void visit_keysetExpressionList(Ast_KeysetExpressionList* keyset_list)
{
  assert(0);
}

/** CONTROL **/
/* 1 */ internal void visit_controlDeclaration(Ast_ControlDeclaration* control_decl)
{
  control_decl->id = ++node_id;
  visit_typeDeclaration((Ast_TypeDeclaration*)control_decl->proto);
  if (control_decl->ctor_params) {
    visit_parameterList((Ast_ParameterList*)control_decl->ctor_params);
  }
  visit_controlLocalDeclarations((Ast_ControlLocalDeclarations*)control_decl->local_decls);
  visit_blockStatement((Ast_BlockStatement*)control_decl->apply_stmt);
}

/* 2 */ internal void visit_controlTypeDeclaration(Ast_ControlTypeDeclaration* type_decl)
{
  type_decl->id = ++node_id;
  visit_name((Ast_Name*)type_decl->name);
  if (type_decl->type_params) {
    visit_typeParameterList((Ast_TypeParameterList*)type_decl->type_params);
  }
  visit_parameterList((Ast_ParameterList*)type_decl->params);
}

/* 3 */ internal void visit_controlLocalDeclarations(Ast_ControlLocalDeclarations* local_decls)
{
  assert(0);
}

/* 4 */ internal void visit_controlLocalDeclaration(Ast_ControlLocalDeclaration* local_decl)
{
  assert(0);
}

/** EXTERN **/
/* 1 */ internal void visit_externDeclaration(Ast_ExternDeclaration* extern_decl)
{
  extern_decl->id = ++node_id;
  Ast* inner_decl = extern_decl->decl;
  if (inner_decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration((Ast_ExternTypeDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_functionPrototype) {
    visit_functionPrototype((Ast_FunctionPrototype*)inner_decl);
  }
  else assert(0);
}

/* 2 */ internal void visit_externTypeDeclaration(Ast_ExternTypeDeclaration* type_decl)
{
  assert(0);
}
/* 3 */ internal void visit_methodPrototypes(Ast_MethodPrototypes* prototypes)
{
  assert(0);
}
/* 4 */ internal void visit_functionPrototype(Ast_FunctionPrototype* func_decl)
{
  assert(0);
}

/** TYPES **/
/* 1 */ internal void visit_typeRef(Ast_TypeRef* type_ref)
{
  assert(0);
}
/* 2 */ internal void visit_namedType(Ast_NamedType* type)
{
  assert(0);
}
/* 3 */ internal void visit_tupleType(Ast_TupleType* type)
{
  assert(0);
}
/* 4 */ internal void visit_headerStackType(Ast_HeaderTypeDeclaration* type_decl)
{
  assert(0);
}
/* 5 */ internal void visit_specializedType(Ast_SpecializedType* type_decl)
{
  assert(0);
}
/* 6 */ internal void visit_baseTypeBool(Ast_BoolType* bool_type)
{
  assert(0);
}
/* 7 */ internal void visit_baseTypeInteger(Ast_IntegerType* int_type)
{
  assert(0);
}
/* 8 */ internal void visit_baseTypeBit(Ast_BitType* bit_type)
{
  assert(0);
}
/* 9 */ internal void visit_baseTypeVarbit(Ast_VarbitType* varbit_type)
{
  assert(0);
}
/* 10 */ internal void visit_baseTypeString(Ast_StringType* str_type)
{
  assert(0);
}
/* 11 */ internal void visit_baseTypeVoid(Ast_VoidType* void_type)
{
  assert(0);
}
/* 12 */ internal void visit_baseTypeError(Ast_ErrorType* error_type)
{
  assert(0);
}
/* 13 */ internal void visit_integerTypeSize(Ast_IntegerTypeSize* type_size)
{
  assert(0);
}
/* 14 */ internal void visit_typeParameterList(Ast_TypeParameterList* param_list)
{
  assert(0);
}
/* 15 */ internal void visit_realTypeArg(Ast_RealTypeArg* type_arg)
{
  assert(0);
}
/* 16 */ internal void visit_typeArg(Ast_TypeArg* type_arg)
{
  assert(0);
}
/* 17 */ internal void visit_realTypeArgumentList(Ast_RealTypeArgumentList* arg_list)
{
  assert(0);
}
/* 18 */ internal void visit_typeArgumentList(Ast_TypeArgumentList* arg_list)
{
  assert(0);
}

/* 19 */ internal void visit_typeDeclaration(Ast_TypeDeclaration* type_decl)
{
  type_decl->id = ++node_id;
  Ast* inner_decl = type_decl->decl;
  if (inner_decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration((Ast_DerivedTypeDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration((Ast_TypedefDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration((Ast_ParserTypeDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration((Ast_ControlTypeDeclaration*)inner_decl);
  } else if (inner_decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration((Ast_PackageTypeDeclaration*)inner_decl);
  }
  else assert(0);
}

/* 20 */ internal void visit_derivedTypeDeclaration(Ast_DerivedTypeDeclaration* type_decl)
{
  assert(0);
}
/* 21 */ internal void visit_headerTypeDeclaration(Ast_HeaderTypeDeclaration* header_decl)
{
  assert(0);
}
/* 22 */ internal void visit_headerUnionDeclaration(Ast_HeaderUnionDeclaration* union_decl)
{
  assert(0);
}
/* 23 */ internal void visit_structTypeDeclaration(Ast_StructTypeDeclaration* struct_decl)
{
  assert(0);
}
/* 24 */ internal void visit_structFieldList(Ast_StructFieldList* field_list)
{
  assert(0);
}
/* 25 */ internal void visit_structField(Ast_StructField* field)
{
  assert(0);
}
/* 26 */ internal void visit_enumDeclaration(Ast_EnumDeclaration* enum_decl)
{
  assert(0);
}

/* 27 */ internal void visit_errorDeclaration(Ast_ErrorDeclaration* error_decl)
{
  error_decl->id = ++node_id;
  visit_identifierList((Ast_IdentifierList*)error_decl->fields);
}

/* 28 */ internal void visit_matchKindDeclaration(Ast_MatchKindDeclaration* match_decl)
{
  match_decl->id = ++node_id;
  visit_identifierList((Ast_IdentifierList*)match_decl->fields);
}

/* 29 */ internal void visit_identifierList(Ast_IdentifierList* ident_list)
{
  assert(0);
}
/* 30 */ internal void visit_specifiedIdentifierList(Ast_SpecifiedIdentifierList* ident_list)
{
  assert(0);
}
/* 31 */ internal void visit_specifiedIdentifier(Ast_SpecifiedIdentifier* ident)
{
  assert(0);
}
/* 32 */ internal void visit_typedefDeclaration(Ast_TypedefDeclaration* typedef_decl)
{
  assert(0);
}

/** STATEMENTS **/
/* 1 */ internal void visit_assignmentStatement(Ast_AssignmentStatement* assgn_stmt)
{
  assert(0);
}
/* 2 */ internal void visit_functionCall(Ast_FunctionCall* func_call)
{
  assert(0);
}
/* 3 */ internal void visit_returnStatement(Ast_ReturnStatement* return_stmt)
{
  assert(0);
}
/* 4 */ internal void visit_exitStatement(Ast_ExitStatement* exit_stmt)
{
  assert(0);
}
/* 5 */ internal void visit_conditionalStatement(Ast_ConditionalStatement* cond_stmt)
{
  assert(0);
}
/* 6 */ internal void visit_directApplication(Ast_DirectApplication* applic_stmt)
{
  assert(0);
}
/* 7 */ internal void visit_statement(Ast_Statement* stmt)
{
  assert(0);
}
/* 8 */ internal void visit_blockStatement(Ast_BlockStatement* block_stmt)
{
  assert(0);
}
/* 9 */ internal void visit_statementOrDeclList(Ast_StatementOrDeclList* stmt_list)
{
  assert(0);
}
/* 10 */ internal void visit_switchStatement(Ast_SwitchStatement* switch_stmt)
{
  assert(0);
}
/* 11 */ internal void visit_switchCases(Ast_SwitchCases* switch_cases)
{
  assert(0);
}
/* 12 */ internal void visit_switchCase(Ast_SwitchCase* switch_case)
{
  assert(0);
}
/* 13 */ internal void visit_switchLabel(Ast_SwitchLabel* label)
{
  assert(0);
}
/* 14 */ internal void visit_statementOrDeclaration(Ast_StatementOrDeclaration* stmt)
{
  assert(0);
}

/** TABLES **/
/* 1 */ internal void visit_tableDeclaration(Ast_TableDeclaration* table_decl)
{
  assert(0);
}
/* 2 */ internal void visit_tablePropertyList(Ast_TablePropertyList* prop_list)
{
  assert(0);
}
/* 3 */ internal void visit_tableProperty(Ast_TableProperty* table_prop)
{
  assert(0);
}
/* 4 */ internal void visit_keyProperty(Ast_KeyProperty* key_prop)
{
  assert(0);
}
/* 5 */ internal void visit_keyElementList(Ast_KeyElementList* element_list)
{
  assert(0);
}
/* 6 */ internal void visit_keyElement(Ast_KeyElement* element)
{
  assert(0);
}
/* 7 */ internal void visit_actionsProperty(Ast_ActionsProperty* actions_prop)
{
  assert(0);
}
/* 8 */ internal void visit_actionList(Ast_ActionList* action_list)
{
  assert(0);
}
/* 9 */ internal void visit_actionRef(Ast_ActionRef* action_ref)
{
  assert(0);
}
/* 10 */ internal void visit_entriesProperty(Ast_EntriesProperty* entries_prop)
{
  assert(0);
}
/* 11 */ internal void visit_entriesList(Ast_EntriesList* entries_list)
{
  assert(0);
}
/* 12 */ internal void visit_entry(Ast_Entry* entry)
{
  assert(0);
}
/* 13 */ internal void visit_simpleProperty(Ast_SimpleProperty* simple_prop)
{
  assert(0);
}

/* 14 */ internal void visit_actionDeclaration(Ast_ActionDeclaration* action_decl)
{
  action_decl->id = ++node_id;
  visit_name((Ast_Name*)action_decl->name);
  visit_parameterList((Ast_ParameterList*)action_decl->params);
  visit_blockStatement((Ast_BlockStatement*)action_decl->stmt);
}

/** VARIABLES **/

/* 1 */ internal void
visit_variableDeclaration(Ast_VarDeclaration* var_decl)
{
  var_decl->id = ++node_id;
  visit_typeRef((Ast_TypeRef*)var_decl->type);
  visit_name((Ast_Name*)var_decl->name);
  if (var_decl->init_expr) {
    visit_expression((Ast_Expression*)var_decl->init_expr);
  }
}

/** EXPRESSIONS **/
/* 1 */ internal void visit_functionDeclaration(Ast_FunctionDeclaration* func_decl)
{
  func_decl->id = ++node_id;
  visit_functionPrototype((Ast_FunctionPrototype*)func_decl->proto);
  visit_blockStatement((Ast_BlockStatement*)func_decl->stmt);
}

/* 2 */ internal void visit_argumentList(Ast_ArgumentList* arg_list)
{
  assert(0);
}
/* 3 */ internal void visit_argument(Ast_Argument* arg)
{
  assert(0);
}
/* 4 */ internal void visit_kvPair(Ast_KVPair* pair)
{
  assert(0);
}
/* 5 */ internal void visit_expressionList(Ast_ExpressionList* expr_list)
{
  assert(0);
}
/* 6 */ internal void visit_lvalueExpression(Ast_LvalueExpression* lvalue_expr)
{
  assert(0);
}
/* 7 */ internal void visit_expression(Ast_Expression* expr)
{
  assert(0);
}
/* 8 */ internal void visit_castExpression(Ast_CastExpression* cast_expr)
{
  assert(0);
}
/* 9 */ internal void visit_unaryExpression(Ast_UnaryExpression* unary_expr)
{
  assert(0);
}
/* 10 */ internal void visit_binaryExpression(Ast_BinaryExpression* binary_expr)
{
  assert(0);
}
/* 11 */ internal void visit_memberSelector(Ast_MemberSelector* selector)
{
  assert(0);
}
/* 12 */ internal void visit_arraySubscript(Ast_ArraySubscript* subscript)
{
  assert(0);
}
/* 13 */ internal void visit_indexExpression(Ast_IndexExpression* index_expr)
{
  assert(0);
}
/* 14 */ internal void visit_integerLiteral(Ast_IntegerLiteral* int_literal)
{
  assert(0);
}
/* 15 */ internal void visit_booleanLiteral(Ast_BooleanLiteral* bool_literal)
{
  assert(0);
}
/* 16 */ internal void visit_stringLiteral(Ast_StringLiteral* str_literal)
{
  assert(0);
}

void
node_id_pass(Ast_P4Program* p4program)
{
  visit_p4program(p4program);
}
