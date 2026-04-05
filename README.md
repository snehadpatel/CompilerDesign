# 🚀 Python-to-C Transpilation System

[![Streamlit App](https://static.streamlit.io/badges/streamlit_badge.svg)](https://compilerdesign-xo2vsa4vnmguh6cqbhrwbd.streamlit.app)
![Python](https://img.shields.io/badge/Python-3.8+-3776AB?style=flat&logo=python&logoColor=white)
![C](https://img.shields.io/badge/C-Language-A8B9CC?style=flat&logo=c&logoColor=white)
![GCC](https://img.shields.io/badge/Compiler-GCC-004482?style=flat&logo=gnu)
![Makefile](https://img.shields.io/badge/Build-Makefile-blue?style=flat)

A sophisticated, dependency-free Python-to-C transpiler engineered for high-fidelity source-to-source translation. This system implements a comprehensive compilation pipeline—from a custom Lexical Analyzer to a Recursive Descent Parser—built from the ground up without external parser generators like Flex or Bison.

---

## 🌐 Try it Online!

The transpiler is live and ready for use! Convert your Python code to C instantly via the web interface:

👉 **[Launch the Transpiler UI](https://compilerdesign-xo2vsa4vnmguh6cqbhrwbd.streamlit.app)**

---

## 🛠️ Technical Architecture

### 1. Lexical Analysis (Scanner)
The Lexer performs high-speed tokenization with a specialized **Indentation Tracker**. Using a stack-based algorithm, it converts Python's whitespace-dependent scoping into explicit `TOKEN_INDENT` and `TOKEN_DEDENT` markers, ensuring perfect block-level parity with C's brace-based scope.

### 2. Syntax Analysis (Parser)
Our **Recursive Descent Parser** validates the token stream against a Python-subset grammar. Advanced features include:
- **Recursive Type Inference**: Dynamically resolves variable types (`int`, `float`, `char*`) by analyzing assignment chains.
- **Symbol Table Management**: A two-pass scanning system that identifies global declarations and module imports to automate header management.

### 3. Progressive Code Generation
The system translates Pythonic high-level abstractions into performant, native C:
- **Array Slicing**: Implemented via inline memory operations for safety and speed.
- **Iterative Printing**: Intelligent formatting of various Python types into standard C `printf` calls.
- **Static Memory Profiling**: Conditional inclusion of `<math.h>`, `<string.h>`, and `<stdio.h>` to keep binaries lean.

---

## ✨ Key Capabilities

- **Compound Logic**: Native mapping of complex boolean expressions (`and`, `or`, `not`).
- **Python Built-ins**: Direct support for `len()`, `ord()`, and `chr()`.
- **Dynamic Array Simulation**: Support for `list.append()` mechanics within fixed-size static buffers.
- **Semantic Mapping**: Seamless translation of `if-elif-else` and `for/while` loops.

---

## 📂 Project Structure

```text
.
├── src/               # Core Transpiler Logic (Lexer, Parser, Codegen)
├── ui/                # Streamlit Web Interface
│   ├── app.py         # Main UI Application
│   └── bridge.py      # Python-to-C Execution Wrapper
├── tests/             # Comprehensive Test Suite (.py cases)
├── outputs/           # Generated C Source Files
├── Makefile           # CLI Build System
├── requirements.txt   # UI Python Dependencies
└── run_ui.sh          # Local UI Launch Script
```

---

## 🚀 Getting Started

### Method A: Web Interface (Recommended)
1. Navigate to the [Online Transpiler](https://compilerdesign-xo2vsa4vnmguh6cqbhrwbd.streamlit.app).
2. Paste your Python code or upload a `.py` file.
3. Download the generated `.c` code instantly.

### Method B: Local Command Line (CLI)
Build the transpiler from source:
```bash
make
```
Convert a file:
```bash
./py2c tests/test_simple.py
```

### Method C: Local Web UI
Run the Streamlit interface on your machine:
```bash
bash run_ui.sh
```

---

**Course:** Compiler Design Laboratory  
**Scope:** Python-to-C Transpilation System  
**Developed by:** Sneha Patel
