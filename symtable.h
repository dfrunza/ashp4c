#pragma once
#include "arena.h"
#include "hashmap.h"
#include "token.h"


enum ObjectKind {
  Object_NONE,
  Object_NameRef,
  Object_TypeRef,
  Object_Keyword,
  Object_VarDecl,
  Object_Param,
  Object_StructField,
  Object_EnumField,
  Object_ConstDecl,
  Object_Action,
  Object_Parser,
  Object_ParserState,
  Object_Control,
  Object_Function,
  Object_Instantiation,
  Object_Table,
  Object_TypeParam,
  Object_VoidType,
  Object_BoolType,
  Object_IntType,
  Object_BitType,
  Object_VarbitType,
  Object_StringType,
  Object_ErrorType,
  Object_MatchKind,
  Object_StructType,
  Object_HeaderType,
  Object_HeaderUnionType,
  Object_EnumType,
  Object_Type,
  Object_Typedef,
  Object_ParserType,
  Object_ControlType,
  Object_FunctionType,
  Object_ExternType,
  Object_PackageType,
};

struct ObjectDescriptor {
  char* name;
  enum ObjectKind object_kind;
  struct Ast* ast;
  struct ObjectDescriptor* next_in_scope;
};  

struct Object_Keyword {
  struct ObjectDescriptor;
  enum TokenClass token_klass;
};

struct SymtableEntry {
  char* name;
  struct ObjectDescriptor* ns_keyword;
  struct ObjectDescriptor* ns_type;
  struct ObjectDescriptor* ns_general;
};

struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  struct Scope* first_child_scope;
  struct Scope* right_sibling_scope;
  struct Hashmap symtable;
};


void symtable_init(struct Arena* symtable_storage_, struct Arena* temp_storage_);
void scope_init(struct Scope* scope, int capacity_log2);
struct SymtableEntry* symtable_get_or_create_entry(struct Scope* scope, char* name);
struct SymtableEntry* register_identifier(struct Scope* scope, struct ObjectDescriptor* descriptor, int line_nr);
struct SymtableEntry* register_type(struct Scope* scope, struct ObjectDescriptor* descriptor, int line_nr);
struct SymtableEntry* register_keyword(struct Scope* scope, struct ObjectDescriptor* descriptor);

struct Scope* new_scope(int capacity_log2);
struct Scope* push_scope();
struct Scope* pop_scope();
struct Scope* get_root_scope();
struct Scope* get_current_scope();
struct SymtableEntry* scope_resolve_name(struct Scope* scope, char* name);

