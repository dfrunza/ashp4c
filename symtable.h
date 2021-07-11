#pragma once
#include "arena.h"
#include "hash.h"
#include "token.h"


enum IdentKind
{
  Ident_None,
  Ident_Keyword,
  Ident_Type,
  Ident_Ident,
};

struct Ident {
  enum IdentKind ident_kind;
  char* name;
  int scope_level;
  struct Ident* next_in_scope;
};  

struct Ident_Keyword {
  struct Ident;
  enum TokenClass token_klass;
};

struct SymtableEntry {
  char* name;
  struct Ident* id_kw;
  struct Ident* id_type;
  struct Ident* id_ident;
  struct SymtableEntry* next_entry;
};


void symtable_init();
void symtable_set_storage(struct Arena* symtable_storage_);
struct SymtableEntry* get_symtable_entry(char* name);
int push_scope();
void pop_scope();
