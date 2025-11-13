#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

/** PROGRAM **/

static void visit_p4program(TypeChecker* checker, Ast* p4program);
static void visit_declarationList(TypeChecker* checker, Ast* decl_list);
static void visit_declaration(TypeChecker* checker, Ast* decl);
static void visit_name(TypeChecker* checker, Ast* name);
static void visit_parameterList(TypeChecker* checker, Ast* params);
static void visit_parameter(TypeChecker* checker, Ast* param);
static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* package_decl);
static void visit_instantiation(TypeChecker* checker, Ast* inst);

/** PARSER **/

static void visit_parserDeclaration(TypeChecker* checker, Ast* parser_decl);
static void visit_parserTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_parserLocalElements(TypeChecker* checker, Ast* local_elements);
static void visit_parserLocalElement(TypeChecker* checker, Ast* local_element);
static void visit_parserStates(TypeChecker* checker, Ast* states);
static void visit_parserState(TypeChecker* checker, Ast* state);
static void visit_parserStatements(TypeChecker* checker, Ast* stmts);
static void visit_parserStatement(TypeChecker* checker, Ast* stmt);
static void visit_parserBlockStatement(TypeChecker* checker, Ast* block_stmt);
static void visit_transitionStatement(TypeChecker* checker, Ast* transition_stmt);
static void visit_stateExpression(TypeChecker* checker, Ast* state_expr);
static void visit_selectExpression(TypeChecker* checker, Ast* select_expr);
static void visit_selectCaseList(TypeChecker* checker, Ast* case_list);
static void visit_selectCase(TypeChecker* checker, Ast* select_case);
static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr);
static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr);
static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr);
static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list);

/** CONTROL **/

static void visit_controlDeclaration(TypeChecker* checker, Ast* control_decl);
static void visit_controlTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_controlLocalDeclarations(TypeChecker* checker, Ast* local_decls);
static void visit_controlLocalDeclaration(TypeChecker* checker, Ast* local_decl);

/** EXTERN **/

static void visit_externDeclaration(TypeChecker* checker, Ast* extern_decl);
static void visit_externTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_methodPrototypes(TypeChecker* checker, Ast* protos, Type* ctor_ty, char* ctor_strname);
static void visit_functionPrototype(TypeChecker* checker, Ast* func_proto, Type* ctor_ty, char* ctor_strname);

/** TYPES **/

static void visit_typeRef(TypeChecker* checker, Ast* type_ref);
static void visit_tupleType(TypeChecker* checker, Ast* type);
static void visit_headerStackType(TypeChecker* checker, Ast* type_decl);
static void visit_baseTypeBoolean(TypeChecker* checker, Ast* bool_type);
static void visit_baseTypeInteger(TypeChecker* checker, Ast* int_type);
static void visit_baseTypeBit(TypeChecker* checker, Ast* bit_type);
static void visit_baseTypeVarbit(TypeChecker* checker, Ast* varbit_type);
static void visit_baseTypeString(TypeChecker* checker, Ast* str_type);
static void visit_baseTypeVoid(TypeChecker* checker, Ast* void_type);
static void visit_baseTypeError(TypeChecker* checker, Ast* error_type);
static void visit_integerTypeSize(TypeChecker* checker, Ast* type_size);
static void visit_realTypeArg(TypeChecker* checker, Ast* type_arg);
static void visit_typeArg(TypeChecker* checker, Ast* type_arg);
static void visit_typeArgumentList(TypeChecker* checker, Ast* args);
static void visit_typeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_derivedTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_headerTypeDeclaration(TypeChecker* checker, Ast* header_decl);
static void visit_headerUnionDeclaration(TypeChecker* checker, Ast* union_decl);
static void visit_structTypeDeclaration(TypeChecker* checker, Ast* struct_decl);
static void visit_structFieldList(TypeChecker* checker, Ast* fields);
static void visit_structField(TypeChecker* checker, Ast* field);
static void visit_enumDeclaration(TypeChecker* checker, Ast* enum_decl);
static void visit_errorDeclaration(TypeChecker* checker, Ast* error_decl);
static void visit_matchKindDeclaration(TypeChecker* checker, Ast* match_decl);
static void visit_identifierList(TypeChecker* checker, Ast* ident_list, Type* enum_ty, Type* idents_ty, int* i);
static void visit_specifiedIdentifierList(TypeChecker* checker, Ast* ident_list, Type* enum_ty);
static void visit_specifiedIdentifier(TypeChecker* checker, Ast* ident, Type* enum_ty);
static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl);

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt);
static void visit_functionCall(TypeChecker* checker, Ast* func_call);
static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt);
static void visit_exitStatement(TypeChecker* checker, Ast* exit_stmt);
static void visit_conditionalStatement(TypeChecker* checker, Ast* cond_stmt);
static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt);
static void visit_statement(TypeChecker* checker, Ast* stmt);
static void visit_blockStatement(TypeChecker* checker, Ast* block_stmt);
static void visit_statementOrDeclList(TypeChecker* checker, Ast* stmt_list);
static void visit_switchStatement(TypeChecker* checker, Ast* switch_stmt);
static void visit_switchCases(TypeChecker* checker, Ast* switch_cases);
static void visit_switchCase(TypeChecker* checker, Ast* switch_case);
static void visit_switchLabel(TypeChecker* checker, Ast* label);
static void visit_statementOrDeclaration(TypeChecker* checker, Ast* stmt);

/** TABLES **/

static void visit_tableDeclaration(TypeChecker* checker, Ast* table_decl);
static void visit_tablePropertyList(TypeChecker* checker, Ast* prop_list);
static void visit_tableProperty(TypeChecker* checker, Ast* table_prop);
static void visit_keyProperty(TypeChecker* checker, Ast* key_prop);
static void visit_keyElementList(TypeChecker* checker, Ast* element_list);
static void visit_keyElement(TypeChecker* checker, Ast* element);
static void visit_actionsProperty(TypeChecker* checker, Ast* actions_prop);
static void visit_actionList(TypeChecker* checker, Ast* action_list);
static void visit_actionRef(TypeChecker* checker, Ast* action_ref);
static void visit_entriesProperty(TypeChecker* checker, Ast* entries_prop);
static void visit_entriesList(TypeChecker* checker, Ast* entries_list);
static void visit_entry(TypeChecker* checker, Ast* entry);
static void visit_simpleProperty(TypeChecker* checker, Ast* simple_prop);
static void visit_actionDeclaration(TypeChecker* checker, Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl);
static void visit_argumentList(TypeChecker* checker, Ast* args);
static void visit_argument(TypeChecker* checker, Ast* arg);
static void visit_expressionList(TypeChecker* checker, Ast* expr_list);
static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr);
static void visit_expression(TypeChecker* checker, Ast* expr);
static void visit_castExpression(TypeChecker* checker, Ast* cast_expr);
static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr);
static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr);
static void visit_memberSelector(TypeChecker* checker, Ast* selector);
static void visit_arraySubscript(TypeChecker* checker, Ast* subscript);
static void visit_indexExpression(TypeChecker* checker, Ast* index_expr);
static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal);
static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal);
static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal);
static void visit_default(TypeChecker* checker, Ast* default_);
static void visit_dontcare(TypeChecker* checker, Ast* dontcare);

