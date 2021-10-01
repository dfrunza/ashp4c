#pragma once
#include "arena.h"
#include "hash.h"
#include "token.h"


enum ObjectKind {
  Object_NONE,
  Object_Keyword,
  Object_Variable,
  Object_Constant,
  Object_Action,
  Object_Parser,
  Object_ParserState,
  Object_Control,
  Object_Error,
  Object_MatchKind,
  Object_Function,
  Object_Instantiation,
  Object_Table,
};

enum TypeKind {
  Type_NONE,
  Type_TypeParam,
  Type_Basic,
  Type_Struct,
  Type_Header,
  Type_HeaderUnion,
  Type_Enum,
  Type_Type,
  Type_Typedef,
  Type_Parser,
  Type_Control,
  Type_Function,
  Type_Extern,
  Type_Package,
};

struct ObjectDescriptor {
  char* name;
  union {
    enum ObjectKind object_kind;
    enum TypeKind type_kind;
  };
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
struct SymtableEntry* find_symtable_entry(struct Scope* scope, char* name);
struct SymtableEntry* get_symtable_entry(struct Scope* scope, char* name);
struct SymtableEntry* register_ident(struct Scope* scope, struct ObjectDescriptor* descriptor, int line_nr);
struct SymtableEntry* register_type(struct Scope* scope, struct ObjectDescriptor* descriptor, int line_nr);
struct SymtableEntry* register_keyword(struct Scope* scope, struct ObjectDescriptor* descriptor);

struct Scope* new_scope(int capacity_log2);
struct Scope* push_scope();
struct Scope* pop_scope();
struct Scope* get_root_scope();
struct Scope* get_current_scope();
struct SymtableEntry* scope_resolve_name(struct Scope* scope, char* name);

