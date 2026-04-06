def generate_c_code(functions, main_code, include_math=False, main_defined=False):

    code = []
    code.append("#include <stdio.h>")

    if include_math:
        code.append("#include <math.h>")

    code.append("")

    # FUNCTIONS FIRST
    for line in functions:
        code.append(line)

    code.append("")

    if not main_defined:
        code.append("")
        #  MAIN FUNCTION
        code.append("int main() {")

        for line in main_code:
            code.append("    " + line)

        code.append("    return 0;")
        code.append("}")
    else:
        # If main is already defined in functions, we skip wrapping
        # but still add any global main_code if it exists
        if main_code:
            code.append("")
            for line in main_code:
                code.append(line)

    return code