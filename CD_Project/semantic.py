def semantic_analysis(token_stream, symbol_table):

    for i, (ttype, token) in enumerate(token_stream):

        if ttype == "IDENTIFIER":

            if i+2 < len(token_stream):
                next_token = token_stream[i+1][1]
                value = token_stream[i+2][1]

                if next_token == "=":

                    if value.isdigit():
                        symbol_table[token]["datatype"] = "int"

                    elif "." in value:
                        symbol_table[token]["datatype"] = "float"

                    elif value.startswith('"') or value.startswith("'"):
                        symbol_table[token]["datatype"] = "char*"

                    elif value.startswith("["):
                        symbol_table[token]["datatype"] = "array"

                    else:
                        symbol_table[token]["datatype"] = "int"

    return symbol_table