#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal int node_id = 0;

/** PROGRAM **/
internal void visit_p4program(Ast_P4Program* p4program);
internal void visit_declarationList(Ast_List* decl_list);
internal void visit_declaration(Ast_Declaration* decl);
internal void visit_name(Ast_Name* name);
internal void visit_parameterList(Ast_List* params);
internal void visit_parameter(Ast_Parameter* param);
internal void visit_packageTypeDeclaration(Ast_PackageTypeDeclaration* type_decl);
internal void visit_instantiation(Ast_Instantiation* inst);
/** PARSER **/
internal void visit_parserDeclaration(Ast_ParserDeclaration* parser_decl);
internal void visit_parserTypeDeclaration(Ast_ParserPrototype* type_decl);
internal void visit_parserLocalElements(Ast_List* local_elements);
internal void visit_parserLocalElement(Ast_ParserLocalElement* local_element);
internal void visit_parserStates(Ast_List* parser_states);
internal void visit_parserState(Ast_ParserState* parser_state);
internal void visit_parserStatements(Ast_List* parser_stmts);
internal void visit_parserStatement(Ast_ParserStatement* parser_stmt);
internal void visit_parserBlockStatement(Ast_ParserBlockStatement* block_stmt);
internal void visit_transitionStatement(Ast_TransitionStatement* trans_stmt);
internal void visit_stateExpression(Ast_StateExpression* state_expr);
internal void visit_selectExpression(Ast_SelectExpression* select_expr);
internal void visit_selectCaseList(Ast_List* case_list);
internal void visit_selectCase(Ast_SelectCase* select_case);
internal void visit_keysetExpression(Ast_KeysetExpression* keyset_expr);
internal void visit_tupleKeysetExpression(Ast_TupleKeysetExpression* tuple_expr);
internal void visit_keysetExpressionList(Ast_List* keyset_list);
/** CONTROL **/
internal void visit_controlDeclaration(Ast_ControlDeclaration* ctrl_decl);
internal void visit_controlTypeDeclaration(Ast_ControlPrototype* type_decl);
internal void visit_controlLocalDeclarations(Ast_List* local_decls);
internal void visit_controlLocalDeclaration(Ast_ControlLocalDeclaration* local_decl);
/** EXTERN **/
internal void visit_externDeclaration(Ast_ExternDeclaration* extern_decl);
internal void visit_externType(Ast_ExternType* type_decl);
internal void visit_methodPrototypes(Ast_List* prototypes);
internal void visit_functionPrototype(Ast_FunctionPrototype* func_decl);
/** TYPES **/
internal void visit_typeRef(Ast_TypeRef* type_ref);
internal void visit_namedType(Ast_NamedType* type);
internal void visit_tupleType(Ast_TupleType* type);
internal void visit_headerStackType(Ast_HeaderTypeDeclaration* type_decl);
internal void visit_specializedType(Ast_SpecializedType* type_decl);
internal void visit_baseTypeBool(Ast_BoolType* bool_type);
internal void visit_baseTypeInteger(Ast_IntegerType* int_type);
internal void visit_baseTypeBit(Ast_BitType* bit_type);
internal void visit_baseTypeVarbit(Ast_VarbitType* varbit_type);
internal void visit_baseTypeString(Ast_StringType* str_type);
internal void visit_baseTypeVoid(Ast_VoidType* void_type);
internal void visit_baseTypeError(Ast_ErrorType* error_type);
internal void visit_integerTypeSize(Ast_IntegerTypeSize* type_size);
internal void visit_typeParameterList(Ast_List* param_list);
internal void visit_realTypeArg(Ast_RealTypeArg* type_arg);
internal void visit_typeArg(Ast_TypeArg* type_arg);
internal void visit_realTypeArgumentList(Ast_List* arg_list);
internal void visit_typeArgumentList(Ast_List* arg_list);
internal void visit_typeDeclaration(Ast_TypeDeclaration* type_decl);
internal void visit_derivedTypeDeclaration(Ast_DerivedTypeDeclaration* type_decl);
internal void visit_headerTypeDeclaration(Ast_HeaderTypeDeclaration* header_decl);
internal void visit_headerUnionDeclaration(Ast_HeaderUnionDeclaration* union_decl);
internal void visit_structTypeDeclaration(Ast_StructTypeDeclaration* struct_decl);
internal void visit_structFieldList(Ast_List* field_list);
internal void visit_structField(Ast_StructField* field);
internal void visit_enumDeclaration(Ast_EnumDeclaration* enum_decl);
internal void visit_errorDeclaration(Ast_ErrorDeclaration* error_decl);
internal void visit_matchKindDeclaration(Ast_MatchKindDeclaration* match_decl);
internal void visit_identifierList(Ast_List* ident_list);
internal void visit_specifiedIdentifierList(Ast_List* ident_list);
internal void visit_specifiedIdentifier(Ast_SpecifiedIdent* ident);
internal void visit_typedefDeclaration(Ast_TypedefDeclaration* typedef_decl);
/** STATEMENTS **/
internal void visit_assignmentStatement(Ast_AssignmentStatement* assgn_stmt);
internal void visit_functionCall(Ast_FunctionCall* func_call);
internal void visit_returnStatement(Ast_ReturnStatement* return_stmt);
internal void visit_exitStatement(Ast_ExitStatement* exit_stmt);
internal void visit_conditionalStatement(Ast_ConditionalStatement* cond_stmt);
internal void visit_directApplication(Ast_DirectApplication* applic_stmt);
internal void visit_statement(Ast_Statement* stmt);
internal void visit_blockStatement(Ast_BlockStatement* block_stmt);
internal void visit_statementOrDeclList(Ast_List* stmt_list);
internal void visit_switchStatement(Ast_SwitchStatement* switch_stmt);
internal void visit_switchCases(Ast_List* switch_cases);
internal void visit_switchCase(Ast_SwitchCase* switch_case);
internal void visit_switchLabel(Ast_SwitchLabel* label);
internal void visit_statementOrDeclaration(Ast_StatementOrDeclaration* stmt);
/** TABLES **/
internal void visit_tableDeclaration(Ast_TableDeclaration* table_decl);
internal void visit_tablePropertyList(Ast_List* prop_list);
internal void visit_tableProperty(Ast_TableProperty* table_prop);
internal void visit_keyProperty(Ast_KeyProperty* key_prop);
internal void visit_keyElementList(Ast_List* element_list);
internal void visit_keyElement(Ast_KeyElement* element);
internal void visit_actionsProperty(Ast_ActionsProperty* actions_prop);
internal void visit_actionList(Ast_List* action_list);
internal void visit_actionRef(Ast_ActionRef* action_ref);
internal void visit_entriesProperty(Ast_EntriesProperty* entries_prop);
internal void visit_entriesList(Ast_List* entries_list);
internal void visit_entry(Ast_Entry* entry);
internal void visit_simpleProperty(Ast_SimpleProperty* simple_prop);
internal void visit_actionDeclaration(Ast_ActionDeclaration* action_decl);
/** VARIABLES **/
internal void visit_variableDeclarataion(Ast_VarDeclaration* var_decl);
/** EXPRESSIONS **/
internal void visit_functionDeclaration(Ast_FunctionDeclaration* func_decl);
internal void visit_argumentList(Ast_List* arg_list);
internal void visit_argument(Ast_Argument* arg);
internal void visit_kvPair(Ast_KVPair* pair);
internal void visit_expressionList(Ast_List* expr_list);
internal void visit_lvalueExpression(Ast_LvalueExpression* lvalue_expr);
internal void visit_expression(Ast_Expression* expr);
internal void visit_castExpression(Ast_CastExpression* cast_expr);
internal void visit_unaryExpression(Ast_UnaryExpression* unary_expr);
internal void visit_binaryExpression(Ast_BinaryExpression* binary_expr);
internal void visit_memberSelector(Ast_MemberSelector* selector);
internal void visit_arraySubscript(Ast_ArraySubscript* subscript);
internal void visit_indexExpression(Ast_IndexExpression* index_expr);
internal void visit_integerLiteral(Ast_IntegerLiteral* int_literal);
internal void visit_booleanLiteral(Ast_BooleanLiteral* bool_literal);
internal void visit_stringLiteral(Ast_StringLiteral* str_literal);

