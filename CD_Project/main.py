import os
from transformer import transform_to_c
from generator import generate_c_code

def run_compiler(code, filename):
    lines = code.split("\n")

    # Create output folder
    folder = f"outputs/{filename}"
    os.makedirs(folder, exist_ok=True)

    # STEP 1: Transformation (Simplified Rule-Based)
    # Passing an empty symbol table for compatibility
    functions, main_code, include_math, main_defined = transform_to_c(lines, {})

    # STEP 2: Code Generation
    c_code = generate_c_code(functions, main_code, include_math, main_defined)

    # SAVE THE FINAL OUTPUTS
    # Final C file
    path = f"{folder}/{filename}.c"
    with open(path, "w") as f:
        f.write("\n".join(c_code))

    return "\n".join(c_code), path