#pragma once
#include "arena.h"
#include "hash.h"


enum IdentKind
{
  Ident_None,
  Ident_Keyword,
  Ident_Type,
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
  struct Ident* ns_kw;
  struct Ident* ns_type;
  struct SymtableEntry* next_entry;
};


void symtable_init(struct Arena* symtable_storage_);
struct SymtableEntry* get_symtable_entry(char* name);
int new_scope();
void delete_scope();
