#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    else {
        return n * factorial(n - 1);
    }
}
int main() {
    printf("%s\n", "Factorial Test");
    int x = 5;
    int res = factorial(x);
    printf("%d\n", res);
    return 0;
}
