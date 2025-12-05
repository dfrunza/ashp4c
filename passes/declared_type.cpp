#include <stdio.h>
#include <basic.h>
#include <cstring.h>
#include <type_checker.h>
#include <passes/declared_type.h>

void DeclaredTypePass::define_builtin_types()
{
  char* base_types[] = {
    "void",
    "bool",
    "int",
    "bit",
    "varbit",
    "string",
    "error",
    "match_kind",
    "_",
  };
  char* arithmetic_ops[] = {
    "+", "-", "*", "/"
  };
  char* logical_ops[] = {
    "&&", "||"
  };
  char* relational_ops[] = {
    "==", "!=", "<", ">", "<=", ">="
  };
  char* bitwise_ops[] = {
    "&", "|", "^", "<<", ">>"
  };
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  Ast* ast;
  Type* ty, *params_ty;

  for (int i = 0; i < sizeof(base_types)/sizeof(base_types[0]); i++) {
    name_entry = root_scope->lookup(base_types[i], NameSpace::TYPE);
    name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
    type_env->insert(name_decl->ast, name_decl->type, 0);
  }

  ast = root_scope->builtin_lookup("accept", NameSpace::VAR)->ast;
  ty = (Type*)type_array->append();
  ty->ty_former = TypeEnum::STATE;
  type_env->insert(ast, ty, 0);

  ast = root_scope->builtin_lookup("reject", NameSpace::VAR)->ast;
  ty = (Type*)type_array->append();
  ty->ty_former = TypeEnum::STATE;
  type_env->insert(ast, ty, 0);

  for (int i = 0; i < sizeof(arithmetic_ops)/sizeof(arithmetic_ops[0]); i++) {
    ty = (Type*)type_array->append();
    ty->strname = arithmetic_ops[i];
    ty->ty_former = TypeEnum::FUNCTION;
    params_ty = (Type*)type_array->append();
    params_ty->ty_former = TypeEnum::PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = storage->allocate<Type *>(params_ty->product.count);
    params_ty->product.members[0] = root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    params_ty->product.members[1] = root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    name_decl = root_scope->bind_name(storage, ty->strname, NameSpace::TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(logical_ops)/sizeof(logical_ops[0]); i++) {
    ty = (Type*)type_array->append();
    ty->strname = logical_ops[i];
    ty->ty_former = TypeEnum::FUNCTION;
    params_ty = (Type*)type_array->append();
    params_ty->ty_former = TypeEnum::PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = storage->allocate<Type *>(params_ty->product.count);
    params_ty->product.members[0] = root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
    params_ty->product.members[1] = root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
    name_decl = root_scope->bind_name(storage, ty->strname, NameSpace::TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(relational_ops)/sizeof(relational_ops[0]); i++) {
    ty = (Type*)type_array->append();
    ty->strname = relational_ops[i];
    ty->ty_former = TypeEnum::FUNCTION;
    params_ty = (Type*)type_array->append();
    params_ty->ty_former = TypeEnum::PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = storage->allocate<Type *>(params_ty->product.count);
    params_ty->product.members[0] = root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    params_ty->product.members[1] = root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
    name_decl = root_scope->bind_name(storage, ty->strname, NameSpace::TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(bitwise_ops)/sizeof(bitwise_ops[0]); i++) {
    ty = (Type*)type_array->append();
    ty->strname = bitwise_ops[i];
    ty->ty_former = TypeEnum::FUNCTION;
    params_ty = (Type*)type_array->append();
    params_ty->ty_former = TypeEnum::PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = storage->allocate<Type *>(params_ty->product.count);
    params_ty->product.members[0] = root_scope->builtin_lookup("bit", NameSpace::TYPE)->type;
    params_ty->product.members[1] = root_scope->builtin_lookup("bit", NameSpace::TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = root_scope->builtin_lookup("bit", NameSpace::TYPE)->type;
    name_decl = root_scope->bind_name(storage, ty->strname, NameSpace::TYPE);
    name_decl->type = ty;
  }
}

void DEBUG_print_type_env(Map<Type, void>* env)
{
  Ast* ast;
  MapEntry<Type, void>* m;
  Type* ty;
  int i;

  i = 0;
  for (m = env->first; m != 0; m = m->next) {
    ast = (Ast*)m->key;
    ty = (Type*)m->value;
    if (ty->strname) {
      printf("[%d] 0x%x %s ... %d:%d\n", i, ty, ty->strname, ast->line_no, ast->column_no);
    } else {
      if (ast) {
        printf("[%d] 0x%x %s ... %d:%d\n", i, ty, TypeEnum_to_string(ty->ty_former), ast->line_no, ast->column_no);
      } else {
        printf("[%d] 0x%x %s\n", i, ty, TypeEnum_to_string(ty->ty_former));
      }
    }
    i += 1;
  }
}

void DEBUG_print_type_array(Array<Type>* type_array)
{
  Type* ty;
  int i;

  for (i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)type_array->get(i);
    ty = ty->actual_type();

    if (ty->strname) {
      printf("[%d] 0x%x %s %s\n", i, ty, ty->strname, TypeEnum_to_string(ty->ty_former));
    } else {
      printf("[%d] 0x%x %s\n", i, ty, TypeEnum_to_string(ty->ty_former));
    }
  }
}

void DeclaredTypePass::do_pass()
{
  Ast* name;
  Type* ref_ty, *ty;
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  type_env = storage->allocate<Map<Ast, Type>>();
  type_env->storage = storage;
  type_equiv_pairs = Array<Type>::create(storage, 2);

  define_builtin_types();
  visit_p4program(p4program);
  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)type_array->get(i);
    if (ty->ty_former == TypeEnum::NAMEREF) {
      name = ty->nameref.name;
      name_entry = ty->nameref.scope->lookup(name->name.strname, NameSpace::TYPE);
      name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
      if (name_decl) {
        ref_ty = (Type*)type_env->lookup(name_decl->ast, 0);
        assert(ref_ty);
        name_decl->type = ref_ty;
        ty->ty_former = TypeEnum::TYPE;
        ty->type.type = ref_ty;
        if (name_decl->next_in_scope) {
          error("%s:%d:%d: error: ambiguous type reference `%s`.",
                source_file, name->line_no, name->column_no, name->name.strname);
        }
      } else error("%s:%d:%d: error: unresolved type reference `%s`.",
                   source_file, name->line_no, name->column_no, name->name.strname);
    }
  }
  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)type_array->get(i);
    if (ty->ty_former == TypeEnum::TYPEDEF) {
      ref_ty = ty->typedef_.ref->actual_type();
      while (ref_ty->ty_former == TypeEnum::TYPEDEF) {
        ref_ty = ref_ty->typedef_.ref->actual_type();
      }
      ty->ty_former = TypeEnum::TYPE;
      ty->type.type = ref_ty;
    }
  }
  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)type_array->get(i);
    if (ty->ty_former == TypeEnum::TYPE) {
      ref_ty = ty->type.type->actual_type();
      while (ref_ty->ty_former == TypeEnum::TYPE) {
        ref_ty = ref_ty->type.type->actual_type();
      }
      ty->ty_former = TypeEnum::TYPE;
      ty->type.type = ref_ty;
    }
  }
}

