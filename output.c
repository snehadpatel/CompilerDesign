#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
int my_list[100] = {10, 20, 30}; int my_list_len = 3; 
printf("%d\n", my_list);

append(40);
printf("%d\n", my_list);
int val = my_list;
printf("%d\n", val);
return 0;
}