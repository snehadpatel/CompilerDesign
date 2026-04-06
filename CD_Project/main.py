import os
from lexer import lexical_analysis
from syntax import syntax_analysis, detect_non_python
from semantic import semantic_analysis
from transformer import transform_to_c
from intermediate import generate_intermediate
from generator import generate_c_code
from optimizer import optimize_code


def run_compiler(code, filename):

    lines = code.split("\n")

    # Create output folder
    folder = f"outputs/{filename}"
    os.makedirs(folder, exist_ok=True)

    # STEP 1: Detect non-python (C-like) code
    non_python_errors = detect_non_python(lines)
    if non_python_errors:
        return non_python_errors[0], None

    # STEP 2: Syntax Analysis
    syntax_errors = syntax_analysis(lines)
    if syntax_errors:
        return syntax_errors[0], None

    # STEP 3: Lexical Analysis
    tokens, symbol_table = lexical_analysis(lines)

    # STEP 4: Semantic Analysis
    symbol_table = semantic_analysis(tokens, symbol_table)

    # STEP 5: Transformation (UPDATED)
    functions, main_code, include_math = transform_to_c(lines, symbol_table)

    #  STEP 6: Optimization
    functions = optimize_code(functions)
    main_code = optimize_code(main_code)

    #  STEP 7: Intermediate Representation
    ir = generate_intermediate(main_code)

    #  STEP 8: Code Generation (UPDATED)
    c_code = generate_c_code(functions, main_code, include_math)

    #  SAVE ALL PHASE OUTPUTS

    # Tokens
    with open(f"{folder}/tokens.txt", "w") as f:
        for t in tokens:
            f.write(str(t) + "\n")

    # Syntax
    with open(f"{folder}/syntax.txt", "w") as f:
        if syntax_errors:
            f.write("\n".join(syntax_errors))
        else:
            f.write("No syntax errors")

    # Symbol Table
    with open(f"{folder}/symbol_table.txt", "w") as f:
        f.write(str(symbol_table))

    # IR
    with open(f"{folder}/ir.txt", "w") as f:
        f.write("\n".join(ir))

    # Final C file
    path = f"{folder}/{filename}.c"
    with open(path, "w") as f:
        f.write("\n".join(c_code))

    return "\n".join(c_code), path