/** PROGRAM **/

void DeclaredTypePass::visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  visit_declarationList(p4program->p4program.decl_list);
}

void DeclaredTypePass::visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  Tree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_declaration(Ast* decl)
{
  assert(decl->kind == AstEnum::declaration);
  if (decl->declaration.decl->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::externDeclaration) {
    visit_externDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::actionDeclaration) {
    visit_actionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::functionDeclaration) {
    visit_functionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserDeclaration) {
    visit_parserDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlDeclaration) {
    visit_controlDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::typeDeclaration) {
    visit_typeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::errorDeclaration) {
    visit_errorDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::matchKindDeclaration) {
    visit_matchKindDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::instantiation) {
    visit_instantiation(decl->declaration.decl);
  } else assert(0);
}

void DeclaredTypePass::visit_name(Ast* name)
{
  assert(name->kind == AstEnum::name);
  Type* name_ty;

  name_ty = (Type*)type_array->append();
  name_ty->ty_former = TypeEnum::NAMEREF;
  name_ty->strname = name->name.strname;
  name_ty->ast = name;
  name_ty->nameref.name = name;
  name_ty->nameref.scope = (Scope*)scope_map->lookup(name, 0);
  type_env->insert(name, name_ty, 0);
}

void DeclaredTypePass::visit_parameterList(Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  Tree* ast;
  Type* params_ty;
  int i;

  params_ty = (Type*)type_array->append();
  params_ty->ty_former = TypeEnum::PRODUCT;
  params_ty->ast = params;
  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(container_of(ast, Ast, tree));
    params_ty->product.count += 1;
  }
  if (params_ty->product.count > 0) {
    params_ty->product.members = storage->allocate<Type *>(params_ty->product.count);
  }
  i = 0;
  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    params_ty->product.members[i] = (Type*)type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == params_ty->product.count);
  type_env->insert(params, params_ty, 0);
}

void DeclaredTypePass::visit_parameter(Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  NameDeclaration* name_decl;
  Type* param_ty;

  visit_typeRef(param->parameter.type);
  param_ty = (Type*)type_env->lookup(param->parameter.type, 0);
  type_env->insert(param, param_ty, 0);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr);
  }
  name_decl = (NameDeclaration*)decl_map->lookup(param, 0);
  name_decl->type = param_ty;
}

void DeclaredTypePass::visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* package_ty;

  visit_parameterList(type_decl->packageTypeDeclaration.params);
  name = type_decl->packageTypeDeclaration.name;
  package_ty = (Type*)type_array->append();
  package_ty->ty_former = TypeEnum::PACKAGE;
  package_ty->strname = name->name.strname;
  package_ty->ast = type_decl;
  package_ty->package.params = (Type*)type_env->lookup(type_decl->packageTypeDeclaration.params, 0);
  type_env->insert(type_decl, package_ty, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(type_decl, 0);
  name_decl->type = package_ty;
}

