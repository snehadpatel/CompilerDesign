#include <stdio.h>

int is_prime(int n) {
    if (n < 2) {
        return 0;
    }
    int i = 2;
    while (i * i <= n) {
        if (n % i == 0) {
            return 0;
        }
        i = i + 1;
    }
    return 1;
}
int main() {
    for (int num = 2; num < 20; num += 1) {
        if (is_prime(num)) {
            printf("%d\n", num);
        }
    }
    return 0;
}
