#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal void
traverse_ast(Ast* ast, AstVisitor* walk_in, AstVisitor* walk_out)
{
  walk_in(ast);

  /** PROGRAM **/
  if (ast->kind == AST_p4program) {
    Ast_P4Program* program = (Ast_P4Program*)ast;
    traverse_ast(program->decl_list, walk_in, walk_out);
  } else if (ast->kind == AST_declarationList) {
    Ast_List* decl_list = (Ast_List*)ast;
    for (ListItem* li = decl_list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_declaration) {
    Ast_Declaration* decl = (Ast_Declaration*)ast;
    traverse_ast(decl->decl, walk_in, walk_out);
  } else if (ast->kind == AST_name) {
  } else if (ast->kind == AST_parameterList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_parameter) {
    Ast_Parameter* param = (Ast_Parameter*)ast;
    traverse_ast(param->name, walk_in, walk_out);
    traverse_ast(param->type, walk_in, walk_out);
    if (param->init_expr) {
      traverse_ast(param->init_expr, walk_in, walk_out);
    }
  } else if (ast->kind == AST_packageTypeDeclaration) {
    Ast_PackageTypeDeclaration* package_decl = (Ast_PackageTypeDeclaration*)ast;
    traverse_ast(package_decl->name, walk_in, walk_out);
    if (package_decl->type_params) {
      traverse_ast(package_decl->type_params, walk_in, walk_out);
    }
    if (package_decl->params) {
      traverse_ast(package_decl->params, walk_in, walk_out);
    }
  } else if (ast->kind == AST_instantiation) {
    Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
    traverse_ast(inst_decl->type_ref, walk_in, walk_out);
    if (inst_decl->args) {
      traverse_ast(inst_decl->args, walk_in, walk_out);
    }
    traverse_ast(inst_decl->name, walk_in, walk_out);
  }

  /** PARSER **/
  else if (ast->kind == AST_parserDeclaration) {
    Ast_ParserDeclaration* parser_decl = (Ast_ParserDeclaration*)ast;
    traverse_ast(parser_decl->proto, walk_in, walk_out);
    if (parser_decl->ctor_params) {
      traverse_ast(parser_decl->ctor_params, walk_in, walk_out);
    }
    if (parser_decl->local_elements) {
      traverse_ast(parser_decl->local_elements, walk_in, walk_out);
    }
    if (parser_decl->states) {
      traverse_ast(parser_decl->states, walk_in, walk_out);
    }
  } else if (ast->kind == AST_parserTypeDeclaration) {
    Ast_ParserPrototype* parser_proto = (Ast_ParserPrototype*)ast;
    traverse_ast(parser_proto->name, walk_in, walk_out);
    if (parser_proto->type_params) {
      traverse_ast(parser_proto->type_params, walk_in, walk_out);
    }
    if (parser_proto->params) {
      traverse_ast(parser_proto->params, walk_in, walk_out);
    }
  } else if (ast->kind == AST_parserLocalElements) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_parserLocalElement) {
    Ast_ParserLocalElement* local_elem = (Ast_ParserLocalElement*)ast;
    traverse_ast(local_elem->element, walk_in, walk_out);
  } else if (ast->kind == AST_parserStates) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_parserState) {
    Ast_ParserState* parser_state = (Ast_ParserState*)ast;
    traverse_ast(parser_state->name, walk_in, walk_out);
    if (parser_state->stmt_list) {
      traverse_ast(parser_state->stmt_list, walk_in, walk_out);
    }
    if (parser_state->transition_stmt) {
      traverse_ast(parser_state->transition_stmt, walk_in, walk_out);
    }
  } else if (ast->kind == AST_parserStatements) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_parserStatement) {
    Ast_ParserStmt* parser_stmt = (Ast_ParserStmt*)ast;
    traverse_ast(parser_stmt->stmt, walk_in, walk_out);
  } else if (ast->kind == AST_parserBlockStatement) {
    Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
    traverse_ast(block_stmt->stmt_list, walk_in, walk_out);
  } else if (ast->kind == AST_transitionStatement) {
    Ast_TransitionStmt* transition_stmt = (Ast_TransitionStmt*)ast;
    traverse_ast(transition_stmt->stmt, walk_in, walk_out);
  } else if (ast->kind == AST_stateExpression) {
    Ast_StateExpression* state_expr = (Ast_StateExpression*)ast;
    traverse_ast(state_expr->expr, walk_in, walk_out);
  } else if (ast->kind == AST_selectExpression) {
    Ast_SelectExpression* select_expr = (Ast_SelectExpression*)ast;
    traverse_ast(select_expr->expr_list, walk_in, walk_out);
    traverse_ast(select_expr->case_list, walk_in, walk_out);
  } else if (ast->kind == AST_selectCaseList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_selectCase) {
    Ast_SelectCase* select_case = (Ast_SelectCase*)ast;
    traverse_ast(select_case->keyset_expr, walk_in, walk_out);
    traverse_ast(select_case->name, walk_in, walk_out);
  } else if (ast->kind == AST_keysetExpression) {
    Ast_KeysetExpression* keyset_expr = (Ast_KeysetExpression*)ast;
    traverse_ast(keyset_expr->expr, walk_in, walk_out);
  } else if (ast->kind == AST_keysetExpressionList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_tupleKeysetExpression) {
    Ast_TupleKeysetExpression* keyset_expr = (Ast_TupleKeysetExpression*)ast;
    traverse_ast(keyset_expr->expr_list, walk_in, walk_out);
  } else if (ast->kind == AST_defaultKeysetExpression) {
  } else if (ast->kind == AST_dontcareKeysetExpression) {
  }

  /** CONTROL **/
  else if (ast->kind == AST_controlDeclaration) {
    Ast_ControlDeclaration* control_decl = (Ast_ControlDeclaration*)ast;
    traverse_ast(control_decl->proto, walk_in, walk_out);
    if (control_decl->ctor_params) {
      traverse_ast(control_decl->ctor_params, walk_in, walk_out);
    }
    if (control_decl->local_decls) {
      traverse_ast(control_decl->local_decls, walk_in, walk_out);
    }
    if (control_decl->apply_stmt) {
      traverse_ast(control_decl->apply_stmt, walk_in, walk_out);
    }
  } else if (ast->kind == AST_controlTypeDeclaration) {
    Ast_ControlPrototype* control_proto = (Ast_ControlPrototype*)ast;
    traverse_ast(control_proto->name, walk_in, walk_out);
    if (control_proto->type_params) {
      traverse_ast(control_proto->type_params, walk_in, walk_out);
    }
    if (control_proto->params) {
      traverse_ast(control_proto->params, walk_in, walk_out);
    }
  } else if (ast->kind == AST_controlLocalDeclarations) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_controlLocalDeclaration) {
    Ast_ControlLocalDeclaration* local_decl = (Ast_ControlLocalDeclaration*)ast;
    traverse_ast(local_decl->decl, walk_in, walk_out);
  }

  /** EXTERN **/
  else if (ast->kind == AST_externDeclaration) {
    Ast_ExternDeclaration* extern_decl = (Ast_ExternDeclaration*)ast;
    traverse_ast(extern_decl->decl, walk_in, walk_out);
  } else if (ast->kind == AST_externType) {
    Ast_ExternType* extern_type = (Ast_ExternType*)ast;
    traverse_ast(extern_type->name, walk_in, walk_out);
    traverse_ast(extern_type->type_params, walk_in, walk_out);
    traverse_ast(extern_type->method_protos, walk_in, walk_out);
  } else if (ast->kind == AST_methodPrototypes) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_functionPrototype || ast->kind == AST_methodPrototype) {
    Ast_FunctionPrototype* func_proto = (Ast_FunctionPrototype*)ast;
    if (func_proto->return_type) {
      traverse_ast(func_proto->return_type, walk_in, walk_out);
    }
    traverse_ast(func_proto->name, walk_in, walk_out);
    if (func_proto->type_params) {
      traverse_ast(func_proto->type_params, walk_in, walk_out);
    }
    if (func_proto->params) {
      traverse_ast(func_proto->params, walk_in, walk_out);
    }
  }

  /** TYPES **/
  else if (ast->kind == AST_typeRef) {
    Ast_TypeRef* type_ref = (Ast_TypeRef*)ast;
    traverse_ast(type_ref->type, walk_in, walk_out);
  }
  else if (ast->kind == AST_namedType) {
    Ast_NamedType* named_type = (Ast_NamedType*)ast;
    traverse_ast(named_type->type, walk_in, walk_out);
  } else if (ast->kind == AST_tupleType) {
    Ast_TupleType* type_ref = (Ast_TupleType*)ast;
    Ast_List* type_args = (Ast_List*)type_ref->type_args;
    if (type_args) {
      for (ListItem* li = type_args->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, walk_in, walk_out);
      }
    }
  } else if (ast->kind == AST_headerStackType) {
    Ast_HeaderStackType* type_ref = (Ast_HeaderStackType*)ast;
    traverse_ast(type_ref->name, walk_in, walk_out);
    traverse_ast(type_ref->stack_expr, walk_in, walk_out);
  } else if (ast->kind == AST_specializedType) {
    Ast_SpecializedType* specd_type = (Ast_SpecializedType*)ast;
    traverse_ast(specd_type->name, walk_in, walk_out);
    if (specd_type->type_args) {
      traverse_ast(specd_type->type_args, walk_in, walk_out);
    }
  } else if (ast->kind == AST_baseTypeBool) {
    Ast_BoolType* bool_type = (Ast_BoolType*)ast;
    traverse_ast(bool_type->name, walk_in, walk_out);
  } else if (ast->kind == AST_baseTypeInteger) {
    Ast_IntegerType* int_type = (Ast_IntegerType*)ast;
    traverse_ast(int_type->name, walk_in, walk_out);
  } else if (ast->kind == AST_baseTypeBit) {
    Ast_BitType* bit_type = (Ast_BitType*)ast;
    traverse_ast(bit_type->name, walk_in, walk_out);
  } else if (ast->kind == AST_baseTypeVarbit) {
    Ast_VarbitType* varbit_type = (Ast_VarbitType*)ast;
    traverse_ast(varbit_type->name, walk_in, walk_out);
  } else if (ast->kind == AST_baseTypeString) {
    Ast_StringType* string_type = (Ast_StringType*)ast;
    traverse_ast(string_type->name, walk_in, walk_out);
  } else if (ast->kind == AST_baseTypeVoid) {
    Ast_VoidType* void_type = (Ast_VoidType*)ast;
    traverse_ast(void_type->name, walk_in, walk_out);
  } else if (ast->kind == AST_baseTypeError) {
    Ast_ErrorType* error_type = (Ast_ErrorType*)ast; 
    traverse_ast(error_type->name, walk_in, walk_out);
  } else if (ast->kind == AST_integerTypeSize) {
  } else if (ast->kind == AST_typeParameterList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_realTypeArgument) {
    Ast_RealTypeArgument* type_arg = (Ast_RealTypeArgument*)ast;
    traverse_ast(type_arg->arg, walk_in, walk_out);
  } else if (ast->kind == AST_typeArgument) {
    Ast_TypeArgument* type_arg = (Ast_TypeArgument*)ast;
    traverse_ast(type_arg->arg, walk_in, walk_out);
  } else if (ast->kind == AST_dontcareTypeArgument) {
  } else if (ast->kind == AST_realTypeArgumentList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_typeArgumentList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_typeDeclaration) {
    Ast_TypeDeclaration* type_decl = (Ast_TypeDeclaration*)ast;
    traverse_ast(type_decl->decl, walk_in, walk_out);
  } else if (ast->kind == AST_derivedTypeDeclaration) {
    Ast_DerivedTypeDeclaration* type_decl = (Ast_DerivedTypeDeclaration*)ast;
    traverse_ast(type_decl->decl, walk_in, walk_out);
  } else if (ast->kind == AST_headerTypeDeclaration) {
    Ast_HeaderTypeDeclaration* header_decl = (Ast_HeaderTypeDeclaration*)ast;
    traverse_ast(header_decl->name, walk_in, walk_out);
    if (header_decl->fields) {
      traverse_ast(header_decl->fields, walk_in, walk_out);
    }
  } else if (ast->kind == AST_headerUnionDeclaration) {
    Ast_HeaderUnionDeclaration* union_decl = (Ast_HeaderUnionDeclaration*)ast;
    traverse_ast(union_decl->name, walk_in, walk_out);
    if (union_decl->fields) {
      traverse_ast(union_decl->fields, walk_in, walk_out);
    }
  } else if (ast->kind == AST_structTypeDeclaration) {
    Ast_StructTypeDeclaration* struct_decl = (Ast_StructTypeDeclaration*)ast;
    traverse_ast(struct_decl->name, walk_in, walk_out);
    if (struct_decl->fields) {
      traverse_ast(struct_decl->fields, walk_in, walk_out);
    }
  } else if (ast->kind == AST_structFieldList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_structField) {
    Ast_StructField* field = (Ast_StructField*)ast;
    traverse_ast(field->type, walk_in, walk_out);
    traverse_ast(field->name, walk_in, walk_out);
  } else if (ast->kind == AST_enumDeclaration) {
    Ast_EnumDeclaration* enum_decl = (Ast_EnumDeclaration*)ast;
    traverse_ast(enum_decl->type_size, walk_in, walk_out);
    traverse_ast(enum_decl->name, walk_in, walk_out);
    if (enum_decl->fields) {
      traverse_ast(enum_decl->fields, walk_in, walk_out);
    }
  } else if (ast->kind == AST_errorDeclaration) {
    Ast_ErrorDeclaration* error_decl = (Ast_ErrorDeclaration*)ast;
    if (error_decl->fields) {
      traverse_ast(error_decl->fields, walk_in, walk_out);
    }
  } else if (ast->kind == AST_matchKindDeclaration) {
    Ast_MatchKindDeclaration* match_decl = (Ast_MatchKindDeclaration*)ast;
    if (match_decl->fields) {
      traverse_ast(match_decl->fields, walk_in, walk_out);
    }
  } else if (ast->kind == AST_identifierList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_specifiedIdentifierList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_specifiedIdentifier) {
    Ast_SpecifiedIdent* id = (Ast_SpecifiedIdent*)ast;
    traverse_ast(id->name, walk_in, walk_out);
    if (id->init_expr) {
      traverse_ast(id->init_expr, walk_in, walk_out);
    }
  } else if (ast->kind == AST_typedefDeclaration) {
    Ast_TypedefDeclaration* typedef_decl = (Ast_TypedefDeclaration*)ast;
    traverse_ast(typedef_decl->name, walk_in, walk_out);
    traverse_ast(typedef_decl->type_ref, walk_in, walk_out);
  }

  /** STATEMENTS **/
  else if (ast->kind == AST_assignmentStatement) {
    Ast_AssignmentStmt* assign_stmt = (Ast_AssignmentStmt*)ast;
    traverse_ast(assign_stmt->lvalue, walk_in, walk_out);
    traverse_ast(assign_stmt->expr, walk_in, walk_out);
  } else if (ast->kind == AST_emptyStatement) {
  } else if (ast->kind == AST_returnStatement) {
    Ast_ReturnStmt* return_stmt = (Ast_ReturnStmt*)ast;
    if (return_stmt->expr) {
      traverse_ast(return_stmt->expr, walk_in, walk_out);
    }
  } else if (ast->kind == AST_exitStatement) {
  } else if (ast->kind == AST_conditionalStatement) {
    Ast_ConditionalStmt* if_stmt = (Ast_ConditionalStmt*)ast;
    traverse_ast(if_stmt->cond_expr, walk_in, walk_out);
    if (if_stmt->else_stmt) {
      traverse_ast(if_stmt->else_stmt, walk_in, walk_out);
    }
  } else if (ast->kind == AST_directApplication) {
    Ast_DirectApplication* apply_stmt = (Ast_DirectApplication*)ast;
    traverse_ast(apply_stmt->lhs_expr, walk_in, walk_out);
    traverse_ast(apply_stmt->args, walk_in, walk_out);
  } else if (ast->kind == AST_statement) {
    Ast_Statement* stmt = (Ast_Statement*)ast;
    traverse_ast(stmt->stmt, walk_in, walk_out);
  } else if (ast->kind == AST_blockStatement) {
    Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
    if (block_stmt->stmt_list) {
      traverse_ast(block_stmt->stmt_list, walk_in, walk_out);
    }
  } else if (ast->kind == AST_statementOrDecl) {
    Ast_Statement* stmt = (Ast_Statement*)ast;
    traverse_ast(stmt->stmt, walk_in, walk_out);
  } else if (ast->kind == AST_statementOrDeclList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_switchStatement) {
    Ast_SwitchStmt* switch_stmt = (Ast_SwitchStmt*)ast;
    traverse_ast(switch_stmt->expr, walk_in, walk_out);
    if (switch_stmt->switch_cases) {
      traverse_ast(switch_stmt->switch_cases, walk_in, walk_out);
    }
  } else if (ast->kind == AST_switchCases) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_switchCase) {
    Ast_SwitchCase* switch_case = (Ast_SwitchCase*)ast;
    traverse_ast(switch_case->label, walk_in, walk_out);
    if (switch_case->stmt) {
      traverse_ast(switch_case->stmt, walk_in, walk_out);
    }
  }
  else if (ast->kind == AST_switchLabel) {
    Ast_SwitchLabel* switch_label = (Ast_SwitchLabel*)ast;
    traverse_ast(switch_label->label, walk_in, walk_out);
  } else if (ast->kind == AST_defaultSwitchLabel) {
  }

  /** TABLES **/
  else if (ast->kind == AST_tableDeclaration) {
    Ast_TableDeclaration* table_decl = (Ast_TableDeclaration*)ast;
    traverse_ast(table_decl->name, walk_in, walk_out);
    if (table_decl->prop_list) {
      traverse_ast(table_decl->prop_list, walk_in, walk_out);
    }
  } else if (ast->kind == AST_tablePropertyList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_tableProperty) {
    Ast_TableProperty* table_prop = (Ast_TableProperty*)ast;
    traverse_ast(table_prop->prop, walk_in, walk_out);
  } else if (ast->kind == AST_tableKey) {
    Ast_TableKey* table_key = (Ast_TableKey*)ast;
    traverse_ast(table_key->keyelem_list, walk_in, walk_out);
  } else if (ast->kind == AST_tableActions) {
    Ast_TableActions* table_actions = (Ast_TableActions*)ast;
    traverse_ast(table_actions->action_list, walk_in, walk_out);
  } else if (ast->kind == AST_tableEntries) {
    Ast_TableEntries* table_entries = (Ast_TableEntries*)ast;
    traverse_ast(table_entries->entries_list, walk_in, walk_out);
  } else if (ast->kind == AST_simpleTableProperty) {
    Ast_SimpleTableProperty* table_prop = (Ast_SimpleTableProperty*)ast;
    traverse_ast(table_prop->name, walk_in, walk_out);
    traverse_ast(table_prop->init_expr, walk_in, walk_out);
  }
  else if (ast->kind == AST_keyElementList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_keyElement) {
    Ast_KeyElement* keyelem = (Ast_KeyElement*)ast;
    traverse_ast(keyelem->expr, walk_in, walk_out);
    traverse_ast(keyelem->name, walk_in, walk_out);
  } else if (ast->kind == AST_actionList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_actionRef) {
    Ast_ActionRef* action_ref = (Ast_ActionRef*)ast;
    traverse_ast(action_ref->name, walk_in, walk_out);
    if (action_ref->args) {
      traverse_ast(action_ref->args, walk_in, walk_out);
    }
  } else if (ast->kind == AST_entriesList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_entry) {
    Ast_Entry* entry = (Ast_Entry*)ast;
    traverse_ast(entry->keyset, walk_in, walk_out);
    traverse_ast(entry->action, walk_in, walk_out);
  } else if (ast->kind == AST_actionDeclaration) {
    Ast_ActionDeclaration* action_decl = (Ast_ActionDeclaration*)ast;
    traverse_ast(action_decl->name, walk_in, walk_out);
    if (action_decl->params) {
      traverse_ast(action_decl->params, walk_in, walk_out);
    }
    if (action_decl->stmt) {
      traverse_ast(action_decl->stmt, walk_in, walk_out);
    }
  }

  /** VARIABLES **/
  else if (ast->kind == AST_variableDeclaration) {
    Ast_VarDeclaration* var_decl = (Ast_VarDeclaration*)ast;
    traverse_ast(var_decl->name, walk_in, walk_out);
    traverse_ast(var_decl->type, walk_in, walk_out);
    if (var_decl->init_expr) {
      traverse_ast(var_decl->init_expr, walk_in, walk_out);
    }
  } else if (ast->kind == AST_constantDeclaration) {
    Ast_ConstDeclaration* const_decl = (Ast_ConstDeclaration*)ast;
    traverse_ast(const_decl->type, walk_in, walk_out);
    traverse_ast(const_decl->name, walk_in, walk_out);
    traverse_ast(const_decl->init_expr, walk_in, walk_out);
  }

  /** EXPRESSIONS **/
  else if (ast->kind == AST_functionDeclaration) {
    Ast_FunctionDeclaration* func_decl = (Ast_FunctionDeclaration*)ast;
    traverse_ast(func_decl->proto, walk_in, walk_out);
    if (func_decl->stmt) {
      traverse_ast(func_decl->stmt, walk_in, walk_out);
    }
  } else if (ast->kind == AST_argumentList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_argument) {
    Ast_Argument* arg = (Ast_Argument*)ast;
    traverse_ast(arg->arg, walk_in, walk_out);
  } else if (ast->kind == AST_dontcareArgument) {
  } else if (ast->kind == AST_kvPair) {
    Ast_KVPair* kv_expr = (Ast_KVPair*)ast;
    traverse_ast(kv_expr->name, walk_in, walk_out);
    traverse_ast(kv_expr->expr, walk_in, walk_out);
  } else if (ast->kind == AST_expressionList) {
    Ast_List* list = (Ast_List*)ast;
    for (ListItem* li = list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, walk_in, walk_out);
    }
  } else if (ast->kind == AST_lvalue) {
    Ast_Expression* lvalue = (Ast_Expression*)ast;
    traverse_ast(lvalue->expr, walk_in, walk_out);
    if (lvalue->type_args) {
      traverse_ast(lvalue->type_args, walk_in, walk_out);
    }
  } else if (ast->kind == AST_expression) {
    Ast_Expression* expr = (Ast_Expression*)ast;
    traverse_ast(expr->expr, walk_in, walk_out);
    if (expr->type_args) {
      traverse_ast(expr->type_args, walk_in, walk_out);
    }
  } else if (ast->kind == AST_binaryExpression) {
    Ast_BinaryExpression* binary_expr = (Ast_BinaryExpression*)ast;
    traverse_ast(binary_expr->left_operand, walk_in, walk_out);
    traverse_ast(binary_expr->right_operand, walk_in, walk_out);
  } else if (ast->kind == AST_unaryExpression) {
    Ast_UnaryExpression* unary_expr = (Ast_UnaryExpression*)ast;
    traverse_ast(unary_expr->operand, walk_in, walk_out);
  } else if (ast->kind == AST_functionCall) {
    Ast_FunctionCall* call_expr = (Ast_FunctionCall*)ast;
    traverse_ast(call_expr->callee_expr, walk_in, walk_out);
    if (call_expr->args) {
      traverse_ast(call_expr->args, walk_in, walk_out);
    }
  } else if (ast->kind == AST_memberSelector) {
    Ast_MemberSelector* member_expr = (Ast_MemberSelector*)ast;
    traverse_ast(member_expr->lhs_expr, walk_in, walk_out);
    traverse_ast(member_expr->member_name, walk_in, walk_out);
  } else if (ast->kind == AST_castExpression) {
    Ast_CastExpression* cast_expr = (Ast_CastExpression*)ast;
    traverse_ast(cast_expr->to_type, walk_in, walk_out);
    traverse_ast(cast_expr->expr, walk_in, walk_out);
  } else if (ast->kind == AST_arraySubscript) {
    Ast_ArraySubscript* subscript_expr = (Ast_ArraySubscript*)ast;
    traverse_ast(subscript_expr->lhs_expr, walk_in, walk_out);
    traverse_ast(subscript_expr->index_expr, walk_in, walk_out);
  } else if (ast->kind == AST_arrayIndex) {
    Ast_ArrayIndex* index = (Ast_ArrayIndex*)ast;
    traverse_ast(index->start_index, walk_in, walk_out);
    traverse_ast(index->end_index, walk_in, walk_out);
  } else if (ast->kind == AST_integerLiteral) {
  } else if (ast->kind == AST_booleanLiteral) {
  } else if (ast->kind == AST_stringLiteral) {
  }
  else assert(0);

  walk_out(ast);
}

void traverse_p4program(Ast_P4Program* p4program, AstVisitor* walk_in, AstVisitor* walk_out)
{
  traverse_ast((Ast*)p4program, walk_in, walk_out);
}
