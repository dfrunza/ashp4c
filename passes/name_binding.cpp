#include <stdio.h>
#include <basic.h>
#include <passes/name_binding.h>

struct BuiltinName {
  char* strname;
  enum NameSpace ns;
};

struct BuiltinType {
  char* strname;
  enum TypeEnum ty_former;
};

void NameBindingPass::define_builtin_names()
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
    name = storage->allocate<Ast>();
    name->kind = AstEnum::name;
    name->name.strname = builtin_names[i].strname;
    name_decl = root_scope->bind_name(storage, name->name.strname, builtin_names[i].ns);
    name_decl->ast = name;
  }
  for (int i = 0; i < sizeof(builtin_types)/sizeof(builtin_types[0]); i++) {
    name_entry = root_scope->lookup(builtin_types[i].strname, NameSpace::TYPE);
    name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
    ty = type_array->append();
    ty->ty_former = builtin_types[i].ty_former;
    ty->strname = name_decl->strname;
    ty->ast = name_decl->ast;
    name_decl->type = ty;
  }

  ty = root_scope->lookup_builtin("error", NameSpace::TYPE)->type;
  ty->enum_.fields = type_array->append();
  ty->enum_.fields->ty_former = TypeEnum::PRODUCT;

  ty = root_scope->lookup_builtin("match_kind", NameSpace::TYPE)->type;
  ty->enum_.fields = type_array->append();
  ty->enum_.fields->ty_former = TypeEnum::PRODUCT;
}

void DEBUG_scope_decls(Scope* scope)
{
  NameEntry* name_entry;
  NameDeclaration* decl;
  int count = 0;
  enum NameSpace ns[] = {NameSpace::VAR, NameSpace::TYPE, NameSpace::KEYWORD};

  printf("Names in scope 0x%x\n\n", scope);
  StrmapIterator<NameEntry> it(&scope->name_table);
  StrmapEntry<NameEntry>* he = it.next();
  while (he) {
    name_entry = he->value;
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

void NameBindingPass::do_pass()
{
  current_scope = root_scope;
  decl_map = storage->allocate<Map<Ast, NameDeclaration>>();
  decl_map->storage = storage;
  type_array = Array<Type>::create(storage, 5);
  define_builtin_names();
  visit_p4program(p4program);
  assert(current_scope == root_scope);
}

/** PROGRAM **/

void NameBindingPass::visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  Scope* prev_scope;

  prev_scope = current_scope;
  current_scope = scope_map->lookup(p4program, 0);
  visit_declarationList(p4program->p4program.decl_list);
  current_scope = prev_scope;
}

void NameBindingPass::visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  Tree<Ast>* tree;

  for (tree = decl_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_declaration(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_declaration(Ast* decl)
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

void NameBindingPass::visit_name(Ast* name)
{
  MapEntry<Ast, Scope>* m;
  assert(name->kind == AstEnum::name);
  m = scope_map->insert(name, current_scope, 0);
  assert(m);
}

void NameBindingPass::visit_parameterList(Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  Tree<Ast>* tree;

  for (tree = params->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_parameter(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_parameter(Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(param->parameter.type);
  visit_name(param->parameter.name);
  name = param->parameter.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::VAR);
  name_decl->ast = param;
  decl_map->insert(param, name_decl, 0);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr);
  }
}

void NameBindingPass::visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(type_decl->packageTypeDeclaration.name);
  name = type_decl->packageTypeDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = type_decl;
  decl_map->insert(type_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(type_decl, 0);
  visit_parameterList(type_decl->packageTypeDeclaration.params);
  current_scope = prev_scope;
}

void NameBindingPass::visit_instantiation(Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(inst->instantiation.type);
  visit_argumentList(inst->instantiation.args);
  name = inst->instantiation.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::VAR);
  name_decl->ast = inst;
  decl_map->insert(inst, name_decl, 0);
}

/** PARSER **/

void NameBindingPass::visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  Scope* prev_scope;

  visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(parser_decl, 0);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
  current_scope = prev_scope;
}

void NameBindingPass::visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(type_decl->parserTypeDeclaration.name);
  name = type_decl->parserTypeDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = type_decl;
  decl_map->insert(type_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(type_decl, 0);
  visit_parameterList(type_decl->parserTypeDeclaration.params);
  visit_methodPrototypes(type_decl->parserTypeDeclaration.method_protos, name_decl);
  current_scope = prev_scope;
}

void NameBindingPass::visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  Tree<Ast>* tree;

  for (tree = local_elements->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_parserLocalElement(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
}

void NameBindingPass::visit_parserStates(Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  Tree<Ast>* tree;

  for (tree = states->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_parserState(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_parserState(Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = state->parserState.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::VAR);
  name_decl->ast = state;
  decl_map->insert(state, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(state, 0);
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
  current_scope = prev_scope;
}

void NameBindingPass::visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  Tree<Ast>* tree;

  for (tree = stmts->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_parserStatement(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_parserStatement(Ast* stmt)
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

void NameBindingPass::visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  Scope* prev_scope;

  prev_scope = current_scope;
  current_scope = scope_map->lookup(block_stmt, 0);
  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
  current_scope = prev_scope;
}

void NameBindingPass::visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  visit_stateExpression(transition_stmt->transitionStatement.stmt);
}

void NameBindingPass::visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    visit_name(state_expr->stateExpression.expr);
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
}

void NameBindingPass::visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  visit_expressionList(select_expr->selectExpression.expr_list);
  visit_selectCaseList(select_expr->selectExpression.case_list);
}

void NameBindingPass::visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  Tree<Ast>* tree;

  for (tree = case_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_selectCase(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AstEnum::selectCase);
  visit_keysetExpression(select_case->selectCase.keyset_expr);
  visit_name(select_case->selectCase.name);
}

void NameBindingPass::visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
}

void NameBindingPass::visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
}

void NameBindingPass::visit_simpleKeysetExpression(Ast* simple_expr)
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

void NameBindingPass::visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  Tree<Ast>* tree;

  for (tree = expr_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_simpleKeysetExpression(Ast::from_tree(tree));
  }
}