static void define_builtin_types(TypeChecker* checker)
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
    name_entry = checker->root_scope->lookup(base_types[i], NameSpace::TYPE);
    name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
    checker->type_env->insert(name_decl->ast, name_decl->type, 0);
  }

  ast = checker->root_scope->builtin_lookup("accept", NameSpace::VAR)->ast;
  ty = (Type*)checker->type_array->append(sizeof(Type));
  ty->ty_former = TypeEnum::STATE;
  checker->type_env->insert(ast, ty, 0);

  ast = checker->root_scope->builtin_lookup("reject", NameSpace::VAR)->ast;
  ty = (Type*)checker->type_array->append(sizeof(Type));
  ty->ty_former = TypeEnum::STATE;
  checker->type_env->insert(ast, ty, 0);

  for (int i = 0; i < sizeof(arithmetic_ops)/sizeof(arithmetic_ops[0]); i++) {
    ty = (Type*)checker->type_array->append(sizeof(Type));
    ty->strname = arithmetic_ops[i];
    ty->ty_former = TypeEnum::FUNCTION;
    params_ty = (Type*)checker->type_array->append(sizeof(Type));
    params_ty->ty_former = TypeEnum::PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = (Type**)checker->storage->malloc(params_ty->product.count * sizeof(Type*));
    params_ty->product.members[0] = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    params_ty->product.members[1] = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    name_decl = checker->root_scope->bind(checker->storage, ty->strname, NameSpace::TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(logical_ops)/sizeof(logical_ops[0]); i++) {
    ty = (Type*)checker->type_array->append(sizeof(Type));
    ty->strname = logical_ops[i];
    ty->ty_former = TypeEnum::FUNCTION;
    params_ty = (Type*)checker->type_array->append(sizeof(Type));
    params_ty->ty_former = TypeEnum::PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = (Type**)checker->storage->malloc(params_ty->product.count * sizeof(Type*));
    params_ty->product.members[0] = checker->root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
    params_ty->product.members[1] = checker->root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = checker->root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
    name_decl = checker->root_scope->bind(checker->storage, ty->strname, NameSpace::TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(relational_ops)/sizeof(relational_ops[0]); i++) {
    ty = (Type*)checker->type_array->append(sizeof(Type));
    ty->strname = relational_ops[i];
    ty->ty_former = TypeEnum::FUNCTION;
    params_ty = (Type*)checker->type_array->append(sizeof(Type));
    params_ty->ty_former = TypeEnum::PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = (Type**)checker->storage->malloc(params_ty->product.count * sizeof(Type*));
    params_ty->product.members[0] = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    params_ty->product.members[1] = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = checker->root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
    name_decl = checker->root_scope->bind(checker->storage, ty->strname, NameSpace::TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(bitwise_ops)/sizeof(bitwise_ops[0]); i++) {
    ty = (Type*)checker->type_array->append(sizeof(Type));
    ty->strname = bitwise_ops[i];
    ty->ty_former = TypeEnum::FUNCTION;
    params_ty = (Type*)checker->type_array->append(sizeof(Type));
    params_ty->ty_former = TypeEnum::PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = (Type**)checker->storage->malloc(params_ty->product.count * sizeof(Type*));
    params_ty->product.members[0] = checker->root_scope->builtin_lookup("bit", NameSpace::TYPE)->type;
    params_ty->product.members[1] = checker->root_scope->builtin_lookup("bit", NameSpace::TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = checker->root_scope->builtin_lookup("bit", NameSpace::TYPE)->type;
    name_decl = checker->root_scope->bind(checker->storage, ty->strname, NameSpace::TYPE);
    name_decl->type = ty;
  }
}

static bool structural_type_equiv(TypeChecker* checker, Type* left, Type* right)
{
  Type* type_pair;
  int i;

  if (left == 0 && right == 0) {
    return 1;
  } else if (left == 0 || right == 0) {
    return 0;
  }

  left = left->actual_type();
  right = right->actual_type();
  if (left == right) return 1;

  for (i = 0; i < checker->type_equiv_pairs->elem_count; i++) {
    type_pair = (Type*)checker->type_equiv_pairs->get(i, sizeof(Type));
    assert(type_pair->ty_former == TypeEnum::TUPLE);
    if ((left == type_pair->tuple.left || left == type_pair->tuple.right) &&
        (right == type_pair->tuple.left || right == type_pair->tuple.right)) {
      return 1;
    }
  }

  type_pair = (Type*)checker->type_equiv_pairs->append(sizeof(Type));
  type_pair->ty_former = TypeEnum::TUPLE;
  type_pair->tuple.left = left;
  type_pair->tuple.right = right;

  if (left->ty_former == TypeEnum::VOID || left->ty_former == TypeEnum::STRING ||
      left->ty_former == TypeEnum::BOOL || left->ty_former == TypeEnum::INT ||
      left->ty_former == TypeEnum::BIT || left->ty_former == TypeEnum::VARBIT) {
    if (right->ty_former == left->ty_former) {
      return 1;
    }
    return 0;
  } else if (left->ty_former == TypeEnum::ANY) {
    return 1;
  } else if (left->ty_former == TypeEnum::ENUM) {
    if (right->ty_former == left->ty_former) {
      return cstr_match(left->strname, right->strname);
    }
    return 0;
  } else if (left->ty_former == TypeEnum::EXTERN) {
    if (right->ty_former == left->ty_former) {
      return cstr_match(left->strname, right->strname);
    }
    return 0;
  } else if (left->ty_former == TypeEnum::TABLE) {
    if (right->ty_former == left->ty_former) {
      return cstr_match(left->strname, right->strname);
    }
    return 0;
  } else if (left->ty_former == TypeEnum::PRODUCT) {
    if (right->ty_former == left->ty_former) {
      if (left->product.count != right->product.count) {
        return 0;
      }
      for (int i = 0; i < left->product.count; i++) {
        if (!structural_type_equiv(checker, left->product.members[i], left->product.members[i])) {
          return 0;
        }
      }
      return 1;
    }
    return 0;
  } else if (left->ty_former == TypeEnum::FUNCTION) {
    if (right->ty_former == left->ty_former) {
      if (!structural_type_equiv(checker, left->function.return_, right->function.return_)) {
        return 0;
      }
      if (!structural_type_equiv(checker, left->function.params, right->function.params)) {
        return 0;
      }
      return 1;
    }
    return 0;
  } else if (left->ty_former == TypeEnum::PACKAGE) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->package.params, right->package.params);
    }
    return 0;
  } else if (left->ty_former == TypeEnum::PARSER) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->parser.params, right->parser.params);
    }
    return 0;
  } else if (left->ty_former == TypeEnum::CONTROL) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->control.params, right->control.params);
    }
    return 0;
  } else if (left->ty_former == TypeEnum::STRUCT) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->struct_.fields, right->struct_.fields);
    }
    return 0;
  } else if (left->ty_former == TypeEnum::HEADER) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->struct_.fields, right->struct_.fields);
    }
    return 0;
  } else if (left->ty_former == TypeEnum::STACK) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->header_stack.element, right->header_stack.element);
    }
    return 0;
  } else assert(0);

  assert(0);
  return 0;
}

