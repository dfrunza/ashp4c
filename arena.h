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

struct PageBlock {
  struct PageBlock* next_block;
  struct PageBlock* prev_block;
  uint8_t* memory_begin;
  uint8_t* memory_end;
};

struct Arena {
  struct PageBlock* owned_pages;
  void* memory_avail;
  void* memory_limit;
};

struct ArenaUsage {
  int total;
  int free;
  int in_use;
  int arena_count;
};

void alloc_memory(int memory_amount);
void* arena_push(struct Arena* arena, uint32_t size);
#define arena_push_struct(arena, type) ({ \
  type* o = arena_push(arena, sizeof(type)); \
  memset(o, 0, sizeof(type)); \
  o; \
})
void arena_delete(struct Arena* arena);

struct ArenaUsage arena_get_usage(struct Arena* arena);
void arena_print_usage(struct Arena* arena, char* title);


struct ListLink {
  struct ListLink* prev;
  struct ListLink* next;
  void* object;
};

struct List {
  struct ListLink sentinel;
  struct ListLink* head;
  struct ListLink* tail;
  int link_count;
};

void list_init(struct List* list);
void list_append_link(struct List* list, struct ListLink* link);
struct ListLink* list_first_link(struct List* list);

// Max 1,048,575 elements
#define ARRAY_MAX_SEGMENT 20

struct UnboundedArray {
  void* segment_table[ARRAY_MAX_SEGMENT];
  int elem_size;
  int elem_count;
  int capacity;
  struct Arena* storage;
};

void array_init(struct UnboundedArray* array, int elem_size, struct Arena* storage);
void* array_get(struct UnboundedArray* array, int i);
void* array_set(struct UnboundedArray* array, int i, void* elem);
void* array_append(struct UnboundedArray* array, void* elem);

enum HashmapKeyType {
  HASHMAP_KEY_STRING = 1,
  HASHMAP_KEY_BLOB,
  HASHMAP_KEY_INT,
};

struct Hashmap {
  struct UnboundedArray entries;
  enum HashmapKeyType key_type;
  int capacity_log2;
  int capacity;
  int entry_count;
};

struct HashmapKey {
  uint32_t h;
  union {
    uint8_t* s_key;
    uint8_t* b_key;
    uint32_t i_key;
  };
  int keylen;
};

struct HashmapEntry {
  struct HashmapKey key;
  void* object;
  struct HashmapEntry* next_entry;
};

struct HashmapEntryIterator {
  struct Hashmap* hashmap;
  int i;
  struct HashmapEntry* entry;
};

void hashmap_init(struct Hashmap* hashmap, enum HashmapKeyType type, int capacity_log2, struct Arena* storage);
void hashmap_hash_key(enum HashmapKeyType key_type, /*in/out*/ struct HashmapKey* key, int capacity_log2);
struct HashmapEntry* hashmap_get_or_create_entry(struct Hashmap* hashmap, struct HashmapKey* key);
struct HashmapEntry* hashmap_get_entry(struct Hashmap* hashmap, struct HashmapKey* key);
void hashmap_iter_init(struct HashmapEntryIterator* it, struct Hashmap* hashmap);
struct HashmapEntry* hashmap_iter_next(struct HashmapEntryIterator* it);

