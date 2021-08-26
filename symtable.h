#pragma once
#include "arena.h"
#include "hash.h"
#include "token.h"


enum ObjectKind {
  Object_NONE_,
  Object_Keyword,
  Object_Type,
  //Object_Ident,
  Object_Var,
  Object_Const,
  Object_Action,
  Object_Table,
  Object_FunctionProto,
  Object_StructField,
  Object_Instantiation,
  Object_EnumId,
};

struct Object {
  enum ObjectKind symbol_kind;
  char* name;
  struct Ast* ast;
  struct Object* next_in_scope;
};  

struct Object_Keyword {
  struct Object;
  enum TokenClass token_klass;
};

struct SymtableEntry {
  char* name;
  struct Object* id_kw;
  struct Object* id_type;
  struct Object* id_ident;
  struct SymtableEntry* next_entry;
};

struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  int capacity_log2;
  int capacity;
  int entry_count;
  struct UnboundedArray symtable;
};


void scope_init();
void symtable_set_storage(struct Arena* symtable_storage_);
struct SymtableEntry* get_symtable_entry(struct Scope* scope, char* name);
struct Object* new_ident(struct Scope* scope, char* name, enum ObjectKind kind, struct Ast* ast, int line_nr);
struct Object* new_type(struct Scope* scope, char* name, struct Ast* ast, int line_nr);

struct Scope* push_scope();
struct Scope* pop_scope();
struct Scope* get_current_scope();
struct SymtableEntry* scope_resolve_name(struct Scope* scope, char* name);