bool type_equiv(TypeChecker* checker, Type* left, Type* right)
{
  checker->type_equiv_pairs->elem_count = 0;
  return structural_type_equiv(checker, left, right);
}

Type* Type::actual_type()
{
  if (!this) { return 0; }
  if (this->ty_former == TypeEnum::TYPE) {
    return this->type.type;
  }
  return this;
}

Type* Type::effective_type()
{
  Type* applied_ty;

  applied_ty = this->actual_type();
  if (!applied_ty) { return 0; }
  if (this->ty_former == TypeEnum::FUNCTION) {
    return this->function.return_->actual_type();
  } else if (this->ty_former == TypeEnum::FIELD) {
    return this->field.type->actual_type();
  } else if (this->ty_former == TypeEnum::STACK) {
    return this->header_stack.element->actual_type();
  }
  return applied_ty;
}

char* TypeEnum_to_string(enum TypeEnum type)
{
  switch(type) {
    case TypeEnum::NONE: return "NONE";
    case TypeEnum::VOID: return "VOID";
    case TypeEnum::BOOL: return "BOOL";
    case TypeEnum::INT: return "INT";
    case TypeEnum::BIT: return "BIT";
    case TypeEnum::VARBIT: return "VARBIT";
    case TypeEnum::STRING: return "STRING";
    case TypeEnum::ANY: return "ANY";
    case TypeEnum::ENUM: return "ENUM";
    case TypeEnum::TYPEDEF: return "TYPEDEF";
    case TypeEnum::FUNCTION: return "FUNCTION";
    case TypeEnum::EXTERN: return "EXTERN";
    case TypeEnum::PACKAGE: return "PACKAGE";
    case TypeEnum::PARSER: return "PARSER";
    case TypeEnum::CONTROL: return "CONTROL";
    case TypeEnum::TABLE: return "TABLE";
    case TypeEnum::STRUCT: return "STRUCT";
    case TypeEnum::HEADER: return "HEADER";
    case TypeEnum::UNION: return "UNION";
    case TypeEnum::STACK: return "STACK";
    case TypeEnum::STATE: return "STATE";
    case TypeEnum::FIELD: return "FIELD";
    case TypeEnum::ERROR: return "ERROR";
    case TypeEnum::MATCH_KIND: return "MATCH_KIND";
    case TypeEnum::NAMEREF: return "NAMEREF";
    case TypeEnum::TYPE: return "TYPE";
    case TypeEnum::TUPLE: return "TUPLE";
    case TypeEnum::PRODUCT: return "TYPE_PRODUCT";

    default: return "?";
  }
  assert(0);
  return 0;
}

void Debug_print_type_env(Map* env)
{
  Ast* ast;
  MapEntry* m;
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

void Debug_print_type_array(Array* type_array)
{
  Type* ty;
  int i;

  for (i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)type_array->get(i, sizeof(Type));
    ty = ty->actual_type();

    if (ty->strname) {
      printf("[%d] 0x%x %s %s\n", i, ty, ty->strname, TypeEnum_to_string(ty->ty_former));
    } else {
      printf("[%d] 0x%x %s\n", i, ty, TypeEnum_to_string(ty->ty_former));
    }
  }
}

