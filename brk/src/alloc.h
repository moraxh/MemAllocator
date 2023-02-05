#include <unistd.h>
#include <stdio.h>

// Check if is 32 or 64-bits system
#if __x86_64__ || __ppc64__
#define SYSBYTES 8
#else
#define SYSBYTES 4
#endif

// Align a size of memory to 32 or 64 bits for a faster memory access
size_t align(size_t size);

// Alloc Memory
void * alloc(size_t size);

// Dealloc Memory
void dealloc(void * addr);
