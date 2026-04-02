#include "parser.h"
#include "lexer.h"
#include "codegen.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static Token current_token;
static int inside_main = 0;
static int main_started = 0;

static void advance_token() {
    current_token = lexer_next_token();
    // printf("Token: %s [%s] at line %d\n", token_type_to_string(current_token.type), current_token.text, current_token.line);
}

static void match(TokenType type) {
    if (current_token.type == type) {
        advance_token();
    } else {
        printf("Error: Expected %s but got %s [%s] at line %d\n", 
               token_type_to_string(type), token_type_to_string(current_token.type), 
               current_token.text, current_token.line);
        advance_token();
    }
}

static void parse_expression(char *buf);

static void parse_print() {
    match(TOKEN_KEYWORD); // print
    match(TOKEN_LPAREN);
    
    char expr[128] = {0};
    if (current_token.type == TOKEN_STRING) {
        strcpy(expr, current_token.text);
        advance_token();
        codegen_write("printf(\"%%s\\n\", %s);", expr);
    } else {
        parse_expression(expr);
        codegen_write("printf(\"%%d\\n\", %s);", expr);
    }
    
    match(TOKEN_RPAREN);
}

static void parse_input(const char *var_name) {
    if (strcmp(current_token.text, "int") == 0) {
        match(TOKEN_KEYWORD); // int
        match(TOKEN_LPAREN);
        if (strcmp(current_token.text, "input") == 0) {
            match(TOKEN_KEYWORD); // input
            match(TOKEN_LPAREN);
            if (current_token.type == TOKEN_STRING) {
                codegen_write("printf(\"%%s\", %s); ", current_token.text);
                advance_token();
            }
            match(TOKEN_RPAREN);
        }
        match(TOKEN_RPAREN);
        codegen_write("scanf(\"%%d\", &%s);", var_name);
    } else if (strcmp(current_token.text, "input") == 0) {
        match(TOKEN_KEYWORD); // input
        match(TOKEN_LPAREN);
        if (current_token.type == TOKEN_STRING) {
            codegen_write("printf(\"%%s\", %s); ", current_token.text);
            advance_token();
        }
        match(TOKEN_RPAREN);
        codegen_write("scanf(\"%%s\", %s);", var_name);
    }
}

static void parse_expression(char *buf) {
    while (current_token.type == TOKEN_NUMBER || current_token.type == TOKEN_IDENTIFIER || 
           current_token.type == TOKEN_STRING || current_token.type == TOKEN_LPAREN || 
           current_token.type == TOKEN_KEYWORD || current_token.type == TOKEN_OPERATOR) {
        
        if (current_token.type == TOKEN_KEYWORD) {
            if (strcmp(current_token.text, "and") == 0) strcat(buf, " && ");
            else if (strcmp(current_token.text, "or") == 0) strcat(buf, " || ");
            else if (strcmp(current_token.text, "not") == 0) strcat(buf, " ! ");
            else if (strcmp(current_token.text, "True") == 0) strcat(buf, " 1 ");
            else if (strcmp(current_token.text, "False") == 0) strcat(buf, " 0 ");
            else break; // Not part of expr
            advance_token();
        } else if (current_token.type == TOKEN_OPERATOR) {
            strcat(buf, " ");
            strcat(buf, current_token.text);
            strcat(buf, " ");
            advance_token();
        } else if (current_token.type == TOKEN_LPAREN) {
            strcat(buf, "(");
            advance_token();
            parse_expression(buf);
            if (current_token.type == TOKEN_RPAREN) {
                strcat(buf, ")");
                advance_token();
            }
        } else {
            // Identifier or literal
            strcat(buf, current_token.text);
            advance_token();
        }

        // Handle commas for list elements or function arguments in expr (simplified)
        if (current_token.type == TOKEN_COMMA) {
            strcat(buf, ", ");
            advance_token();
        }
        
        // Stop if we hit symbols that end an expression in this context
        if (current_token.type == TOKEN_COLON || current_token.type == TOKEN_NEWLINE || 
            current_token.type == TOKEN_EOF || current_token.type == TOKEN_RPAREN) break;
    }
}

