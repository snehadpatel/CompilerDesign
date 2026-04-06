#include <stdio.h>


int main() {
    int n;
    scanf("%d", &n);
    float i = 1;
    while(i <= n) {
    if(i == 5) {
    break;
    }
    if(i % 2 == 0) {
    i += 1;
    continue;
    }
    printf("%d\n", i);
    i += 1;
    }
    return 0;
}