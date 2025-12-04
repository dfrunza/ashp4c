#pragma once
#include <stddef.h>

#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

void assert_(char* message, char* file, int line);
#define assert(expr) do { if(!(expr)) assert_(#expr, __FILE__, __LINE__); } while(0)
void error_(char* file, int line, char* message, ...);
#define error(msg, ...) error_(__FILE__, __LINE__, (msg), ## __VA_ARGS__)
#define container_of(member_ptr, container_type, member_name) \
    ( (container_type*)((char*)member_ptr - offsetof(container_type, member_name)) )
