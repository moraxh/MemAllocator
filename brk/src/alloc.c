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

// Linked list of free blocks
Header * head;

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
void setUsed(Header * block, int used) {
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
int isUsed(Header * block) {
    return block->header & 1;
}

// ----------------------------------------------------------------
// Align a x size to 64 bits or 32 bits
// Ex.
// 64 bit system {
//     5 bytes  = 8 bytes
//     20 bytes = 24 bytes
//     39 btes  = 40 bytes
// }
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
    // if returns NULL is OOM(Out of Memory)
    if(sbrk(align(size + sizeof(Header))) == (void *) - 1) {
        return NULL;
    }

    return addr;
}

void * alloc(size_t size) {
    // There is free blocks?
    if (head != NULL) {
        // Seach throug linked list
    }

    Header * block = request_memory(size);
    block->header = align(size);
    setUsed(block, 1);

    return ((void *)block + sizeof(Header));
}

// Deallocates a block from a given pointer
typedef struct FreeList {
    Header * next;
} FreeList;

void printFreeList(Header * head) {
    if (head == NULL) {
        printf("Lista Vacia\n");
        return;
    }

    FreeList * s_node;
    do {
        s_node = (FreeList *)((void*)head + sizeof(Header));

        printf("Head: %p\n", head);
        printf("FreeList: %p\n", s_node);
        printf("Next: %p\n\n", s_node->next);
        head = s_node->next;
    }
    while(s_node->next != NULL);
    printf("------------------------------------\n");
}

// To "delete" a reserved memory space we are gonna set
// it state to unused, and add it to a free linked list,
// this list will be stored inside the data block
//
//  (Header *)first
//               |
//               +------+            +-----------------+             +------------> NULL
//                      |            |                 |             |
//                 +----▼---+--------|---------+   +---▼----+--------|---------+
//                 | Header | (FreeList *)Next |   | Header | (FreeList *)Next |
//                 +--------+------------------+   +--------+------------------+
//
void dealloc(void * addr) {
    // Set block parameters (unused) and create a node of the
    // free linked list
    Header * block = (Header *)(addr - sizeof(Header));
    FreeList * b_node = (FreeList *)((void *)block + sizeof(Header));
    b_node->next = NULL;
    setUsed(block, 0);

    // If FreeList is empty
    if (head == NULL) {
        head = block;
        return;
    }

    // Append to the sorted list
    Header * s_header = head;
    FreeList * s_node = NULL;

    do {
        s_node = (FreeList *)((void *)s_header + sizeof(Header));

        // Is block smaller than the first element?
        if (((void *)block < (void *)s_header) && s_header == head) {
            b_node->next = s_header;
            head = block;
            return;
        }

        // Is block direction bigger than the actual?
        if ((void *)block > (void *)s_header) {

            // Is the actual node the last one?
            if(s_node->next == NULL) {
                break;
            }

            // If direction is smaller than the next one
            if((void *)block < (void *)s_node->next) {
                // Insert between blocks
                b_node->next = s_node->next;
                s_node->next = block;
                return;
            }
        }

        s_header = s_node->next;
    }
    while(s_node->next != NULL); // This will never break the loop

    // Insert block to the tail
    s_node->next = block;
}
