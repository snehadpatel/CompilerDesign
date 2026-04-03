#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static const char *source_code;
static int cursor = 0;
static int line = 1;
static int col = 1;

static int indent_stack[100];
static int indent_top = 0;
static int pending_dedents = 0;
static int expect_indent = 1; // 1 at start of file or after newline

void lexer_init(const char *source) {
    source_code = source;
    cursor = 0;
    line = 1;
    col = 1;
    indent_top = 0;
    indent_stack[0] = 0; // Base level
    pending_dedents = 0;
    expect_indent = 1;
}

static char peek() {
    return source_code[cursor];
}

static char advance() {
    char c = source_code[cursor++];
    if (c == '\n') {
        line++;
        col = 1;
    } else {
        col++;
    }
    return c;
}

static void skip_whitespace_and_comments() {
    while (1) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '#') {
            while (peek() != '\n' && peek() != '\0') advance();
        } else {
            break;
        }
    }
}

Token lexer_next_token() {
    while (1) {
        if (pending_dedents > 0) {
            pending_dedents--;
            Token t = {TOKEN_DEDENT, "DEDENT", line, col};
            return t;
        }

        // Handle indentation at start of line
        if (expect_indent) {
            int current_indent = 0;
            while (peek() == ' ' || peek() == '\t') {
                if (advance() == '\t') current_indent += 4; // Simplified tab handling
                else current_indent++;
            }

            // Skip empty lines or comment lines
            if (peek() == '\n' || peek() == '#') {
                if (peek() == '#') {
                    while (peek() != '\n' && peek() != '\0') advance();
                }
                if (peek() == '\n') advance();
                expect_indent = 1;
                continue; // Use loop instead of recursion
            } else if (peek() == '\0') {
                break; // Exit loop to handle EOF normally
            }

            expect_indent = 0;
            if (current_indent > indent_stack[indent_top]) {
                indent_stack[++indent_top] = current_indent;
                Token t = {TOKEN_INDENT, "INDENT", line, col};
                return t;
            } else if (current_indent < indent_stack[indent_top]) {
                while (indent_top > 0 && indent_stack[indent_top] > current_indent) {
                    indent_top--;
                    pending_dedents++;
                }
                pending_dedents--; // One DEDENT is returned now
                Token t = {TOKEN_DEDENT, "DEDENT", line, col};
                return t;
            }
        }
        break; // Exit loop if not returning an indent/dedent token
    }

    skip_whitespace_and_comments();

    char c = peek();
    if (c == '\0') {
        // Emit remaining DEDENTs at EOF
        if (indent_top > 0) {
            indent_top--;
            Token t = {TOKEN_DEDENT, "DEDENT", line, col};
            return t;
        }
        Token t = {TOKEN_EOF, "EOF", line, col};
        return t;
    }

    if (c == '\n') {
        advance();
        expect_indent = 1;
        Token t = {TOKEN_NEWLINE, "NEWLINE", line, col};
        return t;
    }

    if (isdigit(c)) {
        Token t = {TOKEN_NUMBER, "", line, col};
        int i = 0;
        while (isdigit(peek()) || peek() == '.') {
            t.text[i++] = advance();
        }
        t.text[i] = '\0';
        return t;
    }

    if (isalpha(c) || c == '_') {
        Token t = {TOKEN_IDENTIFIER, "", line, col};
        int i = 0;
        while (isalnum(peek()) || peek() == '_') {
            t.text[i++] = advance();
        }
        t.text[i] = '\0';

        // Check for keywords
        const char *keywords[] = {"def", "if", "elif", "else", "for", "in", "range", "while", "return", "print", "input", "int", "float", "str", "True", "False", "None", "and", "or", "not", "import", NULL};
        for (int k = 0; keywords[k]; k++) {
            if (strcmp(t.text, keywords[k]) == 0) {
                t.type = TOKEN_KEYWORD;
                break;
            }
        }
        return t;
    }

    if (c == '"' || c == '\'') {
        char quote = advance();
        Token t = {TOKEN_STRING, "", line, col};
        int i = 0;
        t.text[i++] = quote; // Keep quote
        // Check for triple quotes
        if (peek() == quote && source_code[cursor+1] == quote) {
            t.text[i++] = advance();
            t.text[i++] = advance();
            while (!(peek() == quote && source_code[cursor+1] == quote && source_code[cursor+2] == quote) && peek() != '\0') {
                t.text[i++] = advance();
            }
            if (peek() != '\0') { 
                t.text[i++] = advance(); 
                t.text[i++] = advance(); 
                t.text[i++] = advance(); 
            }
        } else {
            while (peek() != quote && peek() != '\0') {
                t.text[i++] = advance();
            }
            if (peek() == quote) t.text[i++] = advance();
        }
        t.text[i] = '\0';
        return t;
    }

    // Punctuation and Operators
    TokenType type = TOKEN_ERROR;
    char text[3] = {c, '\0', '\0'};
    advance();

    if (c == ':') type = TOKEN_COLON;
    else if (c == '(') type = TOKEN_LPAREN;
    else if (c == ')') type = TOKEN_RPAREN;
    else if (c == '[') type = TOKEN_LBRACKET;
    else if (c == ']') type = TOKEN_RBRACKET;
    else if (c == ',') type = TOKEN_COMMA;
    else if (c == '.') type = TOKEN_OPERATOR;
    else if (c == '=') {
        if (peek() == '=') {
            text[1] = advance();
            type = TOKEN_OPERATOR;
        } else {
            type = TOKEN_ASSIGN;
        }
    }
    else if (c == '<' || c == '>' || c == '!') {
        if (peek() == '=') {
            text[1] = advance();
        }
        type = TOKEN_OPERATOR;
    }
    else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%') {
        type = TOKEN_OPERATOR;
    }

    Token t = {type, "", line, col};
    strncpy(t.text, text, 255);
    return t;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_KEYWORD: return "KEYWORD";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_OPERATOR: return "OPERATOR";
        case TOKEN_INDENT: return "INDENT";
        case TOKEN_DEDENT: return "DEDENT";
        case TOKEN_NEWLINE: return "NEWLINE";
        case TOKEN_COLON: return "COLON";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_ASSIGN: return "ASSIGN";
        default: return "ERROR";
    }
}
