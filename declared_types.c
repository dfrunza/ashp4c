#include <stdint.h>
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
    name_entry = scope_lookup(checker->root_scope, base_types[i], NAMESPACE_TYPE);
    name_decl = name_entry->ns[NAMESPACE_TYPE >> 1];
    map_insert(checker->type_env, name_decl->ast, name_decl->type, 0);
  }

  ast = builtin_lookup(checker->root_scope, "accept", NAMESPACE_VAR)->ast;
  ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  ty->ty_former = TYPE_STATE;
  map_insert(checker->type_env, ast, ty, 0);

  ast = builtin_lookup(checker->root_scope, "reject", NAMESPACE_VAR)->ast;
  ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  ty->ty_former = TYPE_STATE;
  map_insert(checker->type_env, ast, ty, 0);

  for (int i = 0; i < sizeof(arithmetic_ops)/sizeof(arithmetic_ops[0]); i++) {
    ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    ty->strname = arithmetic_ops[i];
    ty->ty_former = TYPE_FUNCTION;
    params_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    params_ty->ty_former = TYPE_PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = arena_malloc(checker->storage, params_ty->product.count*sizeof(Type*));
    params_ty->product.members[0] = builtin_lookup(checker->root_scope, "int", NAMESPACE_TYPE)->type;
    params_ty->product.members[1] = builtin_lookup(checker->root_scope, "int", NAMESPACE_TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = builtin_lookup(checker->root_scope, "int", NAMESPACE_TYPE)->type;
    name_decl = scope_bind(checker->storage, checker->root_scope, ty->strname, NAMESPACE_TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(logical_ops)/sizeof(logical_ops[0]); i++) {
    ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    ty->strname = logical_ops[i];
    ty->ty_former = TYPE_FUNCTION;
    params_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    params_ty->ty_former = TYPE_PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = arena_malloc(checker->storage, params_ty->product.count*sizeof(Type*));
    params_ty->product.members[0] = builtin_lookup(checker->root_scope, "bool", NAMESPACE_TYPE)->type;
    params_ty->product.members[1] = builtin_lookup(checker->root_scope, "bool", NAMESPACE_TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = builtin_lookup(checker->root_scope, "bool", NAMESPACE_TYPE)->type;
    name_decl = scope_bind(checker->storage, checker->root_scope, ty->strname, NAMESPACE_TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(relational_ops)/sizeof(relational_ops[0]); i++) {
    ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    ty->strname = relational_ops[i];
    ty->ty_former = TYPE_FUNCTION;
    params_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    params_ty->ty_former = TYPE_PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = arena_malloc(checker->storage, params_ty->product.count*sizeof(Type*));
    params_ty->product.members[0] = builtin_lookup(checker->root_scope, "int", NAMESPACE_TYPE)->type;
    params_ty->product.members[1] = builtin_lookup(checker->root_scope, "int", NAMESPACE_TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = builtin_lookup(checker->root_scope, "bool", NAMESPACE_TYPE)->type;
    name_decl = scope_bind(checker->storage, checker->root_scope, ty->strname, NAMESPACE_TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(bitwise_ops)/sizeof(bitwise_ops[0]); i++) {
    ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    ty->strname = bitwise_ops[i];
    ty->ty_former = TYPE_FUNCTION;
    params_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    params_ty->ty_former = TYPE_PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = arena_malloc(checker->storage, params_ty->product.count*sizeof(Type*));
    params_ty->product.members[0] = builtin_lookup(checker->root_scope, "bit", NAMESPACE_TYPE)->type;
    params_ty->product.members[1] = builtin_lookup(checker->root_scope, "bit", NAMESPACE_TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = builtin_lookup(checker->root_scope, "bit", NAMESPACE_TYPE)->type;
    name_decl = scope_bind(checker->storage, checker->root_scope, ty->strname, NAMESPACE_TYPE);
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

  left = actual_type(left);
  right = actual_type(right);
  if (left == right) return 1;

  for (i = 0; i < checker->type_equiv_pairs->elem_count; i++) {
    type_pair = array_get(checker->type_equiv_pairs, i, sizeof(Type));
    assert(type_pair->ty_former == TYPE_TUPLE);
    if ((left == type_pair->tuple.left || left == type_pair->tuple.right) &&
        (right == type_pair->tuple.left || right == type_pair->tuple.right)) {
      return 1;
    }
  }

  type_pair = array_append(checker->storage, checker->type_equiv_pairs, sizeof(Type));
  type_pair->ty_former = TYPE_TUPLE;
  type_pair->tuple.left = left;
  type_pair->tuple.right = right;

  if (left->ty_former == TYPE_VOID || left->ty_former == TYPE_STRING ||
      left->ty_former == TYPE_BOOL || left->ty_former == TYPE_INT ||
      left->ty_former == TYPE_BIT || left->ty_former == TYPE_VARBIT) {
    if (right->ty_former == left->ty_former) {
      return 1;
    }
    return 0;
  } else if (left->ty_former == TYPE_ANY) {
    return 1;
  } else if (left->ty_former == TYPE_ENUM) {
    if (right->ty_former == left->ty_former) {
      return cstr_match(left->strname, right->strname);
    }
    return 0;
  } else if (left->ty_former == TYPE_EXTERN) {
    if (right->ty_former == left->ty_former) {
      return cstr_match(left->strname, right->strname);
    }
    return 0;
  } else if (left->ty_former == TYPE_TABLE) {
    if (right->ty_former == left->ty_former) {
      return cstr_match(left->strname, right->strname);
    }
    return 0;
  } else if (left->ty_former == TYPE_PRODUCT) {
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
  } else if (left->ty_former == TYPE_FUNCTION) {
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
  } else if (left->ty_former == TYPE_PACKAGE) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->package.params, right->package.params);
    }
    return 0;
  } else if (left->ty_former == TYPE_PARSER) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->parser.params, right->parser.params);
    }
    return 0;
  } else if (left->ty_former == TYPE_CONTROL) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->control.params, right->control.params);
    }
    return 0;
  } else if (left->ty_former == TYPE_STRUCT) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->struct_.fields, right->struct_.fields);
    }
    return 0;
  } else if (left->ty_former == TYPE_HEADER) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(checker, left->struct_.fields, right->struct_.fields);
    }
    return 0;
  } else if (left->ty_former == TYPE_HEADER_STACK) {
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

Type* actual_type(Type* type)
{
  if (!type) { return 0; }
  if (type->ty_former == TYPE_TYPE) {
    return type->type.type;
  }
  return type;
}

Type* effective_type(Type* type)
{
  Type* applied_ty = actual_type(type);
  if (!applied_ty) { return 0; }
  if (type->ty_former == TYPE_FUNCTION) {
    return actual_type(type->function.return_);
  } else if (type->ty_former == TYPE_FIELD) {
    return actual_type(type->field.type);
  } else if (type->ty_former == TYPE_HEADER_STACK) {
    return actual_type(type->header_stack.element);
  }
  return applied_ty;
}

char* TypeEnum_to_string(enum TypeEnum type)
{
  switch(type) {
    case TYPE_NONE: return "TYPE_NONE";
    case TYPE_VOID: return "TYPE_VOID";
    case TYPE_BOOL: return "TYPE_BOOL";
    case TYPE_INT: return "TYPE_INT";
    case TYPE_BIT: return "TYPE_BIT";
    case TYPE_VARBIT: return "TYPE_VARBIT";
    case TYPE_STRING: return "TYPE_STRING";
    case TYPE_ANY: return "TYPE_ANY";
    case TYPE_ENUM: return "TYPE_ENUM";
    case TYPE_TYPEDEF: return "TYPE_TYPEDEF";
    case TYPE_FUNCTION: return "TYPE_FUNCTION";
    case TYPE_EXTERN: return "TYPE_EXTERN";
    case TYPE_PACKAGE: return "TYPE_PACKAGE";
    case TYPE_PARSER: return "TYPE_PARSER";
    case TYPE_CONTROL: return "TYPE_CONTROL";
    case TYPE_TABLE: return "TYPE_TABLE";
    case TYPE_STRUCT: return "TYPE_STRUCT";
    case TYPE_HEADER: return "TYPE_HEADER";
    case TYPE_HEADER_UNION: return "TYPE_HEADER_UNION";
    case TYPE_HEADER_STACK: return "TYPE_HEADER_STACK";
    case TYPE_STATE: return "TYPE_STATE";                            
    case TYPE_FIELD: return "TYPE_FIELD";
    case TYPE_ERROR: return "TYPE_ERROR";
    case TYPE_MATCH_KIND: return "TYPE_MATCH_KIND";
    case TYPE_NAMEREF: return "TYPE_NAMEREF";
    case TYPE_TYPE: return "TYPE_TYPE";
    case TYPE_TUPLE: return "TYPE_TUPLE";
    case TYPE_PRODUCT: return "TYPE_PRODUCT";

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
    ty = array_get(type_array, i, sizeof(Type));
    ty = actual_type(ty);

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

  checker->type_env = arena_malloc(checker->storage, sizeof(Map));
  checker->type_env->storage = checker->storage;
  checker->type_equiv_pairs = array_create(checker->storage, sizeof(Type), 2);

  define_builtin_types(checker);
  visit_p4program(checker, checker->p4program);
  for (int i = 0; i < checker->type_array->elem_count; i++) {
    ty = array_get(checker->type_array, i, sizeof(Type));
    if (ty->ty_former == TYPE_NAMEREF) {
      name = ty->nameref.name;
      name_entry = scope_lookup(ty->nameref.scope, name->name.strname, NAMESPACE_TYPE);
      name_decl = name_entry->ns[NAMESPACE_TYPE >> 1];
      if (name_decl) {
        ref_ty = map_lookup(checker->type_env, name_decl->ast, 0);
        assert(ref_ty);
        name_decl->type = ref_ty;
        ty->ty_former = TYPE_TYPE;
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
    ty = array_get(checker->type_array, i, sizeof(Type));
    if (ty->ty_former == TYPE_TYPEDEF) {
      ref_ty = actual_type(ty->typedef_.ref);
      while (ref_ty->ty_former == TYPE_TYPEDEF) {
        ref_ty = actual_type(ref_ty->typedef_.ref);
      }
      ty->ty_former = TYPE_TYPE;
      ty->type.type = ref_ty;
    }
  }
  for (int i = 0; i < checker->type_array->elem_count; i++) {
    ty = array_get(checker->type_array, i, sizeof(Type));
    if (ty->ty_former == TYPE_TYPE) {
      ref_ty = actual_type(ty->type.type);
      while (ref_ty->ty_former == TYPE_TYPE) {
        ref_ty = actual_type(ref_ty->type.type);
      }
      ty->ty_former = TYPE_TYPE;
      ty->type.type = ref_ty;
    }
  }
}

/** PROGRAM **/

static void visit_p4program(TypeChecker* checker, Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  visit_declarationList(checker, p4program->p4program.decl_list);
}

static void visit_declarationList(TypeChecker* checker, Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_declaration(TypeChecker* checker, Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    visit_externDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    visit_instantiation(checker, decl->declaration.decl);
  } else assert(0);
}

static void visit_name(TypeChecker* checker, Ast* name)
{
  assert(name->kind == AST_name);
  Type* name_ty;

  name_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  name_ty->ty_former = TYPE_NAMEREF;
  name_ty->strname = name->name.strname;
  name_ty->ast = name;
  name_ty->nameref.name = name;
  name_ty->nameref.scope = map_lookup(checker->scope_map, name, 0);
  map_insert(checker->type_env, name, name_ty, 0);
}

static void visit_parameterList(TypeChecker* checker, Ast* params)
{
  assert(params->kind == AST_parameterList);
  AstTree* ast;
  Type* params_ty;
  int i;

  params_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  params_ty->ty_former = TYPE_PRODUCT;
  params_ty->ast = params;
  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(checker, container_of(ast, Ast, tree));
    params_ty->product.count += 1;
  }
  if (params_ty->product.count > 0) {
    params_ty->product.members = arena_malloc(checker->storage, params_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    params_ty->product.members[i] = map_lookup(checker->type_env, container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == params_ty->product.count);
  map_insert(checker->type_env, params, params_ty, 0);
}

static void visit_parameter(TypeChecker* checker, Ast* param)
{
  assert(param->kind == AST_parameter);
  NameDeclaration* name_decl;
  Type* param_ty;

  visit_typeRef(checker, param->parameter.type);
  param_ty = map_lookup(checker->type_env, param->parameter.type, 0);
  map_insert(checker->type_env, param, param_ty, 0);
  if (param->parameter.init_expr) {
    visit_expression(checker, param->parameter.init_expr);
  }
  name_decl = map_lookup(checker->decl_map, param, 0);
  name_decl->type = param_ty;
}

static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* package_ty;

  visit_parameterList(checker, type_decl->packageTypeDeclaration.params);
  name = type_decl->packageTypeDeclaration.name;
  package_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  package_ty->ty_former = TYPE_PACKAGE;
  package_ty->strname = name->name.strname;
  package_ty->ast = type_decl;
  package_ty->package.params = map_lookup(checker->type_env, type_decl->packageTypeDeclaration.params, 0);
  map_insert(checker->type_env, type_decl, package_ty, 0);
  name_decl = map_lookup(checker->decl_map, type_decl, 0);
  name_decl->type = package_ty;
}

static void visit_instantiation(TypeChecker* checker, Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  Type* inst_ty;
  NameDeclaration* name_decl;

  visit_typeRef(checker, inst->instantiation.type);
  visit_argumentList(checker, inst->instantiation.args);
  inst_ty = map_lookup(checker->type_env, inst->instantiation.type, 0);
  map_insert(checker->type_env, inst, inst_ty, 0);
  name_decl = map_lookup(checker->decl_map, inst, 0);
  name_decl->type = inst_ty;
}

/** PARSER **/

static void visit_parserDeclaration(TypeChecker* checker, Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  Type* parser_ty;

  visit_typeDeclaration(checker, parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(checker, parser_decl->parserDeclaration.ctor_params);
    parser_ty = map_lookup(checker->type_env, parser_decl->parserDeclaration.proto, 0);
    parser_ty->parser.ctor_params = map_lookup(checker->type_env, parser_decl->parserDeclaration.ctor_params, 0);
  }
  visit_parserLocalElements(checker, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(checker, parser_decl->parserDeclaration.states);
}

static void visit_parserTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* parser_ty, *methods_ty;

  visit_parameterList(checker, type_decl->parserTypeDeclaration.params);
  name = type_decl->parserTypeDeclaration.name;
  parser_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  parser_ty->ty_former = TYPE_PARSER;
  parser_ty->strname = name->name.strname;
  parser_ty->ast = type_decl;
  parser_ty->parser.params = map_lookup(checker->type_env, type_decl->parserTypeDeclaration.params, 0);
  map_insert(checker->type_env, type_decl, parser_ty, 0);
  visit_methodPrototypes(checker, type_decl->parserTypeDeclaration.method_protos, 0, 0);
  methods_ty = map_lookup(checker->type_env, type_decl->parserTypeDeclaration.method_protos, 0);
  parser_ty->parser.methods = methods_ty;
  name_decl = map_lookup(checker->decl_map, type_decl, 0);
  name_decl->type = parser_ty;
}

static void visit_parserLocalElements(TypeChecker* checker, Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserLocalElement(TypeChecker* checker, Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    visit_instantiation(checker, local_element->parserLocalElement.element);
  } else assert(0);
}

static void visit_parserStates(TypeChecker* checker, Ast* states)
{
  assert(states->kind == AST_parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserState(TypeChecker* checker, Ast* state)
{
  assert(state->kind == AST_parserState);
  Ast* name;
  NameDeclaration* name_decl;
  Type* state_ty;

  name = state->parserState.name;
  state_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  state_ty->ty_former = TYPE_STATE;
  state_ty->strname = name->name.strname;
  state_ty->ast = state;
  visit_parserStatements(checker, state->parserState.stmt_list);
  visit_transitionStatement(checker, state->parserState.transition_stmt);
  map_insert(checker->type_env, state, state_ty, 0);
  name_decl = map_lookup(checker->decl_map, state, 0);
  name_decl->type = state_ty;
}

static void visit_parserStatements(TypeChecker* checker, Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserStatement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visit_functionCall(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
}

static void visit_parserBlockStatement(TypeChecker* checker, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  visit_parserStatements(checker, block_stmt->parserBlockStatement.stmt_list);
}

static void visit_transitionStatement(TypeChecker* checker, Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression(checker, transition_stmt->transitionStatement.stmt);
}

static void visit_stateExpression(TypeChecker* checker, Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    ;
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(checker, state_expr->stateExpression.expr);
  } else assert(0);
}

static void visit_selectExpression(TypeChecker* checker, Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(checker, select_expr->selectExpression.expr_list);
  visit_selectCaseList(checker, select_expr->selectExpression.case_list);
}

static void visit_selectCaseList(TypeChecker* checker, Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(checker, container_of(ast, Ast, tree));
  }
}

static void visit_selectCase(TypeChecker* checker, Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(checker, select_case->selectCase.keyset_expr);
}

static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(checker, keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(checker, keyset_expr->keysetExpression.expr);
  } else assert(0);
}

static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visit_simpleExpressionList(checker, tuple_expr->tupleKeysetExpression.expr_list);
}

static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visit_expression(checker, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visit_default(checker, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visit_dontcare(checker, simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(checker, container_of(ast, Ast, tree));
  }
}

/** CONTROL **/

static void visit_controlDeclaration(TypeChecker* checker, Ast* control_decl) {
  assert(control_decl->kind == AST_controlDeclaration);
  Type* control_ty;

  visit_typeDeclaration(checker, control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(checker, control_decl->controlDeclaration.ctor_params);
    control_ty = map_lookup(checker->type_env, control_decl->controlDeclaration.proto, 0);
    control_ty->control.ctor_params = map_lookup(checker->type_env, control_decl->controlDeclaration.ctor_params, 0);
  }
  visit_controlLocalDeclarations(checker, control_decl->controlDeclaration.local_decls);
  visit_blockStatement(checker, control_decl->controlDeclaration.apply_stmt);
}

static void visit_controlTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* control_ty, *methods_ty;

  visit_parameterList(checker, type_decl->controlTypeDeclaration.params);
  name = type_decl->controlTypeDeclaration.name;
  control_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  control_ty->ty_former = TYPE_CONTROL;
  control_ty->strname = name->name.strname;
  control_ty->ast = type_decl;
  control_ty->control.params = map_lookup(checker->type_env, type_decl->packageTypeDeclaration.params, 0);
  map_insert(checker->type_env, type_decl, control_ty, 0);
  visit_methodPrototypes(checker, type_decl->controlTypeDeclaration.method_protos, 0, 0);
  methods_ty = map_lookup(checker->type_env, type_decl->controlTypeDeclaration.method_protos, 0);
  control_ty->control.methods = methods_ty;
  name_decl = map_lookup(checker->decl_map, type_decl, 0);
  name_decl->type = control_ty;
}

static void visit_controlLocalDeclarations(TypeChecker* checker, Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_controlLocalDeclaration(TypeChecker* checker, Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    visit_instantiation(checker, local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void visit_externDeclaration(TypeChecker* checker, Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration(checker, extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    visit_functionPrototype(checker, extern_decl->externDeclaration.decl, 0, 0);
  } else assert(0);
}

static void visit_externTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* extern_ty, *methods_ty, *ctors_ty;

  name = type_decl->externTypeDeclaration.name;
  extern_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  extern_ty->ty_former = TYPE_EXTERN;
  extern_ty->strname = name->name.strname;
  extern_ty->ast = type_decl;
  map_insert(checker->type_env, type_decl, extern_ty, 0);
  visit_methodPrototypes(checker, type_decl->externTypeDeclaration.method_protos, extern_ty, name->name.strname);
  methods_ty = map_lookup(checker->type_env, type_decl->externTypeDeclaration.method_protos, 0);
  extern_ty->extern_.methods = methods_ty;
  ctors_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  ctors_ty->ty_former = TYPE_PRODUCT;
  ctors_ty->ast = type_decl;
  for (int i = 0; i < methods_ty->product.count; i++) {
    if (cstr_match(methods_ty->product.members[i]->strname, name->name.strname)) {
      ctors_ty->product.count += 1;
    }
  }
  if (ctors_ty->product.count > 0) {
    ctors_ty->product.members = arena_malloc(checker->storage, ctors_ty->product.count*sizeof(Type*));
  }
  for (int i = 0; i < methods_ty->product.count; i++) {
    if (cstr_match(methods_ty->product.members[i]->strname, name->name.strname)) {
      ctors_ty->product.members[i] = methods_ty->product.members[i];
    }
  }
  extern_ty->extern_.ctors = ctors_ty;
  name_decl = map_lookup(checker->decl_map, type_decl, 0);
  name_decl->type = extern_ty;
}

static void visit_methodPrototypes(TypeChecker* checker, Ast* protos, Type* ctor_ty, char* ctor_strname)
{
  assert(protos->kind == AST_methodPrototypes);
  AstTree* ast;
  Type* methods_ty;
  int i;

  methods_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  methods_ty->ty_former = TYPE_PRODUCT;
  methods_ty->ast = protos;
  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(checker, container_of(ast, Ast, tree), ctor_ty, ctor_strname);
    methods_ty->product.count += 1;
  }
  if (methods_ty->product.count > 0) {
    methods_ty->product.members = arena_malloc(checker->storage, methods_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    methods_ty->product.members[i] = map_lookup(checker->type_env, container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == methods_ty->product.count);
  map_insert(checker->type_env, protos, methods_ty, 0);
}

static void visit_functionPrototype(TypeChecker* checker, Ast* func_proto, Type* ctor_ty, char* ctor_strname)
{
  assert(func_proto->kind == AST_functionPrototype);
  Ast* name, *return_type;
  NameDeclaration* name_decl;
  Type* func_ty;

  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(checker, func_proto->functionPrototype.return_type);
  }
  visit_parameterList(checker, func_proto->functionPrototype.params);
  name = func_proto->functionPrototype.name;
  func_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  func_ty->ty_former = TYPE_FUNCTION;
  func_ty->strname = name->name.strname;
  func_ty->ast = func_proto;
  func_ty->function.params = map_lookup(checker->type_env, func_proto->functionPrototype.params, 0);
  map_insert(checker->type_env, func_proto, func_ty, 0);
  return_type = func_proto->functionPrototype.return_type;
  if (return_type) {
    func_ty->function.return_ = map_lookup(checker->type_env, return_type, 0);
  } else if (cstr_match(name->name.strname, ctor_strname)) {
    func_ty->function.return_ = ctor_ty;
  } else assert(0);
  name_decl = map_lookup(checker->decl_map, func_proto, 0);
  name_decl->type = func_ty;
}

/** TYPES **/

static void visit_typeRef(TypeChecker* checker, Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  Type* ref_ty;

  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    visit_baseTypeBit(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    visit_baseTypeString(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    visit_baseTypeError(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    visit_name(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(checker, type_ref->typeRef.type);
  } else assert(0);
  ref_ty = map_lookup(checker->type_env, type_ref->typeRef.type, 0);
  map_insert(checker->type_env, type_ref, ref_ty, 0);
}

static void visit_tupleType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  Type* tuple_ty;

  visit_typeArgumentList(checker, type_decl->tupleType.type_args);
  tuple_ty = map_lookup(checker->type_env, type_decl->tupleType.type_args, 0);
  map_insert(checker->type_env, type_decl, tuple_ty, 0);
}

static void visit_headerStackType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  Type* stack_ty;

  visit_typeRef(checker, type_decl->headerStackType.type);
  visit_expression(checker, type_decl->headerStackType.stack_expr);
  stack_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  stack_ty->ty_former = TYPE_HEADER_STACK;
  stack_ty->ast = type_decl;
  map_insert(checker->type_env, type_decl, stack_ty, 0);
  stack_ty->header_stack.element = map_lookup(checker->type_env, type_decl->headerStackType.type, 0);
}

static void visit_baseTypeBoolean(TypeChecker* checker, Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  NameDeclaration* name_decl;

  name_decl = map_lookup(checker->decl_map, bool_type, 0);
  map_insert(checker->type_env, bool_type, name_decl->type, 0);
}

static void visit_baseTypeInteger(TypeChecker* checker, Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  NameDeclaration* name_decl;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(checker, int_type->baseTypeInteger.size);
  }
  name_decl = map_lookup(checker->decl_map, int_type, 0);
  map_insert(checker->type_env, int_type, name_decl->type, 0);
}

static void visit_baseTypeBit(TypeChecker* checker, Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  NameDeclaration* name_decl;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(checker, bit_type->baseTypeBit.size);
  }
  name_decl = map_lookup(checker->decl_map, bit_type, 0);
  map_insert(checker->type_env, bit_type, name_decl->type, 0);
}

static void visit_baseTypeVarbit(TypeChecker* checker, Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  NameDeclaration* name_decl;

  visit_integerTypeSize(checker, varbit_type->baseTypeVarbit.size);
  name_decl = map_lookup(checker->decl_map, varbit_type, 0);
  map_insert(checker->type_env, varbit_type, name_decl->type, 0);
}

static void visit_baseTypeString(TypeChecker* checker, Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  NameDeclaration* name_decl;

  name_decl = map_lookup(checker->decl_map, str_type, 0);
  map_insert(checker->type_env, str_type, name_decl->type, 0);
}

static void visit_baseTypeVoid(TypeChecker* checker, Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  NameDeclaration* name_decl;

  name_decl = map_lookup(checker->decl_map, void_type, 0);
  map_insert(checker->type_env, void_type, name_decl->type, 0);
}

static void visit_baseTypeError(TypeChecker* checker, Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  NameDeclaration* name_decl;

  name_decl = map_lookup(checker->decl_map, error_type, 0);
  map_insert(checker->type_env, error_type, name_decl->type, 0);
}

static void visit_integerTypeSize(TypeChecker* checker, Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void visit_realTypeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visit_typeRef(checker, type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(checker, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  Type* arg_ty;

  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(checker, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(checker, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(checker, type_arg->typeArg.arg);
  } else assert(0);
  arg_ty = map_lookup(checker->type_env, type_arg->typeArg.arg, 0);
  map_insert(checker->type_env, type_arg, arg_ty, 0);
}

static void visit_typeArgumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AST_typeArgumentList);
  AstTree* ast;
  Type* args_ty;
  int i;

  args_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  args_ty->ty_former = TYPE_PRODUCT;
  args_ty->ast = args;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(checker, container_of(ast, Ast, tree));
    args_ty->product.count += 1;
  }
  if (args_ty->product.count > 0) {
    args_ty->product.members = arena_malloc(checker->storage, args_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    args_ty->product.members[i] = map_lookup(checker->type_env, container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == args_ty->product.count);
  map_insert(checker->type_env, args, args_ty, 0);
}

static void visit_typeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  Type* decl_ty;

  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else assert(0);
  decl_ty = map_lookup(checker->type_env, type_decl->typeDeclaration.decl, 0);
  map_insert(checker->type_env, type_decl, decl_ty, 0);
}

static void visit_derivedTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  Type* decl_ty;

  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  decl_ty = map_lookup(checker->type_env, type_decl->derivedTypeDeclaration.decl, 0);
  map_insert(checker->type_env, type_decl, decl_ty, 0);
}

static void visit_headerTypeDeclaration(TypeChecker* checker, Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* header_ty;

  visit_structFieldList(checker, header_decl->headerTypeDeclaration.fields);
  name = header_decl->headerTypeDeclaration.name;
  header_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  header_ty->ty_former = TYPE_HEADER;
  header_ty->strname = name->name.strname;
  header_ty->ast = header_decl;
  map_insert(checker->type_env, header_decl, header_ty, 0);
  header_ty->struct_.fields = map_lookup(checker->type_env, header_decl->headerTypeDeclaration.fields, 0);
  name_decl = map_lookup(checker->decl_map, header_decl, 0);
  name_decl->type = header_ty;
}

static void visit_headerUnionDeclaration(TypeChecker* checker, Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* union_ty;

  visit_structFieldList(checker, union_decl->headerUnionDeclaration.fields);
  name = union_decl->headerUnionDeclaration.name;
  union_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  union_ty->ty_former = TYPE_HEADER_UNION;
  union_ty->strname = name->name.strname;
  union_ty->ast = union_decl;
  map_insert(checker->type_env, union_decl, union_ty, 0);
  union_ty->struct_.fields = map_lookup(checker->type_env, union_decl->headerUnionDeclaration.fields, 0);
  name_decl = map_lookup(checker->decl_map, union_decl, 0);
  name_decl->type = union_ty;
}

static void visit_structTypeDeclaration(TypeChecker* checker, Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* struct_ty;

  visit_structFieldList(checker, struct_decl->structTypeDeclaration.fields);
  name = struct_decl->structTypeDeclaration.name;
  struct_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  struct_ty->ty_former = TYPE_STRUCT;
  struct_ty->strname = name->name.strname;
  struct_ty->ast = struct_decl;
  map_insert(checker->type_env, struct_decl, struct_ty, 0);
  struct_ty->struct_.fields = map_lookup(checker->type_env, struct_decl->structTypeDeclaration.fields, 0);
  name_decl = map_lookup(checker->decl_map, struct_decl, 0);
  name_decl->type = struct_ty;
}

static void visit_structFieldList(TypeChecker* checker, Ast* fields)
{
  assert(fields->kind == AST_structFieldList);
  AstTree* ast;
  Type* fields_ty;
  int i;

  fields_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  fields_ty->ty_former = TYPE_PRODUCT;
  fields_ty->ast = fields;
  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(checker, container_of(ast, Ast, tree));
    fields_ty->product.count += 1;
  }
  if (fields_ty->product.count > 0) {
    fields_ty->product.members = arena_malloc(checker->storage, fields_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    fields_ty->product.members[i] = map_lookup(checker->type_env, container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == fields_ty->product.count);
  map_insert(checker->type_env, fields, fields_ty, 0);
}

static void visit_structField(TypeChecker* checker, Ast* field)
{
  assert(field->kind == AST_structField);
  Ast* name;
  NameDeclaration* name_decl;
  Type* field_ty;

  visit_typeRef(checker, field->structField.type);
  name = field->structField.name;
  field_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  field_ty->ty_former = TYPE_FIELD;
  field_ty->strname = name->name.strname;
  field_ty->ast = field;
  field_ty->field.type = map_lookup(checker->type_env, field->structField.type, 0);
  map_insert(checker->type_env, field, field_ty, 0);
  name_decl = map_lookup(checker->decl_map, field, 0);
  name_decl->type = field_ty;
}

static void visit_enumDeclaration(TypeChecker* checker, Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* enum_ty;

  name = enum_decl->enumDeclaration.name;
  enum_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  enum_ty->ty_former = TYPE_ENUM;
  enum_ty->strname = name->name.strname;
  enum_ty->ast = enum_decl;
  map_insert(checker->type_env, enum_decl, enum_ty, 0);
  visit_specifiedIdentifierList(checker, enum_decl->enumDeclaration.fields, enum_ty);
  enum_ty->enum_.fields = map_lookup(checker->type_env, enum_decl->enumDeclaration.fields, 0);
  name_decl = map_lookup(checker->decl_map, enum_decl, 0);
  name_decl->type = enum_ty;
}

static void visit_errorDeclaration(TypeChecker* checker, Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  Type* error_ty, *fields_ty;

  error_ty = builtin_lookup(checker->root_scope, "error", NAMESPACE_TYPE)->type;
  fields_ty = error_ty->enum_.fields;
  if (error_ty->enum_.field_count > 0 && fields_ty->product.members == 0) {
    fields_ty->product.count = error_ty->enum_.field_count;
    fields_ty->product.members = arena_malloc(checker->storage, fields_ty->product.count*sizeof(Type*));
  }
  visit_identifierList(checker, error_decl->errorDeclaration.fields, error_ty,
      error_ty->enum_.fields, &error_ty->enum_.i);
}

static void visit_matchKindDeclaration(TypeChecker* checker, Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  Type* match_kind_ty, *fields_ty;

  match_kind_ty = builtin_lookup(checker->root_scope, "match_kind", NAMESPACE_TYPE)->type;
  fields_ty = match_kind_ty->enum_.fields;
  if (match_kind_ty->enum_.field_count > 0 && fields_ty->product.members == 0) {
    fields_ty->product.count = match_kind_ty->enum_.field_count;
    fields_ty->product.members = arena_malloc(checker->storage, fields_ty->product.count*sizeof(Type*));
  }
  visit_identifierList(checker, match_decl->matchKindDeclaration.fields, match_kind_ty,
      match_kind_ty->enum_.fields, &match_kind_ty->enum_.i);
}

static void visit_identifierList(TypeChecker* checker, Ast* ident_list, Type* enum_ty, Type* idents_ty, int* i)
{
  assert(ident_list->kind == AST_identifierList);
  AstTree* ast;
  NameDeclaration* name_decl;
  Type* name_ty;
  int j;

  j = *i;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    name_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
    name_ty->ty_former = TYPE_FIELD;
    name_ty->strname = container_of(ast, Ast, tree)->name.strname;
    name_ty->ast = container_of(ast, Ast, tree);
    name_ty->field.type = enum_ty;
    map_insert(checker->type_env, ast, name_ty, 0);
    name_decl = map_lookup(checker->decl_map, container_of(ast, Ast, tree), 0);
    name_decl->type = name_ty;
    idents_ty->product.members[j] = map_lookup(checker->type_env, container_of(ast, Ast, tree), 0);
    j += 1;
  }
  *i = j;
}

static void visit_specifiedIdentifierList(TypeChecker* checker, Ast* ident_list, Type* enum_ty)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  AstTree* ast;
  Type* idents_ty;
  int i;

  idents_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  idents_ty->ty_former = TYPE_PRODUCT;
  idents_ty->ast = ident_list;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(checker, container_of(ast, Ast, tree), enum_ty);
    idents_ty->product.count += 1;
  }
  if (idents_ty->product.count > 0) {
    idents_ty->product.members = arena_malloc(checker->storage, idents_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    idents_ty->product.members[i] = map_lookup(checker->type_env, container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == idents_ty->product.count);
  map_insert(checker->type_env, ident_list, idents_ty, 0);
}

static void visit_specifiedIdentifier(TypeChecker* checker, Ast* ident, Type* enum_ty)
{
  assert(ident->kind == AST_specifiedIdentifier);
  Ast* name;
  NameDeclaration* name_decl;
  Type* ident_ty;

  name = ident->specifiedIdentifier.name;
  ident_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  ident_ty->ty_former = TYPE_FIELD;
  ident_ty->strname = name->name.strname;
  ident_ty->ast = ident;
  ident_ty->field.type = enum_ty;
  map_insert(checker->type_env, ident, ident_ty, 0);
  name_decl = map_lookup(checker->decl_map, ident, 0);
  name_decl->type = ident_ty;
}

static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* typedef_ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(checker, typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  name = typedef_decl->typedefDeclaration.name;
  typedef_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  typedef_ty->ty_former = TYPE_TYPEDEF;
  typedef_ty->strname = name->name.strname;
  typedef_ty->ast = typedef_decl;
  map_insert(checker->type_env, typedef_decl, typedef_ty, 0);
  typedef_ty->typedef_.ref = map_lookup(checker->type_env, typedef_decl->typedefDeclaration.type_ref, 0);
  name_decl = map_lookup(checker->decl_map, typedef_decl, 0);
  name_decl->type = typedef_ty;
}

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(checker, assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(checker, assign_stmt->assignmentStatement.rhs_expr);
}

static void visit_functionCall(TypeChecker* checker, Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(checker, func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(checker, func_call->functionCall.args);
}

static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(checker, return_stmt->returnStatement.expr);
  }
}

static void visit_exitStatement(TypeChecker* checker, Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
}

static void visit_conditionalStatement(TypeChecker* checker, Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visit_expression(checker, cond_stmt->conditionalStatement.cond_expr);
  visit_statement(checker, cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(checker, cond_stmt->conditionalStatement.else_stmt);
  }
}

static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(checker, applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(checker, applic_stmt->directApplication.args);
}

static void visit_statement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    visit_blockStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    visit_switchStatement(checker, stmt->statement.stmt);
  } else assert(0);
}

static void visit_blockStatement(TypeChecker* checker, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList(checker, block_stmt->blockStatement.stmt_list);
}

static void visit_statementOrDeclList(TypeChecker* checker, Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_switchStatement(TypeChecker* checker, Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(checker, switch_stmt->switchStatement.expr);
  visit_switchCases(checker, switch_stmt->switchStatement.switch_cases);
}

static void visit_switchCases(TypeChecker* checker, Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(checker, container_of(ast, Ast, tree));
  }
}

static void visit_switchCase(TypeChecker* checker, Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel(checker, switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(checker, switch_case->switchCase.stmt);
  }
}

static void visit_switchLabel(TypeChecker* checker, Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    ;
  } else if (label->switchLabel.label->kind == AST_default) {
    visit_default(checker, label->switchLabel.label);
  } else assert(0);
}