/** CONTROL **/

void NameBindingPass::visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AstEnum::controlDeclaration);
  Scope* prev_scope;

  visit_typeDeclaration(control_decl->controlDeclaration.proto);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(control_decl, 0);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
  current_scope = prev_scope;
}

void NameBindingPass::visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(type_decl->controlTypeDeclaration.name);
  name = type_decl->controlTypeDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = type_decl;
  decl_map->insert(type_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(type_decl, 0);
  visit_parameterList(type_decl->controlTypeDeclaration.params);
  visit_methodPrototypes(type_decl->controlTypeDeclaration.method_protos, name_decl);
  current_scope = prev_scope;
}

void NameBindingPass::visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  Tree<Ast>* tree;

  for (tree = local_decls->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_controlLocalDeclaration(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_controlLocalDeclaration(Ast* local_decl)
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

void NameBindingPass::visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    visit_functionPrototype(extern_decl->externDeclaration.decl);
  } else assert(0);
}

void NameBindingPass::visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = type_decl->externTypeDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = type_decl;
  decl_map->insert(type_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(type_decl, 0);
  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos, name_decl);
  current_scope = prev_scope;
}

void NameBindingPass::visit_methodPrototypes(Ast* protos, NameDeclaration* name_decl)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  Tree<Ast>* tree;

  for (tree = protos->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_functionPrototype(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(func_proto->functionPrototype.name);
  name = func_proto->functionPrototype.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = func_proto;
  decl_map->insert(func_proto, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(func_proto, 0);
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }
  visit_parameterList(func_proto->functionPrototype.params);
  current_scope = prev_scope;
}

/** TYPES **/

void NameBindingPass::visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AstEnum::typeRef);
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
}

void NameBindingPass::visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  visit_typeArgumentList(type_decl->tupleType.type_args);
}

void NameBindingPass::visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr);
}

void NameBindingPass::visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = root_scope->lookup("bool", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  decl_map->insert(bool_type, name_decl, 0);
}

void NameBindingPass::visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  name_entry = root_scope->lookup("int", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  decl_map->insert(int_type, name_decl, 0);
}

void NameBindingPass::visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  name_entry = root_scope->lookup("bit", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  decl_map->insert(bit_type, name_decl, 0);
}

void NameBindingPass::visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  name_entry = root_scope->lookup("varbit", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  decl_map->insert(varbit_type, name_decl, 0);
}

void NameBindingPass::visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AstEnum::baseTypeString);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = root_scope->lookup("string", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  decl_map->insert(str_type, name_decl, 0);
}

void NameBindingPass::visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = root_scope->lookup("void", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  decl_map->insert(void_type, name_decl, 0);
}

void NameBindingPass::visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = root_scope->lookup("error", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  decl_map->insert(error_type, name_decl, 0);
}

void NameBindingPass::visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
}

void NameBindingPass::visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

