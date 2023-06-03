#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal void
traverse_ast(Ast* ast, AstVisitor visitor)
{
  visitor(ast, WALK_IN);
  if (ast->kind == AST_p4program) {
    Ast_P4Program* program = (Ast_P4Program*)ast;
    traverse_ast(program->decl_list, visitor);
  } else if (ast->kind == AST_declarationList) {
    Ast_List* decl_list = (Ast_List*)ast;
    for (DListItem* li = decl_list->members.sentinel.next;
         li != 0; li = li->next) {
      traverse_ast(li->object, visitor);
    }
  } else if (ast->kind == AST_declaration) {
    Ast_Declaration* decl = (Ast_Declaration*)ast;
    traverse_ast(decl->decl, visitor);
  } else if (ast->kind == AST_binaryExpression) {
    Ast_BinaryExpr* bin_expr = (Ast_BinaryExpr*)ast;
    traverse_ast(bin_expr->left_operand, visitor);
    traverse_ast(bin_expr->right_operand, visitor);
  } else if (ast->kind == AST_unaryExpression) {
    Ast_UnaryExpr* una_expr = (Ast_UnaryExpr*)ast;
    traverse_ast(una_expr->operand, visitor);
  } else if (ast->kind == AST_functionCall) {
    Ast_FunctionCall* call_expr = (Ast_FunctionCall*)ast;
    traverse_ast(call_expr->callee_expr, visitor);
    Ast_Expression* callee_expr = (Ast_Expression*)call_expr->callee_expr;
    Ast_List* type_args = (Ast_List*)callee_expr->type_args;
    if (type_args) {
      for (DListItem* li = type_args->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* args = (Ast_List*)call_expr->args;
    if (args) {
      for (DListItem* li = args->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_memberSelectExpression) {
    Ast_MemberSelect* member_expr = (Ast_MemberSelect*)ast;
    traverse_ast(member_expr->lhs_expr, visitor);
    traverse_ast(member_expr->member_name, visitor);
  } else if (ast->kind == AST_castExpression) {
    Ast_CastExpr* cast_expr = (Ast_CastExpr*)ast;
    traverse_ast(cast_expr->to_type, visitor);
    traverse_ast(cast_expr->expr, visitor);
  } else if (ast->kind == AST_arraySubscript) {
    Ast_ArraySubscript* subscript_expr = (Ast_ArraySubscript*)ast;
    traverse_ast(subscript_expr->index, visitor);
    if (subscript_expr->end_index) {
      traverse_ast(subscript_expr->end_index, visitor);
    }
  } else if (ast->kind == AST_kvPairExpression) {
    Ast_KVPairExpr* kv_expr = (Ast_KVPairExpr*)ast;
    traverse_ast(kv_expr->name, visitor);
    traverse_ast(kv_expr->expr, visitor);
  } else if (ast->kind == AST_integerLiteral) {
  } else if (ast->kind == AST_booleanLiteral) {
  } else if (ast->kind == AST_stringLiteral) {
  } else if (ast->kind == AST_tableActions) {
    Ast_TableActions* table_actions = (Ast_TableActions*)ast;
    Ast_List* action_list = (Ast_List*)table_actions->action_list;
    if (action_list) {
      for (DListItem* li = action_list->members.sentinel.next;
           li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_tableProperty) {
    Ast_TableProperty* table_prop = (Ast_TableProperty*)ast;
    if (table_prop->init_expr) {
      traverse_ast(table_prop->init_expr, visitor);
    }
  } else if (ast->kind == AST_tableKey) {
    Ast_TableKey* table_key = (Ast_TableKey*)ast;
    Ast_List* keyelem_list = (Ast_List*)table_key->keyelem_list;
    if (keyelem_list) {
      for (DListItem* li = keyelem_list->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_tableEntries) {
    Ast_TableEntries* table_entries = (Ast_TableEntries*)ast;
    Ast_List* entries = (Ast_List*)table_entries->entries;
    if (entries) {
      for (DListItem* li = entries->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_tableEntry) {
    Ast_TableEntry* entry = (Ast_TableEntry*)ast;
    traverse_ast(entry->keyset, visitor);
    traverse_ast(entry->action, visitor);
  } else if (ast->kind == AST_tupleKeysetExpression) {
    Ast_TupleKeyset* keyset = (Ast_TupleKeyset*)ast;
    Ast_List* expr_list = (Ast_List*)keyset->expr_list;
    if (expr_list) {
      for (DListItem* li = expr_list->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_keyElement) {
    Ast_KeyElement* keyelem = (Ast_KeyElement*)ast;
    traverse_ast(keyelem->expr, visitor);
    traverse_ast(keyelem->name, visitor);
  } else if (ast->kind == AST_actionRef) {
    Ast_ActionRef* action_ref = (Ast_ActionRef*)ast;
    traverse_ast(action_ref->name, visitor);
    Ast_List* args = (Ast_List*)action_ref->args;
    if (args) {
      for (DListItem* li = args->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_parameter) {
    Ast_Param* param = (Ast_Param*)ast;
    traverse_ast(param->type, visitor);
  } else if (ast->kind == AST_defaultKeysetExpression) {
  } else if (ast->kind == AST_switchCase) {
    Ast_SwitchCase* switch_case = (Ast_SwitchCase*)ast;
    traverse_ast(switch_case->label, visitor);
    traverse_ast(switch_case->stmt, visitor);
  } else if (ast->kind == AST_parserState) {
    Ast_ParserState* state = (Ast_ParserState*)ast;
    Ast_List* stmt_list = (Ast_List*)state->stmt_list;
    if (stmt_list) {
      for (DListItem* li = stmt_list->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    traverse_ast(state->transition_stmt, visitor);
  } else if (ast->kind == AST_selectExpression) {
    Ast_SelectExpr* select_expr = (Ast_SelectExpr*)ast;
    Ast_List* expr_list = (Ast_List*)select_expr->expr_list;
    if (expr_list) {
      for (DListItem* li = expr_list->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* case_list = (Ast_List*)select_expr->case_list;
    if (case_list) {
      for (DListItem* li = case_list->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_selectCase) {
    Ast_SelectCase* select_case = (Ast_SelectCase*)ast;
    traverse_ast(select_case->keyset, visitor);
    traverse_ast(select_case->name, visitor);
  } else if (ast->kind == AST_variableDeclaration) {
    Ast_Var* var_decl = (Ast_Var*)ast;
    traverse_ast(var_decl->type, visitor);
    if (var_decl->init_expr) {
      traverse_ast(var_decl->init_expr, visitor);
    }
  } else if (ast->kind == AST_blockStatement) {
    Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
    Ast_List* stmt_list = (Ast_List*)block_stmt->stmt_list;
    if (stmt_list) {
      for (DListItem* li = stmt_list->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_tableDeclaration) {
    Ast_Table* table_decl = (Ast_Table*)ast;
    Ast_List* prop_list = (Ast_List*)table_decl->prop_list;
    if (prop_list) {
      for (DListItem* li = prop_list->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_conditionalStatement) {
    Ast_IfStmt* if_stmt = (Ast_IfStmt*)ast;
    traverse_ast(if_stmt->cond_expr, visitor);
    if (if_stmt->else_stmt) {
      traverse_ast(if_stmt->else_stmt, visitor);
    }
  } else if (ast->kind == AST_switchStatement) {
    Ast_SwitchStmt* switch_stmt = (Ast_SwitchStmt*)ast;
    traverse_ast(switch_stmt->expr, visitor);
    Ast_List* switch_cases = (Ast_List*)switch_stmt->switch_cases;
    if (switch_cases) {
      for (DListItem* li = switch_cases->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_assignmentStatement) {
    Ast_AssignmentStmt* assign_stmt = (Ast_AssignmentStmt*)ast;
    traverse_ast(assign_stmt->lvalue, visitor);
    traverse_ast(assign_stmt->expr, visitor);
  } else if (ast->kind == AST_returnStatement) {
    Ast_ReturnStmt* return_stmt = (Ast_ReturnStmt*)ast;
    if (return_stmt->expr) {
      traverse_ast(return_stmt->expr, visitor);
    }
  } else if (ast->kind == AST_exitStatement) {
  } else if (ast->kind == AST_emptyStatement) {
  } else if (ast->kind == AST_structField) {
    Ast_StructField* field = (Ast_StructField*)ast;
    traverse_ast(field->type, visitor);
  } else if (ast->kind == AST_baseTypeBool) {
    Ast_BoolType* bool_type = (Ast_BoolType*)ast;
    traverse_ast(bool_type->name, visitor);
  } else if (ast->kind == AST_baseTypeInt) {
    Ast_IntType* int_type = (Ast_IntType*)ast;
    traverse_ast(int_type->name, visitor);
  } else if (ast->kind == AST_baseTypeBit) {
    Ast_BitType* bit_type = (Ast_BitType*)ast;
    traverse_ast(bit_type->name, visitor);
  } else if (ast->kind == AST_baseTypeVarbit) {
    Ast_VarbitType* varbit_type = (Ast_VarbitType*)ast;
    traverse_ast(varbit_type->name, visitor);
  } else if (ast->kind == AST_baseTypeString) {
    Ast_StringType* string_type = (Ast_StringType*)ast;
    traverse_ast(string_type->name, visitor);
  } else if (ast->kind == AST_baseTypeVoid) {
    Ast_VoidType* void_type = (Ast_VoidType*)ast;
    traverse_ast(void_type->name, visitor);
  } else if (ast->kind == AST_baseTypeError) {
    Ast_ErrorType* error_type = (Ast_ErrorType*)ast; 
    traverse_ast(error_type->name, visitor);
  } else if (ast->kind == AST_headerStackType) {
    Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
    traverse_ast(type_ref->name, visitor);
    traverse_ast(type_ref->stack_expr, visitor);
  } else if (ast->kind == AST_specializedType) {
    Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
    traverse_ast(speclzd_type->name, visitor);
    Ast_List* type_args = (Ast_List*)speclzd_type->type_args;
    if (type_args) {
      for (DListItem* li = type_args->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_tupleType) {
    Ast_Tuple* type_ref = (Ast_Tuple*)ast;
    Ast_List* type_args = (Ast_List*)type_ref->type_args;
    if (type_args) {
      for (DListItem* li = type_args->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_dontcareArgument) {
  } else if (ast->kind == AST_name) {
  } else if (ast->kind == AST_specifiedIdentifier) {
    Ast_SpecifiedIdent* id = (Ast_SpecifiedIdent*)ast;
    traverse_ast(id->name, visitor);
    if (id->init_expr) {
      traverse_ast(id->init_expr, visitor);
    }
  } else if (ast->kind == AST_controlDeclaration) {
    Ast_Control* ctrl_decl = (Ast_Control*)ast;
    Ast_ControlProto* ctrl_proto = (Ast_ControlProto*)ctrl_decl->proto;
    Ast_List* type_params = (Ast_List*)ctrl_proto->type_params;
    if (type_params) {
      for (DListItem* li = type_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* params = (Ast_List*)ctrl_proto->params;
    if (params) {
      for (DListItem* li = params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* ctor_params = (Ast_List*)ctrl_decl->ctor_params;
    if (ctor_params) {
      for (DListItem* li = ctor_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* local_decls = (Ast_List*)ctrl_decl->local_decls;
    if (local_decls) {
      for (DListItem* li = local_decls->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    if (ctrl_decl->apply_stmt) {
      traverse_ast(ctrl_decl->apply_stmt, visitor);
    }
  } else if (ast->kind == AST_controlTypeDeclaration) {
    Ast_ControlProto* ctrl_proto = (Ast_ControlProto*)ast;
    Ast_List* type_params = (Ast_List*)ctrl_proto->type_params;
    if (type_params) {
      for (DListItem* li = type_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* params = (Ast_List*)ctrl_proto->params;
    if (params) {
      for (DListItem* li = params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_externDeclaration) {
    Ast_ExternType* extern_decl = (Ast_ExternType*)ast;
    Ast_List* type_params = (Ast_List*)extern_decl->type_params;
    if (type_params) {
      for (DListItem* li = type_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* method_protos = (Ast_List*)extern_decl->method_protos;
    if (method_protos) {
      for (DListItem* li = method_protos->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_structTypeDeclaration) {
    Ast_Struct* struct_decl = (Ast_Struct*)ast;
    Ast_List* fields = (Ast_List*)struct_decl->fields;
    if (fields) {
      for (DListItem* li = fields->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_headerTypeDeclaration) {
    Ast_Header* header_decl = (Ast_Header*)ast;
    Ast_List* fields = (Ast_List*)header_decl->fields;
    if (fields) {
      for (DListItem* li = fields->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_headerUnionDeclaration) {
    Ast_HeaderUnion* union_decl = (Ast_HeaderUnion*)ast;
    Ast_List* fields = (Ast_List*)union_decl->fields;
    if (fields) {
      for (DListItem* li = fields->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_packageTypeDeclaration) {
    Ast_Package* package_decl = (Ast_Package*)ast;
    Ast_List* type_params = (Ast_List*)package_decl->type_params;
    if (type_params) {
      for (DListItem* li = type_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* params = (Ast_List*)package_decl->params;
    if (params) {
      for (DListItem* li = params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_parserDeclaration) {
    Ast_Parser* parser_decl = (Ast_Parser*)ast;
    Ast_ParserProto* proto = (Ast_ParserProto*)parser_decl->proto;
    Ast_List* type_params = (Ast_List*)proto->type_params;
    if (type_params) {
      for (DListItem* li = type_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* params = (Ast_List*)proto->params;
    if (params) {
      for (DListItem* li = params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* ctor_params = (Ast_List*)parser_decl->ctor_params;
    if (ctor_params) {
      for (DListItem* li = ctor_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* local_elements = (Ast_List*)parser_decl->local_elements;
    if (local_elements) {
      for (DListItem* li = local_elements->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* states = (Ast_List*)parser_decl->states;
    if (states) {
      for (DListItem* li = states->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_parserTypeDeclaration) {
    Ast_ParserProto* proto_decl = (Ast_ParserProto*)ast;
    Ast_List* type_params = (Ast_List*)proto_decl->type_params;
    if (type_params) {
      for (DListItem* li = type_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* params = (Ast_List*)proto_decl->params;
    if (params) {
      for (DListItem* li = params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_instantiation) {
    Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
    traverse_ast(inst_decl->type_ref, visitor);
    Ast_List* args = (Ast_List*)inst_decl->args;
    if (args) {
      for (DListItem* li = args->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_typedefDeclaration) {
    Ast_TypeDef* type_decl = (Ast_TypeDef*)ast;
    traverse_ast(type_decl->type_ref, visitor);
  } else if (ast->kind == AST_functionDeclaration) {
    Ast_Function* func_decl = (Ast_Function*)ast;
    Ast_FunctionProto* func_proto = (Ast_FunctionProto*)func_decl->proto;
    if (func_proto->return_type) {
      traverse_ast(func_proto->return_type, visitor);
    }
    Ast_List* type_params = (Ast_List*)func_proto->type_params;
    if (type_params) {
      for (DListItem* li = type_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* params = (Ast_List*)func_proto->params;
    if (params) {
      for (DListItem* li = params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    if (func_decl->stmt) {
      traverse_ast(func_decl->stmt, visitor);
      Ast_BlockStmt* func_body = (Ast_BlockStmt*)func_decl->stmt;
      Ast_List* stmt_list = (Ast_List*)func_body->stmt_list;
      if (stmt_list) {
        for (DListItem* li = stmt_list->members.sentinel.next;
            li != 0; li = li->next) {
          traverse_ast(li->object, visitor);
        }
      }
    }
  } else if (ast->kind == AST_functionPrototype) {
    Ast_FunctionProto* func_proto = (Ast_FunctionProto*)ast;
    if (func_proto->return_type) {
      traverse_ast(func_proto->return_type, visitor);
    }
    Ast_List* type_params = (Ast_List*)func_proto->type_params;
    if (type_params) {
      for (DListItem* li = type_params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    Ast_List* params = (Ast_List*)func_proto->params;
    if (params) {
      for (DListItem* li = params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_constantDeclaration) {
    Ast_Const* const_decl = (Ast_Const*)ast;
    traverse_ast(const_decl->type, visitor);
    traverse_ast(const_decl->expr, visitor);
  } else if (ast->kind == AST_enumDeclaration) {
    Ast_EnumDeclaration* enum_decl = (Ast_EnumDeclaration*)ast;
    Ast_List* fields = (Ast_List*)enum_decl->fields;
    if (fields) {
      for (DListItem* li = fields->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_actionDeclaration) {
    Ast_Action* action_decl = (Ast_Action*)ast;
    Ast_List* params = (Ast_List*)action_decl->params;
    if (params) {
      for (DListItem* li = params->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
    if (action_decl->stmt) {
      traverse_ast(action_decl->stmt, visitor);
      Ast_BlockStmt* action_body = (Ast_BlockStmt*)action_decl->stmt;
      Ast_List* stmt_list = (Ast_List*)action_body->stmt_list;
      if (stmt_list) {
        for (DListItem* li = stmt_list->members.sentinel.next;
            li != 0; li = li->next) {
          traverse_ast(li->object, visitor);
        }
      }
    }
  } else if (ast->kind == AST_matchKindDeclaration) {
    Ast_MatchKindDeclaration* match_decl = (Ast_MatchKindDeclaration*)ast;
    Ast_List* fields = (Ast_List*)match_decl->fields;
    if (fields) {
      for (DListItem* li = fields->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  } else if (ast->kind == AST_errorDeclaration) {
    Ast_ErrorDeclaration* error_decl = (Ast_ErrorDeclaration*)ast;
    Ast_List* fields = (Ast_List*)error_decl->fields;
    if (fields) {
      for (DListItem* li = fields->members.sentinel.next;
          li != 0; li = li->next) {
        traverse_ast(li->object, visitor);
      }
    }
  }
  else assert(0);
  visitor(ast, WALK_OUT);
}

void traverse_p4program(Ast_P4Program* p4program, AstVisitor visitor)
{
  traverse_ast((Ast*)p4program, visitor);
}
