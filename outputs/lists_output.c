#include <stdio.h>

int main() {
    int my_list[100] = {10, 20, 30};
    int my_list_len = 3;
    printf("%p\n", (void*)my_list);
    my_list[my_list_len++] = 40;
    printf("%p\n", (void*)my_list);
    int val = my_list[0];
    printf("%d\n", val);
    return 0;
}
