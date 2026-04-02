# Python to C Converter

This is a custom transpiler I built for my Compiler Design Lab. It translates a subset of Python code into C code. It doesn't use any external parsing libraries, just a custom lexer and recursive descent parser.

## What it can do:
- **Print and Inputs**: Translates `print()` to `printf()` and `input()` to `scanf()`.
- **Formatting**: Automatically converts Python's indentation into proper C `{ }` blocks.
- **Loops & Conditionals**: Supports `if`, `elif`, `else`, `while`, and `for` loops (using `range`).
- **Functions**: Handles `def` with parameters and `return`.
- **Lists**: Basic fixed-size arrays, indexing, and an `.append()` equivalent using dynamic length tracking.
- **Built-in functions**: Has custom mapping for `len()`, `ord()`, and `chr()`.

## Files
- `src/lexer.c` & `lexer.h`: Tokenizer that handles the indentation tracking.
- `src/parser.c` & `parser.h`: The main recursive descent logic and code generation.
- `src/codegen.c` & `codegen.h`: Output file formatting logic.
- `src/main.c`: Opens the file and starts the parser.
- `tests/`: A few example python scripts I used for testing.
- `Makefile`: To build the project.

## How to run it

First, make sure you have GCC and Make installed.
If you're on Mac/Linux, you probably already have them or can get them via xcode tools.
If you're on Windows, you can use WSL or install MinGW.

**1. Build the compiler:**

If you are on Mac/Linux (or WSL):
```bash
make
```

If you are using MinGW natively on Windows:
```cmd
mingw32-make
```

**2. Run it:**

Just pass a python file to the generated executable:

Mac/Linux/WSL:
```bash
./py2c tests/test_simple.py
```

Windows CMD:
```cmd
py2c.exe tests\test_simple.py
```

This will parse the Python code and generate an equivalent C file in the `outputs/` folder (named `outputs/output.c`).

**3. Compile the output C code:**

You can then compile the generated C code and run it normally:

Mac/Linux:
```bash
gcc outputs/output.c -o my_c_program
./my_c_program
```

Windows CMD:
```cmd
gcc outputs\output.c -o my_c_program.exe
my_c_program.exe
```

## Limitations
- Only infers simple types, everything mostly defaults to `int`.
- Advanced list operations like slicing won't work.
- It's a lab assignment, so error handling for really broken Python code is pretty barebones.