static void visit_statementOrDeclaration(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    visit_statement(checker, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    visit_instantiation(checker, stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void visit_tableDeclaration(TypeChecker* checker, Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* table_ty, *methods_ty;

  visit_tablePropertyList(checker, table_decl->tableDeclaration.prop_list);
  name = table_decl->tableDeclaration.name;
  table_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  table_ty->ty_former = TYPE_TABLE;
  table_ty->strname = name->name.strname;
  table_ty->ast = table_decl;
  map_insert(checker->type_env, table_decl, table_ty, 0);
  visit_methodPrototypes(checker, table_decl->tableDeclaration.method_protos, 0, 0);
  methods_ty = map_lookup(checker->type_env, table_decl->tableDeclaration.method_protos, 0);
  table_ty->table.methods = methods_ty;
  name_decl = map_lookup(checker->decl_map, table_decl, 0);
  name_decl->type = table_ty;
}

static void visit_tablePropertyList(TypeChecker* checker, Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(checker, container_of(ast, Ast, tree));
  }
}

static void visit_tableProperty(TypeChecker* checker, Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    visit_keyProperty(checker, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    visit_actionsProperty(checker, table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    visit_entriesProperty(checker, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    visit_simpleProperty(checker, table_prop->tableProperty.prop);
  }
#endif  
  else assert(0);
}

static void visit_keyProperty(TypeChecker* checker, Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList(checker, key_prop->keyProperty.keyelem_list);
}

static void visit_keyElementList(TypeChecker* checker, Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_keyElement(TypeChecker* checker, Ast* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression(checker, element->keyElement.expr);
}

static void visit_actionsProperty(TypeChecker* checker, Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(checker, actions_prop->actionsProperty.action_list);
}

static void visit_actionList(TypeChecker* checker, Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(checker, container_of(ast, Ast, tree));
  }
}

static void visit_actionRef(TypeChecker* checker, Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  if (action_ref->actionRef.args) {
    visit_argumentList(checker, action_ref->actionRef.args);
  }
}

#if 0
static void visit_entriesProperty(TypeChecker* checker, Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList(checker, entries_prop->entriesProperty.entries_list);
}

static void visit_entriesList(TypeChecker* checker, Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(checker, container_of(ast, Ast, tree));
  }
}

static void visit_entry(TypeChecker* checker, Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(checker, entry->entry.keyset);
  visit_actionRef(checker, entry->entry.action);
}

static void visit_simpleProperty(TypeChecker* checker, Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  visit_expression(checker, simple_prop->simpleProperty.init_expr);
}
#endif

static void visit_actionDeclaration(TypeChecker* checker, Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  NameDeclaration* name_decl;
  Ast* name;
  Type* action_ty;

  visit_parameterList(checker, action_decl->actionDeclaration.params);
  visit_blockStatement(checker, action_decl->actionDeclaration.stmt);
  name = action_decl->actionDeclaration.name;
  action_ty = array_append(checker->storage, checker->type_array, sizeof(Type));
  action_ty->ty_former = TYPE_FUNCTION;
  action_ty->strname = name->name.strname;
  action_ty->ast = action_decl;
  action_ty->function.params = map_lookup(checker->type_env, action_decl->actionDeclaration.params, 0);
  map_insert(checker->type_env, action_decl, action_ty, 0);
  action_ty->function.return_ = builtin_lookup(checker->root_scope, "void", NAMESPACE_TYPE)->type;
  name_decl = map_lookup(checker->decl_map, action_decl, 0);
  name_decl->type = action_ty;
}

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  NameDeclaration* name_decl;
  Type* var_ty;

  visit_typeRef(checker, var_decl->variableDeclaration.type);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(checker, var_decl->variableDeclaration.init_expr);
  }
  var_ty = map_lookup(checker->type_env, var_decl->variableDeclaration.type, 0);
  map_insert(checker->type_env, var_decl, var_ty, 0);
  name_decl = map_lookup(checker->decl_map, var_decl, 0);
  name_decl->type = var_ty;
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visit_functionPrototype(checker, func_decl->functionDeclaration.proto, 0, 0);
  visit_blockStatement(checker, func_decl->functionDeclaration.stmt);
}

static void visit_argumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AST_argumentList);
  AstTree* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(checker, container_of(ast, Ast, tree));
  }
}

