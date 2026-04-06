# Generate fibonacci sequence
n = 10
fib = [0, 1]
for i in range(2, n, 1):
    next_val = fib[i - 1] + fib[i - 2]
    fib.append(next_val)

for i in range(0, n, 1):
    val = fib[i]
    print(val)