void NameBindingPass::visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::name) {
    visit_name(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
}

void NameBindingPass::visit_typeArgumentList(Ast* arg_list)
{
  assert(arg_list->kind == AstEnum::typeArgumentList);
  Tree<Ast>* tree;

  for (tree = arg_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_typeArg(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::typeDeclaration);
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
}

void NameBindingPass::visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::derivedTypeDeclaration);
  if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerTypeDeclaration) {
    visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerUnionDeclaration) {
    visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::structTypeDeclaration) {
    visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::enumDeclaration) {
    visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
}

void NameBindingPass::visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = header_decl->headerTypeDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = header_decl;
  decl_map->insert(header_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(header_decl, 0);
  visit_structFieldList(header_decl->headerTypeDeclaration.fields, name_decl);
  current_scope = prev_scope;
}

void NameBindingPass::visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = union_decl->headerUnionDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = union_decl;
  decl_map->insert(union_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(union_decl, 0);
  visit_structFieldList(union_decl->headerUnionDeclaration.fields, name_decl);
  current_scope = prev_scope;
}

void NameBindingPass::visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = struct_decl->structTypeDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = struct_decl;
  decl_map->insert(struct_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(struct_decl, 0);
  visit_structFieldList(struct_decl->structTypeDeclaration.fields, name_decl);
  current_scope = prev_scope;
}

void NameBindingPass::visit_structFieldList(Ast* field_list, NameDeclaration* name_decl)
{
  assert(field_list->kind == AstEnum::structFieldList);
  Tree<Ast>* tree;

  for (tree = field_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_structField(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_structField(Ast* field)
{
  assert(field->kind == AstEnum::structField);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(field->structField.type);
  name = field->structField.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = field;
  decl_map->insert(field, name_decl, 0);
}

void NameBindingPass::visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = enum_decl->enumDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = enum_decl;
  decl_map->insert(enum_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(enum_decl, 0);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields, name_decl);
  current_scope = prev_scope;
}

void NameBindingPass::visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
  Scope* prev_scope;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  Type* error_ty;

  name_entry = root_scope->lookup("error", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  error_ty = name_decl->type;
  decl_map->insert(error_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(error_decl, 0);
  error_ty->enum_.field_count += visit_identifierList(error_decl->errorDeclaration.fields);
  current_scope = prev_scope;
}

void NameBindingPass::visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
  Scope* prev_scope;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  Type* match_kind_ty;

  name_entry = root_scope->lookup("match_kind", NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  match_kind_ty = name_decl->type;
  decl_map->insert(match_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(match_decl, 0);
  match_kind_ty->enum_.field_count += visit_identifierList(match_decl->matchKindDeclaration.fields);
  current_scope = prev_scope;
}

int NameBindingPass::visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::identifierList);
  Ast* name;
  Tree<Ast>* tree;
  NameDeclaration* name_decl;
  int count = 0;

  for (tree = ident_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    name = Ast::from_tree(tree);
    name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
    name_decl->ast = name;
    decl_map->insert(name, name_decl, 0);
    count += 1;
  }
  return count;
}

void NameBindingPass::visit_specifiedIdentifierList(Ast* ident_list, NameDeclaration* name_decl)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  Tree<Ast>* tree;

  for (tree = ident_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_specifiedIdentifier(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  Ast* name;
  NameDeclaration* name_decl;

  name = ident->specifiedIdentifier.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = ident;
  decl_map->insert(ident, name_decl, 0);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr);
  }
}

void NameBindingPass::visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  Ast* name;
  NameDeclaration* name_decl;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  name = typedef_decl->typedefDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = typedef_decl;
  decl_map->insert(typedef_decl, name_decl, 0);
}

/** STATEMENTS **/

void NameBindingPass::visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr);
}

void NameBindingPass::visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AstEnum::functionCall);
  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(func_call->functionCall.args);
}

void NameBindingPass::visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr);
  }
}

void NameBindingPass::visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
}

void NameBindingPass::visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  visit_expression(cond_stmt->conditionalStatement.cond_expr);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

void NameBindingPass::visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  if (applic_stmt->directApplication.name->kind == AstEnum::name) {
    visit_name(applic_stmt->directApplication.name);
  } else if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(applic_stmt->directApplication.args);
}

void NameBindingPass::visit_statement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::statement);
  Scope* prev_scope;

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
    prev_scope = current_scope;
    current_scope = scope_map->lookup(stmt, 0);
    visit_blockStatement(stmt->statement.stmt);
    current_scope = prev_scope;
  } else if (stmt->statement.stmt->kind == AstEnum::exitStatement) {
    visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::returnStatement) {
    visit_returnStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::switchStatement) {
    visit_switchStatement(stmt->statement.stmt);
  } else assert(0);
}

void NameBindingPass::visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
}