static void visit_argument(TypeChecker* checker, Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(checker, arg->argument.arg);
  } else assert(0);
}

static void visit_expressionList(TypeChecker* checker, Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(checker, container_of(ast, Ast, tree));
  }
}

static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    ;
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(checker, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(checker, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

static void visit_expression(TypeChecker* checker, Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    ;
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, expr->expression.expr);
  } else assert(0);
}

static void visit_castExpression(TypeChecker* checker, Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef(checker, cast_expr->castExpression.type);
  visit_expression(checker, cast_expr->castExpression.expr);
}

static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(checker, unary_expr->unaryExpression.operand);
}

static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression(checker, binary_expr->binaryExpression.left_operand);
  visit_expression(checker, binary_expr->binaryExpression.right_operand);
}

static void visit_memberSelector(TypeChecker* checker, Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(checker, selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, selector->memberSelector.lhs_expr);
  } else assert(0);
}

static void visit_arraySubscript(TypeChecker* checker, Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(checker, subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(checker, subscript->arraySubscript.index_expr);
}

static void visit_indexExpression(TypeChecker* checker, Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  Type* ty;

  visit_expression(checker, index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(checker, index_expr->indexExpression.end_index);
  }
  ty = builtin_lookup(checker->root_scope, "int", NAMESPACE_TYPE)->type;
  map_insert(checker->type_env, index_expr, ty, 0);
}

static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
  Type* ty;

  ty = builtin_lookup(checker->root_scope, "bool", NAMESPACE_TYPE)->type;
  map_insert(checker->type_env, bool_literal, ty, 0);
}

static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
  Type* ty;

  ty = builtin_lookup(checker->root_scope, "int", NAMESPACE_TYPE)->type;
  map_insert(checker->type_env, int_literal, ty, 0);
}

static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
  Type* ty;

  ty = builtin_lookup(checker->root_scope, "string", NAMESPACE_TYPE)->type;
  map_insert(checker->type_env, str_literal, ty, 0);
}

static void visit_default(TypeChecker* checker, Ast* default_)
{
  assert(default_->kind == AST_default);
  Type* ty;

  ty = builtin_lookup(checker->root_scope, "_", NAMESPACE_TYPE)->type;
  map_insert(checker->type_env, default_, ty, 0);
}

static void visit_dontcare(TypeChecker* checker, Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
  Type* ty;

  ty = builtin_lookup(checker->root_scope, "_", NAMESPACE_TYPE)->type;
  map_insert(checker->type_env, dontcare, ty, 0);
}

