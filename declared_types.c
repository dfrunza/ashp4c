#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static char*  source_file;
static Arena* storage;
static Scope* root_scope;
static Map*   type_env, *scope_map, *decl_map;
static Array* type_array;
static Array* type_equiv_pairs;

/** PROGRAM **/

static void visit_p4program(Ast* p4program);
static void visit_declarationList(Ast* decl_list);
static void visit_declaration(Ast* decl);
static void visit_name(Ast* name);
static void visit_parameterList(Ast* params);
static void visit_parameter(Ast* param);
static void visit_packageTypeDeclaration(Ast* package_decl);
static void visit_instantiation(Ast* inst);

/** PARSER **/

static void visit_parserDeclaration(Ast* parser_decl);
static void visit_parserTypeDeclaration(Ast* type_decl);
static void visit_parserLocalElements(Ast* local_elements);
static void visit_parserLocalElement(Ast* local_element);
static void visit_parserStates(Ast* states);
static void visit_parserState(Ast* state);
static void visit_parserStatements(Ast* stmts);
static void visit_parserStatement(Ast* stmt);
static void visit_parserBlockStatement(Ast* block_stmt);
static void visit_transitionStatement(Ast* transition_stmt);
static void visit_stateExpression(Ast* state_expr);
static void visit_selectExpression(Ast* select_expr);
static void visit_selectCaseList(Ast* case_list);
static void visit_selectCase(Ast* select_case);
static void visit_keysetExpression(Ast* keyset_expr);
static void visit_tupleKeysetExpression(Ast* tuple_expr);
static void visit_simpleKeysetExpression(Ast* simple_expr);
static void visit_simpleExpressionList(Ast* expr_list);

/** CONTROL **/

static void visit_controlDeclaration(Ast* control_decl);
static void visit_controlTypeDeclaration(Ast* type_decl);
static void visit_controlLocalDeclarations(Ast* local_decls);
static void visit_controlLocalDeclaration(Ast* local_decl);

/** EXTERN **/

static void visit_externDeclaration(Ast* extern_decl);
static void visit_externTypeDeclaration(Ast* type_decl);
static void visit_methodPrototypes(Ast* protos, Type* ctor_ty, char* ctor_strname);
static void visit_functionPrototype(Ast* func_proto, Type* ctor_ty, char* ctor_strname);

/** TYPES **/

static void visit_typeRef(Ast* type_ref);
static void visit_tupleType(Ast* type);
static void visit_headerStackType(Ast* type_decl);
static void visit_baseTypeBoolean(Ast* bool_type);
static void visit_baseTypeInteger(Ast* int_type);
static void visit_baseTypeBit(Ast* bit_type);
static void visit_baseTypeVarbit(Ast* varbit_type);
static void visit_baseTypeString(Ast* str_type);
static void visit_baseTypeVoid(Ast* void_type);
static void visit_baseTypeError(Ast* error_type);
static void visit_integerTypeSize(Ast* type_size);
static void visit_realTypeArg(Ast* type_arg);
static void visit_typeArg(Ast* type_arg);
static void visit_typeArgumentList(Ast* args);
static void visit_typeDeclaration(Ast* type_decl);
static void visit_derivedTypeDeclaration(Ast* type_decl);
static void visit_headerTypeDeclaration(Ast* header_decl);
static void visit_headerUnionDeclaration(Ast* union_decl);
static void visit_structTypeDeclaration(Ast* struct_decl);
static void visit_structFieldList(Ast* fields);
static void visit_structField(Ast* field);
static void visit_enumDeclaration(Ast* enum_decl);
static void visit_errorDeclaration(Ast* error_decl);
static void visit_matchKindDeclaration(Ast* match_decl);
static void visit_identifierList(Ast* ident_list, Type* enum_ty, Type* idents_ty, int* i);
static void visit_specifiedIdentifierList(Ast* ident_list, Type* enum_ty);
static void visit_specifiedIdentifier(Ast* ident, Type* enum_ty);
static void visit_typedefDeclaration(Ast* typedef_decl);

/** STATEMENTS **/

static void visit_assignmentStatement(Ast* assign_stmt);
static void visit_functionCall(Ast* func_call);
static void visit_returnStatement(Ast* return_stmt);
static void visit_exitStatement(Ast* exit_stmt);
static void visit_conditionalStatement(Ast* cond_stmt);
static void visit_directApplication(Ast* applic_stmt);
static void visit_statement(Ast* stmt);
static void visit_blockStatement(Ast* block_stmt);
static void visit_statementOrDeclList(Ast* stmt_list);
static void visit_switchStatement(Ast* switch_stmt);
static void visit_switchCases(Ast* switch_cases);
static void visit_switchCase(Ast* switch_case);
static void visit_switchLabel(Ast* label);
static void visit_statementOrDeclaration(Ast* stmt);

/** TABLES **/

