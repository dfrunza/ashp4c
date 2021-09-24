#include "basic.h"
#include "hash.h"
#include "ast.h"
#include <memory.h>  // memset


internal struct Arena* attr_storage;


//void*
//ast_getattr(struct Ast* ast, char* attr_name)
//{
//  uint32_t h = hash_string(attr_name, AST_ATTRTABLE_CAPACITY_LOG2);
//  struct AstAttribute* entry = ast->attrs[h];
//  while (entry) {
//    if (cstr_match(entry->name, attr_name))
//      break;
//    entry = entry->next_attr;
//  }
//  void* attr_value = 0;
//  if (entry) {
//    attr_value = entry->value;
//  }
//  return attr_value;
//}
//
//void
//ast_setattr(struct Ast* ast, char* attr_name, void* attr_value, enum AstAttributeType attr_type)
//{
//  uint32_t h = hash_string(attr_name, AST_ATTRTABLE_CAPACITY_LOG2);
//  struct AstAttribute* entry = ast->attrs[h];
//  while (entry) {
//    if (cstr_match(entry->name, attr_name))
//      break;
//    entry = entry->next_attr;
//  }
//  if (!entry) {
//    if (ast->attr_count >= AST_ATTRTABLE_CAPACITY) {
//      printf("Maximum AST-attribute capacity has been reached.");
//      exit(1);
//    }
//    entry = arena_push(attr_storage, sizeof(*entry));
//    memset(entry, 0, sizeof(*entry));
//    entry->name = attr_name;
//    entry->next_attr = ast->attrs[h];
//    ast->attrs[h] = entry;
//    ast->attr_count += 1;
//  }
//  entry->type = attr_type;
//  entry->value = attr_value;
//}
//
//void*
//ast_delattr(struct Ast* ast, char* attr_name)
//{
//  assert(!"TODO");
//  return 0;
//}
//
//struct AstAttribute*
//ast_attriter_init(struct AstAttributeIterator* iter, struct Ast* ast)
//{
//  memset(iter, 0, sizeof(*iter));
//  iter->ast = ast;
//  for (iter->table_i = 0; iter->table_i < AST_ATTRTABLE_CAPACITY; iter->table_i++) {
//    iter->attr_at = iter->ast->attrs[iter->table_i];
//    if (iter->attr_at) {
//      break;
//    }
//  }
//  return iter->attr_at;
//}
//
//struct AstAttribute*
//ast_attriter_get_next(struct AstAttributeIterator* iter)
//{
//  if (!iter->attr_at) {
//    return iter->attr_at;
//  }
//  iter->attr_at = iter->attr_at->next_attr;
//  if (!iter->attr_at) {
//    for (++iter->table_i; iter->table_i < AST_ATTRTABLE_CAPACITY; iter->table_i++) {
//      iter->attr_at = iter->ast->attrs[iter->table_i];
//      if (iter->attr_at) {
//        break;
//      }
//    }
//  }
//  return iter->attr_at;
//}

void
ast_attr_set_storage(struct Arena* attr_storage_)
{
  attr_storage = attr_storage_;
}

