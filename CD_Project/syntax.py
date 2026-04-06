def syntax_analysis(lines):

    errors = []
    indent_stack = [0]

    for i, line in enumerate(lines, start=1):

        stripped = line.rstrip()

        if not stripped:
            continue

        indent = len(line) - len(line.lstrip())

        if indent > indent_stack[-1]:
            indent_stack.append(indent)
        else:
            while indent < indent_stack[-1]:
                indent_stack.pop()

            if indent != indent_stack[-1]:
                errors.append(f"Line {i}: Indentation Error")

        if stripped.startswith(("if", "for", "while", "def", "elif", "else")):
            if not stripped.endswith(":"):
                errors.append(f"Line {i}: Missing ':'")

    return errors


def detect_non_python(lines):

    errors = []

    for i, line in enumerate(lines, start=1):

        stripped = line.strip()

        if not stripped:
            continue

        # Ignore comments
        if stripped.startswith("#"):
            continue

        # Ignore strings (basic check)
        if stripped.startswith('"') or stripped.startswith("'"):
            continue

        # Detect real C patterns ONLY

        # printf / scanf
        if "printf(" in stripped or "scanf(" in stripped:
            errors.append(f"Line {i}: C function detected")
            continue

        # C-style semicolon at end (not inside Python)
        if stripped.endswith(";") and not stripped.startswith("for"):
            errors.append(f"Line {i}: C-style ';' detected")
            continue

        # Curly braces ONLY if standalone
        if stripped == "{" or stripped == "}":
            errors.append(f"Line {i}: C block syntax detected")
            continue

    return errors