void declared_types(TypeChecker* checker)
{
  Ast* name;
  Type* ref_ty, *ty;
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  checker->type_env = (Map*)checker->storage->malloc(sizeof(Map));
  checker->type_env->storage = checker->storage;
  checker->type_equiv_pairs = Array::create(checker->storage, sizeof(Type), 2);

  define_builtin_types(checker);
  visit_p4program(checker, checker->p4program);
  for (int i = 0; i < checker->type_array->elem_count; i++) {
    ty = (Type*)checker->type_array->get(i, sizeof(Type));
    if (ty->ty_former == TypeEnum::NAMEREF) {
      name = ty->nameref.name;
      name_entry = ty->nameref.scope->lookup(name->name.strname, NameSpace::TYPE);
      name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
      if (name_decl) {
        ref_ty = (Type*)checker->type_env->lookup(name_decl->ast, 0);
        assert(ref_ty);
        name_decl->type = ref_ty;
        ty->ty_former = TypeEnum::TYPE;
        ty->type.type = ref_ty;
        if (name_decl->next_in_scope) {
          error("%s:%d:%d: error: ambiguous type reference `%s`.",
                checker->source_file, name->line_no, name->column_no, name->name.strname);
        }
      } else error("%s:%d:%d: error: unresolved type reference `%s`.",
                   checker->source_file, name->line_no, name->column_no, name->name.strname);
    }
  }
  for (int i = 0; i < checker->type_array->elem_count; i++) {
    ty = (Type*)checker->type_array->get(i, sizeof(Type));
    if (ty->ty_former == TypeEnum::TYPEDEF) {
      ref_ty = ty->typedef_.ref->actual_type();
      while (ref_ty->ty_former == TypeEnum::TYPEDEF) {
        ref_ty = ref_ty->typedef_.ref->actual_type();
      }
      ty->ty_former = TypeEnum::TYPE;
      ty->type.type = ref_ty;
    }
  }
  for (int i = 0; i < checker->type_array->elem_count; i++) {
    ty = (Type*)checker->type_array->get(i, sizeof(Type));
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

static void visit_p4program(TypeChecker* checker, Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  visit_declarationList(checker, p4program->p4program.decl_list);
}

static void visit_declarationList(TypeChecker* checker, Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_declaration(TypeChecker* checker, Ast* decl)
{
  assert(decl->kind == AstEnum::declaration);
  if (decl->declaration.decl->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::externDeclaration) {
    visit_externDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::actionDeclaration) {
    visit_actionDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::functionDeclaration) {
    visit_functionDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserDeclaration) {
    visit_parserDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlDeclaration) {
    visit_controlDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::typeDeclaration) {
    visit_typeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::errorDeclaration) {
    visit_errorDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::matchKindDeclaration) {
    visit_matchKindDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::instantiation) {
    visit_instantiation(checker, decl->declaration.decl);
  } else assert(0);
}

static void visit_name(TypeChecker* checker, Ast* name)
{
  assert(name->kind == AstEnum::name);
  Type* name_ty;

  name_ty = (Type*)checker->type_array->append(sizeof(Type));
  name_ty->ty_former = TypeEnum::NAMEREF;
  name_ty->strname = name->name.strname;
  name_ty->ast = name;
  name_ty->nameref.name = name;
  name_ty->nameref.scope = (Scope*)checker->scope_map->lookup(name, 0);
  checker->type_env->insert(name, name_ty, 0);
}

static void visit_parameterList(TypeChecker* checker, Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  AstTree* ast;
  Type* params_ty;
  int i;

  params_ty = (Type*)checker->type_array->append(sizeof(Type));
  params_ty->ty_former = TypeEnum::PRODUCT;
  params_ty->ast = params;
  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(checker, container_of(ast, Ast, tree));
    params_ty->product.count += 1;
  }
  if (params_ty->product.count > 0) {
    params_ty->product.members = (Type**)checker->storage->malloc(params_ty->product.count * sizeof(Type*));
  }
  i = 0;
  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    params_ty->product.members[i] = (Type*)checker->type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == params_ty->product.count);
  checker->type_env->insert(params, params_ty, 0);
}

static void visit_parameter(TypeChecker* checker, Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  NameDeclaration* name_decl;
  Type* param_ty;

  visit_typeRef(checker, param->parameter.type);
  param_ty = (Type*)checker->type_env->lookup(param->parameter.type, 0);
  checker->type_env->insert(param, param_ty, 0);
  if (param->parameter.init_expr) {
    visit_expression(checker, param->parameter.init_expr);
  }
  name_decl = (NameDeclaration*)checker->decl_map->lookup(param, 0);
  name_decl->type = param_ty;
}

static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* package_ty;

  visit_parameterList(checker, type_decl->packageTypeDeclaration.params);
  name = type_decl->packageTypeDeclaration.name;
  package_ty = (Type*)checker->type_array->append(sizeof(Type));
  package_ty->ty_former = TypeEnum::PACKAGE;
  package_ty->strname = name->name.strname;
  package_ty->ast = type_decl;
  package_ty->package.params = (Type*)checker->type_env->lookup(type_decl->packageTypeDeclaration.params, 0);
  checker->type_env->insert(type_decl, package_ty, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(type_decl, 0);
  name_decl->type = package_ty;
}

static void visit_instantiation(TypeChecker* checker, Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  Type* inst_ty;
  NameDeclaration* name_decl;

  visit_typeRef(checker, inst->instantiation.type);
  visit_argumentList(checker, inst->instantiation.args);
  inst_ty = (Type*)checker->type_env->lookup(inst->instantiation.type, 0);
  checker->type_env->insert(inst, inst_ty, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(inst, 0);
  name_decl->type = inst_ty;
}

/** PARSER **/

static void visit_parserDeclaration(TypeChecker* checker, Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  Type* parser_ty;

  visit_typeDeclaration(checker, parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(checker, parser_decl->parserDeclaration.ctor_params);
    parser_ty = (Type*)checker->type_env->lookup(parser_decl->parserDeclaration.proto, 0);
    parser_ty->parser.ctor_params = (Type*)checker->type_env->lookup(parser_decl->parserDeclaration.ctor_params, 0);
  }
  visit_parserLocalElements(checker, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(checker, parser_decl->parserDeclaration.states);
}

static void visit_parserTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* parser_ty, *methods_ty;

  visit_parameterList(checker, type_decl->parserTypeDeclaration.params);
  name = type_decl->parserTypeDeclaration.name;
  parser_ty = (Type*)checker->type_array->append(sizeof(Type));
  parser_ty->ty_former = TypeEnum::PARSER;
  parser_ty->strname = name->name.strname;
  parser_ty->ast = type_decl;
  parser_ty->parser.params = (Type*)checker->type_env->lookup(type_decl->parserTypeDeclaration.params, 0);
  checker->type_env->insert(type_decl, parser_ty, 0);
  visit_methodPrototypes(checker, type_decl->parserTypeDeclaration.method_protos, 0, 0);
  methods_ty = (Type*)checker->type_env->lookup(type_decl->parserTypeDeclaration.method_protos, 0);
  parser_ty->parser.methods = methods_ty;
  name_decl = (NameDeclaration*)checker->decl_map->lookup(type_decl, 0);
  name_decl->type = parser_ty;
}

static void visit_parserLocalElements(TypeChecker* checker, Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserLocalElement(TypeChecker* checker, Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    visit_instantiation(checker, local_element->parserLocalElement.element);
  } else assert(0);
}

static void visit_parserStates(TypeChecker* checker, Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserState(TypeChecker* checker, Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  Ast* name;
  NameDeclaration* name_decl;
  Type* state_ty;

  name = state->parserState.name;
  state_ty = (Type*)checker->type_array->append(sizeof(Type));
  state_ty->ty_former = TypeEnum::STATE;
  state_ty->strname = name->name.strname;
  state_ty->ast = state;
  visit_parserStatements(checker, state->parserState.stmt_list);
  visit_transitionStatement(checker, state->parserState.transition_stmt);
  checker->type_env->insert(state, state_ty, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(state, 0);
  name_decl->type = state_ty;
}

static void visit_parserStatements(TypeChecker* checker, Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserStatement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AstEnum::parserStatement);
  if (stmt->parserStatement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::parserBlockStatement) {
    visit_parserBlockStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else assert(0);
}

static void visit_parserBlockStatement(TypeChecker* checker, Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  visit_parserStatements(checker, block_stmt->parserBlockStatement.stmt_list);
}

static void visit_transitionStatement(TypeChecker* checker, Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  visit_stateExpression(checker, transition_stmt->transitionStatement.stmt);
}

static void visit_stateExpression(TypeChecker* checker, Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    ;
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(checker, state_expr->stateExpression.expr);
  } else assert(0);
}

static void visit_selectExpression(TypeChecker* checker, Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  visit_expressionList(checker, select_expr->selectExpression.expr_list);
  visit_selectCaseList(checker, select_expr->selectExpression.case_list);
}

static void visit_selectCaseList(TypeChecker* checker, Ast* case_list)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(checker, container_of(ast, Ast, tree));
  }
}

static void visit_selectCase(TypeChecker* checker, Ast* select_case)
{
  assert(select_case->kind == AstEnum::selectCase);
  visit_keysetExpression(checker, select_case->selectCase.keyset_expr);
}

static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(checker, keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(checker, keyset_expr->keysetExpression.expr);
  } else assert(0);
}

static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  visit_simpleExpressionList(checker, tuple_expr->tupleKeysetExpression.expr_list);
}

static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
    visit_expression(checker, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
    visit_default(checker, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
    visit_dontcare(checker, simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(checker, container_of(ast, Ast, tree));
  }
}

/** CONTROL **/

static void visit_controlDeclaration(TypeChecker* checker, Ast* control_decl) {
  assert(control_decl->kind == AstEnum::controlDeclaration);
  Type* control_ty;

  visit_typeDeclaration(checker, control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(checker, control_decl->controlDeclaration.ctor_params);
    control_ty = (Type*)checker->type_env->lookup(control_decl->controlDeclaration.proto, 0);
    control_ty->control.ctor_params = (Type*)checker->type_env->lookup(control_decl->controlDeclaration.ctor_params, 0);
  }
  visit_controlLocalDeclarations(checker, control_decl->controlDeclaration.local_decls);
  visit_blockStatement(checker, control_decl->controlDeclaration.apply_stmt);
}

static void visit_controlTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* control_ty, *methods_ty;

  visit_parameterList(checker, type_decl->controlTypeDeclaration.params);
  name = type_decl->controlTypeDeclaration.name;
  control_ty = (Type*)checker->type_array->append(sizeof(Type));
  control_ty->ty_former = TypeEnum::CONTROL;
  control_ty->strname = name->name.strname;
  control_ty->ast = type_decl;
  control_ty->control.params = (Type*)checker->type_env->lookup(type_decl->packageTypeDeclaration.params, 0);
  checker->type_env->insert(type_decl, control_ty, 0);
  visit_methodPrototypes(checker, type_decl->controlTypeDeclaration.method_protos, 0, 0);
  methods_ty = (Type*)checker->type_env->lookup(type_decl->controlTypeDeclaration.method_protos, 0);
  control_ty->control.methods = methods_ty;
  name_decl = (NameDeclaration*)checker->decl_map->lookup(type_decl, 0);
  name_decl->type = control_ty;
}