void DeclaredTypePass::visit_instantiation(Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  Type* inst_ty;
  NameDeclaration* name_decl;

  visit_typeRef(inst->instantiation.type);
  visit_argumentList(inst->instantiation.args);
  inst_ty = (Type*)type_env->lookup(inst->instantiation.type, 0);
  type_env->insert(inst, inst_ty, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(inst, 0);
  name_decl->type = inst_ty;
}

/** PARSER **/

void DeclaredTypePass::visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  Type* parser_ty;

  visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
    parser_ty = (Type*)type_env->lookup(parser_decl->parserDeclaration.proto, 0);
    parser_ty->parser.ctor_params = (Type*)type_env->lookup(parser_decl->parserDeclaration.ctor_params, 0);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

void DeclaredTypePass::visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* parser_ty, *methods_ty;

  visit_parameterList(type_decl->parserTypeDeclaration.params);
  name = type_decl->parserTypeDeclaration.name;
  parser_ty = (Type*)type_array->append();
  parser_ty->ty_former = TypeEnum::PARSER;
  parser_ty->strname = name->name.strname;
  parser_ty->ast = type_decl;
  parser_ty->parser.params = (Type*)type_env->lookup(type_decl->parserTypeDeclaration.params, 0);
  type_env->insert(type_decl, parser_ty, 0);
  visit_methodPrototypes(type_decl->parserTypeDeclaration.method_protos, 0, 0);
  methods_ty = (Type*)type_env->lookup(type_decl->parserTypeDeclaration.method_protos, 0);
  parser_ty->parser.methods = methods_ty;
  name_decl = (NameDeclaration*)decl_map->lookup(type_decl, 0);
  name_decl->type = parser_ty;
}

void DeclaredTypePass::visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  Tree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
}

void DeclaredTypePass::visit_parserStates(Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  Tree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_parserState(Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  Ast* name;
  NameDeclaration* name_decl;
  Type* state_ty;

  name = state->parserState.name;
  state_ty = (Type*)type_array->append();
  state_ty->ty_former = TypeEnum::STATE;
  state_ty->strname = name->name.strname;
  state_ty->ast = state;
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
  type_env->insert(state, state_ty, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(state, 0);
  name_decl->type = state_ty;
}

void DeclaredTypePass::visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  Tree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_parserStatement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::parserStatement);
  if (stmt->parserStatement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::parserBlockStatement) {
    visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else assert(0);
}

void DeclaredTypePass::visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
}

void DeclaredTypePass::visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  visit_stateExpression(transition_stmt->transitionStatement.stmt);
}

void DeclaredTypePass::visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    ;
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
}

void DeclaredTypePass::visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  visit_expressionList(select_expr->selectExpression.expr_list);
  visit_selectCaseList(select_expr->selectExpression.case_list);
}

void DeclaredTypePass::visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  Tree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AstEnum::selectCase);
  visit_keysetExpression(select_case->selectCase.keyset_expr);
}

void DeclaredTypePass::visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
}

void DeclaredTypePass::visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
}

void DeclaredTypePass::visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
    visit_expression(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
    visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
    visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

void DeclaredTypePass::visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  Tree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(container_of(ast, Ast, tree));
  }
}

/** CONTROL **/

void DeclaredTypePass::visit_controlDeclaration(Ast* control_decl) {
  assert(control_decl->kind == AstEnum::controlDeclaration);
  Type* control_ty;

  visit_typeDeclaration(control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
    control_ty = (Type*)type_env->lookup(control_decl->controlDeclaration.proto, 0);
    control_ty->control.ctor_params = (Type*)type_env->lookup(control_decl->controlDeclaration.ctor_params, 0);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
}

void DeclaredTypePass::visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* control_ty, *methods_ty;

  visit_parameterList(type_decl->controlTypeDeclaration.params);
  name = type_decl->controlTypeDeclaration.name;
  control_ty = (Type*)type_array->append();
  control_ty->ty_former = TypeEnum::CONTROL;
  control_ty->strname = name->name.strname;
  control_ty->ast = type_decl;
  control_ty->control.params = (Type*)type_env->lookup(type_decl->packageTypeDeclaration.params, 0);
  type_env->insert(type_decl, control_ty, 0);
  visit_methodPrototypes(type_decl->controlTypeDeclaration.method_protos, 0, 0);
  methods_ty = (Type*)type_env->lookup(type_decl->controlTypeDeclaration.method_protos, 0);
  control_ty->control.methods = methods_ty;
  name_decl = (NameDeclaration*)decl_map->lookup(type_decl, 0);
  name_decl->type = control_ty;
}

