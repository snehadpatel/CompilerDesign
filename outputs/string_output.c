#include <stdio.h>

int main() {
    char* message = "Hello, Compiler Lab";
    int message_len = 19;
    printf("%s\n", message);
    int length = message_len;
    printf("%d\n", length);
    if (length > 20) {
        printf("%s\n", "Long message");
    }
    else if (length == 19) {
        printf("%s\n", "Medium message");
    }
    else {
        printf("%s\n", "Short message");
    }
    return 0;
}
