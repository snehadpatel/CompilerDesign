#include <stdio.h>
#include <math.h>


int main() {
    printf("Testing Math Library\n");
    float val = 16.0;
    int root = sqrt(val);
    printf("Square root of 16 is:\n");
    printf("%d\n", root);
    int p = pow(2.0, 3.0);
    printf("2 to the power 3 is:\n");
    printf("%d\n", p);
    return 0;
}
