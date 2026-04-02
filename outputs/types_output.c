#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int val_int = 5;
    float val_float = 3.14;
    char* val_str = "Compiler Design";
    int val_derived = val_int;
    printf("%d\n", val_int);
    printf("%d\n", val_float);
    printf("%d\n", val_str);
    printf("%d\n", val_derived);
    return 0;
}
