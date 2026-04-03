#include <stdio.h>

int main() {
    int height = 5;
    for (int i = 0; i < height; i += 1) {
        int row_count = i + 1;
        for (int j = 0; j < row_count; j += 1) {
            printf("%d\n", j);
        }
        printf("Row complete\n");
    }
    printf("Pyramid complete\n");
    return 0;
}
