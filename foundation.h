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
} List;

void      list_create(List* list);
ListItem* list_first_item(List* list);
void      list_append_item(List* list, ListItem* item);

typedef struct UnboundedArray {
  void** segment_table;
  int segment_count;
  int elem_size;
  int elem_count;
  int capacity;
} UnboundedArray;

void  array_create(UnboundedArray* array, Arena* storage, int elem_size, int max_capacity);
void  array_extend(UnboundedArray* array, Arena* storage);
void* array_get(UnboundedArray* array, int i);
void* array_set(UnboundedArray* array, int i, void* elem);
void* array_append(UnboundedArray* array, Arena* storage, void* elem);

enum HashmapKeyType {
  HASHMAP_KEY_NONE = 0,
  HASHMAP_KEY_STRING,
  HASHMAP_KEY_BYTES,
  HASHMAP_KEY_UINT32,
  HASHMAP_KEY_UINT64,
};

typedef struct HashmapKey {
  uint32_t h;
  union {
    char*    str_key;
    uint8_t* bytes_key;
    uint32_t u32_key;
    uint64_t u64_key;
  };
  int keylen;
} HashmapKey;

typedef struct HashmapEntry {
  HashmapKey key;
  struct HashmapEntry* next_entry;
  void* value[];
} HashmapEntry;

typedef struct Hashmap {
  UnboundedArray entries;
  enum HashmapKeyType key_type;
  int capacity;
  int capacity_log2;
  int value_size;
  int entry_count;
} Hashmap;

typedef struct HashmapCursor {
  int i;
  HashmapEntry* entry;
} HashmapCursor;

void          hashmap_hash_key(enum HashmapKeyType key_type, /* in/out */ HashmapKey* key, int length_log2);
void          hashmap_create(Hashmap* hashmap, Arena* storage, enum HashmapKeyType key_type, int entry_size,
                  int capacity, int max_capacity);
HashmapEntry* hashmap_lookup_entry(Hashmap* hashmap, HashmapKey* key);
void*         hashmap_lookup(Hashmap* hashmap, enum HashmapKeyType key_type, ...);
HashmapEntry* hashmap_get_entry(Hashmap* hashmap, Arena* storage, HashmapKey* key);
void*         hashmap_get(Hashmap* hashmap, Arena* storage, enum HashmapKeyType key_type, ...);
void          hashmap_set(Hashmap* hashmap, Arena* storage, void* value, enum HashmapKeyType key_type, ...);
void          hashmap_cursor_begin(HashmapCursor* cursor);
HashmapEntry* hashmap_cursor_next_entry(HashmapCursor* cursor, Hashmap* hashmap);
void*         hashmap_cursor_next(HashmapCursor* cursor, Hashmap* hashmap);

