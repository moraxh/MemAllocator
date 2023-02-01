#include "alloc.h"
#include <stdio.h>

int main() {
    int * a = alloc(4);
    printf("%d", *a);
}
