#include "alloc.h"

// Since sbrk() can only increase or decrease
// its position we need a form to deallocate
// blocks that are between allocated blocks
// for this we need to use a header to store
// if its used and its size
// +-------------------------------------+
// |   Header   |          Data          |
// +-------------------------------------+
typedef struct Header{
    // It'll store codified size and used state
    size_t header;

} Header;

// We will use last digit of a size in binary to store if its used or not
// Ex.
// 8 bytes = 0000 1000
//                   ^
//                   This 0 represents if the block is used or not
//
// For this the program use bitwise operators to change that digit
// If its used
// 1 = Used         1 = 0000 0001
// ~1 = Free       ~1 = 1111 1110
//
// Ex. Imagine we will store 16 bytes
// To set the state of used use | bitwise operator
//
// 16 bytes | 1 byte
// 16 bytes = 0001 0000
// 1 byte   = 0000 0001
//           ‾‾‾‾‾‾‾‾‾‾‾
//            0001 0001
//
// To change it state to free we use & bitwise operator with ~1
// 16B | 1B = 0001 0001
// ~1       = 1111 1110
//           ‾‾‾‾‾‾‾‾‾‾‾
//            0001 0000 = 16 bytes
//
// As you see this operation returns the original size so we can use this to check the size even if its used or not
inline void setUsed(Header * block, int used) {
    if (used) {
        block->header |= 1;
    }
    else {
        block->header &= ~1;
    }
}

size_t getSize(Header * block) {
    return block->header & ~1;
}

// To check if it used or not we'll use & bitwise operator
// (From the above example)
//
// Used = 0001 0001
// Free = 0001 0000
//
// var & 1
// Used = 0001 0001
// 1    = 0000 0001
//       ‾‾‾‾‾‾‾‾‾‾‾
//        0000 0001 = 1 = Its used
//
// var & 1
// Free = 0001 0000
// 1    = 0000 0001
//       ‾‾‾‾‾‾‾‾‾‾‾
//        0000 0000 = Not used
//
inline int isUsed(Header * block) {
    return block->header & 1L;
}


// ----------------------------------------------------------------
// Align a x size to 64 bits or 32 bits
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
    Header * addr = (Header *)sbrk(0);

    // Check if memory can be allocated
    // NULL is Out of Memory
    if(sbrk(align(size + sizeof(Header))) == (void *) - 1) {
        return NULL;
    }

    return addr;
}

void * alloc(size_t size) {
}