void DeclaredTypePass::visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  Tree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_controlLocalDeclaration(Ast* local_decl)
{
  assert(local_decl->kind == AstEnum::controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::actionDeclaration) {
    visit_actionDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::tableDeclaration) {
    visit_tableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::instantiation) {
    visit_instantiation(local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

void DeclaredTypePass::visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    visit_functionPrototype(extern_decl->externDeclaration.decl, 0, 0);
  } else assert(0);
}

void DeclaredTypePass::visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* extern_ty, *methods_ty, *ctors_ty;

  name = type_decl->externTypeDeclaration.name;
  extern_ty = (Type*)type_array->append();
  extern_ty->ty_former = TypeEnum::EXTERN;
  extern_ty->strname = name->name.strname;
  extern_ty->ast = type_decl;
  type_env->insert(type_decl, extern_ty, 0);
  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos, extern_ty, name->name.strname);
  methods_ty = (Type*)type_env->lookup(type_decl->externTypeDeclaration.method_protos, 0);
  extern_ty->extern_.methods = methods_ty;
  ctors_ty = (Type*)type_array->append();
  ctors_ty->ty_former = TypeEnum::PRODUCT;
  ctors_ty->ast = type_decl;
  for (int i = 0; i < methods_ty->product.count; i++) {
    if (cstring::match(methods_ty->product.members[i]->strname, name->name.strname)) {
      ctors_ty->product.count += 1;
    }
  }
  if (ctors_ty->product.count > 0) {
    ctors_ty->product.members = storage->allocate<Type *>(ctors_ty->product.count);
  }
  for (int i = 0; i < methods_ty->product.count; i++) {
    if (cstring::match(methods_ty->product.members[i]->strname, name->name.strname)) {
      ctors_ty->product.members[i] = methods_ty->product.members[i];
    }
  }
  extern_ty->extern_.ctors = ctors_ty;
  name_decl = (NameDeclaration*)decl_map->lookup(type_decl, 0);
  name_decl->type = extern_ty;
}

void DeclaredTypePass::visit_methodPrototypes(Ast* protos, Type* ctor_ty, char* ctor_strname)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  Tree* ast;
  Type* methods_ty;
  int i;

  methods_ty = (Type*)type_array->append();
  methods_ty->ty_former = TypeEnum::PRODUCT;
  methods_ty->ast = protos;
  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(container_of(ast, Ast, tree), ctor_ty, ctor_strname);
    methods_ty->product.count += 1;
  }
  if (methods_ty->product.count > 0) {
    methods_ty->product.members = storage->allocate<Type *>(methods_ty->product.count);
  }
  i = 0;
  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    methods_ty->product.members[i] = (Type*)type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == methods_ty->product.count);
  type_env->insert(protos, methods_ty, 0);
}

void DeclaredTypePass::visit_functionPrototype(Ast* func_proto, Type* ctor_ty, char* ctor_strname)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
  Ast* name, *return_type;
  NameDeclaration* name_decl;
  Type* func_ty;

  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }
  visit_parameterList(func_proto->functionPrototype.params);
  name = func_proto->functionPrototype.name;
  func_ty = (Type*)type_array->append();
  func_ty->ty_former = TypeEnum::FUNCTION;
  func_ty->strname = name->name.strname;
  func_ty->ast = func_proto;
  func_ty->function.params = (Type*)type_env->lookup(func_proto->functionPrototype.params, 0);
  type_env->insert(func_proto, func_ty, 0);
  return_type = func_proto->functionPrototype.return_type;
  if (return_type) {
    func_ty->function.return_ = (Type*)type_env->lookup(return_type, 0);
  } else if (cstring::match(name->name.strname, ctor_strname)) {
    func_ty->function.return_ = ctor_ty;
  } else assert(0);
  name_decl = (NameDeclaration*)decl_map->lookup(func_proto, 0);
  name_decl->type = func_ty;
}

/** TYPES **/

void DeclaredTypePass::visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AstEnum::typeRef);
  Type* ref_ty;

  if (type_ref->typeRef.type->kind == AstEnum::baseTypeBoolean) {
    visit_baseTypeBoolean(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeInteger) {
    visit_baseTypeInteger(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeBit) {
    visit_baseTypeBit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVarbit) {
    visit_baseTypeVarbit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeString) {
    visit_baseTypeString(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVoid) {
    visit_baseTypeVoid(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeError) {
    visit_baseTypeError(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::name) {
    visit_name(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
  ref_ty = (Type*)type_env->lookup(type_ref->typeRef.type, 0);
  type_env->insert(type_ref, ref_ty, 0);
}

void DeclaredTypePass::visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  Type* tuple_ty;

  visit_typeArgumentList(type_decl->tupleType.type_args);
  tuple_ty = (Type*)type_env->lookup(type_decl->tupleType.type_args, 0);
  type_env->insert(type_decl, tuple_ty, 0);
}

void DeclaredTypePass::visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  Type* stack_ty;

  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr);
  stack_ty = (Type*)type_array->append();
  stack_ty->ty_former = TypeEnum::STACK;
  stack_ty->ast = type_decl;
  type_env->insert(type_decl, stack_ty, 0);
  stack_ty->header_stack.element = (Type*)type_env->lookup(type_decl->headerStackType.type, 0);
}

void DeclaredTypePass::visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  NameDeclaration* name_decl;

  name_decl = (NameDeclaration*)decl_map->lookup(bool_type, 0);
  type_env->insert(bool_type, name_decl->type, 0);
}

void DeclaredTypePass::visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  NameDeclaration* name_decl;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  name_decl = (NameDeclaration*)decl_map->lookup(int_type, 0);
  type_env->insert(int_type, name_decl->type, 0);
}

void DeclaredTypePass::visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  NameDeclaration* name_decl;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  name_decl = (NameDeclaration*)decl_map->lookup(bit_type, 0);
  type_env->insert(bit_type, name_decl->type, 0);
}

