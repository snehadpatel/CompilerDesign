# Python to C Converter

This project is a custom compiler tool designed to translate a subset of Python source code into semantically equivalent C code. It was developed to meet the requirements of Compiler Design Lab.

## Features Supported

The current implementation acts as a transpiler, successfully handling the following Python constructs:
- **I/O Operations**: Translates `print()` to `printf()` and integer/string `input()` to `scanf()`.
- **Indentation & Blocks**: Accurately tracks Python's indentation-based scoping and automatically generates C-style `{ }` block delimiters.
- **Control Flow**: Supports `if`, `elif`, `else` conditionals and `while` loop constructions.
- **For Loops**: Converts Python's `for i in range(start, end, step)` into standard C `for (int i = start; i < end; i += step)` loops.
- **Functions**: Handles function definitions (`def`) with parameter processing and `return` statements.
- **Lists (Arrays)**: Supports basic list initialization, element accessing via indexing (`list[i]`), and simulated appending using length tracking.
- **Code Beautification**: The generated output maintains proper C styling, managing consistent indentation and newline separations for readability.
- **Header Injection**: Automatically injects required C headers (`<stdio.h>`, `<stdlib.h>`, `<string.h>`).

## Prerequisites

- GCC (GNU Compiler Collection) or Clang
- Make

## Project Structure

- `src/` - Contains the source code for the transpiler
  - `lexer.c` & `lexer.h` - Tokenization and indentation tracking
  - `parser.c` & `parser.h` - Syntactic analysis and code generation logic
  - `codegen.c` & `codegen.h` - Output file formatting and writing utilities
  - `main.c` - Entry point and file reading handler
- `tests/` - Example Python scripts (`test_simple.py`, `test_loops.py`, `test_lists.py`)
- `Makefile` - Build configuration

## How to Build

To compile the transpiler executable, simply run the following command in the root of the project directory:

```bash
make
```

To clean the build files:

```bash
make clean
```

## How to Use

Once compiled, execute the generated `py2c` binary, passing the target Python source script as an argument:

```bash
./py2c tests/test_simple.py
```

The tool will process the script and generate a corresponding C file named `output.c` in the `outputs/` directory. You can then view or compile this output file:

```bash
cat outputs/output.c
gcc outputs/output.c -o my_c_program
./my_c_program
```

## Known Limitations

- Types are inferred basically; numerical variables default to `int`.
- Advanced list operations (like slicing or complex multi-type elements) are unsupported.
- Error handling is basic; deeply malformed Python syntax may cause parsing errors.
