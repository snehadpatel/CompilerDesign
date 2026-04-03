#include <stdio.h>

int main() {
    int my_list[100] = {10, 20, 30};
    int my_list_len = 3;
    printf("[");
    for (int i = 0; i < my_list_len; i++) {
        printf("%d", my_list[i]);
        if (i < my_list_len - 1) printf(", ");
    }
    printf("]\n");
    my_list[my_list_len++] = 40;
    printf("[");
    for (int i = 0; i < my_list_len; i++) {
        printf("%d", my_list[i]);
        if (i < my_list_len - 1) printf(", ");
    }
    printf("]\n");
    int val = my_list[0];
    printf("%d\n", val);
    return 0;
}