static void visit_controlLocalDeclarations(TypeChecker* checker, Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_controlLocalDeclaration(TypeChecker* checker, Ast* local_decl)
{
  assert(local_decl->kind == AstEnum::controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::actionDeclaration) {
    visit_actionDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::tableDeclaration) {
    visit_tableDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::instantiation) {
    visit_instantiation(checker, local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void visit_externDeclaration(TypeChecker* checker, Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    visit_externTypeDeclaration(checker, extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    visit_functionPrototype(checker, extern_decl->externDeclaration.decl, 0, 0);
  } else assert(0);
}

static void visit_externTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* extern_ty, *methods_ty, *ctors_ty;

  name = type_decl->externTypeDeclaration.name;
  extern_ty = (Type*)checker->type_array->append(sizeof(Type));
  extern_ty->ty_former = TypeEnum::EXTERN;
  extern_ty->strname = name->name.strname;
  extern_ty->ast = type_decl;
  checker->type_env->insert(type_decl, extern_ty, 0);
  visit_methodPrototypes(checker, type_decl->externTypeDeclaration.method_protos, extern_ty, name->name.strname);
  methods_ty = (Type*)checker->type_env->lookup(type_decl->externTypeDeclaration.method_protos, 0);
  extern_ty->extern_.methods = methods_ty;
  ctors_ty = (Type*)checker->type_array->append(sizeof(Type));
  ctors_ty->ty_former = TypeEnum::PRODUCT;
  ctors_ty->ast = type_decl;
  for (int i = 0; i < methods_ty->product.count; i++) {
    if (cstr_match(methods_ty->product.members[i]->strname, name->name.strname)) {
      ctors_ty->product.count += 1;
    }
  }
  if (ctors_ty->product.count > 0) {
    ctors_ty->product.members = (Type**)checker->storage->malloc(ctors_ty->product.count * sizeof(Type*));
  }
  for (int i = 0; i < methods_ty->product.count; i++) {
    if (cstr_match(methods_ty->product.members[i]->strname, name->name.strname)) {
      ctors_ty->product.members[i] = methods_ty->product.members[i];
    }
  }
  extern_ty->extern_.ctors = ctors_ty;
  name_decl = (NameDeclaration*)checker->decl_map->lookup(type_decl, 0);
  name_decl->type = extern_ty;
}

static void visit_methodPrototypes(TypeChecker* checker, Ast* protos, Type* ctor_ty, char* ctor_strname)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  AstTree* ast;
  Type* methods_ty;
  int i;

  methods_ty = (Type*)checker->type_array->append(sizeof(Type));
  methods_ty->ty_former = TypeEnum::PRODUCT;
  methods_ty->ast = protos;
  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(checker, container_of(ast, Ast, tree), ctor_ty, ctor_strname);
    methods_ty->product.count += 1;
  }
  if (methods_ty->product.count > 0) {
    methods_ty->product.members = (Type**)checker->storage->malloc(methods_ty->product.count * sizeof(Type*));
  }
  i = 0;
  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    methods_ty->product.members[i] = (Type*)checker->type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == methods_ty->product.count);
  checker->type_env->insert(protos, methods_ty, 0);
}

static void visit_functionPrototype(TypeChecker* checker, Ast* func_proto, Type* ctor_ty, char* ctor_strname)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
  Ast* name, *return_type;
  NameDeclaration* name_decl;
  Type* func_ty;

  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(checker, func_proto->functionPrototype.return_type);
  }
  visit_parameterList(checker, func_proto->functionPrototype.params);
  name = func_proto->functionPrototype.name;
  func_ty = (Type*)checker->type_array->append(sizeof(Type));
  func_ty->ty_former = TypeEnum::FUNCTION;
  func_ty->strname = name->name.strname;
  func_ty->ast = func_proto;
  func_ty->function.params = (Type*)checker->type_env->lookup(func_proto->functionPrototype.params, 0);
  checker->type_env->insert(func_proto, func_ty, 0);
  return_type = func_proto->functionPrototype.return_type;
  if (return_type) {
    func_ty->function.return_ = (Type*)checker->type_env->lookup(return_type, 0);
  } else if (cstr_match(name->name.strname, ctor_strname)) {
    func_ty->function.return_ = ctor_ty;
  } else assert(0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(func_proto, 0);
  name_decl->type = func_ty;
}

/** TYPES **/