static void visit_tableDeclaration(Ast* table_decl);
static void visit_tablePropertyList(Ast* prop_list);
static void visit_tableProperty(Ast* table_prop);
static void visit_keyProperty(Ast* key_prop);
static void visit_keyElementList(Ast* element_list);
static void visit_keyElement(Ast* element);
static void visit_actionsProperty(Ast* actions_prop);
static void visit_actionList(Ast* action_list);
static void visit_actionRef(Ast* action_ref);
static void visit_entriesProperty(Ast* entries_prop);
static void visit_entriesList(Ast* entries_list);
static void visit_entry(Ast* entry);
static void visit_simpleProperty(Ast* simple_prop);
static void visit_actionDeclaration(Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(Ast* func_decl);
static void visit_argumentList(Ast* args);
static void visit_argument(Ast* arg);
static void visit_expressionList(Ast* expr_list);
static void visit_lvalueExpression(Ast* lvalue_expr);
static void visit_expression(Ast* expr);
static void visit_castExpression(Ast* cast_expr);
static void visit_unaryExpression(Ast* unary_expr);
static void visit_binaryExpression(Ast* binary_expr);
static void visit_memberSelector(Ast* selector);
static void visit_arraySubscript(Ast* subscript);
static void visit_indexExpression(Ast* index_expr);
static void visit_booleanLiteral(Ast* bool_literal);
static void visit_integerLiteral(Ast* int_literal);
static void visit_stringLiteral(Ast* str_literal);
static void visit_default(Ast* default_);
static void visit_dontcare(Ast* dontcare);

static void
setup_builtin_types()
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
    name_entry = scope_lookup(root_scope, base_types[i], NAMESPACE_TYPE);
    name_decl = name_entry->ns[NAMESPACE_TYPE >> 1];
    map_insert(storage, type_env, name_decl->ast, name_decl->type, 0);
  }

  ast = builtin_lookup(root_scope, "accept", NAMESPACE_VAR)->ast;
  ty = array_append(storage, type_array, sizeof(Type));
  ty->ty_former = TYPE_STATE;
  map_insert(storage, type_env, ast, ty, 0);

  ast = builtin_lookup(root_scope, "reject", NAMESPACE_VAR)->ast;
  ty = array_append(storage, type_array, sizeof(Type));
  ty->ty_former = TYPE_STATE;
  map_insert(storage, type_env, ast, ty, 0);

  for (int i = 0; i < sizeof(arithmetic_ops)/sizeof(arithmetic_ops[0]); i++) {
    ty = array_append(storage, type_array, sizeof(Type));
    ty->strname = arithmetic_ops[i];
    ty->ty_former = TYPE_FUNCTION;
    params_ty = array_append(storage, type_array, sizeof(Type));
    params_ty->ty_former = TYPE_PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = arena_malloc(storage, params_ty->product.count*sizeof(Type*));
    params_ty->product.members[0] = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
    params_ty->product.members[1] = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
    name_decl = scope_bind(storage, root_scope, ty->strname, NAMESPACE_TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(logical_ops)/sizeof(logical_ops[0]); i++) {
    ty = array_append(storage, type_array, sizeof(Type));
    ty->strname = logical_ops[i];
    ty->ty_former = TYPE_FUNCTION;
    params_ty = array_append(storage, type_array, sizeof(Type));
    params_ty->ty_former = TYPE_PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = arena_malloc(storage, params_ty->product.count*sizeof(Type*));
    params_ty->product.members[0] = builtin_lookup(root_scope, "bool", NAMESPACE_TYPE)->type;
    params_ty->product.members[1] = builtin_lookup(root_scope, "bool", NAMESPACE_TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = builtin_lookup(root_scope, "bool", NAMESPACE_TYPE)->type;
    name_decl = scope_bind(storage, root_scope, ty->strname, NAMESPACE_TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(relational_ops)/sizeof(relational_ops[0]); i++) {
    ty = array_append(storage, type_array, sizeof(Type));
    ty->strname = relational_ops[i];
    ty->ty_former = TYPE_FUNCTION;
    params_ty = array_append(storage, type_array, sizeof(Type));
    params_ty->ty_former = TYPE_PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = arena_malloc(storage, params_ty->product.count*sizeof(Type*));
    params_ty->product.members[0] = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
    params_ty->product.members[1] = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = builtin_lookup(root_scope, "bool", NAMESPACE_TYPE)->type;
    name_decl = scope_bind(storage, root_scope, ty->strname, NAMESPACE_TYPE);
    name_decl->type = ty;
  }
  for (int i = 0; i < sizeof(bitwise_ops)/sizeof(bitwise_ops[0]); i++) {
    ty = array_append(storage, type_array, sizeof(Type));
    ty->strname = bitwise_ops[i];
    ty->ty_former = TYPE_FUNCTION;
    params_ty = array_append(storage, type_array, sizeof(Type));
    params_ty->ty_former = TYPE_PRODUCT;
    params_ty->product.count = 2;
    params_ty->product.members = arena_malloc(storage, params_ty->product.count*sizeof(Type*));
    params_ty->product.members[0] = builtin_lookup(root_scope, "bit", NAMESPACE_TYPE)->type;
    params_ty->product.members[1] = builtin_lookup(root_scope, "bit", NAMESPACE_TYPE)->type;
    ty->function.params = params_ty;
    ty->function.return_ = builtin_lookup(root_scope, "bit", NAMESPACE_TYPE)->type;
    name_decl = scope_bind(storage, root_scope, ty->strname, NAMESPACE_TYPE);
    name_decl->type = ty;
  }
}

static bool
structural_type_equiv(Type* left, Type* right)
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

  for (i = 0; i < type_equiv_pairs->elem_count; i++) {
    type_pair = array_get(type_equiv_pairs, i, sizeof(Type));
    assert(type_pair->ty_former == TYPE_TUPLE);
    if ((left == type_pair->tuple.left || left == type_pair->tuple.right) &&
        (right == type_pair->tuple.left || right == type_pair->tuple.right)) {
      return 1;
    }
  }

  type_pair = array_append(storage, type_equiv_pairs, sizeof(Type));
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
        if (!structural_type_equiv(left->product.members[i], left->product.members[i])) {
          return 0;
        }
      }
      return 1;
    }
    return 0;
  } else if (left->ty_former == TYPE_FUNCTION) {
    if (right->ty_former == left->ty_former) {
      if (!structural_type_equiv(left->function.return_, right->function.return_)) {
        return 0;
      }
      if (!structural_type_equiv(left->function.params, right->function.params)) {
        return 0;
      }
      return 1;
    }
    return 0;
  } else if (left->ty_former == TYPE_PACKAGE) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(left->package.params, right->package.params);
    }
    return 0;
  } else if (left->ty_former == TYPE_PARSER) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(left->parser.params, right->parser.params);
    }
    return 0;
  } else if (left->ty_former == TYPE_CONTROL) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(left->control.params, right->control.params);
    }
    return 0;
  } else if (left->ty_former == TYPE_STRUCT) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(left->struct_.fields, right->struct_.fields);
    }
    return 0;
  } else if (left->ty_former == TYPE_HEADER) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(left->struct_.fields, right->struct_.fields);
    }
    return 0;
  } else if (left->ty_former == TYPE_HEADER_STACK) {
    if (right->ty_former == left->ty_former) {
      return structural_type_equiv(left->header_stack.element, right->header_stack.element);
    }
    return 0;
  } else assert(0);

  assert(0);
  return 0;
}

