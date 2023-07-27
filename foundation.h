#pragma once

typedef uint32_t bool;
#define true 1u
#define false 0u
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

void    _assert(char* message, char* file, int line);
#define  assert(EXPR)  do { if(!(EXPR)) _assert(#EXPR, __FILE__, __LINE__); } while(0)
void    _error(char* file, int line, char* message, ...);
#define  error(MSG, ...)  _error(__FILE__, __LINE__, (MSG), ## __VA_ARGS__)
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
  int item_size;
  int item_count;
  Arena* storage;
} List;

void      _list_create(List* list, Arena* storage, int item_size);
#define    list_create(LIST, STORAGE, ITEM_TYPE)  _list_create(LIST, STORAGE, sizeof(ITEM_TYPE))
ListItem* _list_create_item(List* list);
#define    list_create_item(LIST, ITEM_TYPE)  ((ITEM_TYPE*)_list_create_item(LIST))
ListItem* _list_first_item(List* list);
#define    list_first_item(LIST, ITEM_TYPE)  ((ITEM_TYPE*)_list_first_item(LIST))
void       list_append_item(List* list, ListItem* item, int count);

typedef void* ArrayElement;

typedef struct UnboundedArray {
  ArrayElement* segment_table;
  int segment_length;
  int elem_size;
  int elem_count;
  int capacity;
  Arena* storage;
} UnboundedArray;

void         array_create(UnboundedArray* array, Arena* storage, int elem_size, int max_capacity);
void         array_extend(UnboundedArray* array);
ArrayElement array_get(UnboundedArray* array, int i);
ArrayElement array_set(UnboundedArray* array, int i, ArrayElement elem);
ArrayElement array_append(UnboundedArray* array, ArrayElement elem);

enum HashmapKeyType {
  HASHMAP_KEY_STRING = 1,
  HASHMAP_KEY_UINT32,
  HASHMAP_KEY_BYTES,
};

typedef struct Hashmap {
  UnboundedArray entries;
  enum HashmapKeyType key_type;
  int capacity;
  int capacity_log2;
  int entry_size;
  int entry_count;
} Hashmap;

typedef struct HashmapKey {
  uint32_t h;
  union {
    char*    str_key;
    uint8_t* bytes_key;
    uint32_t uint32_key;
  };
  int keylen;
} HashmapKey;

typedef struct HashmapEntry {
  HashmapKey key;
  struct HashmapEntry* next_entry;
} HashmapEntry;

typedef struct HashmapCursor {
  Hashmap* hashmap;
  int i;
  HashmapEntry* entry;
} HashmapCursor;

void          _hashmap_create(Hashmap* hashmap, Arena* storage, enum HashmapKeyType key_type, int entry_size,
                              int capacity, int max_capacity);
#define        hashmap_create(HASHMAP, STORAGE, KEY_TYPE, ENTRY_TYPE, CAPACITY, MAX_CAPACITY) \
  _hashmap_create(HASHMAP, STORAGE, KEY_TYPE, sizeof(ENTRY_TYPE), CAPACITY, MAX_CAPACITY)
HashmapEntry* _hashmap_lookup_entry(Hashmap* hashmap, HashmapKey* key);
HashmapEntry* _hashmap_lookup_entry_va(Hashmap* hashmap, enum HashmapKeyType key_type, ...);
#define        hashmap_lookup_entry(HASHMAP, KEY_TYPE, KEY, ENTRY_TYPE) \
  ((ENTRY_TYPE*)_hashmap_lookup_entry_va(HASHMAP, KEY_TYPE, KEY))
HashmapEntry* _hashmap_get_entry(Hashmap* hashmap, HashmapKey* key);
HashmapEntry* _hashmap_get_entry_va(Hashmap* hashmap, enum HashmapKeyType key_type, ...);
#define        hashmap_get_entry(HASHMAP, KEY_TYPE, KEY, ENTRY_TYPE) \
  ((ENTRY_TYPE*)_hashmap_get_entry_va(HASHMAP, KEY_TYPE, KEY))
void           hashmap_cursor_reset(HashmapCursor* it, Hashmap* hashmap);
HashmapEntry* _hashmap_move_cursor(HashmapCursor* it);
#define        hashmap_move_cursor(CURSOR, ENTRY_TYPE)  ((ENTRY_TYPE*)_hashmap_move_cursor(CURSOR))
void           hashmap_hash_key(enum HashmapKeyType key_type, /* in/out */ HashmapKey* key, int length_log2);

