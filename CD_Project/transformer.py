import re

def transform_to_c(lines, symbol_table):

    functions = []
    main_code = []

    indent_stack = [0]
    include_math = False
    inside_function = False

    for line in lines:

        stripped = line.strip()

        if not stripped:
            continue

        if stripped.startswith("#"):
            continue

        indent = len(line) - len(line.lstrip())

        # Close blocks
        while indent < indent_stack[-1]:
            indent_stack.pop()
            if inside_function:
                functions.append("}")
            else:
                main_code.append("}")

        # FUNCTION DEFINITION
        if stripped.startswith("def"):
            inside_function = True
            name = stripped.split()[1].split("(")[0]
            params = stripped.split("(")[1].split(")")[0]

            param_list = []
            if params:
                for p in params.split(","):
                    param_list.append(f"float {p.strip()}")

            functions.append(f"float {name}({', '.join(param_list)})" + " {")
            indent_stack.append(indent + 4)

        # RETURN
        elif stripped.startswith("return"):
            ret_val = stripped.replace("return", "").strip()
            if inside_function:
                functions.append(f"return {ret_val};")
            else:
                main_code.append(f"return {ret_val};")

        # INPUT
        elif "input(" in stripped:
            var = stripped.split("=")[0].strip()
            main_code.append(f"int {var};")
            main_code.append(f'scanf("%d", &{var});')

        # PRINT
        elif stripped.startswith("print"):
            content = stripped.replace("print(", "").rstrip(")")
            if content.startswith("f"):
                content = content[1:].strip('"').strip("'")
                variables = re.findall(r"\{(.*?)\}", content)
                format_str = re.sub(r"\{.*?\}", "%f", content)
                line = f'printf("{format_str}\\n", {", ".join(variables)});'
            else:
                line = f'printf("%d\\n", {content});'

            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)

        # WHILE LOOP
        elif stripped.startswith("while"):
            cond = stripped.replace("while", "").replace(":", "").strip()
            line = f"while({cond}) {{"
            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)
            indent_stack.append(indent + 4)

        # += 
        elif "+=" in stripped:
            var, val = stripped.split("+=")
            line = f"{var.strip()} += {val.strip()};"
            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)

        # //=
        elif "//=" in stripped:
            var, val = stripped.split("//=")
            line = f"{var.strip()} /= {val.strip()};"
            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)

        # FIXED: Modulo assignment (only when % = is used for assignment)
        elif "%=" in stripped:                     # Changed from "%" in stripped and "=" in stripped
            var, expr = stripped.split("%=")
            line = f"{var.strip()} %= {expr.strip()};"
            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)

        # POWER **
        elif "**" in stripped:
            include_math = True
            var = stripped.split("=")[0].strip()
            base, power = stripped.split("=")[1].split("**")
            line = f"float {var} = pow({base.strip()}, {power.strip()});"
            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)

        # SQRT
        elif "sqrt(" in stripped:
            include_math = True
            main_code.append(stripped + ";")

        # FUNCTION CALL
        elif "(" in stripped and ")" in stripped and "=" not in stripped:
            main_code.append(stripped + ";")

        # LIST ASSIGNMENT (Basic)
        elif stripped.startswith("[") and stripped.endswith("]") and "=" in stripped:
            var = stripped.split("=")[0].strip()
            values = stripped.split("=")[1].strip()[1:-1]
            line = f"int {var}[] = {{{values}}};"
            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)

        # FOR LOOP
        elif stripped.startswith("for "):
            if "range(" in stripped:
                # Simple: for i in range(n):
                match = re.search(r"for\s+(\w+)\s+in\s+range\((.+)\):", stripped)
                if match:
                    var = match.group(1)
                    limit = match.group(2)
                    line = f"for(int {var}=0; {var}<{limit}; {var}++) {{"
                else:
                    line = f"// Unsupported for: {stripped}"
            else:
                line = f"// Unsupported for: {stripped}"

            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)
            indent_stack.append(indent + 4)

        # IF / ELIF / ELSE
        elif stripped.startswith(("if ", "elif ", "else")):
            if stripped.startswith("if "):
                cond = stripped.replace("if", "").replace(":", "").strip()
                line = f"if({cond}) {{"
            elif stripped.startswith("elif "):
                cond = stripped.replace("elif", "").replace(":", "").strip()
                line = f"else if({cond}) {{"
            else:
                line = "else {"
            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)
            indent_stack.append(indent + 4)

        # BREAK / CONTINUE
        elif stripped == "break":
            if inside_function:
                functions.append("break;")
            else:
                main_code.append("break;")
        elif stripped == "continue":
            if inside_function:
                functions.append("continue;")
            else:
                main_code.append("continue;")

        # TRY / EXCEPT (Basic)
        elif stripped.startswith("try:"):
            if inside_function:
                functions.append("// try: (C has no direct equivalent)")
            else:
                main_code.append("// try: (C has no direct equivalent)")
            indent_stack.append(indent + 4)
        elif stripped.startswith("except"):
            if inside_function:
                functions.append("// except: block")
            else:
                main_code.append("// except: block")

        # GENERAL ASSIGNMENT (Catch-all - must be at the end)
        elif "=" in stripped:
            line = "float " + stripped + ";"
            if inside_function:
                functions.append(line)
            else:
                main_code.append(line)

    # Close all blocks
    while len(indent_stack) > 1:
        indent_stack.pop()
        if inside_function:
            functions.append("}")
        else:
            main_code.append("}")

    return functions, main_code, include_math