# Program to list even numbers from 1 to N

def list_even_numbers(n):
    """Return a list of even numbers from 1 to n."""
    if n < 2:
        return []  # No even numbers if n < 2
    return [num for num in range(2, n + 1, 2)]  # Step of 2 for efficiency

def main():
    try:
        # Take user input
        n = int(input("Enter a positive integer N: "))
        
        if n <= 0:
            print("Please enter a number greater than 0.")
            return
        
        even_numbers = list_even_numbers(n)
        
        if even_numbers:
            print(f"Even numbers from 1 to {n}: {even_numbers}")
        else:
            print(f"There are no even numbers between 1 and {n}.")
    
    except ValueError:
        print("Invalid input. Please enter an integer.")

if __name__ == "__main__":
    main()
