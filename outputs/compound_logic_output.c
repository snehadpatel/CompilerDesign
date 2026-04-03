#include <stdio.h>

int main() {
    int status1 =  1 ;
    int status2 =  0 ;
    if (status1 &&  ! status2) {
        printf("Logic evaluates to True\n");
    }
    int check = 15;
    if (check > 10 && check < 20) {
        printf("Safe zone\n");
    }
    else if (check <= 10 || check >= 20) {
        printf("Danger zone\n");
    }
    else {
        printf("Unknown\n");
    }
    return 0;
}
