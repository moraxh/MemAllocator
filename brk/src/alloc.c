#include "alloc.h"

// Since sbrk() can only increase or decrease we can't
// "free" a block that is between two blocks we are
// gonna use a header to establish it's size, a pointer
// to the next block and a "boolean" that indicate if its
// free or not.

// +-------------------------------------+
// |            |                        |
// |   Header   |          Data          |
// |            |                        |
// +-------------------------------------+
typedef struct Header{
    // Size of the data
    size_t size;

    // This space is free?
    char is_free;

    // Next Header
    struct Header * next;
} Header;

Header * first = NULL;
Header * last = NULL;

// ----------------------------------------------------------------
// Align a size to 64 bits or 32 bits
size_t align(size_t size) {

    if((size % SYSBYTES) != 0) {
        return ((size / SYSBYTES) + 1) * SYSBYTES;
    }
    else {
        return size;
    }
}

// ----------------------------------------------------------------
// Allocs a space of given memory

Header * request_memory(size_t size) {
    // Get current memory direction
    Header * dir = (Header *)sbrk(0);

    // Check if memory can be allocated
    // NULL is Out of Memory
    if(sbrk(align(size + sizeof(Header))) == (void *) - 1) {
        return NULL;
    }

    return dir;
}

void * alloc(size_t size) {
    Header * block = request_memory(size);
    // Set data
    block->size = align(size);
    block->is_free = 0;
    block->next = NULL;


    if(first == NULL) {
        first = block;
        last = block;
    }
    else {
        last->next = block;
        last = block;
    }

    return block + align(size);
}

// ----------------------------------------------------------------
// Dealloc Memory

void dealloc(void * addr) {

}