static void parse_block() {
    match(TOKEN_COLON);
    match(TOKEN_NEWLINE);
    match(TOKEN_INDENT);
    codegen_write(" {");
    codegen_newline();
    
    while (current_token.type != TOKEN_DEDENT && current_token.type != TOKEN_EOF) {
        if (current_token.type == TOKEN_KEYWORD) {
            if (strcmp(current_token.text, "if") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("if (");
                char cond[128] = {0};
                parse_expression(cond);
                codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "elif") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("else if (");
                char cond[128] = {0};
                parse_expression(cond);
                codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "else") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("else");
                parse_block();
            } else if (strcmp(current_token.text, "while") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("while (");
                char cond[128] = {0};
                parse_expression(cond);
                codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "return") == 0) {
                match(TOKEN_KEYWORD);
                char expr[128] = {0};
                parse_expression(expr);
                codegen_write("return %s;", expr);
                codegen_newline();
            } else if (strcmp(current_token.text, "for") == 0) {
                match(TOKEN_KEYWORD);
                char var[64]; strcpy(var, current_token.text); advance_token();
                match(TOKEN_KEYWORD); // in
                match(TOKEN_KEYWORD); // range
                match(TOKEN_LPAREN);
                char start[32] = "0", end[32] = "0", step[32] = "1";
                char arg1[32] = {0}; parse_expression(arg1);
                if (current_token.type == TOKEN_COMMA) {
                    advance_token();
                    strcpy(start, arg1);
                    parse_expression(end);
                    if (current_token.type == TOKEN_COMMA) {
                        advance_token();
                        parse_expression(step);
                    }
                } else {
                    strcpy(end, arg1);
                }
                match(TOKEN_RPAREN);
                codegen_write("for (int %s = %s; %s < %s; %s += %s)", var, start, var, end, var, step);
                parse_block();
            } else if (strcmp(current_token.text, "print") == 0) {
                parse_print();
                codegen_newline();
            } else {
                advance_token();
            }
        } else if (current_token.type == TOKEN_IDENTIFIER) {
            char name[64];
            strcpy(name, current_token.text);
            match(TOKEN_IDENTIFIER);
            if (current_token.type == TOKEN_ASSIGN) {
                match(TOKEN_ASSIGN);
                if (current_token.type == TOKEN_KEYWORD && (strcmp(current_token.text, "int") == 0 || strcmp(current_token.text, "input") == 0)) {
                    parse_input(name);
                } else if (current_token.type == TOKEN_LBRACKET) {
                    match(TOKEN_LBRACKET);
                    codegen_write("int %s[100] = {", name);
                    while (current_token.type != TOKEN_RBRACKET) {
                        codegen_write("%s", current_token.text);
                        advance_token();
                        if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
                    }
                    match(TOKEN_RBRACKET);
                    codegen_write("}; int %s_len = 3; // Placeholder length", name);
                } else {
                    char expr[128] = {0};
                    parse_expression(expr);
                    codegen_write("%s = %s;", name, expr);
                }
            } else if (current_token.type == TOKEN_LPAREN) {
                match(TOKEN_LPAREN);
                codegen_write("%s(", name);
                while (current_token.type != TOKEN_RPAREN) {
                    codegen_write("%s", current_token.text);
                    advance_token();
                    if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
                }
                match(TOKEN_RPAREN);
                codegen_write(");");
            } else if (current_token.type == TOKEN_OPERATOR && current_token.text[0] == '.') {
                // Method call (e.g. list.append)
                advance_token(); // .
                char method[64]; strcpy(method, current_token.text); advance_token();
                match(TOKEN_LPAREN);
                if (strcmp(method, "append") == 0) {
                    char val[64] = {0}; parse_expression(val);
                    codegen_write("%s[%s_len++] = %s;", name, name, val);
                }
                match(TOKEN_RPAREN);
            }
            codegen_newline();
        } else if (current_token.type == TOKEN_NEWLINE) {
            advance_token();
        } else {
            advance_token();
        }
    }
    
    match(TOKEN_DEDENT);
    codegen_end_block();
    codegen_newline();
}

