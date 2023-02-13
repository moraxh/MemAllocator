#include "alloc.h"

// TODO
// Merge free blocks
// Split free blocks
// Test recent changes

// Since sbrk() can only increase or decrease
// its position we need a form to deallocate
// blocks that are between allocated blocks
// for this we need to use a header to store
// if its used and its size
//
// Allocated Block
// +-------------------------------------+
// |   Header   |          Data          |
// +-------------------------------------+
//
// Deallocated Block
// +-------------------------------------+
// |   Header   |      (FreeList *)      |
// +-------------------------------------+

typedef struct Header {
    size_t info; // Store size and used state
} Header;

typedef struct FreeList {
    Header * next;
} FreeList;

// Linked list of free blocks
Header * head;

// ------------------ FUNCTIONS --------------------------
// Header operations
void setUsed(Header * block, int isused);
size_t getSize(Header * block);
int isUsed(Header * block);
size_t align(size_t size);

// Allocator
Header * requestMemory(size_t size);
void * alloc(size_t size);

// Deallocator
Header * bestFitSearch(size_t size);
FreeList * getNodeFreeList(Header * block);
void appendToFreeList(Header * block, FreeList * block_freenode);
void removeFromFreeList(Header * block);
void dealloc(void * addr);
// -------------------------------------------------------


// ------------------------- HEADER OPERATIONS -------------------------------
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
void setUsed(Header * block, int isused) {
    if (isused) {
        block->info |= 1;
    }
    else {
        block->info &= ~1;
    }
}

size_t getSize(Header * block) {
    return block->info & ~1;
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
    return block->info & 1;
}

// Align a x size to 64 bits or 32 bits
// Ex.
//     5 bytes  = 8 bytes
//     20 bytes = 24 bytes
size_t align(size_t size) {

    if((size % SYSBYTES) != 0) {
        return ((size / SYSBYTES) + 1) * SYSBYTES;
    }
    else {
        return size;
    }
}
// ---------------------------------------------------------------------------

// ----------------------------- ALLOCATOR -----------------------------------
Header * requestMemory(size_t size) {
    // Get current memory direction
    Header * addr = (Header *)sbrk(0);

    // if returns NULL is OOM(Out of Memory)
    if(sbrk(align(size + sizeof(Header))) == (void *) - 1) {
        return NULL;
    }

    return addr;
}

Header * bestFitSearch(size_t size) {
    if (head == NULL) { return NULL; }

    Header * actual_header = head;
    // Try to search a block of the exact size
    do {
        if(size == getSize(actual_header)) {
            return actual_header;
        }

        actual_header = (Header *)getNodeFreeList(actual_header)->next;
    }
    while(actual_header != NULL);

    // Try to search a bigger block
    actual_header = head;
    do {
        if(size < getSize(actual_header)) {
            return actual_header;
        }

        actual_header = (Header *)getNodeFreeList(actual_header)->next;
    }
    while(actual_header != NULL);

    return NULL; // NULL means not found
}

void * alloc(size_t size) {
    // There is free blocks in linked free list?
    if (head != NULL) {
        Header * free_block = bestFitSearch(size);
        if (free_block != NULL) {
            return free_block + sizeof(Header);
        }
    }

    Header * block = requestMemory(size);
    if (block == NULL) { return NULL; }

    block->info = align(size);
    setUsed(block, 1);

    return ((void *)block + sizeof(Header));
}
// ---------------------------------------------------------------------


// ---------------------------- DEALLOCATOR ----------------------------
FreeList * getNodeFreeList(Header * block) {
    return (FreeList *)((void *)block + sizeof(Header));
}

void appendToFreeList(Header * block, FreeList * block_freenode) {
    Header * actual_header = head;
    FreeList * actual_freenode = NULL;

    do {
        actual_freenode = getNodeFreeList(actual_header);

        // Is block smaller than the first element?
        if (((void *)block < (void *)actual_header) && actual_header == head) {
            block_freenode->next = actual_header;
            head = block;
            return;
        }

        // Is block direction bigger than the actual?
        if ((void *)block > (void *)actual_header) {

            // Is the actual node the last one?
            if(actual_freenode->next == NULL) {
                break;
            }

            // If direction is smaller than the next one
            if((void *)block < (void *)actual_freenode->next) {
                // Insert between blocks
                block_freenode->next = actual_freenode->next;
                actual_freenode->next = block;
                return;
            }
        }

        actual_header = actual_freenode->next;
    }
    while(actual_freenode->next != NULL); // This will never break the loop

    // Insert block to the tail
    actual_freenode->next = block;
}

// Remove a block from free list
void removeFromFreeList(Header * block) {
    // Free List only has one element
    if(head == block && getNodeFreeList(block)->next == NULL) {
        head = NULL;
        return;
    }

    Header * actual_node = head;
    Header * prev_block = NULL;

    do {
        if(getNodeFreeList(actual_node)->next == block) {
            prev_block = actual_node;
        }
        actual_node = getNodeFreeList(actual_node)->next;
    }
    while(actual_node != NULL);

    if (prev_block == NULL) { return; }

    // If block is the tail of the linked list
    if (getNodeFreeList(prev_block)->next == NULL) {
        getNodeFreeList(prev_block)->next = NULL;
        return;
    }

    // Link prev block to the next of the current block
    //
    //
    //                                 +--------------------------------+ +---------------
    //                                 |                                | |              |
    // +-------------+-----------------|--+    +--------+---------------v-|--+    +------v------+--------------------+
    // |  Prev Block | (FreeList *)->next |    |  Block | (FreeList *)->next |    |  Next Block | (FreeList *)->next |
    // +-------------+--------------------+    +--------+--------------------+    +-------------+--------------------+
    getNodeFreeList(prev_block)->next = getNodeFreeList(getNodeFreeList(prev_block)->next)->next;
}


// To "delete" a reserved memory space we are gonna set
// it state to unused, and add it to a free linked list,
// this list will be stored inside the data block
//
//  (Header *)first
//               |
//               +------+            +-----------------+             +------------> NULL
//                      |            |                 |             |
//                 +----v---+--------|---------+   +---v----+--------|---------+
//                 | Header | (FreeList *)Next |   | Header | (FreeList *)Next |
//                 +--------+------------------+   +--------+------------------+
//
void dealloc(void * addr) {
    // Set block parameters (unused) and create a node of the
    // free linked list
    Header * block = (Header *)(addr - sizeof(Header));
    FreeList * block_freenode = getNodeFreeList(block);
    block_freenode->next = NULL;
    setUsed(block, 0);

    // If FreeList is empty
    if (head == NULL) {
        head = block;
        return;
    }

    appendToFreeList(block, block_freenode);
}
