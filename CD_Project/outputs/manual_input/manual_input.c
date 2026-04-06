#include <stdio.h>

int list_even_numbers(int n) {
    if(n < 2) {
        // return {}  # No even numbers if n < 2 (List return not fully supported)
        return 0;
    }
    // return {num for num in range(2, n + 1, 2)}  # Step of 2 for efficiency (List return not fully supported)
    return 0;
}
int main() {
    // Take user input
    int n;
    scanf("%d", &n);
    if(n <= 0) {
        printf("%s\n", "Please enter a number greater than 0.");
        return;
    }
    int even_numbers = list_even_numbers(n);
    if(even_numbers) {
        printf("Even numbers from 1 to %d: %d\n", n, even_numbers);
    }
    else {
        printf("There are no even numbers between 1 and %d.\n", n);
    }
    printf("%s\n", "Invalid input. Please enter an integer.");
}


// Program to list even numbers from 1 to N