void DeclaredTypePass::visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  NameDeclaration* name_decl;

  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  name_decl = (NameDeclaration*)decl_map->lookup(varbit_type, 0);
  type_env->insert(varbit_type, name_decl->type, 0);
}

void DeclaredTypePass::visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AstEnum::baseTypeString);
  NameDeclaration* name_decl;

  name_decl = (NameDeclaration*)decl_map->lookup(str_type, 0);
  type_env->insert(str_type, name_decl->type, 0);
}

void DeclaredTypePass::visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  NameDeclaration* name_decl;

  name_decl = (NameDeclaration*)decl_map->lookup(void_type, 0);
  type_env->insert(void_type, name_decl->type, 0);
}

void DeclaredTypePass::visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  NameDeclaration* name_decl;

  name_decl = (NameDeclaration*)decl_map->lookup(error_type, 0);
  type_env->insert(error_type, name_decl->type, 0);
}

void DeclaredTypePass::visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
}

void DeclaredTypePass::visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

void DeclaredTypePass::visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  Type* arg_ty;

  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::name) {
    visit_name(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
  arg_ty = (Type*)type_env->lookup(type_arg->typeArg.arg, 0);
  type_env->insert(type_arg, arg_ty, 0);
}

void DeclaredTypePass::visit_typeArgumentList(Ast* args)
{
  assert(args->kind == AstEnum::typeArgumentList);
  Tree* ast;
  Type* args_ty;
  int i;

  args_ty = (Type*)type_array->append();
  args_ty->ty_former = TypeEnum::PRODUCT;
  args_ty->ast = args;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(container_of(ast, Ast, tree));
    args_ty->product.count += 1;
  }
  if (args_ty->product.count > 0) {
    args_ty->product.members = storage->allocate<Type *>(args_ty->product.count);
  }
  i = 0;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    args_ty->product.members[i] = (Type*)type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == args_ty->product.count);
  type_env->insert(args, args_ty, 0);
}

void DeclaredTypePass::visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::typeDeclaration);
  Type* decl_ty;

  if (type_decl->typeDeclaration.decl->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::typedefDeclaration) {
    visit_typedefDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::packageTypeDeclaration) {
    visit_packageTypeDeclaration(type_decl->typeDeclaration.decl);
  } else assert(0);
  decl_ty = (Type*)type_env->lookup(type_decl->typeDeclaration.decl, 0);
  type_env->insert(type_decl, decl_ty, 0);
}

void DeclaredTypePass::visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::derivedTypeDeclaration);
  Type* decl_ty;

  if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerTypeDeclaration) {
    visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerUnionDeclaration) {
    visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::structTypeDeclaration) {
    visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::enumDeclaration) {
    visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  decl_ty = (Type*)type_env->lookup(type_decl->derivedTypeDeclaration.decl, 0);
  type_env->insert(type_decl, decl_ty, 0);
}

void DeclaredTypePass::visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* header_ty;

  visit_structFieldList(header_decl->headerTypeDeclaration.fields);
  name = header_decl->headerTypeDeclaration.name;
  header_ty = (Type*)type_array->append();
  header_ty->ty_former = TypeEnum::HEADER;
  header_ty->strname = name->name.strname;
  header_ty->ast = header_decl;
  type_env->insert(header_decl, header_ty, 0);
  header_ty->struct_.fields = (Type*)type_env->lookup(header_decl->headerTypeDeclaration.fields, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(header_decl, 0);
  name_decl->type = header_ty;
}

void DeclaredTypePass::visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* union_ty;

  visit_structFieldList(union_decl->headerUnionDeclaration.fields);
  name = union_decl->headerUnionDeclaration.name;
  union_ty = (Type*)type_array->append();
  union_ty->ty_former = TypeEnum::UNION;
  union_ty->strname = name->name.strname;
  union_ty->ast = union_decl;
  type_env->insert(union_decl, union_ty, 0);
  union_ty->struct_.fields = (Type*)type_env->lookup(union_decl->headerUnionDeclaration.fields, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(union_decl, 0);
  name_decl->type = union_ty;
}

