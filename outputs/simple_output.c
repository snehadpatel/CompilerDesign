#include <stdio.h>

int greet(int name) {
    printf("%s\n", "Hello ");
    printf("%d\n", name);
}
int main() {
    int val; 
    printf("%s", "Enter value: ");
    scanf("%d", &val);
    if (val > 10) {
        printf("%s\n", "Greater than 10");
    }
    else {
        printf("%s\n", "Small");
    }
    greet(val);
    return 0;
}
