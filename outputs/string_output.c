#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char* message = "Hello, Compiler Lab";
    printf("%d\n", message);
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
