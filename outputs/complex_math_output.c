#include <stdio.h>

int complex_calc(int a, int b) {
    int res = (a + b) * (a - b) / 2;
    return res;
}
int main() {
    float val_a = 15.5;
    float val_b = 4.5;
    int ans = complex_calc(val_a, val_b);
    printf("Calculation Result:\n");
    printf("%d\n", ans);
    return 0;
}
