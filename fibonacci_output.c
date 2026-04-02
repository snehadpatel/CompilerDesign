#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int n = 10;
    int fib[100] = {0, 1};
    int fib_len = 2;
    for (int i = 2; i < n; i += 11) {
        int next_val = fib[i - 1] + fib[i - 2];
        fib[fib_len++] = next_val;
    }
    for (int i = 0; i < n; i += 11) {
        int val = fib[i];
        printf("%d\n", val);
    }
    return 0;
}
