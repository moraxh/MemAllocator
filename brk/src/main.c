#include "alloc.h"

// TODO Split blocks

int main() {
    printf("PID: %d, BRK: %p\n", getpid(), sbrk(0));

    int *a = alloc(sizeof(a));
    int *b = alloc(sizeof(b));
    int *c = alloc(sizeof(c));
    int *d = alloc(sizeof(d));
    int *e = alloc(sizeof(e));

    *a = 1;
    *b = 2;
    *c = 3;
    *d = 4;
    *e = 5;

    dealloc(a);
    dealloc(b);
    dealloc(c);
    dealloc(d);
    dealloc(e);

    printFreeList();

    int *j = alloc(sizeof(j));
    *j = 15;

    printFreeList();
}
