import re
import keyword

KEYWORDS = set(keyword.kwlist)

OPERATORS = {
    "+", "-", "*", "/", "%", "=", "==", "!=", ">", "<", ">=", "<=",
    "+=", "-=", "*=", "/=", "%=",
    "and", "or", "not", "in", "is"
}

SYMBOLS = {
    "(", ")", "{", "}", "[", "]",
    ":", ",", ".", ";"
}

def tokenize(line):
    tokens = re.findall(r'"[^"]*"|\'[^\']*\'|\w+|==|!=|>=|<=|[^\s\w]', line)
    return tokens

def lexical_analysis(lines):
    token_stream = []
    symbol_table = {}

    for line in lines:

        line = line.strip()

        if line.startswith("#"):
            continue

        tokens = tokenize(line)

        for token in tokens:

            if token in KEYWORDS:
                token_stream.append(("KEYWORD", token))

            elif token in OPERATORS:
                token_stream.append(("OPERATOR", token))

            elif token in SYMBOLS:
                token_stream.append(("SYMBOL", token))

            elif re.match(r'^\d+(\.\d+)?$', token):
                token_stream.append(("CONSTANT", token))

            elif re.match(r'^".*"$|^\'.*\'$', token):
                token_stream.append(("STRING", token))

            elif re.match(r'^[a-zA-Z_]\w*$', token):
                token_stream.append(("IDENTIFIER", token))

                if token not in symbol_table:
                    symbol_table[token] = {
                        "type": "variable",
                        "value": None
                    }

    return token_stream, symbol_table