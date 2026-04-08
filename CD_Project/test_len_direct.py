from transformer import transform_to_c

code = '''greeting = "Hello"
print(greeting)

name = "World"
n = len(name)
print(n)'''

lines = code.split('\n')
functions, main_code, include_math, main_defined = transform_to_c(lines, {})

print("Generated C code:")
for line in main_code:
    print(line)
