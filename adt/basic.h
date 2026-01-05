#pragma once
#include <stddef.h>
#include <stdint.h>

#define KILOBYTE 1024
#define MEGABYTE 1024 * KILOBYTE

void assert_(char* message, char* file, int line);
#define assert(expr) do { if(!(expr)) assert_(#expr, __FILE__, __LINE__); } while(0)

void error_(char* file, int line, char* message, ...);
#define error(msg, ...) error_(__FILE__, __LINE__, (msg), ## __VA_ARGS__)

template<class T, class M>
static inline constexpr ptrdiff_t offset_of(const M T::*member) {
    return reinterpret_cast<ptrdiff_t>( &( reinterpret_cast<T*>(0)->*member ) );
}

template<class T, class M>
static inline constexpr T* owner_of(M *ptr, const M T::*member) {
    return reinterpret_cast<T*>( reinterpret_cast<intptr_t>(ptr) - offset_of(member) );
}
