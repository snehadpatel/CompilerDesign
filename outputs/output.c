#include <stdio.h>

int main() {
    int x = 10;
    if (x > 5) {
        printf("%s\n", "Greater");
    }
    else {
        printf("%s\n", "This syntax error will crash the compiler properly now");
    }
    return 0;
}