void DeclaredTypePass::visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* struct_ty;

  visit_structFieldList(struct_decl->structTypeDeclaration.fields);
  name = struct_decl->structTypeDeclaration.name;
  struct_ty = (Type*)type_array->append();
  struct_ty->ty_former = TypeEnum::STRUCT;
  struct_ty->strname = name->name.strname;
  struct_ty->ast = struct_decl;
  type_env->insert(struct_decl, struct_ty, 0);
  struct_ty->struct_.fields = (Type*)type_env->lookup(struct_decl->structTypeDeclaration.fields, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(struct_decl, 0);
  name_decl->type = struct_ty;
}

void DeclaredTypePass::visit_structFieldList(Ast* fields)
{
  assert(fields->kind == AstEnum::structFieldList);
  Tree* ast;
  Type* fields_ty;
  int i;

  fields_ty = (Type*)type_array->append();
  fields_ty->ty_former = TypeEnum::PRODUCT;
  fields_ty->ast = fields;
  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(container_of(ast, Ast, tree));
    fields_ty->product.count += 1;
  }
  if (fields_ty->product.count > 0) {
    fields_ty->product.members = storage->allocate<Type *>(fields_ty->product.count);
  }
  i = 0;
  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    fields_ty->product.members[i] = (Type*)type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == fields_ty->product.count);
  type_env->insert(fields, fields_ty, 0);
}

void DeclaredTypePass::visit_structField(Ast* field)
{
  assert(field->kind == AstEnum::structField);
  Ast* name;
  NameDeclaration* name_decl;
  Type* field_ty;

  visit_typeRef(field->structField.type);
  name = field->structField.name;
  field_ty = (Type*)type_array->append();
  field_ty->ty_former = TypeEnum::FIELD;
  field_ty->strname = name->name.strname;
  field_ty->ast = field;
  field_ty->field.type = (Type*)type_env->lookup(field->structField.type, 0);
  type_env->insert(field, field_ty, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(field, 0);
  name_decl->type = field_ty;
}

void DeclaredTypePass::visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* enum_ty;

  name = enum_decl->enumDeclaration.name;
  enum_ty = (Type*)type_array->append();
  enum_ty->ty_former = TypeEnum::ENUM;
  enum_ty->strname = name->name.strname;
  enum_ty->ast = enum_decl;
  type_env->insert(enum_decl, enum_ty, 0);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields, enum_ty);
  enum_ty->enum_.fields = (Type*)type_env->lookup(enum_decl->enumDeclaration.fields, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(enum_decl, 0);
  name_decl->type = enum_ty;
}

void DeclaredTypePass::visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
  Type* error_ty, *fields_ty;

  error_ty = root_scope->builtin_lookup("error", NameSpace::TYPE)->type;
  fields_ty = error_ty->enum_.fields;
  if (error_ty->enum_.field_count > 0 && fields_ty->product.members == 0) {
    fields_ty->product.count = error_ty->enum_.field_count;
    fields_ty->product.members = storage->allocate<Type *>(fields_ty->product.count);
  }
  visit_identifierList(error_decl->errorDeclaration.fields, error_ty,
      error_ty->enum_.fields, &error_ty->enum_.i);
}

void DeclaredTypePass::visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
  Type* match_kind_ty, *fields_ty;

  match_kind_ty = root_scope->builtin_lookup("match_kind", NameSpace::TYPE)->type;
  fields_ty = match_kind_ty->enum_.fields;
  if (match_kind_ty->enum_.field_count > 0 && fields_ty->product.members == 0) {
    fields_ty->product.count = match_kind_ty->enum_.field_count;
    fields_ty->product.members = storage->allocate<Type *>(fields_ty->product.count);
  }
  visit_identifierList(match_decl->matchKindDeclaration.fields, match_kind_ty,
      match_kind_ty->enum_.fields, &match_kind_ty->enum_.i);
}

void DeclaredTypePass::visit_identifierList(Ast* ident_list, Type* enum_ty, Type* idents_ty, int* i)
{
  assert(ident_list->kind == AstEnum::identifierList);
  Tree* ast;
  NameDeclaration* name_decl;
  Type* name_ty;
  int j;

  j = *i;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    name_ty = (Type*)type_array->append();
    name_ty->ty_former = TypeEnum::FIELD;
    name_ty->strname = container_of(ast, Ast, tree)->name.strname;
    name_ty->ast = container_of(ast, Ast, tree);
    name_ty->field.type = enum_ty;
    type_env->insert(container_of(ast, Ast, tree), name_ty, 0);
    name_decl = (NameDeclaration*)decl_map->lookup(container_of(ast, Ast, tree), 0);
    name_decl->type = name_ty;
    idents_ty->product.members[j] = (Type*)type_env->lookup(container_of(ast, Ast, tree), 0);
    j += 1;
  }
  *i = j;
}

