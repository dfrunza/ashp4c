typedef uint32_t bool;
#define true 1u
#define false 0u
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

void     assert_(char* message, char* file, int line);
#define  assert(EXPR)  do { if(!(EXPR))  assert_(#EXPR, __FILE__, __LINE__); } while(0)
void     error_(char* file, int line, char* message, ...);
#define  error(MSG, ...)  error_(__FILE__, __LINE__, (MSG), ## __VA_ARGS__)
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

/*
 * n |  Capacity
 * 1 => 16
 * 2 => 48
 * 3 => 112
 * 4 => 240
 * 5 => 496
 * 6 => 1008
 * 7 => 2032
 * 8 => 4080
 * 9 => 8176
 * ...
 * C = (2^n - 1)*16
 */

typedef struct SegmentTable {
  int segment_count;
  void* segments[];
} SegmentTable;

typedef struct UnboundedArray {
  int elem_count;
  int capacity;
  SegmentTable data;
} UnboundedArray;

void* segment_locate_cell(SegmentTable* data, int i, int elem_size);
UnboundedArray* array_create(Arena* storage, int elem_size, int max_capacity);
void  array_init(UnboundedArray* array, Arena* storage, int elem_size, int segment_count);
void  array_extend(UnboundedArray* array, Arena* storage, int elem_size);
void* array_get_element(UnboundedArray* array, int i, int elem_size);
void* array_append_element(UnboundedArray* array, Arena* storage, int elem_size);

typedef struct HashmapEntry {
  char* key;
  void* value;
  struct HashmapEntry* next_entry;
} HashmapEntry;

typedef struct HashmapBucket {
  uint32_t h;
  HashmapEntry** entry_slot;
  int last_segment;
} HashmapBucket;

typedef struct Hashmap {
  int entry_count;
  int capacity;
  SegmentTable entries;
} Hashmap;

typedef struct HashmapCursor {
  Hashmap* hashmap;
  int i;
  HashmapEntry* entry;
} HashmapCursor;

Hashmap*      hashmap_create(Arena* storage, int max_capacity);
void          hashmap_init(Hashmap* hashmap, Arena* storage, int segment_count);
HashmapEntry* hashmap_lookup_entry(Hashmap* hashmap, char* key, HashmapBucket* bucket);
HashmapEntry* hashmap_insert_entry(Hashmap* hashmap, Arena* storage, char* key, void* value, bool nop_if_exists);
void          hashmap_cursor_begin(HashmapCursor* cursor, Hashmap* hashmap);
HashmapEntry* hashmap_cursor_next_entry(HashmapCursor* cursor);

typedef struct SetMember {
  struct SetMember* next;
  struct SetMember* left_branch;
  struct SetMember* right_branch;
  void* key;
  void* value;
} SetMember;

typedef struct Set {
  SetMember* first;
  SetMember* root;
} Set;

SetMember* set_lookup_member(Set* set, void* key);
SetMember* set_add_member(Set* set, Arena* storage, void* key, void* value);
SetMember* set_add_or_lookup_member(Set* set, Arena* storage, void* key, void* value);
Set*       set_open_inner_set(Set* set, Arena* storage, void* key);
void*      set_lookup_value(Set* set, void* key, void* default_);
int        set_members_to_array(Set* set, UnboundedArray* array, Arena* storage);
void       set_enumerate_members(Set* set, void (*visitor)(SetMember*));