static void visit_typeRef(TypeChecker* checker, Ast* type_ref)
{
  assert(type_ref->kind == AstEnum::typeRef);
  Type* ref_ty;

  if (type_ref->typeRef.type->kind == AstEnum::baseTypeBoolean) {
    visit_baseTypeBoolean(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeInteger) {
    visit_baseTypeInteger(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeBit) {
    visit_baseTypeBit(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVarbit) {
    visit_baseTypeVarbit(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeString) {
    visit_baseTypeString(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVoid) {
    visit_baseTypeVoid(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeError) {
    visit_baseTypeError(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::name) {
    visit_name(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::headerStackType) {
    visit_headerStackType(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::tupleType) {
    visit_tupleType(checker, type_ref->typeRef.type);
  } else assert(0);
  ref_ty = (Type*)checker->type_env->lookup(type_ref->typeRef.type, 0);
  checker->type_env->insert(type_ref, ref_ty, 0);
}

static void visit_tupleType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  Type* tuple_ty;

  visit_typeArgumentList(checker, type_decl->tupleType.type_args);
  tuple_ty = (Type*)checker->type_env->lookup(type_decl->tupleType.type_args, 0);
  checker->type_env->insert(type_decl, tuple_ty, 0);
}

static void visit_headerStackType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  Type* stack_ty;

  visit_typeRef(checker, type_decl->headerStackType.type);
  visit_expression(checker, type_decl->headerStackType.stack_expr);
  stack_ty = (Type*)checker->type_array->append(sizeof(Type));
  stack_ty->ty_former = TypeEnum::STACK;
  stack_ty->ast = type_decl;
  checker->type_env->insert(type_decl, stack_ty, 0);
  stack_ty->header_stack.element = (Type*)checker->type_env->lookup(type_decl->headerStackType.type, 0);
}

static void visit_baseTypeBoolean(TypeChecker* checker, Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  NameDeclaration* name_decl;

  name_decl = (NameDeclaration*)checker->decl_map->lookup(bool_type, 0);
  checker->type_env->insert(bool_type, name_decl->type, 0);
}

static void visit_baseTypeInteger(TypeChecker* checker, Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  NameDeclaration* name_decl;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(checker, int_type->baseTypeInteger.size);
  }
  name_decl = (NameDeclaration*)checker->decl_map->lookup(int_type, 0);
  checker->type_env->insert(int_type, name_decl->type, 0);
}

static void visit_baseTypeBit(TypeChecker* checker, Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  NameDeclaration* name_decl;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(checker, bit_type->baseTypeBit.size);
  }
  name_decl = (NameDeclaration*)checker->decl_map->lookup(bit_type, 0);
  checker->type_env->insert(bit_type, name_decl->type, 0);
}

static void visit_baseTypeVarbit(TypeChecker* checker, Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  NameDeclaration* name_decl;

  visit_integerTypeSize(checker, varbit_type->baseTypeVarbit.size);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(varbit_type, 0);
  checker->type_env->insert(varbit_type, name_decl->type, 0);
}

static void visit_baseTypeString(TypeChecker* checker, Ast* str_type)
{
  assert(str_type->kind == AstEnum::baseTypeString);
  NameDeclaration* name_decl;

  name_decl = (NameDeclaration*)checker->decl_map->lookup(str_type, 0);
  checker->type_env->insert(str_type, name_decl->type, 0);
}

static void visit_baseTypeVoid(TypeChecker* checker, Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  NameDeclaration* name_decl;

  name_decl = (NameDeclaration*)checker->decl_map->lookup(void_type, 0);
  checker->type_env->insert(void_type, name_decl->type, 0);
}

static void visit_baseTypeError(TypeChecker* checker, Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  NameDeclaration* name_decl;

  name_decl = (NameDeclaration*)checker->decl_map->lookup(error_type, 0);
  checker->type_env->insert(error_type, name_decl->type, 0);
}

static void visit_integerTypeSize(TypeChecker* checker, Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
}

static void visit_realTypeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(checker, type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(checker, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  Type* arg_ty;

  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(checker, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::name) {
    visit_name(checker, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(checker, type_arg->typeArg.arg);
  } else assert(0);
  arg_ty = (Type*)checker->type_env->lookup(type_arg->typeArg.arg, 0);
  checker->type_env->insert(type_arg, arg_ty, 0);
}

static void visit_typeArgumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AstEnum::typeArgumentList);
  AstTree* ast;
  Type* args_ty;
  int i;

  args_ty = (Type*)checker->type_array->append(sizeof(Type));
  args_ty->ty_former = TypeEnum::PRODUCT;
  args_ty->ast = args;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(checker, container_of(ast, Ast, tree));
    args_ty->product.count += 1;
  }
  if (args_ty->product.count > 0) {
    args_ty->product.members = (Type**)checker->storage->malloc(args_ty->product.count * sizeof(Type*));
  }
  i = 0;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    args_ty->product.members[i] = (Type*)checker->type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == args_ty->product.count);
  checker->type_env->insert(args, args_ty, 0);
}

static void visit_typeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::typeDeclaration);
  Type* decl_ty;

  if (type_decl->typeDeclaration.decl->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::typedefDeclaration) {
    visit_typedefDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::packageTypeDeclaration) {
    visit_packageTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else assert(0);
  decl_ty = (Type*)checker->type_env->lookup(type_decl->typeDeclaration.decl, 0);
  checker->type_env->insert(type_decl, decl_ty, 0);
}

static void visit_derivedTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::derivedTypeDeclaration);
  Type* decl_ty;

  if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerTypeDeclaration) {
    visit_headerTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerUnionDeclaration) {
    visit_headerUnionDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::structTypeDeclaration) {
    visit_structTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::enumDeclaration) {
    visit_enumDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  decl_ty = (Type*)checker->type_env->lookup(type_decl->derivedTypeDeclaration.decl, 0);
  checker->type_env->insert(type_decl, decl_ty, 0);
}

static void visit_headerTypeDeclaration(TypeChecker* checker, Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* header_ty;

  visit_structFieldList(checker, header_decl->headerTypeDeclaration.fields);
  name = header_decl->headerTypeDeclaration.name;
  header_ty = (Type*)checker->type_array->append(sizeof(Type));
  header_ty->ty_former = TypeEnum::HEADER;
  header_ty->strname = name->name.strname;
  header_ty->ast = header_decl;
  checker->type_env->insert(header_decl, header_ty, 0);
  header_ty->struct_.fields = (Type*)checker->type_env->lookup(header_decl->headerTypeDeclaration.fields, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(header_decl, 0);
  name_decl->type = header_ty;
}

static void visit_headerUnionDeclaration(TypeChecker* checker, Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* union_ty;

  visit_structFieldList(checker, union_decl->headerUnionDeclaration.fields);
  name = union_decl->headerUnionDeclaration.name;
  union_ty = (Type*)checker->type_array->append(sizeof(Type));
  union_ty->ty_former = TypeEnum::UNION;
  union_ty->strname = name->name.strname;
  union_ty->ast = union_decl;
  checker->type_env->insert(union_decl, union_ty, 0);
  union_ty->struct_.fields = (Type*)checker->type_env->lookup(union_decl->headerUnionDeclaration.fields, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(union_decl, 0);
  name_decl->type = union_ty;
}

static void visit_structTypeDeclaration(TypeChecker* checker, Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* struct_ty;

  visit_structFieldList(checker, struct_decl->structTypeDeclaration.fields);
  name = struct_decl->structTypeDeclaration.name;
  struct_ty = (Type*)checker->type_array->append(sizeof(Type));
  struct_ty->ty_former = TypeEnum::STRUCT;
  struct_ty->strname = name->name.strname;
  struct_ty->ast = struct_decl;
  checker->type_env->insert(struct_decl, struct_ty, 0);
  struct_ty->struct_.fields = (Type*)checker->type_env->lookup(struct_decl->structTypeDeclaration.fields, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(struct_decl, 0);
  name_decl->type = struct_ty;
}

static void visit_structFieldList(TypeChecker* checker, Ast* fields)
{
  assert(fields->kind == AstEnum::structFieldList);
  AstTree* ast;
  Type* fields_ty;
  int i;

  fields_ty = (Type*)checker->type_array->append(sizeof(Type));
  fields_ty->ty_former = TypeEnum::PRODUCT;
  fields_ty->ast = fields;
  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(checker, container_of(ast, Ast, tree));
    fields_ty->product.count += 1;
  }
  if (fields_ty->product.count > 0) {
    fields_ty->product.members = (Type**)checker->storage->malloc(fields_ty->product.count * sizeof(Type*));
  }
  i = 0;
  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    fields_ty->product.members[i] = (Type*)checker->type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == fields_ty->product.count);
  checker->type_env->insert(fields, fields_ty, 0);
}

static void visit_structField(TypeChecker* checker, Ast* field)
{
  assert(field->kind == AstEnum::structField);
  Ast* name;
  NameDeclaration* name_decl;
  Type* field_ty;

  visit_typeRef(checker, field->structField.type);
  name = field->structField.name;
  field_ty = (Type*)checker->type_array->append(sizeof(Type));
  field_ty->ty_former = TypeEnum::FIELD;
  field_ty->strname = name->name.strname;
  field_ty->ast = field;
  field_ty->field.type = (Type*)checker->type_env->lookup(field->structField.type, 0);
  checker->type_env->insert(field, field_ty, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(field, 0);
  name_decl->type = field_ty;
}

static void visit_enumDeclaration(TypeChecker* checker, Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* enum_ty;

  name = enum_decl->enumDeclaration.name;
  enum_ty = (Type*)checker->type_array->append(sizeof(Type));
  enum_ty->ty_former = TypeEnum::ENUM;
  enum_ty->strname = name->name.strname;
  enum_ty->ast = enum_decl;
  checker->type_env->insert(enum_decl, enum_ty, 0);
  visit_specifiedIdentifierList(checker, enum_decl->enumDeclaration.fields, enum_ty);
  enum_ty->enum_.fields = (Type*)checker->type_env->lookup(enum_decl->enumDeclaration.fields, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(enum_decl, 0);
  name_decl->type = enum_ty;
}

static void visit_errorDeclaration(TypeChecker* checker, Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
  Type* error_ty, *fields_ty;

  error_ty = checker->root_scope->builtin_lookup("error", NameSpace::TYPE)->type;
  fields_ty = error_ty->enum_.fields;
  if (error_ty->enum_.field_count > 0 && fields_ty->product.members == 0) {
    fields_ty->product.count = error_ty->enum_.field_count;
    fields_ty->product.members = (Type**)checker->storage->malloc(fields_ty->product.count * sizeof(Type*));
  }
  visit_identifierList(checker, error_decl->errorDeclaration.fields, error_ty,
      error_ty->enum_.fields, &error_ty->enum_.i);
}

static void visit_matchKindDeclaration(TypeChecker* checker, Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
  Type* match_kind_ty, *fields_ty;

  match_kind_ty = checker->root_scope->builtin_lookup("match_kind", NameSpace::TYPE)->type;
  fields_ty = match_kind_ty->enum_.fields;
  if (match_kind_ty->enum_.field_count > 0 && fields_ty->product.members == 0) {
    fields_ty->product.count = match_kind_ty->enum_.field_count;
    fields_ty->product.members = (Type**)checker->storage->malloc(fields_ty->product.count * sizeof(Type*));
  }
  visit_identifierList(checker, match_decl->matchKindDeclaration.fields, match_kind_ty,
      match_kind_ty->enum_.fields, &match_kind_ty->enum_.i);
}

static void visit_identifierList(TypeChecker* checker, Ast* ident_list, Type* enum_ty, Type* idents_ty, int* i)
{
  assert(ident_list->kind == AstEnum::identifierList);
  AstTree* ast;
  NameDeclaration* name_decl;
  Type* name_ty;
  int j;

  j = *i;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    name_ty = (Type*)checker->type_array->append(sizeof(Type));
    name_ty->ty_former = TypeEnum::FIELD;
    name_ty->strname = container_of(ast, Ast, tree)->name.strname;
    name_ty->ast = container_of(ast, Ast, tree);
    name_ty->field.type = enum_ty;
    checker->type_env->insert(ast, name_ty, 0);
    name_decl = (NameDeclaration*)checker->decl_map->lookup(container_of(ast, Ast, tree), 0);
    name_decl->type = name_ty;
    idents_ty->product.members[j] = (Type*)checker->type_env->lookup(container_of(ast, Ast, tree), 0);
    j += 1;
  }
  *i = j;
}

static void visit_specifiedIdentifierList(TypeChecker* checker, Ast* ident_list, Type* enum_ty)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  AstTree* ast;
  Type* idents_ty;
  int i;

  idents_ty = (Type*)checker->type_array->append(sizeof(Type));
  idents_ty->ty_former = TypeEnum::PRODUCT;
  idents_ty->ast = ident_list;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(checker, container_of(ast, Ast, tree), enum_ty);
    idents_ty->product.count += 1;
  }
  if (idents_ty->product.count > 0) {
    idents_ty->product.members = (Type**)checker->storage->malloc(idents_ty->product.count * sizeof(Type*));
  }
  i = 0;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    idents_ty->product.members[i] = (Type*)checker->type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == idents_ty->product.count);
  checker->type_env->insert(ident_list, idents_ty, 0);
}