bool
type_equiv(Type* left, Type* right)
{
  type_equiv_pairs->elem_count = 0;
  return structural_type_equiv(left, right);
}

Type*
actual_type(Type* type)
{
  if (!type) { return 0; }
  if (type->ty_former == TYPE_TYPE) {
    return type->type.type;
  }
  return type;
}

Type*
effective_type(Type* type)
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

char*
TypeEnum_to_string(enum TypeEnum type)
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

void
Debug_print_type_env(Map* env)
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

void
Debug_print_type_array(Array* type_array)
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

Map*
declared_types(Arena* storage_, char* source_file_, Ast* p4program, Scope* root_scope_,
    Array* type_array_, Map* scope_map_, Map* decl_map_)
{
  Ast* name;
  Type* ref_ty, *ty;
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  storage = storage_;
  source_file = source_file_;
  root_scope = root_scope_;
  scope_map = scope_map_;
  decl_map = decl_map_;
  type_array = type_array_;
  type_env = arena_malloc(storage, sizeof(Map));
  type_equiv_pairs = array_create(storage, sizeof(Type), 2);

  setup_builtin_types();
  visit_p4program(p4program);
  for (int i = 0; i < type_array->elem_count; i++) {
    ty = array_get(type_array, i, sizeof(Type));
    if (ty->ty_former == TYPE_NAMEREF) {
      name = ty->nameref.name;
      name_entry = scope_lookup(ty->nameref.scope, name->name.strname, NAMESPACE_TYPE);
      name_decl = name_entry->ns[NAMESPACE_TYPE >> 1];
      if (name_decl) {
        ref_ty = map_lookup(type_env, name_decl->ast, 0);
        assert(ref_ty);
        name_decl->type = ref_ty;
        ty->ty_former = TYPE_TYPE;
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
    ty = array_get(type_array, i, sizeof(Type));
    if (ty->ty_former == TYPE_TYPEDEF) {
      ref_ty = actual_type(ty->typedef_.ref);
      while (ref_ty->ty_former == TYPE_TYPEDEF) {
        ref_ty = actual_type(ref_ty->typedef_.ref);
      }
      ty->ty_former = TYPE_TYPE;
      ty->type.type = ref_ty;
    }
  }
  for (int i = 0; i < type_array->elem_count; i++) {
    ty = array_get(type_array, i, sizeof(Type));
    if (ty->ty_former == TYPE_TYPE) {
      ref_ty = actual_type(ty->type.type);
      while (ref_ty->ty_former == TYPE_TYPE) {
        ref_ty = actual_type(ref_ty->type.type);
      }
      ty->ty_former = TYPE_TYPE;
      ty->type.type = ref_ty;
    }
  }
  return type_env;
}

/** PROGRAM **/

static void
visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  visit_declarationList(p4program->p4program.decl_list);
}

static void
visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  Ast* ast;

  for (ast = decl_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(ast);
  }
}

static void
visit_declaration(Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    visit_externDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    visit_instantiation(decl->declaration.decl);
  } else assert(0);
}

static void
visit_name(Ast* name)
{
  assert(name->kind == AST_name);
  Type* name_ty;

  name_ty = array_append(storage, type_array, sizeof(Type));
  name_ty->ty_former = TYPE_NAMEREF;
  name_ty->strname = name->name.strname;
  name_ty->ast = name;
  name_ty->nameref.name = name;
  name_ty->nameref.scope = map_lookup(scope_map, name, 0);
  map_insert(storage, type_env, name, name_ty, 0);
}

