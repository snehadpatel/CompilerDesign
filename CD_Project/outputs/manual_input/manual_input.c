#include <stdio.h>



int main() {
    int x;
    scanf("%d", &x);
    int y;
    scanf("%d", &y);
    if(x > y) {
        printf("%d is greater than %d\n", x, y);
    }
    else if(x < y) {
        printf("%d is greater than %d\n", y, x);
    }
    else {
        printf("%s\n", "Numbers are equal");
    }
    return 0;
}