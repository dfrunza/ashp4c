#pragma once
#include "arena.h"
#include "hash.h"
#include "token.h"


enum ObjectKind {
  ObjectKind_NONE,
  ObjectKind_Variable,
  ObjectKind_Action,
  ObjectKind_Parser,
  ObjectKind_Control,
  ObjectKind_Error,
  ObjectKind_MatchKind,
  ObjectKind_Function,
};

struct ObjectDescriptor {
  char* name;
  enum ObjectKind objkind;
  struct Ast* ast;
  struct ObjectDescriptor* next_in_scope;
};  

struct Object_Keyword {
  struct ObjectDescriptor;
  enum TokenClass token_klass;
};

struct SymtableEntry {
  char* name;
  struct ObjectDescriptor* id_kw;
  struct ObjectDescriptor* id_type;
  struct ObjectDescriptor* id_ident;
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


void scope_init(struct Scope* scope, int capacity_log2);
void symtable_set_storage(struct Arena* symtable_storage_);
struct SymtableEntry* find_symtable_entry(struct Scope* scope, char* name);
struct SymtableEntry* get_symtable_entry(struct Scope* scope, char* name);
struct ObjectDescriptor* new_ident(struct Scope* scope, char* name, struct Ast* ast, int line_nr);
void new_type(struct Scope* scope, struct ObjectDescriptor* descriptor, int line_nr);

struct Scope* new_scope(int capacity_log2);
struct Scope* push_scope();
struct Scope* pop_scope();
struct Scope* get_current_scope();
struct SymtableEntry* scope_resolve_name(struct Scope* scope, char* name);

