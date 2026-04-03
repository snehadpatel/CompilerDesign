#include <stdio.h>

int greet(int name) {
    printf("Hello \n");
    printf("%d\n", name);
}
int main() {
    int val; 
    printf("Enter value: ");
    scanf("%d", &val);
    if (val > 10) {
        printf("Greater than 10\n");
    }
    else {
        printf("Small\n");
    }
    greet(val);
    return 0;
}
