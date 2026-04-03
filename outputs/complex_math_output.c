#include <stdio.h>

int complex_calc(int a, int b) {
    int res = (a + b) * (a - b) / 2;
    return res;
}
int main() {
    int val_a = 15;
    int val_b = 4;
    int ans = complex_calc(val_a, val_b);
    printf("Calculation Result:\n");
    printf("%d\n", ans);
    return 0;
}
