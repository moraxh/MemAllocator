#include "alloc.h"

// sbrk() can only increase or decrease its
// position we need a form to deallocate
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
    size_t size;
} Header;

typedef struct FreeList {
    Header * next;
} FreeList;

// Linked list of free blocks
Header * head;

// ------------------ FUNCTIONS --------------------------
// Align memory to 32 or 64 bits
size_t align(size_t size);

// Allocator
Header * requestMemory(size_t size);
Header * bestFitSearch(size_t size);
void * alloc(size_t size);

// Deallocator
void dealloc(void * addr);

// FreeList
FreeList * getNodeFreeList(Header * block);
void printFreeList();
void mergeFreeBlocks();
void splitFreeBlocks();
void appendToFreeList(Header * block);
void removeFromFreeList(Header * block);
// -------------------------------------------------------

// Align an "x" size to 64 bits or 32 bits
// Ex.
//     5 bytes  = 8 bytes
//     20 bytes = 24 bytes

size_t align(size_t size) {
    if( (size % SYSBYTES) != 0 ) { return ((size / SYSBYTES) + 1) * SYSBYTES; }
    return size;
}

// ----------------------------- ALLOCATOR -----------------------------------

// Try to move the brk if it failed returns null
Header * requestMemory(size_t size) {
    // Get current brk
    Header * addr = (Header *)sbrk(0);

    // If returns NULL is OOM(Out of Memory)
    if( sbrk(size) == (void *) - 1 ) { return NULL; }

    return addr;
}

Header * bestFitSearch(size_t size) {
    if (head == NULL) { return NULL; }

    Header * block = head;
    // Try to search a block of the exact size
    do {
        if(size == block->size) {
            removeFromFreeList(block);
            return block;
        }

        block = (Header *)getNodeFreeList(block)->next;
    }
    while(block != NULL);

    // Try to search a bigger block
    block = head;
    do {
        if(size < block->size) {
            removeFromFreeList(block);
            return block;
        }

        block = (Header *)getNodeFreeList(block)->next;
    }
    while(block != NULL);

    return NULL; // NULL means not found
}

void * alloc(size_t size) {
    // There is free blocks in linked free list?
    if (head != NULL) {
        Header * free_block = bestFitSearch(size);
        if (free_block != NULL) {
            printf("-- Allocating %ld bytes, in %p, Head: %p\n", size, (void *)free_block + sizeof(Header), free_block);
            return (void *)free_block + sizeof(Header);
        }
    }

    // If not create one
        // Request memory block
    Header * block = requestMemory( align(size + sizeof(Header)) );
    if (block == NULL) { return NULL; }

        // Set size
    block->size = align(size);

    // Print info
    printf("-- Allocating %ld bytes, in %p, Head: %p\n", size, (void *)block + sizeof(Header), block);

    return ((void *)block + sizeof(Header));
}
// ---------------------------------------------------------------------

// ----------------------------- FREELIST ------------------------------
FreeList * getNodeFreeList(Header * block) {
    return (FreeList *)((void *)block + sizeof(Header));
}

void printFreeList() {
    if (head == NULL) { return; }
    Header * block = head;

    printf("\n---FREELIST---\n");
    do {
        printf("Addr: %p, Size: %ld, Next: %p\n", block, block->size, getNodeFreeList(block)->next);
    } while((block = getNodeFreeList(block)->next) != NULL);
    printf("--------------\n");
}

void mergeFreeBlocks() {
    Header * block = head;
    Header * next = NULL;

    do {
        // If next block is next to actual block
        while ( ((void *)block + sizeof(Header) + block->size) == getNodeFreeList(block)->next ) {
            // Update size
            block->size = block->size + getNodeFreeList(block)->next->size + sizeof(Header);
            removeFromFreeList(getNodeFreeList(block)->next);
        }
    } while((block = getNodeFreeList(block)->next) != NULL);
}

void appendToFreeList(Header * block) {
    Header * actual_header = head;
    FreeList * block_freenode = getNodeFreeList(block);
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

    mergeFreeBlocks();
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

    // Search the block
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
    //                                 +------------------------------+ +-------------+
    //                                 |                              | |             |
    // +------------+------------------|-+    +-------+---------------v-|--+    +-----v------+--------------------+
    // | Prev Block | (FreeList *)->next |    | Block | (FreeList *)->next |    | Next Block | (FreeList *)->next |
    // +------------+--------------------+    +-------+--------------------+    +------------+--------------------+
    //
    getNodeFreeList(prev_block)->next = getNodeFreeList(getNodeFreeList(prev_block)->next)->next;
}
// ---------------------------------------------------------------------

// To "delete" a reserved memory space we are add
// it to a free linked list, this list will be
// stored inside the data block.
//
//  (Header *)first
//               |
//               +------+            +-----------------+             +------------> NULL
//                      |            |                 |             |
//                 +----v---+--------|---------+   +---v----+--------|---------+
//                 | Header | (FreeList *)Next |   | Header | (FreeList *)Next |
//                 +--------+------------------+   +--------+------------------+
//

// ---------------------------- DEALLOCATOR ----------------------------
void dealloc(void * addr) {
    Header * block = (Header *)(addr - sizeof(Header));
    printf( "Deallocating %p, Size = %ld\n", block, block->size );
    getNodeFreeList(block)->next = NULL;

    // If FreeList is empty
    if (head == NULL) {
        head = block;
        return;
    }

    appendToFreeList(block);
}
// ---------------------------------------------------------------------