void DeclaredTypePass::visit_specifiedIdentifierList(Ast* ident_list, Type* enum_ty)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  Tree* ast;
  Type* idents_ty;
  int i;

  idents_ty = (Type*)type_array->append();
  idents_ty->ty_former = TypeEnum::PRODUCT;
  idents_ty->ast = ident_list;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(container_of(ast, Ast, tree), enum_ty);
    idents_ty->product.count += 1;
  }
  if (idents_ty->product.count > 0) {
    idents_ty->product.members = storage->allocate<Type *>(idents_ty->product.count);
  }
  i = 0;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    idents_ty->product.members[i] = (Type*)type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == idents_ty->product.count);
  type_env->insert(ident_list, idents_ty, 0);
}

void DeclaredTypePass::visit_specifiedIdentifier(Ast* ident, Type* enum_ty)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  Ast* name;
  NameDeclaration* name_decl;
  Type* ident_ty;

  name = ident->specifiedIdentifier.name;
  ident_ty = (Type*)type_array->append();
  ident_ty->ty_former = TypeEnum::FIELD;
  ident_ty->strname = name->name.strname;
  ident_ty->ast = ident;
  ident_ty->field.type = enum_ty;
  type_env->insert(ident, ident_ty, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(ident, 0);
  name_decl->type = ident_ty;
}

void DeclaredTypePass::visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* typedef_ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  name = typedef_decl->typedefDeclaration.name;
  typedef_ty = (Type*)type_array->append();
  typedef_ty->ty_former = TypeEnum::TYPEDEF;
  typedef_ty->strname = name->name.strname;
  typedef_ty->ast = typedef_decl;
  type_env->insert(typedef_decl, typedef_ty, 0);
  typedef_ty->typedef_.ref = (Type*)type_env->lookup(typedef_decl->typedefDeclaration.type_ref, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(typedef_decl, 0);
  name_decl->type = typedef_ty;
}

/** STATEMENTS **/

void DeclaredTypePass::visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr);
}

void DeclaredTypePass::visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AstEnum::functionCall);
  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(func_call->functionCall.args);
}

void DeclaredTypePass::visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr);
  }
}

void DeclaredTypePass::visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
}

void DeclaredTypePass::visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  visit_expression(cond_stmt->conditionalStatement.cond_expr);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

void DeclaredTypePass::visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(applic_stmt->directApplication.args);
}

void DeclaredTypePass::visit_statement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::statement);
  if (stmt->statement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::conditionalStatement) {
    visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AstEnum::blockStatement) {
    visit_blockStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::exitStatement) {
    visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::returnStatement) {
    visit_returnStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::switchStatement) {
    visit_switchStatement(stmt->statement.stmt);
  } else assert(0);
}

void DeclaredTypePass::visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
}

void DeclaredTypePass::visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  Tree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  visit_expression(switch_stmt->switchStatement.expr);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

void DeclaredTypePass::visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  Tree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(switch_case->switchCase.stmt);
  }
}

void DeclaredTypePass::visit_switchLabel(Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    ;
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    visit_default(label->switchLabel.label);
  } else assert(0);
}

void DeclaredTypePass::visit_statementOrDeclaration(Ast* stmt)
{
  assert(stmt->kind == AstEnum::statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::statement) {
    visit_statement(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::instantiation) {
    visit_instantiation(stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

void DeclaredTypePass::visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* table_ty, *methods_ty;

  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
  name = table_decl->tableDeclaration.name;
  table_ty = (Type*)type_array->append();
  table_ty->ty_former = TypeEnum::TABLE;
  table_ty->strname = name->name.strname;
  table_ty->ast = table_decl;
  type_env->insert(table_decl, table_ty, 0);
  visit_methodPrototypes(table_decl->tableDeclaration.method_protos, 0, 0);
  methods_ty = (Type*)type_env->lookup(table_decl->tableDeclaration.method_protos, 0);
  table_ty->table.methods = methods_ty;
  name_decl = (NameDeclaration*)decl_map->lookup(table_decl, 0);
  name_decl->type = table_ty;
}

void DeclaredTypePass::visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  Tree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_tableProperty(Ast* table_prop)
{
  assert(table_prop->kind == AstEnum::tableProperty);
  if (table_prop->tableProperty.prop->kind == AstEnum::keyProperty) {
    visit_keyProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::actionsProperty) {
    visit_actionsProperty(table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AstEnum::entriesProperty) {
    visit_entriesProperty(checker, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::simpleProperty) {
    visit_simpleProperty(checker, table_prop->tableProperty.prop);
  }
#endif  
  else assert(0);
}

void DeclaredTypePass::visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  visit_keyElementList(key_prop->keyProperty.keyelem_list);
}

void DeclaredTypePass::visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  Tree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_keyElement(Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  visit_expression(element->keyElement.expr);
}

void DeclaredTypePass::visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  visit_actionList(actions_prop->actionsProperty.action_list);
}

void DeclaredTypePass::visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  Tree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AstEnum::actionRef);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args);
  }
}

