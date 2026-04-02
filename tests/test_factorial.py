def factorial(n):
    if n <= 1:
        return 1
    else:
        return n * factorial(n - 1)

print("Factorial Test")
x = 5
res = factorial(x)
print(res)