static void visit_specifiedIdentifier(TypeChecker* checker, Ast* ident, Type* enum_ty)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  Ast* name;
  NameDeclaration* name_decl;
  Type* ident_ty;

  name = ident->specifiedIdentifier.name;
  ident_ty = (Type*)checker->type_array->append(sizeof(Type));
  ident_ty->ty_former = TypeEnum::FIELD;
  ident_ty->strname = name->name.strname;
  ident_ty->ast = ident;
  ident_ty->field.type = enum_ty;
  checker->type_env->insert(ident, ident_ty, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(ident, 0);
  name_decl->type = ident_ty;
}

static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* typedef_ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(checker, typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  name = typedef_decl->typedefDeclaration.name;
  typedef_ty = (Type*)checker->type_array->append(sizeof(Type));
  typedef_ty->ty_former = TypeEnum::TYPEDEF;
  typedef_ty->strname = name->name.strname;
  typedef_ty->ast = typedef_decl;
  checker->type_env->insert(typedef_decl, typedef_ty, 0);
  typedef_ty->typedef_.ref = (Type*)checker->type_env->lookup(typedef_decl->typedefDeclaration.type_ref, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(typedef_decl, 0);
  name_decl->type = typedef_ty;
}

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(checker, assign_stmt->assignmentStatement.rhs_expr);
}

static void visit_functionCall(TypeChecker* checker, Ast* func_call)
{
  assert(func_call->kind == AstEnum::functionCall);
  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(checker, func_call->functionCall.args);
}

static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(checker, return_stmt->returnStatement.expr);
  }
}

static void visit_exitStatement(TypeChecker* checker, Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
}

static void visit_conditionalStatement(TypeChecker* checker, Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  visit_expression(checker, cond_stmt->conditionalStatement.cond_expr);
  visit_statement(checker, cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(checker, cond_stmt->conditionalStatement.else_stmt);
  }
}

static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(checker, applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(checker, applic_stmt->directApplication.args);
}

static void visit_statement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AstEnum::statement);
  if (stmt->statement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::conditionalStatement) {
    visit_conditionalStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AstEnum::blockStatement) {
    visit_blockStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::exitStatement) {
    visit_exitStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::returnStatement) {
    visit_returnStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::switchStatement) {
    visit_switchStatement(checker, stmt->statement.stmt);
  } else assert(0);
}

static void visit_blockStatement(TypeChecker* checker, Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  visit_statementOrDeclList(checker, block_stmt->blockStatement.stmt_list);
}

static void visit_statementOrDeclList(TypeChecker* checker, Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_switchStatement(TypeChecker* checker, Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  visit_expression(checker, switch_stmt->switchStatement.expr);
  visit_switchCases(checker, switch_stmt->switchStatement.switch_cases);
}

static void visit_switchCases(TypeChecker* checker, Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(checker, container_of(ast, Ast, tree));
  }
}

static void visit_switchCase(TypeChecker* checker, Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  visit_switchLabel(checker, switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(checker, switch_case->switchCase.stmt);
  }
}

static void visit_switchLabel(TypeChecker* checker, Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    ;
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    visit_default(checker, label->switchLabel.label);
  } else assert(0);
}