static void
visit_parameterList(Ast* params)
{
  assert(params->kind == AST_parameterList);
  Ast* ast;
  Type* params_ty;
  int i;

  params_ty = array_append(storage, type_array, sizeof(Type));
  params_ty->ty_former = TYPE_PRODUCT;
  params_ty->ast = params;
  for (ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(ast);
    params_ty->product.count += 1;
  }
  if (params_ty->product.count > 0) {
    params_ty->product.members = arena_malloc(storage, params_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    params_ty->product.members[i] = map_lookup(type_env, ast, 0);
    i += 1;
  }
  assert(i == params_ty->product.count);
  map_insert(storage, type_env, params, params_ty, 0);
}

static void
visit_parameter(Ast* param)
{
  assert(param->kind == AST_parameter);
  NameDeclaration* name_decl;
  Type* param_ty;

  visit_typeRef(param->parameter.type);
  param_ty = map_lookup(type_env, param->parameter.type, 0);
  map_insert(storage, type_env, param, param_ty, 0);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr);
  }
  name_decl = map_lookup(decl_map, param, 0);
  name_decl->type = param_ty;
}

static void
visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* package_ty;

  visit_parameterList(type_decl->packageTypeDeclaration.params);
  name = type_decl->packageTypeDeclaration.name;
  package_ty = array_append(storage, type_array, sizeof(Type));
  package_ty->ty_former = TYPE_PACKAGE;
  package_ty->strname = name->name.strname;
  package_ty->ast = type_decl;
  package_ty->package.params = map_lookup(type_env, type_decl->packageTypeDeclaration.params, 0);
  map_insert(storage, type_env, type_decl, package_ty, 0);
  name_decl = map_lookup(decl_map, type_decl, 0);
  name_decl->type = package_ty;
}

static void
visit_instantiation(Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  Type* inst_ty;
  NameDeclaration* name_decl;

  visit_typeRef(inst->instantiation.type);
  visit_argumentList(inst->instantiation.args);
  inst_ty = map_lookup(type_env, inst->instantiation.type, 0);
  map_insert(storage, type_env, inst, inst_ty, 0);
  name_decl = map_lookup(decl_map, inst, 0);
  name_decl->type = inst_ty;
}

/** PARSER **/

static void
visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  Type* parser_ty;

  visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
    parser_ty = map_lookup(type_env, parser_decl->parserDeclaration.proto, 0);
    parser_ty->parser.ctor_params = map_lookup(type_env, parser_decl->parserDeclaration.ctor_params, 0);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

static void
visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* parser_ty, *methods_ty;

  visit_parameterList(type_decl->parserTypeDeclaration.params);
  name = type_decl->parserTypeDeclaration.name;
  parser_ty = array_append(storage, type_array, sizeof(Type));
  parser_ty->ty_former = TYPE_PARSER;
  parser_ty->strname = name->name.strname;
  parser_ty->ast = type_decl;
  parser_ty->parser.params = map_lookup(type_env, type_decl->parserTypeDeclaration.params, 0);
  map_insert(storage, type_env, type_decl, parser_ty, 0);
  visit_methodPrototypes(type_decl->parserTypeDeclaration.method_protos, 0, 0);
  methods_ty = map_lookup(type_env, type_decl->parserTypeDeclaration.method_protos, 0);
  parser_ty->parser.methods = methods_ty;
  name_decl = map_lookup(decl_map, type_decl, 0);
  name_decl->type = parser_ty;
}

static void
visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  Ast* ast;

  for (ast = local_elements->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(ast);
  }
}

static void
visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
}

static void
visit_parserStates(Ast* states)
{
  assert(states->kind == AST_parserStates);
  Ast* ast;

  for (ast = states->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(ast);
  }
}

static void
visit_parserState(Ast* state)
{
  assert(state->kind == AST_parserState);
  Ast* name;
  NameDeclaration* name_decl;
  Type* state_ty;

  name = state->parserState.name;
  state_ty = array_append(storage, type_array, sizeof(Type));
  state_ty->ty_former = TYPE_STATE;
  state_ty->strname = name->name.strname;
  state_ty->ast = state;
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
  map_insert(storage, type_env, state, state_ty, 0);
  name_decl = map_lookup(decl_map, state, 0);
  name_decl->type = state_ty;
}

static void
visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  Ast* ast;

  for (ast = stmts->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(ast);
  }
}

static void
visit_parserStatement(Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visit_functionCall(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
}

static void
visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
}

static void
visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression(transition_stmt->transitionStatement.stmt);
}

static void
visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    ;
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
}

static void
visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(select_expr->selectExpression.expr_list);
  visit_selectCaseList(select_expr->selectExpression.case_list);
}

static void
visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  Ast* ast;

  for (ast = case_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(ast);
  }
}

static void
visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(select_case->selectCase.keyset_expr);
}

static void
visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
}

static void
visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
}

static void
visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visit_expression(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

static void
visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  Ast* ast;

  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(ast);
  }
}

/** CONTROL **/

static void
visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  Type* control_ty;

  visit_typeDeclaration(control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
    control_ty = map_lookup(type_env, control_decl->controlDeclaration.proto, 0);
    control_ty->control.ctor_params = map_lookup(type_env, control_decl->controlDeclaration.ctor_params, 0);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
}

static void
visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* control_ty;

  visit_parameterList(type_decl->controlTypeDeclaration.params);
  name = type_decl->controlTypeDeclaration.name;
  control_ty = array_append(storage, type_array, sizeof(Type));
  control_ty->ty_former = TYPE_CONTROL;
  control_ty->strname = name->name.strname;
  control_ty->ast = type_decl;
  control_ty->control.params = map_lookup(type_env, type_decl->packageTypeDeclaration.params, 0);
  map_insert(storage, type_env, type_decl, control_ty, 0);
  name_decl = map_lookup(decl_map, type_decl, 0);
  name_decl->type = control_ty;
}

