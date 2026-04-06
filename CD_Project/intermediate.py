def generate_intermediate(lines):

    ir = []

    for line in lines:

        if "for" in line:
            ir.append("FOR_LOOP")
        elif "while" in line:
            ir.append("WHILE_LOOP")
        elif "if" in line:
            ir.append("IF")
        elif "printf" in line:
            ir.append("PRINT")
        else:
            ir.append("STATEMENT")

    return ir