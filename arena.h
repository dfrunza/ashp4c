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
#define assert(expr) \
  do { if(!(expr)) assert_(#expr, __FILE__, __LINE__); } while(0)
void error_(char* file, int line, char* message, ...);
#define error(msg, ...) error_(__FILE__, __LINE__, (msg), ## __VA_ARGS__)
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
#define arena_push_struct(arena, type) ({ \
  type* o = arena_push(arena, sizeof(type)); \
  memset(o, 0, sizeof(type)); \
  o; \
})
void arena_delete(Arena* arena);

typedef struct DList {
  struct DList* next;
  struct DList* prev;
  void* object;
} DList;

typedef struct SList {
  struct SList* next;
  void* object;
} SList;

void dlist_concat(DList* tail, DList* head);
void slist_concat(SList* tail, SList* head);

// Max 1,048,575 elements
#define ARRAY_MAX_SEGMENT 20

typedef struct UnboundedArray {
  void* segment_table[ARRAY_MAX_SEGMENT];
  int elem_size;
  int elem_count;
  int capacity;
  Arena* storage;
} UnboundedArray;

void array_init(UnboundedArray* array, int elem_size, Arena* storage);
void* array_get(UnboundedArray* array, int i);
void* array_set(UnboundedArray* array, int i, void* elem);
void* array_append(UnboundedArray* array, void* elem);

enum HashmapKeyType {
  HASHMAP_KEY_STRING = 1,
  HASHMAP_KEY_BLOB,
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
    uint8_t* s_key;
    uint8_t* b_key;
    uint32_t i_key;
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

void hashmap_init(Hashmap* hashmap, enum HashmapKeyType type, int capacity_log2, Arena* storage);
void hashmap_hash_key(enum HashmapKeyType key_type, /*in/out*/ HashmapKey* key, int capacity_log2);
HashmapEntry* hashmap_get_or_create_entry(Hashmap* hashmap, HashmapKey* key);
HashmapEntry* hashmap_get_entry(Hashmap* hashmap, HashmapKey* key);
void hashmap_cursor_init(HashmapCursor* it, Hashmap* hashmap);
HashmapEntry* hashmap_move_cursor(HashmapCursor* it);