static void
visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  Ast* ast;

  for (ast = local_decls->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(ast);
  }
}

static void
visit_controlLocalDeclaration(Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    visit_instantiation(local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void
visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    visit_functionPrototype(extern_decl->externDeclaration.decl, 0, 0);
  } else assert(0);
}

static void
visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* extern_ty, *methods_ty, *ctors_ty;

  name = type_decl->externTypeDeclaration.name;
  extern_ty = array_append(storage, type_array, sizeof(Type));
  extern_ty->ty_former = TYPE_EXTERN;
  extern_ty->strname = name->name.strname;
  extern_ty->ast = type_decl;
  map_insert(storage, type_env, type_decl, extern_ty, 0);
  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos, extern_ty, name->name.strname);
  methods_ty = map_lookup(type_env, type_decl->externTypeDeclaration.method_protos, 0);
  extern_ty->extern_.methods = methods_ty;
  ctors_ty = array_append(storage, type_array, sizeof(Type));
  ctors_ty->ty_former = TYPE_PRODUCT;
  ctors_ty->ast = type_decl;
  for (int i = 0; i < methods_ty->product.count; i++) {
    if (cstr_match(methods_ty->product.members[i]->strname, name->name.strname)) {
      ctors_ty->product.count += 1;
    }
  }
  if (ctors_ty->product.count > 0) {
    ctors_ty->product.members = arena_malloc(storage, ctors_ty->product.count*sizeof(Type*));
  }
  for (int i = 0; i < methods_ty->product.count; i++) {
    if (cstr_match(methods_ty->product.members[i]->strname, name->name.strname)) {
      ctors_ty->product.members[i] = methods_ty->product.members[i];
    }
  }
  extern_ty->extern_.ctors = ctors_ty;
  name_decl = map_lookup(decl_map, type_decl, 0);
  name_decl->type = extern_ty;
}

static void
visit_methodPrototypes(Ast* protos, Type* ctor_ty, char* ctor_strname)
{
  assert(protos->kind == AST_methodPrototypes);
  Ast* ast;
  Type* methods_ty;
  int i;

  methods_ty = array_append(storage, type_array, sizeof(Type));
  methods_ty->ty_former = TYPE_PRODUCT;
  methods_ty->ast = protos;
  for (ast = protos->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(ast, ctor_ty, ctor_strname);
    methods_ty->product.count += 1;
  }
  if (methods_ty->product.count > 0) {
    methods_ty->product.members = arena_malloc(storage, methods_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = protos->first_child;
       ast != 0; ast = ast->right_sibling) {
    methods_ty->product.members[i] = map_lookup(type_env, ast, 0);
    i += 1;
  }
  assert(i == methods_ty->product.count);
  map_insert(storage, type_env, protos, methods_ty, 0);
}

static void
visit_functionPrototype(Ast* func_proto, Type* ctor_ty, char* ctor_strname)
{
  assert(func_proto->kind == AST_functionPrototype);
  Ast* name, *return_type;
  NameDeclaration* name_decl;
  Type* func_ty;

  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }
  visit_parameterList(func_proto->functionPrototype.params);
  name = func_proto->functionPrototype.name;
  func_ty = array_append(storage, type_array, sizeof(Type));
  func_ty->ty_former = TYPE_FUNCTION;
  func_ty->strname = name->name.strname;
  func_ty->ast = func_proto;
  func_ty->function.params = map_lookup(type_env, func_proto->functionPrototype.params, 0);
  map_insert(storage, type_env, func_proto, func_ty, 0);
  return_type = func_proto->functionPrototype.return_type;
  if (return_type) {
    func_ty->function.return_ = map_lookup(type_env, return_type, 0);
  } else if (cstr_match(name->name.strname, ctor_strname)) {
    func_ty->function.return_ = ctor_ty;
  } else assert(0);
  name_decl = map_lookup(decl_map, func_proto, 0);
  name_decl->type = func_ty;
}

/** TYPES **/

static void
visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  Type* ref_ty;

  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    visit_baseTypeBit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    visit_baseTypeString(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    visit_baseTypeError(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    visit_name(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
  ref_ty = map_lookup(type_env, type_ref->typeRef.type, 0);
  map_insert(storage, type_env, type_ref, ref_ty, 0);
}

static void
visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  Type* tuple_ty;

  visit_typeArgumentList(type_decl->tupleType.type_args);
  tuple_ty = map_lookup(type_env, type_decl->tupleType.type_args, 0);
  map_insert(storage, type_env, type_decl, tuple_ty, 0);
}

static void
visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  Type* stack_ty;

  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr);
  stack_ty = array_append(storage, type_array, sizeof(Type));
  stack_ty->ty_former = TYPE_HEADER_STACK;
  stack_ty->ast = type_decl;
  map_insert(storage, type_env, type_decl, stack_ty, 0);
  stack_ty->header_stack.element = map_lookup(type_env, type_decl->headerStackType.type, 0);
}

static void
visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  NameDeclaration* name_decl;

  name_decl = map_lookup(decl_map, bool_type, 0);
  map_insert(storage, type_env, bool_type, name_decl->type, 0);
}

static void
visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  NameDeclaration* name_decl;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  name_decl = map_lookup(decl_map, int_type, 0);
  map_insert(storage, type_env, int_type, name_decl->type, 0);
}

