#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int arr[100] = {10, 20, 30, 40, 50};
    int arr_len = 5;
    int sliced[100];
    int sliced_len = 4 - (1);
    for (int i = 0; i < sliced_len; i++) { sliced[i] = arr[(1) + i]; }
    for (int i = 0; i < 3; i += 1) {
        int val = sliced[i];
        printf("%d\n", val);
    }
    return 0;
}
