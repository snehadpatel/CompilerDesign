#include "parser.h"
#include "lexer.h"
#include "codegen.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static Token current_token;
static int main_started = 0;

// Simple Symbol Table to track declared variables
static char symbols[100][64];
static int symbol_count = 0;

static void add_symbol(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i], name) == 0) return;
    }
    strcpy(symbols[symbol_count++], name);
}

static int is_declared(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i], name) == 0) return 1;
    }
    return 0;
}

static void advance_token() {
    current_token = lexer_next_token();
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

static void parse_expression(char *buf, int stop_at_comma);

static void parse_print() {
    match(TOKEN_KEYWORD); // print
    match(TOKEN_LPAREN);
    
    char expr[256] = {0};
    if (current_token.type == TOKEN_STRING) {
        strcpy(expr, current_token.text);
        advance_token();
        codegen_write("printf(\"%%s\\n\", %s);", expr);
    } else {
        parse_expression(expr, 0);
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

static int current_indent = 0;

static void codegen_indent_internal() {
    for (int i = 0; i < current_indent; i++) codegen_write("    ");
}

static void parse_expression(char *buf, int stop_at_comma) {
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
            parse_expression(buf, 0); 
            if (current_token.type == TOKEN_RPAREN) {
                strcat(buf, ")");
                advance_token();
            }
        } else if (current_token.type == TOKEN_IDENTIFIER) {
            char name[64]; strcpy(name, current_token.text);
            advance_token();
            if (current_token.type == TOKEN_LBRACKET) { // INDEXING
                strcat(buf, name);
                strcat(buf, "[");
                advance_token();
                parse_expression(buf, 0);
                if (current_token.type == TOKEN_RBRACKET) {
                    strcat(buf, "]");
                    advance_token();
                }
            } else if (current_token.type == TOKEN_LPAREN) { // CALL
                if (strcmp(name, "len") == 0) {
                    advance_token(); // (
                    char arg[64] = {0};
                    parse_expression(arg, 0);
                    if (current_token.type == TOKEN_RPAREN) advance_token(); // )
                    strcat(buf, arg);
                    strcat(buf, "_len"); // list_len
                } else if (strcmp(name, "ord") == 0) {
                    advance_token(); // (
                    strcat(buf, "(int)(");
                    parse_expression(buf, 0);
                    if (current_token.type == TOKEN_RPAREN) advance_token(); // )
                    strcat(buf, ")");
                } else if (strcmp(name, "chr") == 0) {
                    advance_token(); // (
                    strcat(buf, "(char)(");
                    parse_expression(buf, 0);
                    if (current_token.type == TOKEN_RPAREN) advance_token(); // )
                    strcat(buf, ")");
                } else {
                    strcat(buf, name);
                    strcat(buf, "(");
                    advance_token();
                    parse_expression(buf, 0);
                    if (current_token.type == TOKEN_RPAREN) {
                        strcat(buf, ")");
                        advance_token();
                    }
                }
            } else {
                strcat(buf, name);
            }
        } else {
            strcat(buf, current_token.text);
            advance_token();
        }

        if (current_token.type == TOKEN_COMMA) {
            if (stop_at_comma) break; 
            strcat(buf, ", ");
            advance_token();
        }
        
        if (current_token.type == TOKEN_COLON || current_token.type == TOKEN_NEWLINE || 
            current_token.type == TOKEN_EOF || current_token.type == TOKEN_RPAREN ||
            current_token.type == TOKEN_RBRACKET) break;
    }
}

static void parse_block() {
    match(TOKEN_COLON);
    if (current_token.type == TOKEN_NEWLINE) advance_token();
    match(TOKEN_INDENT);
    codegen_write(" {");
    codegen_newline();
    current_indent++;
    
    while (current_token.type != TOKEN_DEDENT && current_token.type != TOKEN_EOF) {
        if (current_token.type == TOKEN_NEWLINE) { advance_token(); continue; }
        
        codegen_indent_internal();
        if (current_token.type == TOKEN_KEYWORD) {
            if (strcmp(current_token.text, "if") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("if (");
                char cond[128] = {0}; parse_expression(cond, 0);
                codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "elif") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("else if (");
                char cond[128] = {0}; parse_expression(cond, 0);
                codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "else") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("else");
                parse_block();
            } else if (strcmp(current_token.text, "while") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("while (");
                char cond[128] = {0}; parse_expression(cond, 0);
                codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "return") == 0) {
                match(TOKEN_KEYWORD);
                char expr[128] = {0}; parse_expression(expr, 0);
                codegen_write("return %s;", expr);
                codegen_newline();
            } else if (strcmp(current_token.text, "for") == 0) {
                match(TOKEN_KEYWORD);
                char var[64]; strcpy(var, current_token.text); advance_token();
                match(TOKEN_KEYWORD); match(TOKEN_KEYWORD); // in range
                match(TOKEN_LPAREN);
                char start[32] = "", end[32] = "", step[32] = "1";
                char arg1[32] = {0}; parse_expression(arg1, 1);
                if (current_token.type == TOKEN_COMMA) {
                    advance_token();
                    strcpy(start, arg1);
                    parse_expression(end, 1);
                    if (current_token.type == TOKEN_COMMA) { advance_token(); parse_expression(step, 1); }
                } else { strcpy(start, "0"); strcpy(end, arg1); }
                match(TOKEN_RPAREN);
                codegen_write("for (int %s = %s; %s < %s; %s += %s)", var, start, var, end, var, step);
                parse_block();
            } else if (strcmp(current_token.text, "print") == 0) {
                parse_print();
                codegen_newline();
            } else { advance_token(); }
        } else if (current_token.type == TOKEN_IDENTIFIER) {
            char name[64]; strcpy(name, current_token.text); match(TOKEN_IDENTIFIER);
            if (current_token.type == TOKEN_ASSIGN) {
                match(TOKEN_ASSIGN);
                if (current_token.type == TOKEN_KEYWORD && (strcmp(current_token.text, "int") == 0 || strcmp(current_token.text, "input") == 0)) {
                    parse_input(name);
                } else if (current_token.type == TOKEN_LBRACKET) {
                    match(TOKEN_LBRACKET);
                    if (!is_declared(name)) { codegen_write("int %s[100] = {", name); add_symbol(name); }
                    else codegen_write("%s = {", name);
                    int elem_count = 0;
                    while (current_token.type != TOKEN_RBRACKET) {
                        codegen_write("%s", current_token.text); advance_token();
                        elem_count++;
                        if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
                    }
                    match(TOKEN_RBRACKET);
                    codegen_write("};");
                    codegen_newline();
                    codegen_indent_internal();
                    codegen_write("int %s_len = %d;", name, elem_count);
                } else {
                    char expr[128] = {0}; parse_expression(expr, 0);
                    if (!is_declared(name)) { codegen_write("int %s = %s;", name, expr); add_symbol(name); }
                    else codegen_write("%s = %s;", name, expr);
                }
            } else if (current_token.type == TOKEN_LBRACKET) {
                match(TOKEN_LBRACKET);
                char idx[32] = {0}; parse_expression(idx, 0); match(TOKEN_RBRACKET);
                match(TOKEN_ASSIGN);
                char expr[128] = {0}; parse_expression(expr, 0);
                codegen_write("%s[%s] = %s;", name, idx, expr);
            } else if (current_token.type == TOKEN_LPAREN) {
                match(TOKEN_LPAREN);
                codegen_write("%s(", name);
                while (current_token.type != TOKEN_RPAREN) {
                    char arg[64]={0}; parse_expression(arg, 1);
                    codegen_write("%s", arg);
                    if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
                }
                match(TOKEN_RPAREN);
                codegen_write(");");
            } else if (current_token.type == TOKEN_OPERATOR && current_token.text[0] == '.') {
                advance_token(); char method[64]; strcpy(method, current_token.text); advance_token();
                match(TOKEN_LPAREN);
                if (strcmp(method, "append") == 0) {
                    char val[64] = {0}; parse_expression(val, 1);
                    codegen_write("%s[%s_len++] = %s;", name, name, val);
                }
                match(TOKEN_RPAREN);
            }
            codegen_newline();
        } else { advance_token(); }
    }
    
    match(TOKEN_DEDENT);
    current_indent--;
    codegen_indent_internal();
    codegen_write("}");
    codegen_newline();
}