static void
visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  NameDeclaration* name_decl;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  name_decl = map_lookup(decl_map, bit_type, 0);
  map_insert(storage, type_env, bit_type, name_decl->type, 0);
}

static void
visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  NameDeclaration* name_decl;

  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  name_decl = map_lookup(decl_map, varbit_type, 0);
  map_insert(storage, type_env, varbit_type, name_decl->type, 0);
}

static void
visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  NameDeclaration* name_decl;

  name_decl = map_lookup(decl_map, str_type, 0);
  map_insert(storage, type_env, str_type, name_decl->type, 0);
}

static void
visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  NameDeclaration* name_decl;

  name_decl = map_lookup(decl_map, void_type, 0);
  map_insert(storage, type_env, void_type, name_decl->type, 0);
}

static void
visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  NameDeclaration* name_decl;

  name_decl = map_lookup(decl_map, error_type, 0);
  map_insert(storage, type_env, error_type, name_decl->type, 0);
}

static void
visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void
visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

static void
visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  Type* arg_ty;

  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
  arg_ty = map_lookup(type_env, type_arg->typeArg.arg, 0);
  map_insert(storage, type_env, type_arg, arg_ty, 0);
}

static void
visit_typeArgumentList(Ast* args)
{
  assert(args->kind == AST_typeArgumentList);
  Ast* ast;
  Type* args_ty;
  int i;

  args_ty = array_append(storage, type_array, sizeof(Type));
  args_ty->ty_former = TYPE_PRODUCT;
  args_ty->ast = args;
  for (ast = args->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(ast);
    args_ty->product.count += 1;
  }
  if (args_ty->product.count > 0) {
    args_ty->product.members = arena_malloc(storage, args_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = args->first_child;
       ast != 0; ast = ast->right_sibling) {
    args_ty->product.members[i] = map_lookup(type_env, ast, 0);
    i += 1;
  }
  assert(i == args_ty->product.count);
  map_insert(storage, type_env, args, args_ty, 0);
}

static void
visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  Type* decl_ty;

  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration(type_decl->typeDeclaration.decl);
  } else assert(0);
  decl_ty = map_lookup(type_env, type_decl->typeDeclaration.decl, 0);
  map_insert(storage, type_env, type_decl, decl_ty, 0);
}

static void
visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  Type* decl_ty;

  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  decl_ty = map_lookup(type_env, type_decl->derivedTypeDeclaration.decl, 0);
  map_insert(storage, type_env, type_decl, decl_ty, 0);
}

static void
visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* header_ty;

  visit_structFieldList(header_decl->headerTypeDeclaration.fields);
  name = header_decl->headerTypeDeclaration.name;
  header_ty = array_append(storage, type_array, sizeof(Type));
  header_ty->ty_former = TYPE_HEADER;
  header_ty->strname = name->name.strname;
  header_ty->ast = header_decl;
  map_insert(storage, type_env, header_decl, header_ty, 0);
  header_ty->struct_.fields = map_lookup(type_env, header_decl->headerTypeDeclaration.fields, 0);
  name_decl = map_lookup(decl_map, header_decl, 0);
  name_decl->type = header_ty;
}

static void
visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* union_ty;

  visit_structFieldList(union_decl->headerUnionDeclaration.fields);
  name = union_decl->headerUnionDeclaration.name;
  union_ty = array_append(storage, type_array, sizeof(Type));
  union_ty->ty_former = TYPE_HEADER_UNION;
  union_ty->strname = name->name.strname;
  union_ty->ast = union_decl;
  map_insert(storage, type_env, union_decl, union_ty, 0);
  union_ty->struct_.fields = map_lookup(type_env, union_decl->headerUnionDeclaration.fields, 0);
  name_decl = map_lookup(decl_map, union_decl, 0);
  name_decl->type = union_ty;
}

static void
visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* struct_ty;

  visit_structFieldList(struct_decl->structTypeDeclaration.fields);
  name = struct_decl->structTypeDeclaration.name;
  struct_ty = array_append(storage, type_array, sizeof(Type));
  struct_ty->ty_former = TYPE_STRUCT;
  struct_ty->strname = name->name.strname;
  struct_ty->ast = struct_decl;
  map_insert(storage, type_env, struct_decl, struct_ty, 0);
  struct_ty->struct_.fields = map_lookup(type_env, struct_decl->structTypeDeclaration.fields, 0);
  name_decl = map_lookup(decl_map, struct_decl, 0);
  name_decl->type = struct_ty;
}

