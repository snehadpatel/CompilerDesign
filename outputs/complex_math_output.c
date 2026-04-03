#include <stdio.h>

float complex_calc(float a, float b) {
    float res = (a + b) * (a - b) / 2;
    return res;
}
int main() {
    float val_a = 15.5;
    float val_b = 4.5;
    float ans = complex_calc(val_a, val_b);
    printf("Calculation Result:\n");
    printf("%g\n", ans);
    return 0;
}
