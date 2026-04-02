#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int my_list[100] = {10, 20, 30};
    int my_list_len = 3;
    int n = my_list_len;
    printf("%d\n", n);
    char* c = 'A';
    int val = (int)(c);
    printf("%d\n", val);
    int ch = (char)(66);
    printf("%d\n", ch);
    return 0;
}