void NameBindingPass::visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  Tree<Ast>* tree;

  for (tree = stmt_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_statementOrDeclaration(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  visit_expression(switch_stmt->switchStatement.expr);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

void NameBindingPass::visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  Tree<Ast>* tree;

  for (tree = switch_cases->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_switchCase(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(switch_case->switchCase.stmt);
  }
}

void NameBindingPass::visit_switchLabel(Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    visit_name(label->switchLabel.label);
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    visit_default(label->switchLabel.label);
  } else assert(0);
}

void NameBindingPass::visit_statementOrDeclaration(Ast* stmt)
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

void NameBindingPass::visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = table_decl->tableDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = table_decl;
  decl_map->insert(table_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(table_decl, 0);
  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
  visit_methodPrototypes(table_decl->tableDeclaration.method_protos, name_decl);
  current_scope = prev_scope;
}

void NameBindingPass::visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  Tree<Ast>* tree;

  for (tree = prop_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_tableProperty(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_tableProperty(Ast* table_prop)
{
  assert(table_prop->kind == AstEnum::tableProperty);
  if (table_prop->tableProperty.prop->kind == AstEnum::keyProperty) {
    visit_keyProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::actionsProperty) {
    visit_actionsProperty(table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AstEnum::entriesProperty) {
    visit_entriesProperty(name_binder, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::simpleProperty) {
    visit_simpleProperty(name_binder, table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
}

void NameBindingPass::visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  visit_keyElementList(key_prop->keyProperty.keyelem_list);
}

void NameBindingPass::visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  Tree<Ast>* tree;

  for (tree = element_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_keyElement(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_keyElement(Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  Scope* prev_scope;

  visit_expression(element->keyElement.expr);
  prev_scope = current_scope;
  current_scope = root_scope;
  visit_name(element->keyElement.match);
  current_scope = prev_scope;
}

void NameBindingPass::visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  visit_actionList(actions_prop->actionsProperty.action_list);
}

void NameBindingPass::visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  Tree<Ast>* tree;

  for (tree = action_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_actionRef(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AstEnum::actionRef);
  visit_name(action_ref->actionRef.name);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args);
  }
}

void NameBindingPass::visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = action_decl->actionDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = action_decl;
  decl_map->insert(action_decl, name_decl, 0);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(action_decl, 0);
  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
  current_scope = prev_scope;
}

/** VARIABLES **/

void NameBindingPass::visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(var_decl->variableDeclaration.type);
  visit_name(var_decl->variableDeclaration.name);
  name = var_decl->variableDeclaration.name;
  name_decl = current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
  name_decl->ast = var_decl;
  decl_map->insert(var_decl, name_decl, 0);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr);
  }
}

/** EXPRESSIONS **/

void NameBindingPass::visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  Scope* prev_scope;

  visit_functionPrototype(func_decl->functionDeclaration.proto);
  prev_scope = current_scope;
  current_scope = scope_map->lookup(func_decl, 0);
  visit_blockStatement(func_decl->functionDeclaration.stmt);
  current_scope = prev_scope;
}

void NameBindingPass::visit_argumentList(Ast* arg_list)
{
  assert(arg_list->kind == AstEnum::argumentList);
  Tree<Ast>* tree;

  for (tree = arg_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_argument(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_argument(Ast* arg)
{
  assert(arg->kind == AstEnum::argument);
  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(arg->argument.arg);
  } else if (arg->argument.arg->kind == AstEnum::dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
}

void NameBindingPass::visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::expressionList);
  Tree<Ast>* tree;

  for (tree = expr_list->tree.first_child;
       tree != 0; tree = tree->right_sibling) {
    visit_expression(Ast::from_tree(tree));
  }
}

void NameBindingPass::visit_lvalueExpression(Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    visit_name(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

void NameBindingPass::visit_expression(Ast* expr)
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
    visit_name(expr->expression.expr);
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

void NameBindingPass::visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr);
}

void NameBindingPass::visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand);
}

void NameBindingPass::visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  visit_expression(binary_expr->binaryExpression.left_operand);
  visit_expression(binary_expr->binaryExpression.right_operand);
}

void NameBindingPass::visit_memberSelector(Ast* selector)
{
  assert(selector->kind == AstEnum::memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr);
  } else assert(0);
  visit_name(selector->memberSelector.name);
}

void NameBindingPass::visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
}

void NameBindingPass::visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  visit_expression(index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index);
  }
}

void NameBindingPass::visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
}

void NameBindingPass::visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
}

void NameBindingPass::visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
}

void NameBindingPass::visit_default(Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
}

void NameBindingPass::visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
}