static void parse_statement() {
    if (current_token.type == TOKEN_KEYWORD) {
        if (strcmp(current_token.text, "def") == 0) {
            match(TOKEN_KEYWORD);
            char name[64];
            strcpy(name, current_token.text);
            match(TOKEN_IDENTIFIER);
            match(TOKEN_LPAREN);
            codegen_write("void %s(", name);
            while (current_token.type != TOKEN_RPAREN) {
                codegen_write("int %s", current_token.text); // Assume int
                advance_token();
                if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
            }
            match(TOKEN_RPAREN);
            codegen_write(")");
            parse_block();
        } else if (strcmp(current_token.text, "print") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; }
            parse_print();
            codegen_newline();
        } else if (strcmp(current_token.text, "if") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; }
            match(TOKEN_KEYWORD);
            codegen_write("if (");
            char cond[128] = {0};
            parse_expression(cond);
            codegen_write("%s)", cond);
            parse_block();
        } else if (strcmp(current_token.text, "elif") == 0) {
            match(TOKEN_KEYWORD);
            codegen_write("else if (");
            char cond[128] = {0};
            parse_expression(cond);
            codegen_write("%s)", cond);
            parse_block();
        } else if (strcmp(current_token.text, "else") == 0) {
            match(TOKEN_KEYWORD);
            codegen_write("else");
            parse_block();
        } else if (strcmp(current_token.text, "while") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; }
            match(TOKEN_KEYWORD);
            codegen_write("while (");
            char cond[128] = {0};
            parse_expression(cond);
            codegen_write("%s)", cond);
            parse_block();
        } else if (strcmp(current_token.text, "for") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; }
            match(TOKEN_KEYWORD);
            char var[64]; strcpy(var, current_token.text); advance_token();
            match(TOKEN_KEYWORD); // in
            match(TOKEN_KEYWORD); // range
            match(TOKEN_LPAREN);
            char start[32] = "0", end[32] = "0", step[32] = "1";
            char arg1[32] = {0}; parse_expression(arg1);
            if (current_token.type == TOKEN_COMMA) {
                advance_token();
                strcpy(start, arg1);
                parse_expression(end);
                if (current_token.type == TOKEN_COMMA) { advance_token(); parse_expression(step); }
            } else { strcpy(end, arg1); }
            match(TOKEN_RPAREN);
            codegen_write("for (int %s = %s; %s < %s; %s += %s)", var, start, var, end, var, step);
            parse_block();
        } else { advance_token(); }
    } else if (current_token.type == TOKEN_IDENTIFIER) {
        if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; }
        char name[64];
        strcpy(name, current_token.text);
        advance_token();
        if (current_token.type == TOKEN_ASSIGN) {
            match(TOKEN_ASSIGN);
            if (current_token.type == TOKEN_KEYWORD && (strcmp(current_token.text, "int") == 0 || strcmp(current_token.text, "input") == 0)) {
                codegen_write("int %s; ", name);
                parse_input(name);
            } else if (current_token.type == TOKEN_LBRACKET) {
                match(TOKEN_LBRACKET);
                codegen_write("int %s[100] = {", name);
                while (current_token.type != TOKEN_RBRACKET) {
                    codegen_write("%s", current_token.text);
                    advance_token();
                    if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
                }
                match(TOKEN_RBRACKET);
                codegen_write("}; int %s_len = 3; ", name);
            } else {
                char expr[128] = {0};
                parse_expression(expr);
                codegen_write("int %s = %s;", name, expr);
            }
        } else if (current_token.type == TOKEN_LPAREN) {
            match(TOKEN_LPAREN);
            codegen_write("%s(", name);
            while (current_token.type != TOKEN_RPAREN) {
                codegen_write("%s", current_token.text);
                advance_token();
                if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
            }
            match(TOKEN_RPAREN);
            codegen_write(");");
        } else if (current_token.type == TOKEN_OPERATOR && current_token.text[0] == '.') {
            advance_token(); // .
            char method[64]; strcpy(method, current_token.text); advance_token();
            match(TOKEN_LPAREN);
            if (strcmp(method, "append") == 0) {
                char val[64] = {0}; parse_expression(val);
                codegen_write("%s[%s_len++] = %s;", name, name, val);
            }
            match(TOKEN_RPAREN);
        }
        codegen_newline();
    } else if (current_token.type == TOKEN_NEWLINE) { advance_token(); }
    else { advance_token(); }
}

void parser_run() {
    advance_token();
    codegen_init();
    
    // Preliminary header writing
    codegen_write("#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n");

    while (current_token.type != TOKEN_EOF) {
        parse_statement();
    }
    
    if (main_started) {
        codegen_write("return 0;\n}");
    }
    
    codegen_finalize();
}