static void
visit_structFieldList(Ast* fields)
{
  assert(fields->kind == AST_structFieldList);
  Ast* ast;
  Type* fields_ty;
  int i;

  fields_ty = array_append(storage, type_array, sizeof(Type));
  fields_ty->ty_former = TYPE_PRODUCT;
  fields_ty->ast = fields;
  for (ast = fields->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(ast);
    fields_ty->product.count += 1;
  }
  if (fields_ty->product.count > 0) {
    fields_ty->product.members = arena_malloc(storage, fields_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = fields->first_child;
       ast != 0; ast = ast->right_sibling) {
    fields_ty->product.members[i] = map_lookup(type_env, ast, 0);
    i += 1;
  }
  assert(i == fields_ty->product.count);
  map_insert(storage, type_env, fields, fields_ty, 0);
}

static void
visit_structField(Ast* field)
{
  assert(field->kind == AST_structField);
  Ast* name;
  NameDeclaration* name_decl;
  Type* field_ty;

  visit_typeRef(field->structField.type);
  name = field->structField.name;
  field_ty = array_append(storage, type_array, sizeof(Type));
  field_ty->ty_former = TYPE_FIELD;
  field_ty->strname = name->name.strname;
  field_ty->ast = field;
  field_ty->field.type = map_lookup(type_env, field->structField.type, 0);
  map_insert(storage, type_env, field, field_ty, 0);
  name_decl = map_lookup(decl_map, field, 0);
  name_decl->type = field_ty;
}

static void
visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* enum_ty;

  name = enum_decl->enumDeclaration.name;
  enum_ty = array_append(storage, type_array, sizeof(Type));
  enum_ty->ty_former = TYPE_ENUM;
  enum_ty->strname = name->name.strname;
  enum_ty->ast = enum_decl;
  map_insert(storage, type_env, enum_decl, enum_ty, 0);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields, enum_ty);
  enum_ty->enum_.fields = map_lookup(type_env, enum_decl->enumDeclaration.fields, 0);
  name_decl = map_lookup(decl_map, enum_decl, 0);
  name_decl->type = enum_ty;
}

static void
visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  Type* error_ty, *fields_ty;

  error_ty = builtin_lookup(root_scope, "error", NAMESPACE_TYPE)->type;
  fields_ty = error_ty->enum_.fields;
  if (error_ty->enum_.field_count > 0 && fields_ty->product.members == 0) {
    fields_ty->product.count = error_ty->enum_.field_count;
    fields_ty->product.members = arena_malloc(storage, fields_ty->product.count*sizeof(Type*));
  }
  visit_identifierList(error_decl->errorDeclaration.fields, error_ty,
      error_ty->enum_.fields, &error_ty->enum_.i);
}

static void
visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  Type* match_kind_ty, *fields_ty;

  match_kind_ty = builtin_lookup(root_scope, "match_kind", NAMESPACE_TYPE)->type;
  fields_ty = match_kind_ty->enum_.fields;
  if (match_kind_ty->enum_.field_count > 0 && fields_ty->product.members == 0) {
    fields_ty->product.count = match_kind_ty->enum_.field_count;
    fields_ty->product.members = arena_malloc(storage, fields_ty->product.count*sizeof(Type*));
  }
  visit_identifierList(match_decl->matchKindDeclaration.fields, match_kind_ty,
      match_kind_ty->enum_.fields, &match_kind_ty->enum_.i);
}

static void
visit_identifierList(Ast* ident_list, Type* enum_ty, Type* idents_ty, int* i)
{
  assert(ident_list->kind == AST_identifierList);
  Ast* name;
  NameDeclaration* name_decl;
  Type* name_ty;
  int j;

  j = *i;
  for (name = ident_list->first_child;
       name != 0; name = name->right_sibling) {
    name_ty = array_append(storage, type_array, sizeof(Type));
    name_ty->ty_former = TYPE_FIELD;
    name_ty->strname = name->name.strname;
    name_ty->ast = name;
    name_ty->field.type = enum_ty;
    map_insert(storage, type_env, name, name_ty, 0);
    name_decl = map_lookup(decl_map, name, 0);
    name_decl->type = name_ty;
    idents_ty->product.members[j] = map_lookup(type_env, name, 0);
    j += 1;
  }
  *i = j;
}

static void
visit_specifiedIdentifierList(Ast* ident_list, Type* enum_ty)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  Ast* ast;
  Type* idents_ty;
  int i;

  idents_ty = array_append(storage, type_array, sizeof(Type));
  idents_ty->ty_former = TYPE_PRODUCT;
  idents_ty->ast = ident_list;
  for (ast = ident_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(ast, enum_ty);
    idents_ty->product.count += 1;
  }
  if (idents_ty->product.count > 0) {
    idents_ty->product.members = arena_malloc(storage, idents_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = ident_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    idents_ty->product.members[i] = map_lookup(type_env, ast, 0);
    i += 1;
  }
  assert(i == idents_ty->product.count);
  map_insert(storage, type_env, ident_list, idents_ty, 0);
}

static void
visit_specifiedIdentifier(Ast* ident, Type* enum_ty)
{
  assert(ident->kind == AST_specifiedIdentifier);
  Ast* name;
  NameDeclaration* name_decl;
  Type* ident_ty;

  name = ident->specifiedIdentifier.name;
  ident_ty = array_append(storage, type_array, sizeof(Type));
  ident_ty->ty_former = TYPE_FIELD;
  ident_ty->strname = name->name.strname;
  ident_ty->ast = ident;
  ident_ty->field.type = enum_ty;
  map_insert(storage, type_env, ident, ident_ty, 0);
  name_decl = map_lookup(decl_map, ident, 0);
  name_decl->type = ident_ty;
}

static void
visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* typedef_ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  name = typedef_decl->typedefDeclaration.name;
  typedef_ty = array_append(storage, type_array, sizeof(Type));
  typedef_ty->ty_former = TYPE_TYPEDEF;
  typedef_ty->strname = name->name.strname;
  typedef_ty->ast = typedef_decl;
  map_insert(storage, type_env, typedef_decl, typedef_ty, 0);
  typedef_ty->typedef_.ref = map_lookup(type_env, typedef_decl->typedefDeclaration.type_ref, 0);
  name_decl = map_lookup(decl_map, typedef_decl, 0);
  name_decl->type = typedef_ty;
}

/** STATEMENTS **/

