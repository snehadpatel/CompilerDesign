#include <stdio.h>

int main() {
    char* message = "Hello, Compiler Lab";
    int message_len = 19;
    printf("%s\n", message);
    int length = message_len;
    printf("%d\n", length);
    if (length > 20) {
        printf("Long message\n");
    }
    else if (length == 19) {
        printf("Medium message\n");
    }
    else {
        printf("Short message\n");
    }
    return 0;
}
