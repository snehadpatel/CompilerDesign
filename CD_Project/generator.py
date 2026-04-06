def generate_c_code(functions, main_code, include_math=False):

    code = []
    code.append("#include <stdio.h>")

    if include_math:
        code.append("#include <math.h>")

    code.append("")

    # FUNCTIONS FIRST
    for line in functions:
        code.append(line)

    code.append("")

    #  MAIN FUNCTION
    code.append("int main() {")

    for line in main_code:
        code.append("    " + line)

    code.append("    return 0;")
    code.append("}")

    return code