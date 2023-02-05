#include <unistd.h>
#include <stdio.h>

// Check if is 32 or 64-bits system
#if __x86_64__ || __ppc64__
#define SYSBYTES 8
#else
#define SYSBYTES 4
#endif

// Alloc Memory
void * alloc(size_t size);

// Dealloc Memory
void dealloc(void * addr);
