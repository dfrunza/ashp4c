#pragma once

#define global static
#define internal static
#define external extern
#define true 1u
#define false 0u
#define bool uint32_t
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

void assert_(char* message, char* file, int line);
#define assert(EXPR) \
  do { if(!(EXPR)) assert_(#EXPR, __FILE__, __LINE__); } while(0)
void error_(char* file, int line, char* message, ...);
#define error(MSG, ...) error_(__FILE__, __LINE__, (MSG), ## __VA_ARGS__)
bool cstr_is_letter(char c);
bool cstr_is_digit(char c, int base);
bool cstr_is_ascii_printable(char c);
bool cstr_is_whitespace(char c);
int cstr_len(char* str);
char* cstr_copy(char* dest_str, char* src_str);
void cstr_copy_substr(char* dest_str, char* begin_char, char* end_char);
bool cstr_start_with(char* str, char* prefix);
bool cstr_match(char* str_a, char* str_b);
void cstr_print_substr(char* begin_char, char* end_char);
bool bytes_match(uint8_t* bytes_a, int len_a, uint8_t* bytes_b, int len_b);
int floor_log2(int x);
int ceil_log2(int x);

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

void alloc_memory(int memory_amount);
void* arena_push(Arena* arena, uint32_t size);
#define arena_push_struct(ARENA, TYPE) ({ \
  TYPE* o = arena_push(ARENA, sizeof(TYPE)); \
  memset(o, 0, sizeof(TYPE)); \
  o; \
})
void arena_delete(Arena* arena);

typedef struct ListItem {
  struct ListItem* next;
  struct ListItem* prev;
  void* object;
} ListItem;

typedef struct List {
  ListItem sentinel;
  ListItem* last_item;
  int item_count;
} List;

void list_reset(List* list);
ListItem* list_first_item(List* list);
void list_append_item(List* list, ListItem* item, int count);

// Max 2,048 elements
#define ARRAY_MAX_SEGMENT 11

typedef struct UnboundedArray {
  void* segment_table[ARRAY_MAX_SEGMENT];
  int elem_size;
  int elem_count;
  int capacity;
  Arena* storage;
} UnboundedArray;

void array_create(UnboundedArray* array, int elem_size, Arena* storage);
void* array_get(UnboundedArray* array, int i);
void* array_set(UnboundedArray* array, int i, void* elem);
void* array_append(UnboundedArray* array, void* elem);

enum HashmapKeyType {
  HASHMAP_KEY_STRING = 1,
  HASHMAP_KEY_BIT,
  HASHMAP_KEY_UINT32,
};

typedef struct Hashmap {
  UnboundedArray entries;
  enum HashmapKeyType key_type;
  int capacity_log2;
  int capacity;
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
  void* object;
  struct HashmapEntry* next_entry;
} HashmapEntry;

typedef struct HashmapCursor {
  Hashmap* hashmap;
  int i;
  HashmapEntry* entry;
} HashmapCursor;

void hashmap_create(Hashmap* hashmap, enum HashmapKeyType type, int capacity_log2, Arena* storage);
void hashmap_hash_key(enum HashmapKeyType key_type, /*in/out*/ HashmapKey* key, int capacity_log2);
HashmapEntry* hashmap_get_entry(Hashmap* hashmap, HashmapKey* key);
HashmapEntry* hashmap_get_entry_uint32(Hashmap* map, uint32_t int_key);
HashmapEntry* hashmap_get_entry_string(Hashmap* map, char* str_key);
HashmapEntry* hashmap_lookup_entry(Hashmap* hashmap, HashmapKey* key);
HashmapEntry* hashmap_lookup_entry_uint32(Hashmap* map, uint32_t int_key);
HashmapEntry* hashmap_lookup_entry_string(Hashmap* map, char* str_key);
void hashmap_cursor_reset(HashmapCursor* it, Hashmap* hashmap);
HashmapEntry* hashmap_move_cursor(HashmapCursor* it);

