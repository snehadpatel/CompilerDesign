#include <stdio.h>

int main() {
    int val_int = 5;
    float val_float = 3.14;
    char* val_str = "Compiler Design";
    int val_str_len = 15;
    int val_derived = val_int;
    printf("%d\n", val_int);
    printf("%g\n", val_float);
    printf("%s\n", val_str);
    printf("%d\n", val_derived);
    return 0;
}
