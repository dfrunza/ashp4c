#pragma once

typedef uint32_t bool;
#define true 1u
#define false 0u
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

void    _assert(char* message, char* file, int line);
#define assert(EXPR) \
  do { if(!(EXPR)) _assert(#EXPR, __FILE__, __LINE__); } while(0)
void    _error(char* file, int line, char* message, ...);
#define error(MSG, ...)  _error(__FILE__, __LINE__, (MSG), ## __VA_ARGS__)
bool  cstr_is_letter(char c);
bool  cstr_is_digit(char c, int base);
bool  cstr_is_ascii_printable(char c);
bool  cstr_is_whitespace(char c);
int   cstr_len(char* str);
char* cstr_copy(char* dest_str, char* src_str);
void  cstr_copy_substr(char* dest_str, char* begin_char, char* end_char);
bool  cstr_start_with(char* str, char* prefix);
bool  cstr_match(char* str_a, char* str_b);
void  cstr_print_substr(char* begin_char, char* end_char);
bool  bytes_match(uint8_t* bytes_a, int len_a, uint8_t* bytes_b, int len_b);
int   floor_log2(int x);
int   ceil_log2(int x);

typedef struct PageBlock {
  struct PageBlock* next_block;
  struct PageBlock* prev_block;
  uint8_t* memory_begin;
  uint8_t* memory_end;
} PageBlock;

typedef struct Arena {
  PageBlock* owned_pages;
  void* memory_avail;
  void* memory_limit;
} Arena;

void  reserve_page_memory(int memory_amount);
void* arena_malloc(Arena* arena, uint32_t size);
void  arena_free(Arena* arena);

typedef struct ListItem {
  struct ListItem* next;
  struct ListItem* prev;
} ListItem;

typedef struct List {
  ListItem sentinel;
  ListItem* last_item;
  int item_count;
  Arena* storage;
} List;

void      list_create(List* list, Arena* storage);
ListItem* _list_create_item(List* list, int item_size);
#define   list_create_item(LIST, TYPE)  (TYPE*)_list_create_item(LIST, sizeof(TYPE))
ListItem* _list_first_item(List* list);
#define   list_first_item(LIST, TYPE)  (TYPE*)_list_first_item(LIST)
void      list_append_item(List* list, ListItem* item, int count);

typedef struct UnboundedArray {
  void** segment_table;
  int segment_length;
  int elem_size;
  int elem_count;
  int capacity;
  Arena* storage;
} UnboundedArray;

void  array_create(UnboundedArray* array, Arena* storage, int elem_size, int max_array_length);
void* array_get(UnboundedArray* array, int i);
void* array_set(UnboundedArray* array, int i, void* elem);
void* array_append(UnboundedArray* array, void* elem);

enum HashmapKeyType {
  HASHMAP_KEY_STRING = 1,
  HASHMAP_KEY_UINT32,
  HASHMAP_KEY_BIT,
};

typedef struct Hashmap {
  UnboundedArray entries;
  enum HashmapKeyType key_type;
  int capacity;
  int capacity_log2;
  int entry_count;
} Hashmap;

typedef struct HashmapKey {
  uint32_t h;
  union {
    uint8_t* str_key;
    uint8_t* bit_key;
    uint32_t int_key;
  };
  int keylen;
} HashmapKey;

typedef struct HashmapEntry {
  HashmapKey key;
  struct HashmapEntry* next_entry;
  void* datum;
} HashmapEntry;

typedef struct HashmapCursor {
  Hashmap* hashmap;
  int i;
  HashmapEntry* entry;
} HashmapCursor;

void          hashmap_create(Hashmap* hashmap, Arena* storage, enum HashmapKeyType type, int capacity, int max_capacity);
void          hashmap_hash_key(enum HashmapKeyType key_type, /* in/out */ HashmapKey* key, int length_log2);
HashmapEntry* _hashmap_get_entry(Hashmap* hashmap, HashmapKey* key, int entry_size);
#define       hashmap_get_entry(HASHMAP, KEY, TYPE)  (TYPE*)_hashmap_get_entry(HASHMAP, KEY, sizeof())
HashmapEntry* _hashmap_get_entry_uint32k(Hashmap* map, uint32_t int_key, int entry_size);
#define       hashmap_get_entry_uint32k(HASHMAP, KEY, TYPE)  (TYPE*)_hashmap_get_entry_uint32k(HASHMAP, KEY, sizeof(TYPE))
HashmapEntry* _hashmap_get_entry_stringk(Hashmap* map, char* str_key, int entry_size);
#define       hashmap_get_entry_stringk(HASHMAP, KEY, TYPE)  (TYPE*)_hashmap_get_entry_stringk(HASHMAP, KEY, sizeof(TYPE))
HashmapEntry* _hashmap_lookup_entry(Hashmap* hashmap, HashmapKey* key);
#define       hashmap_lookup_entry(HASHMAP, KEY, TYPE)  (TYPE*)_hashmap_lookup_entry(HASHMAP, KEY)
HashmapEntry* _hashmap_lookup_entry_uint32k(Hashmap* map, uint32_t int_key);
#define       hashmap_lookup_entry_uint32k(HASHMAP, KEY, TYPE)  (TYPE*)_hashmap_lookup_entry_uint32k(HASHMAP, KEY)
HashmapEntry* _hashmap_lookup_entry_stringk(Hashmap* map, char* str_key);
#define       hashmap_lookup_entry_stringk(HASHMAP, KEY, TYPE)  (TYPE*)_hashmap_lookup_entry_stringk(HASHMAP, KEY)
void          hashmap_cursor_reset(HashmapCursor* it, Hashmap* hashmap);
HashmapEntry* _hashmap_move_cursor(HashmapCursor* it);
#define       hashmap_move_cursor(CURSOR, TYPE)  (TYPE*)_hashmap_move_cursor(CURSOR)

