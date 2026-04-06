import re

def get_c_type(value, var_types={}):
    value = value.strip()
    # String literal
    if value.startswith('"') or value.startswith("'"):
        return "char*"
    
    # Known variable
    if value in var_types:
        return var_types[value]
    
    # Float literals
    if "." in value:
        return "float"
        
    # Function call detection: starts with identifier directly followed by (
    # But NOT if it's just a parenthesized expression starting with (
    func_match = re.match(r'^([a-zA-Z_]\w*)\s*\(', value)
    if func_match:
        func_name = func_match.group(1)
        if func_name in var_types:
            return var_types[func_name]
        return "float"

    # Check variables in the expression
    for var, v_type in var_types.items():
        if re.search(r'\b' + re.escape(var) + r'\b', value) and v_type == "float":
            return "float"
            
    return "int"

def transform_to_c(lines, symbol_table):
    functions = []
    main_code = []
    indent_stack = [0]
    include_math = False
    inside_function = False
    main_defined = False
    var_types = {}

    def add_line(content, is_statement=True):
        # Prepend indentation based on stack level (excluding the root stack which is 0)
        # Note: indent_stack has [0, 4, 8, ...]
        # We want (len(indent_stack) - 1) * 4 spaces
        level = len(indent_stack) - (1 if not is_statement else 0)
        if level < 0: level = 0
        
        # Don't double-indent in functions if we're in the main_code block
        # Actually, let's just use the current stack level
        spaces = "    " * (len(indent_stack) - 1)
        
        # If it's a statement, ensure it ends with semicolon unless it's a block opener or comment
        if is_statement and content and not content.endswith(("{", "}", "//")):
            if not content.startswith("//"):
                content += ";"
        
        if inside_function:
            functions.append(spaces + content)
        else:
            main_code.append(spaces + content)

    for line in lines:
        raw_indent = len(line) - len(line.lstrip())
        stripped = line.strip()
        
        # 1. Handle Comments first
        if stripped.startswith("#"):
            comment = stripped.replace("#", "//", 1)
            add_line(comment, is_statement=False)
            continue
        
        if not stripped:
            continue

        # 2. Close blocks if indent decreases
        while raw_indent < indent_stack[-1]:
            indent_stack.pop()
            add_line("}", is_statement=False)
            if len(indent_stack) == 1:
                inside_function = False

        # 3. Ignore specific Python constructs
        if stripped.startswith(("try:", "except ", "except:", 'if __name__ == "__main__":')):
            # We treat these as transparent blocks (don't indent, just process body at current level)
            continue
        
        if "__main__" in stripped and "main()" in stripped and not stripped.startswith("def"):
            # Skip the caller of main if it's inside if __name__
            continue

        # 4. FUNCTION DEFINITION
        if stripped.startswith("def "):
            inside_function = True
            name = stripped.split()[1].split("(")[0]
            
            if name == "main":
                main_defined = True
                add_line("int main() {", is_statement=False)
            else:
                params = stripped.split("(")[1].split(")")[0]
                param_list = []
                if params:
                    for p in params.split(","):
                        p_name = p.strip()
                        param_list.append(f"int {p_name}")
                        var_types[p_name] = "int"
                
                var_types[name] = "int" # Default return type
                add_line(f"int {name}({', '.join(param_list)}) {{", is_statement=False)
            
            indent_stack.append(raw_indent + 4) # This is a guestimate of next level

        # 5. RETURN
        elif stripped.startswith("return"):
            ret_val = stripped.replace("return", "").replace("[", "{").replace("]", "}").strip()
            if not ret_val: 
                add_line("return")
            else:
                # Basic list placeholder for user
                if "{" in ret_val:
                    add_line(f"// return {ret_val} (List return not fully supported)")
                    add_line("return 0")
                else:
                    add_line(f"return {ret_val}")

        # 6. INPUT
        elif "input(" in stripped:
            var = stripped.split("=")[0].strip()
            if "int(input" in stripped:
                var_types[var] = "int"
                add_line(f"int {var}")
                add_line(f'scanf("%d", &{var})')
            else:
                var_types[var] = "float"
                add_line(f"float {var}")
                add_line(f'scanf("%f", &{var})')

        # 7. PRINT
        elif stripped.startswith("print"):
            content = stripped.replace("print(", "").rstrip(")")
            # Handle comments inside print if any (rare)
            if "#" in content: content = content.split("#")[0].strip()

            if content.startswith("f"):
                content = content[1:].strip('"').strip("'")
                variables = re.findall(r"\{(.*?)\}", content)
                
                specifiers = []
                for v in variables:
                    v = v.strip()
                    v_type = var_types.get(v, "int" if v.isdigit() else "float")
                    if v_type == "int": specifiers.append("%d")
                    elif v_type == "char*": specifiers.append("%s")
                    else: specifiers.append("%f")
                
                format_str = content
                for spec in specifiers:
                    format_str = re.sub(r"\{.*?\}", spec, format_str, count=1)
                
                add_line(f'printf("{format_str}\\n", {", ".join(variables)})')
            else:
                content = content.strip()
                if content.startswith('"') or content.startswith("'"):
                    add_line(f'printf("%s\\n", {content})')
                elif "[" in content:
                    add_line(f'// printf("%s", {content}) (List print not fully supported)')
                else:
                    v_type = var_types.get(content, get_c_type(content, var_types))
                    spec = "%d" if v_type == "int" else "%f"
                    add_line(f'printf("{spec}\\n", {content})')

        # 8. WHILE / IF / ELIF / ELSE
        elif stripped.startswith(("while ", "if ", "elif ", "else")):
            if stripped.startswith("while "):
                cond = stripped.replace("while", "").replace(":", "").strip()
                add_line(f"while({cond}) {{", is_statement=False)
            elif stripped.startswith("if "):
                cond = stripped.replace("if", "").replace(":", "").strip()
                add_line(f"if({cond}) {{", is_statement=False)
            elif stripped.startswith("elif "):
                cond = stripped.replace("elif", "").replace(":", "").strip()
                add_line(f"else if({cond}) {{", is_statement=False)
            else:
                add_line("else {", is_statement=False)
            indent_stack.append(raw_indent + 4)

        # 9. FOR LOOP
        elif stripped.startswith("for "):
            match = re.search(r"for\s+(\w+)\s+in\s+range\((.+)\):", stripped)
            if match:
                var, limit = match.group(1), match.group(2)
                var_types[var] = "int"
                add_line(f"for(int {var}=0; {var}<{limit}; {var}++) {{", is_statement=False)
                indent_stack.append(raw_indent + 4)
            else:
                add_line(f"// for {stripped} (Complex loops not supported)")

        # 10. ASSIGNMENTS
        elif "=" in stripped:
            if "+=" in stripped: op = "+="
            elif "-=" in stripped: op = "-="
            elif "*=" in stripped: op = "*="
            elif "/=" in stripped: op = "/="
            elif "%=" in stripped: op = "%="
            else: op = "="

            parts = stripped.split(op)
            var = parts[0].strip()
            expr = parts[1].strip()

            if "**" in expr:
                include_math = True
                base, pwr = expr.split("**")
                expr = f"pow({base.strip()}, {pwr.strip()})"
                var_types[var] = "float"

            # Check for list comprehensions (simple case)
            if "[" in expr and "for" in expr:
                add_line(f"// {var} {op} {expr} (Comprehensions not supported)")
                continue
            
            if var not in var_types and op == "=":
                v_type = get_c_type(expr, var_types)
                var_types[var] = v_type
                add_line(f"{v_type} {var} = {expr}")
            else:
                add_line(f"{var} {op} {expr}")

    # Close remaining blocks
    while len(indent_stack) > 1:
        indent_stack.pop()
        add_line("}", is_statement=False)

    return functions, main_code, include_math, main_defined