static void
visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr);
}

static void
visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(func_call->functionCall.args);
}

static void
visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr);
  }
}

static void
visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
}

static void
visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visit_expression(cond_stmt->conditionalStatement.cond_expr);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

static void
visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(applic_stmt->directApplication.args);
}

static void
visit_statement(Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    visit_blockStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    visit_switchStatement(stmt->statement.stmt);
  } else assert(0);
}

static void
visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
}

static void
visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  Ast* ast;

  for (ast = stmt_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(ast);
  }
}

static void
visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(switch_stmt->switchStatement.expr);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

static void
visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  Ast* ast;

  for (ast = switch_cases->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(ast);
  }
}

static void
visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(switch_case->switchCase.stmt);
  }
}

static void
visit_switchLabel(Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    ;
  } else if (label->switchLabel.label->kind == AST_default) {
    visit_default(label->switchLabel.label);
  } else assert(0);
}

static void
visit_statementOrDeclaration(Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    visit_statement(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    visit_instantiation(stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void
visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Type* table_ty;

  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
  name = table_decl->tableDeclaration.name;
  table_ty = array_append(storage, type_array, sizeof(Type));
  table_ty->ty_former = TYPE_TABLE;
  table_ty->strname = name->name.strname;
  table_ty->ast = table_decl;
  map_insert(storage, type_env, table_decl, table_ty, 0);
  name_decl = map_lookup(decl_map, table_decl, 0);
  name_decl->type = table_ty;
}

static void
visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  Ast* ast;

  for (ast = prop_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(ast);
  }
}

static void
visit_tableProperty(Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    visit_keyProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    visit_actionsProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    visit_entriesProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    visit_simpleProperty(table_prop->tableProperty.prop);
  } else assert(0);
}

static void
visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList(key_prop->keyProperty.keyelem_list);
}

static void
visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  Ast* ast;

  for (ast = element_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(ast);
  }
}

static void
visit_keyElement(Ast* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression(element->keyElement.expr);
}

static void
visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(actions_prop->actionsProperty.action_list);
}

static void
visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  Ast* ast;

  for (ast = action_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(ast);
  }
}

static void
visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args);
  }
}

static void
visit_entriesProperty(Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList(entries_prop->entriesProperty.entries_list);
}

static void
visit_entriesList(Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  Ast* ast;

  for (ast = entries_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(ast);
  }
}

static void
visit_entry(Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(entry->entry.keyset);
  visit_actionRef(entry->entry.action);
}

static void
visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  visit_expression(simple_prop->simpleProperty.init_expr);
}

static void
visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  NameDeclaration* name_decl;
  Ast* name;
  Type* action_ty;

  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
  name = action_decl->actionDeclaration.name;
  action_ty = array_append(storage, type_array, sizeof(Type));
  action_ty->ty_former = TYPE_FUNCTION;
  action_ty->strname = name->name.strname;
  action_ty->ast = action_decl;
  action_ty->function.params = map_lookup(type_env, action_decl->actionDeclaration.params, 0);
  map_insert(storage, type_env, action_decl, action_ty, 0);
  action_ty->function.return_ = builtin_lookup(root_scope, "void", NAMESPACE_TYPE)->type;
  name_decl = map_lookup(decl_map, action_decl, 0);
  name_decl->type = action_ty;
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  NameDeclaration* name_decl;
  Type* var_ty;

  visit_typeRef(var_decl->variableDeclaration.type);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr);
  }
  var_ty = map_lookup(type_env, var_decl->variableDeclaration.type, 0);
  map_insert(storage, type_env, var_decl, var_ty, 0);
  name_decl = map_lookup(decl_map, var_decl, 0);
  name_decl->type = var_ty;
}

/** EXPRESSIONS **/

static void
visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visit_functionPrototype(func_decl->functionDeclaration.proto, 0, 0);
  visit_blockStatement(func_decl->functionDeclaration.stmt);
}

static void
visit_argumentList(Ast* args)
{
  assert(args->kind == AST_argumentList);
  Ast* ast;

  for (ast = args->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(ast);
  }
}

static void
visit_argument(Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(arg->argument.arg);
  } else assert(0);
}

static void
visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  Ast* ast;

  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(ast);
  }
}

static void
visit_lvalueExpression(Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    ;
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

static void
visit_expression(Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    ;
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
}

static void
visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr);
}

static void
visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand);
}

static void
visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression(binary_expr->binaryExpression.left_operand);
  visit_expression(binary_expr->binaryExpression.right_operand);
}

static void
visit_memberSelector(Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr);
  } else assert(0);
}

static void
visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
}

static void
visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  Type* ty;

  visit_expression(index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index);
  }
  ty = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, index_expr, ty, 0);
}

static void
visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
  Type* ty;

  ty = builtin_lookup(root_scope, "bool", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, bool_literal, ty, 0);
}

static void
visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
  Type* ty;

  ty = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, int_literal, ty, 0);
}

static void
visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
  Type* ty;

  ty = builtin_lookup(root_scope, "string", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, str_literal, ty, 0);
}

static void
visit_default(Ast* default_)
{
  assert(default_->kind == AST_default);
  Type* ty;

  ty = builtin_lookup(root_scope, "_", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, default_, ty, 0);
}

static void
visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
  Type* ty;

  ty = builtin_lookup(root_scope, "_", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, dontcare, ty, 0);
}