static void visit_statementOrDeclaration(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AstEnum::statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::statement) {
    visit_statement(checker, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::instantiation) {
    visit_instantiation(checker, stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void visit_tableDeclaration(TypeChecker* checker, Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* table_ty, *methods_ty;

  visit_tablePropertyList(checker, table_decl->tableDeclaration.prop_list);
  name = table_decl->tableDeclaration.name;
  table_ty = (Type*)checker->type_array->append(sizeof(Type));
  table_ty->ty_former = TypeEnum::TABLE;
  table_ty->strname = name->name.strname;
  table_ty->ast = table_decl;
  checker->type_env->insert(table_decl, table_ty, 0);
  visit_methodPrototypes(checker, table_decl->tableDeclaration.method_protos, 0, 0);
  methods_ty = (Type*)checker->type_env->lookup(table_decl->tableDeclaration.method_protos, 0);
  table_ty->table.methods = methods_ty;
  name_decl = (NameDeclaration*)checker->decl_map->lookup(table_decl, 0);
  name_decl->type = table_ty;
}

static void visit_tablePropertyList(TypeChecker* checker, Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(checker, container_of(ast, Ast, tree));
  }
}

static void visit_tableProperty(TypeChecker* checker, Ast* table_prop)
{
  assert(table_prop->kind == AstEnum::tableProperty);
  if (table_prop->tableProperty.prop->kind == AstEnum::keyProperty) {
    visit_keyProperty(checker, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::actionsProperty) {
    visit_actionsProperty(checker, table_prop->tableProperty.prop);
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

static void visit_keyProperty(TypeChecker* checker, Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  visit_keyElementList(checker, key_prop->keyProperty.keyelem_list);
}

static void visit_keyElementList(TypeChecker* checker, Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_keyElement(TypeChecker* checker, Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  visit_expression(checker, element->keyElement.expr);
}

static void visit_actionsProperty(TypeChecker* checker, Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  visit_actionList(checker, actions_prop->actionsProperty.action_list);
}

static void visit_actionList(TypeChecker* checker, Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(checker, container_of(ast, Ast, tree));
  }
}

static void visit_actionRef(TypeChecker* checker, Ast* action_ref)
{
  assert(action_ref->kind == AstEnum::actionRef);
  if (action_ref->actionRef.args) {
    visit_argumentList(checker, action_ref->actionRef.args);
  }
}

#if 0
static void visit_entriesProperty(TypeChecker* checker, Ast* entries_prop)
{
  assert(entries_prop->kind == AstEnum::entriesProperty);
  visit_entriesList(checker, entries_prop->entriesProperty.entries_list);
}

static void visit_entriesList(TypeChecker* checker, Ast* entries_list)
{
  assert(entries_list->kind == AstEnum::entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(checker, container_of(ast, Ast, tree));
  }
}

static void visit_entry(TypeChecker* checker, Ast* entry)
{
  assert(entry->kind == AstEnum::entry);
  visit_keysetExpression(checker, entry->entry.keyset);
  visit_actionRef(checker, entry->entry.action);
}

static void visit_simpleProperty(TypeChecker* checker, Ast* simple_prop)
{
  assert(simple_prop->kind == AstEnum::simpleProperty);
  visit_expression(checker, simple_prop->simpleProperty.init_expr);
}
#endif

static void visit_actionDeclaration(TypeChecker* checker, Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  NameDeclaration* name_decl;
  Ast* name;
  Type* action_ty;

  visit_parameterList(checker, action_decl->actionDeclaration.params);
  visit_blockStatement(checker, action_decl->actionDeclaration.stmt);
  name = action_decl->actionDeclaration.name;
  action_ty = (Type*)checker->type_array->append(sizeof(Type));
  action_ty->ty_former = TypeEnum::FUNCTION;
  action_ty->strname = name->name.strname;
  action_ty->ast = action_decl;
  action_ty->function.params = (Type*)checker->type_env->lookup(action_decl->actionDeclaration.params, 0);
  checker->type_env->insert(action_decl, action_ty, 0);
  action_ty->function.return_ = checker->root_scope->builtin_lookup("void", NameSpace::TYPE)->type;
  name_decl = (NameDeclaration*)checker->decl_map->lookup(action_decl, 0);
  name_decl->type = action_ty;
}

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  NameDeclaration* name_decl;
  Type* var_ty;

  visit_typeRef(checker, var_decl->variableDeclaration.type);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(checker, var_decl->variableDeclaration.init_expr);
  }
  var_ty = (Type*)checker->type_env->lookup(var_decl->variableDeclaration.type, 0);
  checker->type_env->insert(var_decl, var_ty, 0);
  name_decl = (NameDeclaration*)checker->decl_map->lookup(var_decl, 0);
  name_decl->type = var_ty;
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  visit_functionPrototype(checker, func_decl->functionDeclaration.proto, 0, 0);
  visit_blockStatement(checker, func_decl->functionDeclaration.stmt);
}

static void visit_argumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AstEnum::argumentList);
  AstTree* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(checker, container_of(ast, Ast, tree));
  }
}

static void visit_argument(TypeChecker* checker, Ast* arg)
{
  assert(arg->kind == AstEnum::argument);
  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(checker, arg->argument.arg);
  } else assert(0);
}

static void visit_expressionList(TypeChecker* checker, Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::expressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(checker, container_of(ast, Ast, tree));
  }
}

static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    ;
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(checker, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(checker, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

static void visit_expression(TypeChecker* checker, Ast* expr)
{
  assert(expr->kind == AstEnum::expression);
  if (expr->expression.expr->kind == AstEnum::expression) {
    visit_expression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::booleanLiteral) {
    visit_booleanLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::integerLiteral) {
    visit_integerLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::stringLiteral) {
    visit_stringLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::name) {
    ;
  } else if (expr->expression.expr->kind == AstEnum::expressionList) {
    visit_expressionList(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::castExpression) {
    visit_castExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::unaryExpression) {
    visit_unaryExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::binaryExpression) {
    visit_binaryExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::functionCall) {
    visit_functionCall(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(checker, expr->expression.expr);
  } else assert(0);
}

static void visit_castExpression(TypeChecker* checker, Ast* cast_expr)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  visit_typeRef(checker, cast_expr->castExpression.type);
  visit_expression(checker, cast_expr->castExpression.expr);
}

static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(checker, unary_expr->unaryExpression.operand);
}

static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  visit_expression(checker, binary_expr->binaryExpression.left_operand);
  visit_expression(checker, binary_expr->binaryExpression.right_operand);
}

static void visit_memberSelector(TypeChecker* checker, Ast* selector)
{
  assert(selector->kind == AstEnum::memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, selector->memberSelector.lhs_expr);
  } else assert(0);
}

static void visit_arraySubscript(TypeChecker* checker, Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(checker, subscript->arraySubscript.index_expr);
}

static void visit_indexExpression(TypeChecker* checker, Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  Type* ty;

  visit_expression(checker, index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(checker, index_expr->indexExpression.end_index);
  }
  ty = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
  checker->type_env->insert(index_expr, ty, 0);
}

static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
  Type* ty;

  ty = checker->root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
  checker->type_env->insert(bool_literal, ty, 0);
}

static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
  Type* ty;

  ty = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
  checker->type_env->insert(int_literal, ty, 0);
}

static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
  Type* ty;

  ty = checker->root_scope->builtin_lookup("string", NameSpace::TYPE)->type;
  checker->type_env->insert(str_literal, ty, 0);
}

static void visit_default(TypeChecker* checker, Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
  Type* ty;

  ty = checker->root_scope->builtin_lookup("_", NameSpace::TYPE)->type;
  checker->type_env->insert(default_, ty, 0);
}

static void visit_dontcare(TypeChecker* checker, Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
  Type* ty;

  ty = checker->root_scope->builtin_lookup("_", NameSpace::TYPE)->type;
  checker->type_env->insert(dontcare, ty, 0);
}