#if 0
void DeclaredTypePass::visit_entriesProperty(Ast* entries_prop)
{
  assert(entries_prop->kind == AstEnum::entriesProperty);
  visit_entriesList(checker, entries_prop->entriesProperty.entries_list);
}

void DeclaredTypePass::visit_entriesList(Ast* entries_list)
{
  assert(entries_list->kind == AstEnum::entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(checker, container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_entry(Ast* entry)
{
  assert(entry->kind == AstEnum::entry);
  visit_keysetExpression(checker, entry->entry.keyset);
  visit_actionRef(checker, entry->entry.action);
}

void DeclaredTypePass::visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AstEnum::simpleProperty);
  visit_expression(checker, simple_prop->simpleProperty.init_expr);
}
#endif

void DeclaredTypePass::visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  NameDeclaration* name_decl;
  Ast* name;
  Type* action_ty;

  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
  name = action_decl->actionDeclaration.name;
  action_ty = (Type*)type_array->append();
  action_ty->ty_former = TypeEnum::FUNCTION;
  action_ty->strname = name->name.strname;
  action_ty->ast = action_decl;
  action_ty->function.params = (Type*)type_env->lookup(action_decl->actionDeclaration.params, 0);
  type_env->insert(action_decl, action_ty, 0);
  action_ty->function.return_ = root_scope->builtin_lookup("void", NameSpace::TYPE)->type;
  name_decl = (NameDeclaration*)decl_map->lookup(action_decl, 0);
  name_decl->type = action_ty;
}

/** VARIABLES **/

void DeclaredTypePass::visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  NameDeclaration* name_decl;
  Type* var_ty;

  visit_typeRef(var_decl->variableDeclaration.type);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr);
  }
  var_ty = (Type*)type_env->lookup(var_decl->variableDeclaration.type, 0);
  type_env->insert(var_decl, var_ty, 0);
  name_decl = (NameDeclaration*)decl_map->lookup(var_decl, 0);
  name_decl->type = var_ty;
}

/** EXPRESSIONS **/

void DeclaredTypePass::visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  visit_functionPrototype(func_decl->functionDeclaration.proto, 0, 0);
  visit_blockStatement(func_decl->functionDeclaration.stmt);
}

void DeclaredTypePass::visit_argumentList(Ast* args)
{
  assert(args->kind == AstEnum::argumentList);
  Tree* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_argument(Ast* arg)
{
  assert(arg->kind == AstEnum::argument);
  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(arg->argument.arg);
  } else assert(0);
}

void DeclaredTypePass::visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::expressionList);
  Tree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(container_of(ast, Ast, tree));
  }
}

void DeclaredTypePass::visit_lvalueExpression(Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    ;
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

void DeclaredTypePass::visit_expression(Ast* expr)
{
  assert(expr->kind == AstEnum::expression);
  if (expr->expression.expr->kind == AstEnum::expression) {
    visit_expression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::integerLiteral) {
    visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::stringLiteral) {
    visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::name) {
    ;
  } else if (expr->expression.expr->kind == AstEnum::expressionList) {
    visit_expressionList(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::castExpression) {
    visit_castExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::unaryExpression) {
    visit_unaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::binaryExpression) {
    visit_binaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::functionCall) {
    visit_functionCall(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
}

void DeclaredTypePass::visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr);
}

void DeclaredTypePass::visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand);
}

void DeclaredTypePass::visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  visit_expression(binary_expr->binaryExpression.left_operand);
  visit_expression(binary_expr->binaryExpression.right_operand);
}

void DeclaredTypePass::visit_memberSelector(Ast* selector)
{
  assert(selector->kind == AstEnum::memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr);
  } else assert(0);
}

void DeclaredTypePass::visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
}

void DeclaredTypePass::visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  Type* ty;

  visit_expression(index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index);
  }
  ty = root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
  type_env->insert(index_expr, ty, 0);
}

void DeclaredTypePass::visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
  Type* ty;

  ty = root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
  type_env->insert(bool_literal, ty, 0);
}

void DeclaredTypePass::visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
  Type* ty;

  ty = root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
  type_env->insert(int_literal, ty, 0);
}

void DeclaredTypePass::visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
  Type* ty;

  ty = root_scope->builtin_lookup("string", NameSpace::TYPE)->type;
  type_env->insert(str_literal, ty, 0);
}

void DeclaredTypePass::visit_default(Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
  Type* ty;

  ty = root_scope->builtin_lookup("_", NameSpace::TYPE)->type;
  type_env->insert(default_, ty, 0);
}

void DeclaredTypePass::visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
  Type* ty;

  ty = root_scope->builtin_lookup("_", NameSpace::TYPE)->type;
  type_env->insert(dontcare, ty, 0);
}

