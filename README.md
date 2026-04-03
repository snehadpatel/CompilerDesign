# Python-to-C Transpilation System

A robust, dependency-free Python-to-C transpiler developed for the Compiler Design Laboratory. This system implements a full compilation pipeline—from a custom Lexical Analyzer to a Recursive Descent Parser—without the use of external tools like Flex or Bison.

## Technical Architecture

### 1. Lexical Analysis (Scanner)
The Lexer handles the transformation of raw source code into a stream of tokens. A key technical achievement is the **Indentation Tracker**, which uses a stack-based approach to convert Python's whitespace-based scoping into standard C `TOKEN_INDENT` and `TOKEN_DEDENT` markers.

### 2. Syntax Analysis (Parser)
The Parser uses a **Recursive Descent** strategy to validate the token stream against a Python-subset grammar. It performs:
- **Type Inference**: Analyzes assignments to dynamically determine C variable types (`int`, `float`, `char*`).
- **Two-Pass Symbol Scanning**: A preliminary pass identifies module imports and variable declarations to ensure safe header inclusion and scope management.

### 3. Code Generation
The system generates optimized C code that bridges Pythonic abstractions with C-native performance:
- **Array Slicing**: Implemented via replicated inline loops for memory safety.
- **Iterative Printing**: Custom logic to format Python iterables into bracketed C sequences.
- **Memory Optimization**: Minimal header inclusion using conditional detection logic.

## Key Features
- **Conditional Header Inclusion**: Only includes `<math.h>` or `<string.h>` when explicitly required.
- **Built-in Primitive Support**: Native mapping for `len()`, `ord()`, and `chr()`.
- **Compound Boolean Logic**: High-fidelity mapping of `and`, `or`, and `not` keywords.
- **Fixed-Size Dynamic Arrays**: Support for list-append operations with static length tracking.

## Build and Execution

### Build System
The project is managed via a standard Makefile. To build the transpiler:
```bash
make
```

### Conversion Pipeline
To translate a Python source file to C:
```bash
./py2c tests/test_simple.py
```
Outputs are generated in the `outputs/` directory.

### Native Compilation
Compile the generated C output using GCC:
```bash
gcc outputs/simple_output.c -o system_bin
./system_bin
```

---
**Course:** Compiler Design Laboratory  
**Scope:** Python-to-C Transpilation System
