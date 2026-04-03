#include <stdio.h>

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    else {
        return n * factorial(n - 1);
    }
}
int main() {
    printf("Factorial Test\n");
    int x = 5;
    int res = factorial(x);
    printf("%d\n", res);
    return 0;
}