internal void
visit_variableDeclaration(Ast_VarDeclaration* var_decl)
{
  var_decl->id = ++node_id;
#if 0
  visit_typeRef(var_decl->type);
  visit_name(var_decl->name);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr);
  }
#endif
}

internal void
visit_declaration(Ast_Declaration* decl)
{
  decl->id = ++node_id;
  Ast* ast = decl->decl;
  if (ast->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)ast);
  } else if (ast->kind == AST_externDeclaration) {

  } else if (ast->kind == AST_actionDeclaration) {

  } else if (ast->kind == AST_functionDeclaration) {

  } else if (ast->kind == AST_parserDeclaration) {

  } else if (ast->kind == AST_parserTypeDeclaration) {

  } else if (ast->kind == AST_controlDeclaration) {

  } else if (ast->kind == AST_controlTypeDeclaration) {

  } else if (ast->kind == AST_typeDeclaration) {

  } else if (ast->kind == AST_errorDeclaration) {

  } else if (ast->kind == AST_matchKindDeclaration) {

  } else if (ast->kind == AST_instantiation) {

  }
  else assert(0);
}

internal void
visit_declarationList(Ast_List* decl_list)
{
  decl_list->id = ++node_id;
  for (ListItem* li = decl_list->members.sentinel.next;
        li != 0; li = li->next) {
    visit_declaration(li->object);
  }
}

internal void
visit_p4program(Ast_P4Program* p4program)
{
  p4program->id = ++node_id;
  visit_declarationList((Ast_List*)p4program->decl_list);
}

void
node_id_pass(Ast_P4Program* p4program)
{
  visit_p4program(p4program);
}

