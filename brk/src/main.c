#include "alloc.h"

int main() {
    printf("PID: %d, BRK: %p\n", getpid(), sbrk(0));

    int *a = alloc(sizeof(a));
    *a = 666;
    printf("%d\n", *a);

    dealloc(a);

    int *b = alloc(sizeof(a));
    *b = 616;
    printf("%d\n", *b);
}
