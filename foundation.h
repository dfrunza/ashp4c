#include <stddef.h>

typedef uint32_t bool;
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

void assert_(char* message, char* file, int line);
#define assert(expr) do { if(!(expr)) assert_(#expr, __FILE__, __LINE__); } while(0)
void error_(char* file, int line, char* message, ...);
#define error(msg, ...) error_(__FILE__, __LINE__, (msg), ## __VA_ARGS__)
/* macro define offsetof(type, field) ( (int) (((type*)0)->field) ) */
#define container_of(member_ptr, container_type, member_name) \
    ( (container_type*)((char*)member_ptr - offsetof(container_type, member_name)) )

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

void reserve_memory(int amount);
void* arena_malloc(Arena* arena, uint32_t size);
void arena_free(Arena* arena);

/**
 * n  ...  segment count
 * C  ...  capacity (max. nr. of elements)
 *
 * n |  C
 * --+-----
 * 1 | 16
 * 2 | 48
 * 3 | 112
 * 4 | 240
 * 5 | 496
 * 6 | 1008
 * 7 | 2032
 * 8 | 4080
 * 9 | 8176
 * ...
 *
 * C(n) = (2^n - 1)*16
 **/

typedef struct SegmentTable {
  int segment_count;
  void* segments[];
} SegmentTable;

typedef struct Array {
  int elem_count;
  int capacity;
  SegmentTable data;
} Array;

void* segment_locate_cell(SegmentTable* data, int i, int elem_size);
Array* array_create(Arena* storage, int elem_size, int segment_count);
void array_init(Arena* storage, Array* array, int elem_size, int segment_count);
void array_extend(Arena* storage, Array* array, int elem_size);
void* array_get(Array* array, int i, int elem_size);
void* array_append(Arena* storage, Array* array, int elem_size);

typedef struct StrmapEntry {
  char* key;
  void* value;
  struct StrmapEntry* next_entry;
} StrmapEntry;

typedef struct StrmapBucket {
  uint32_t h;
  StrmapEntry** entry_slot;
  int last_segment;
} StrmapBucket;

typedef struct Strmap {
  int entry_count;
  int capacity;
  SegmentTable entries;
} Strmap;

typedef struct StrmapCursor {
  Strmap* strmap;
  int i;
  StrmapEntry* entry;
} StrmapCursor;

Strmap* strmap_create(Arena* storage, int segment_count);
void strmap_init(Arena* storage, Strmap* strmap, int segment_count);
void* strmap_lookup(Strmap* strmap, char* key, StrmapEntry** entry, StrmapBucket* bucket);
StrmapEntry* strmap_insert(Arena* storage, Strmap* strmap, char* key, void* value, bool return_if_found);
void strmap_cursor_begin(StrmapCursor* cursor, Strmap* strmap);
StrmapEntry* strmap_cursor_next(StrmapCursor* cursor);

typedef struct MapEntry {
  struct MapEntry* next;
  struct MapEntry* left_branch;
  struct MapEntry* right_branch;
  void* key;
  void* value;
} MapEntry;

typedef struct Map {
  MapEntry* first;
  MapEntry* root;
} Map;

MapEntry* map_insert(Arena* storage, Map* map, void* key, void* value, bool return_if_found);
void* map_lookup(Map* map, void* key, MapEntry** entry);
int map_count(Map* map);

