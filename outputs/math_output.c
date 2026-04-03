#include <stdio.h>
#include <math.h>


int main() {
    printf("Testing Math Library\n");
    float val = 16.0;
    double root = sqrt(val);
    printf("Square root of 16 is:\n");
    printf("%g\n", root);
    double p = pow(2.0, 3.0);
    printf("2 to the power 3 is:\n");
    printf("%g\n", p);
    return 0;
}