static void parse_statement() {
    if (current_token.type == TOKEN_NEWLINE) { advance_token(); return; }
    
    codegen_indent_internal();
    if (current_token.type == TOKEN_KEYWORD) {
        if (strcmp(current_token.text, "def") == 0) {
            match(TOKEN_KEYWORD);
            char name[64]; strcpy(name, current_token.text); match(TOKEN_IDENTIFIER);
            match(TOKEN_LPAREN);
            codegen_write("void %s(", name);
            while (current_token.type != TOKEN_RPAREN) {
                codegen_write("int %s", current_token.text); advance_token();
                if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
            }
            match(TOKEN_RPAREN);
            codegen_write(")");
            parse_block();
        } else if (strcmp(current_token.text, "print") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; codegen_indent_internal(); }
            parse_print(); codegen_newline();
        } else if (strcmp(current_token.text, "if") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; codegen_indent_internal(); }
            match(TOKEN_KEYWORD);
            codegen_write("if (");
            char cond[128] = {0}; parse_expression(cond, 0);
            codegen_write("%s)", cond);
            parse_block();
        } else if (strcmp(current_token.text, "while") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; codegen_indent_internal(); }
            match(TOKEN_KEYWORD);
            codegen_write("while (");
            char cond[128] = {0}; parse_expression(cond, 0);
            codegen_write("%s)", cond);
            parse_block();
        } else if (strcmp(current_token.text, "for") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; codegen_indent_internal(); }
            match(TOKEN_KEYWORD);
            char var[64]; strcpy(var, current_token.text); advance_token();
            match(TOKEN_KEYWORD); match(TOKEN_KEYWORD);
            match(TOKEN_LPAREN);
            char start[32] = "", end[32] = "", step[32] = "1";
            char arg1[32] = {0}; parse_expression(arg1, 1);
            if (current_token.type == TOKEN_COMMA) {
                advance_token(); strcpy(start, arg1); parse_expression(end, 1);
                if (current_token.type == TOKEN_COMMA) { advance_token(); parse_expression(step, 1); }
            } else { strcpy(start, "0"); strcpy(end, arg1); }
            match(TOKEN_RPAREN);
            codegen_write("for (int %s = %s; %s < %s; %s += %s)", var, start, var, end, var, step);
            parse_block();
        } else { advance_token(); }
    } else if (current_token.type == TOKEN_IDENTIFIER) {
        if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; codegen_indent_internal(); }
        char name[64]; strcpy(name, current_token.text); advance_token();
        if (current_token.type == TOKEN_ASSIGN) {
            match(TOKEN_ASSIGN);
            if (current_token.type == TOKEN_KEYWORD && (strcmp(current_token.text, "int") == 0 || strcmp(current_token.text, "input") == 0)) {
                if (!is_declared(name)) { codegen_write("int %s; ", name); add_symbol(name); }
                parse_input(name);
            } else if (current_token.type == TOKEN_LBRACKET) {
                match(TOKEN_LBRACKET);
                if (!is_declared(name)) { codegen_write("int %s[100] = {", name); add_symbol(name); }
                else codegen_write("%s = {", name);
                int elem_count = 0;
                while (current_token.type != TOKEN_RBRACKET) {
                    codegen_write("%s", current_token.text); advance_token();
                    elem_count++;
                    if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
                }
                match(TOKEN_RBRACKET); 
                codegen_write("};");
                codegen_newline();
                codegen_indent_internal();
                codegen_write("int %s_len = %d;", name, elem_count);
            } else {
                char expr[128] = {0}; parse_expression(expr, 0);
                if (!is_declared(name)) { codegen_write("int %s = %s;", name, expr); add_symbol(name); }
                else codegen_write("%s = %s;", name, expr);
            }
        } else if (current_token.type == TOKEN_LPAREN) {
            match(TOKEN_LPAREN); codegen_write("%s(", name);
            while (current_token.type != TOKEN_RPAREN) {
                char arg[64]={0}; parse_expression(arg, 1);
                codegen_write("%s", arg);
                if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
            }
            match(TOKEN_RPAREN); codegen_write(");");
        } else if (current_token.type == TOKEN_OPERATOR && current_token.text[0] == '.') {
            advance_token(); // .
            char method[64]; strcpy(method, current_token.text); advance_token();
            match(TOKEN_LPAREN);
            if (strcmp(method, "append") == 0) {
                char val[64] = {0}; parse_expression(val, 1);
                codegen_write("%s[%s_len++] = %s;", name, name, val);
            }
            match(TOKEN_RPAREN);
        }
        codegen_newline();
    } else { advance_token(); }
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
        codegen_indent_internal();
        codegen_write("return 0;\n}\n");
    }
    
    codegen_finalize();
}
