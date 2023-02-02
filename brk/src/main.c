#include "alloc.h"
#include <stdio.h>

int main() {
    int * a = alloc(4);
    int * b = alloc(7);
    printf("%d", *b);
    printf("%d", *a);
}
