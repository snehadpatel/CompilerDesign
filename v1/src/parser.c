#include "parser.h"
#include "lexer.h"
#include "codegen.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void parse_statement();
static void parse_block();
static void parse_block_internal();

static Token current_token;
static int main_started = 0;
static int math_used = 0;
static int string_used = 0;
static int symbol_count = 0;
static int current_indent = 0;
static char current_func_name[64] = "";
static int is_slice = 0;
static char slice_arr[64] = {0};
static char slice_start[64] = {0};
static char slice_end[64] = {0};
static int comp_count = 0;
static int last_expr_is_float = 0;
static int last_expr_is_double = 0;


// Simple Symbol Table to track declared variables
static struct Symbol {
    char name[64];
    char type[32];
} symbols[100];

// Track which variables have len() called on them
#define MAX_LEN_VARS 100
static char len_needed_vars[MAX_LEN_VARS][64];
static int len_needed_count = 0;

static int needs_len_var(const char *name) {
    for (int i = 0; i < len_needed_count; i++) {
        if (strcmp(len_needed_vars[i], name) == 0) return 1;
    }
    return 0;
}

// Track which functions have a return statement (to determine return type)
#define MAX_FUNCS 100
static struct FuncInfo {
    char name[64];
    int has_return;
} func_info[MAX_FUNCS];
static int func_info_count = 0;

static int func_has_return(const char *name) {
    for (int i = 0; i < func_info_count; i++) {
        if (strcmp(func_info[i].name, name) == 0) return func_info[i].has_return;
    }
    return 0;
}

// Pre-scanned variable type table (populated from literal assignments before main parse)
static struct PreVar {
    char name[64];
    char type[32];
} pre_vars[200];
static int pre_var_count = 0;

static void add_pre_var(const char *name, const char *type) {
    for (int i = 0; i < pre_var_count; i++) {
        if (strcmp(pre_vars[i].name, name) == 0) {
            strcpy(pre_vars[i].type, type);
            return;
        }
    }
    if (pre_var_count < 200) {
        strncpy(pre_vars[pre_var_count].name, name, sizeof(pre_vars[0].name) - 1);
        pre_vars[pre_var_count].name[sizeof(pre_vars[0].name) - 1] = '\0';
        strncpy(pre_vars[pre_var_count].type, type, sizeof(pre_vars[0].type) - 1);
        pre_vars[pre_var_count].type[sizeof(pre_vars[0].type) - 1] = '\0';
        pre_var_count++;
    }
}

static const char* get_pre_var_type(const char *name) {
    for (int i = 0; i < pre_var_count; i++) {
        if (strcmp(pre_vars[i].name, name) == 0) return pre_vars[i].type;
    }
    return "int";
}

// Function parameter types inferred from call sites
#define MAX_PARAMS 10
static struct FuncCallParamTypes {
    char func_name[64];
    char param_types[MAX_PARAMS][32];
    int param_count;
} func_call_param_types[MAX_FUNCS];
static int func_call_param_types_count = 0;

static const char* get_func_param_type(const char *func_name, int param_idx) {
    for (int i = 0; i < func_call_param_types_count; i++) {
        if (strcmp(func_call_param_types[i].func_name, func_name) == 0) {
            if (param_idx < func_call_param_types[i].param_count) {
                return func_call_param_types[i].param_types[param_idx];
            }
        }
    }
    return "int";
}

static int func_has_float_param(const char *func_name) {
    for (int i = 0; i < func_call_param_types_count; i++) {
        if (strcmp(func_call_param_types[i].func_name, func_name) == 0) {
            for (int j = 0; j < func_call_param_types[i].param_count; j++) {
                if (strcmp(func_call_param_types[i].param_types[j], "float") == 0 ||
                    strcmp(func_call_param_types[i].param_types[j], "double") == 0) return 1;
            }
        }
    }
    return 0;
}

static int func_has_double_param(const char *func_name) {
    for (int i = 0; i < func_call_param_types_count; i++) {
        if (strcmp(func_call_param_types[i].func_name, func_name) == 0) {
            for (int j = 0; j < func_call_param_types[i].param_count; j++) {
                if (strcmp(func_call_param_types[i].param_types[j], "double") == 0) return 1;
            }
        }
    }
    return 0;
}

void parser_init() {
    symbol_count = 0;
    main_started = 0;
    math_used = 0;
    string_used = 0;
    current_indent = 0;
    is_slice = 0;
    last_expr_is_float = 0;
}

static void add_symbol(const char *name, const char *type) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0) return;
    }
    strcpy(symbols[symbol_count].name, name);
    strcpy(symbols[symbol_count].type, type);
    symbol_count++;
}

static int is_declared(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0) return 1;
    }
    return 0;
}

static const char* get_symbol_type(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbols[i].name, name) == 0) return symbols[i].type;
    }
    return "int";
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
        exit(1);
    }
}

static void parse_expression(char *buf, int stop_at_comma);

static void codegen_indent_internal() {
    for (int i = 0; i < current_indent; i++) codegen_write("    ");
}

static void parse_print() {
    match(TOKEN_KEYWORD); // print
    match(TOKEN_LPAREN);
    
    char expr[256] = {0};
    while (current_token.type == TOKEN_LPAREN) advance_token(); // skip (
    
    if (current_token.type == TOKEN_STRING) {
        char full_expr[256];
        strcpy(full_expr, current_token.text);
        advance_token();
        
        if (full_expr[0] == 'f') { // f-string handling: f"Hello {name}"
            char format[256] = {0};
            char args[256] = {0};
            char quote = (full_expr[1] == '"' || full_expr[1] == '\'') ? full_expr[1] : '"';
            char *p = full_expr + 2; 
            char *f = format;
            char *a = args;
            while (*p != quote && *p != '\0') {
                if (*p == '{') {
                    p++;
                    char var[64] = {0}; int vi = 0;
                    while (*p != '}' && *p != '\0') { if (vi < 63) var[vi++] = *p++; else p++; }
                    if (*p == '}') p++;
                    
                    const char *type = get_symbol_type(var);
                    if (strstr(type, "[]")) {
                        // Close printf, call printer, reopen printf
                        *f = '\0';
                        codegen_write_raw("printf(\""); codegen_write_raw(format); codegen_write_raw("\""); codegen_write_raw(args); codegen_write_raw("); ");
                        codegen_write("print_list(%s); ", var);
                        *format = '\0'; f = format; 
                        *args = '\0'; a = args; // Reset args too!
                    } else if (strcmp(type, "float") == 0) { 
                        f += sprintf(f, "%%f"); strcat(a, ", "); strcat(a, var); 
                    } else if (strcmp(type, "char*") == 0) { 
                        f += sprintf(f, "%%s"); strcat(a, ", "); strcat(a, var); 
                    } else { 
                        f += sprintf(f, "%%d"); strcat(a, ", "); strcat(a, var); 
                    }

                } else {
                    *f++ = *p++;
                }
            }
            *f = '\0';
            *f = '\0';
            codegen_write_raw("printf(\"");
            codegen_write_raw(format);
            if (strlen(format) > 0) codegen_write_raw("\\n\"");
            else codegen_write_raw("\""); // Already ended with list printer
            codegen_write_raw(args);
            codegen_write_raw(");");
        } else {
            char inner[256] = {0};
            strncpy(inner, full_expr + 1, strlen(full_expr) - 2);
            codegen_write_raw("printf(\"");
            codegen_write_raw(inner);
            codegen_write_raw("\\n\");");
        }
    } else {
        char expr[128] = {0};
        parse_expression(expr, 0);
        
        const char *type = get_symbol_type(expr);
        if (strcmp(type, "char*") == 0) {
            codegen_write("printf(\"%%s\\n\", %s);", expr);
        } else if (strcmp(type, "char") == 0) {
            codegen_write("printf(\"%%c\\n\", %s);", expr);
        } else if (strcmp(type, "float") == 0) {
            codegen_write("printf(\"%%g\\n\", %s);", expr);
        } else if (strcmp(type, "double") == 0) {
            codegen_write("printf(\"%%g\\n\", %s);", expr);
        } else if (strcmp(type, "int[]") == 0) {
            codegen_write("print_list(%s);", expr);
        } else {
            codegen_write("printf(\"%%d\\n\", %s);", expr); 
        }
    }
    
    while (current_token.type == TOKEN_RPAREN) advance_token(); // skip )
}

static void parse_input(const char *var_name) {
    if (strcmp(current_token.text, "int") == 0) {
        match(TOKEN_KEYWORD); // int
        match(TOKEN_LPAREN);
        if (strcmp(current_token.text, "input") == 0) {
            match(TOKEN_KEYWORD); // input
            match(TOKEN_LPAREN);
            if (current_token.type == TOKEN_STRING) {
                char inner[256] = {0};
                strncpy(inner, current_token.text + 1, strlen(current_token.text) - 2);
                codegen_write("printf(\"%s\");", inner);
                advance_token();
            }
            match(TOKEN_RPAREN);
        }
        match(TOKEN_RPAREN);
        codegen_newline();
        codegen_indent_internal();
        codegen_write("scanf(\"%%d\", &%s);", var_name);
    } else if (strcmp(current_token.text, "input") == 0) {
        match(TOKEN_KEYWORD); // input
        match(TOKEN_LPAREN);
        if (current_token.type == TOKEN_STRING) {
            char inner[256] = {0};
            strncpy(inner, current_token.text + 1, strlen(current_token.text) - 2);
            codegen_write("printf(\"%s\");", inner);
            advance_token();
        }
        match(TOKEN_RPAREN);
        codegen_newline();
        codegen_indent_internal();
        if (strcmp(get_symbol_type(var_name), "int") == 0) {
            codegen_write("scanf(\"%%d\", &%s);", var_name);
        } else {
            codegen_write("scanf(\"%%s\", %s);", var_name);
        }
    }
}

static void parse_expression(char *buf, int stop_at_comma) {
    last_expr_is_float = 0;
    last_expr_is_double = 0;
    while (current_token.type == TOKEN_NUMBER || current_token.type == TOKEN_IDENTIFIER || 
           current_token.type == TOKEN_STRING || current_token.type == TOKEN_LPAREN || 
           current_token.type == TOKEN_KEYWORD || current_token.type == TOKEN_OPERATOR ||
           current_token.type == TOKEN_LBRACKET) {
        
        if (current_token.type == TOKEN_LBRACKET) {
            advance_token();
            // Potential List Comprehension: [item for var in range(...)]
            char item_expr[128] = {0};
            parse_expression(item_expr, 0);
            
            if (current_token.type == TOKEN_KEYWORD && strcmp(current_token.text, "for") == 0) {
                advance_token(); // for
                char var[64]; strcpy(var, current_token.text); advance_token(); // var
                match(TOKEN_KEYWORD); // in
                match(TOKEN_KEYWORD); // range
                match(TOKEN_LPAREN);
                char start[32] = "0", end[32] = "", step[32] = {0};
                char arg1[32] = {0}; parse_expression(arg1, 1);
                if (current_token.type == TOKEN_COMMA) {
                    advance_token(); strcpy(start, arg1); parse_expression(end, 1);
                    if (current_token.type == TOKEN_COMMA) { advance_token(); parse_expression(step, 1); }
                } else { strcpy(end, arg1); }
                if (strlen(step) == 0) strcpy(step, "1");
                match(TOKEN_RPAREN);
                match(TOKEN_RBRACKET);
                
                char comp_name[64]; sprintf(comp_name, "tmp_list_%d", ++comp_count);
                codegen_write("PythonList %s; %s.len = 0;", comp_name, comp_name); codegen_newline();
                codegen_indent_internal();
                codegen_write("for (int %s = %s; %s < %s; %s += %s) {", var, start, var, end, var, step);
                codegen_newline(); current_indent++; codegen_indent_internal();
                codegen_write("%s.data[%s.len++] = %s;", comp_name, comp_name, item_expr);
                codegen_newline(); current_indent--; codegen_indent_internal();
                codegen_write("}"); codegen_newline();
                codegen_indent_internal();
                
                strcat(buf, comp_name);
                add_symbol(comp_name, "int[]");
            } else {
                // Regular list literal [1, 2, 3] or empty list []
                if (strlen(item_expr) == 0 && current_token.type == TOKEN_RBRACKET) {
                    strcat(buf, "(PythonList){0}"); // Proper compound literal
                    advance_token();
                } else {
                    strcat(buf, "["); strcat(buf, item_expr); 
                    if (current_token.type == TOKEN_RBRACKET) { strcat(buf, "]"); advance_token(); }
                }
            }
            continue;
        }
        
        if (current_token.type == TOKEN_KEYWORD) {
            if (strcmp(current_token.text, "and") == 0) strcat(buf, " && ");
            else if (strcmp(current_token.text, "or") == 0) strcat(buf, " || ");
            else if (strcmp(current_token.text, "not") == 0) strcat(buf, " ! ");
            else if (strcmp(current_token.text, "True") == 0) strcat(buf, " 1 ");
            else if (strcmp(current_token.text, "False") == 0) strcat(buf, " 0 ");
            else if (strcmp(current_token.text, "import") == 0) break; // Not part of expr
            else break; // Not part of expr
            advance_token();
        } else if (current_token.type == TOKEN_OPERATOR) {
            if (current_token.text[0] == '.') {
                // Peek at next to see if it's a module we know
                advance_token(); // skip '.'
                if (current_token.type == TOKEN_IDENTIFIER) {
                    // math.sqrt -> sqrt
                    // Handle math specifically
                    strcat(buf, current_token.text);
                    advance_token();
                }
            } else {
                strcat(buf, " ");
                strcat(buf, current_token.text);
                strcat(buf, " ");
                advance_token();
            }
        } else if (current_token.type == TOKEN_LPAREN) {
            strcat(buf, "(");
            advance_token();
            int saved_is_float = last_expr_is_float;
            int saved_is_double = last_expr_is_double;
            parse_expression(buf, 0);
            if (saved_is_float) last_expr_is_float = 1;
            if (saved_is_double) last_expr_is_double = 1;
            if (current_token.type == TOKEN_RPAREN) {
                strcat(buf, ")");
                advance_token();
            }
        } else if (current_token.type == TOKEN_IDENTIFIER) {
            char name[64]; strcpy(name, current_token.text);
            if (strcmp(name, "math") == 0) {
                advance_token();
                if (current_token.type == TOKEN_OPERATOR && current_token.text[0] == '.') {
                    advance_token(); // skip '.'
                    strcat(buf, current_token.text);
                    if (strcmp(current_token.text, "sqrt") == 0 || strcmp(current_token.text, "pow") == 0) {
                        last_expr_is_float = 1;
                        last_expr_is_double = 1;
                    }
                    advance_token();
                } else {
                    strcat(buf, name);
                }
            } else {
                advance_token();
                if (current_token.type == TOKEN_LBRACKET) { // INDEXING
                    advance_token();
                    char inner_expr[64] = {0};
                    parse_expression(inner_expr, 0);
                    if (current_token.type == TOKEN_COLON) { // SLICING
                        advance_token();
                        char inner_expr2[64] = {0};
                        parse_expression(inner_expr2, 0);
                        if (current_token.type == TOKEN_RBRACKET) advance_token();
                        is_slice = 1;
                        strcpy(slice_arr, name);
                        strcpy(slice_start, inner_expr);
                        strcpy(slice_end, inner_expr2);
                        strcpy(buf, "0"); // Dummy expression value
                    } else {
                        const char *type = get_symbol_type(name);
                        if (strstr(type, "[]")) {
                            strcat(buf, name);
                            strcat(buf, ".data[");
                            strcat(buf, inner_expr);
                            if (current_token.type == TOKEN_RBRACKET) advance_token();
                            strcat(buf, "]");
                        } else {
                            strcat(buf, name);
                            strcat(buf, "[");
                            strcat(buf, inner_expr);
                            if (current_token.type == TOKEN_RBRACKET) advance_token();
                            strcat(buf, "]");
                        }
                    }
                } else if (current_token.type == TOKEN_LPAREN) { // CALL
                    if (strcmp(name, "len") == 0) {
                        advance_token(); // (
                        char arg[64] = {0};
                        parse_expression(arg, 0);
                        if (current_token.type == TOKEN_RPAREN) advance_token(); // )
                        strcat(buf, arg);
                        const char *type = get_symbol_type(arg);
                        if (strstr(type, "[]")) strcat(buf, ".len");
                        else strcat(buf, "_len"); // Fallback for raw strings if any
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
                        // If the called function returns a floating-point type, propagate
                        if (func_has_return(name)) {
                            if (func_has_double_param(name))
                                last_expr_is_double = 1;
                            if (func_has_float_param(name))
                                last_expr_is_float = 1;
                        }
                    }
                } else {
                    strcat(buf, name);
                    // Propagate float type when a float variable appears in expression
                    if (strcmp(get_symbol_type(name), "float") == 0 ||
                        strcmp(get_symbol_type(name), "double") == 0)
                        last_expr_is_float = 1;
                }
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

static void parse_import() {
    match(TOKEN_KEYWORD); // import
    if (strcmp(current_token.text, "math") == 0) {
        math_used = 1;
        match(TOKEN_IDENTIFIER);
    } else if (strcmp(current_token.text, "string") == 0) {
        string_used = 1;
        match(TOKEN_IDENTIFIER);
    } else {
        match(TOKEN_IDENTIFIER); // skip unknown
    }
}

static void parse_block_internal() {
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
                const char *type = get_symbol_type(cond);
                if (strstr(type, "[]")) codegen_write("%s.len > 0)", cond);
                else codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "elif") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("else if (");
                char cond[128] = {0}; parse_expression(cond, 0);
                const char *type = get_symbol_type(cond);
                if (strstr(type, "[]")) codegen_write("%s.len > 0)", cond);
                else codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "else") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("else");
                parse_block();
            } else if (strcmp(current_token.text, "while") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("while (");
                char cond[128] = {0}; parse_expression(cond, 0);
                const char *type = get_symbol_type(cond);
                if (strstr(type, "[]")) codegen_write("%s.len > 0)", cond);
                else codegen_write("%s)", cond);
                parse_block();
            } else if (strcmp(current_token.text, "return") == 0) {
                match(TOKEN_KEYWORD);
                char expr[128] = {0}; parse_expression(expr, 0);
                char *trim_expr = expr; while(*trim_expr == ' ') trim_expr++;
                
                if (strstr(trim_expr, "tmp_list_") || trim_expr[0] == '(') { 
                    codegen_write("return %s;", trim_expr);
                } else if (strcmp(trim_expr, "[]") == 0 || strcmp(trim_expr, "{}") == 0) {
                    codegen_write("return (PythonList){0};");
                } else {
                    const char *type = get_symbol_type(trim_expr);
                    if (strstr(type, "[]")) {
                        codegen_write("return %s;", trim_expr);
                    } else {
                        if (strcmp(current_func_name, "main") == 0) {
                             codegen_write("return %s;", strlen(trim_expr) > 0 ? trim_expr : "0");
                        } else {
                             codegen_write("PythonList _ret; _ret.len = 1; _ret.data[0] = %s; return _ret;", strlen(trim_expr) > 0 ? trim_expr : "0");
                        }
                    }
                }
                codegen_newline();
            } else if (strcmp(current_token.text, "for") == 0) {
                match(TOKEN_KEYWORD);
                char var[64]; strcpy(var, current_token.text); advance_token();
                match(TOKEN_KEYWORD); match(TOKEN_KEYWORD); // in range
                match(TOKEN_LPAREN);
                char start[32] = "", end[32] = "", step[32] = "";
                char arg1[32] = {0}; parse_expression(arg1, 1);
                if (current_token.type == TOKEN_COMMA) {
                    advance_token();
                    strcpy(start, arg1);
                    parse_expression(end, 1);
                    if (current_token.type == TOKEN_COMMA) { advance_token(); parse_expression(step, 1); }
                } else { strcpy(start, "0"); strcpy(end, arg1); }
                if (strlen(step) == 0) strcpy(step, "1");
                match(TOKEN_RPAREN);
                codegen_write("for (int %s = %s; %s < %s; %s += %s)", var, start, var, end, var, step);
                parse_block();
            } else if (strcmp(current_token.text, "try") == 0) {
                match(TOKEN_KEYWORD);
                codegen_write("// try block start");
                parse_block();
            } else if (strcmp(current_token.text, "except") == 0) {
                match(TOKEN_KEYWORD);
                while (current_token.type != TOKEN_COLON) advance_token();
                match(TOKEN_COLON);
                codegen_write("// except block skipped");
                if (current_token.type == TOKEN_NEWLINE) advance_token();
                if (current_token.type == TOKEN_INDENT) {
                    advance_token();
                    while (current_token.type != TOKEN_DEDENT && current_token.type != TOKEN_EOF) advance_token();
                    if (current_token.type == TOKEN_DEDENT) advance_token();
                }
            } else if (strcmp(current_token.text, "print") == 0) {
                parse_print();
                codegen_newline();
            } else { advance_token(); }
        } else if (current_token.type == TOKEN_IDENTIFIER) {
            char name[64]; strcpy(name, current_token.text); match(TOKEN_IDENTIFIER);
            if (current_token.type == TOKEN_ASSIGN) {
                match(TOKEN_ASSIGN);
                if (current_token.type == TOKEN_KEYWORD && (strcmp(current_token.text, "int") == 0 || strcmp(current_token.text, "input") == 0)) {
                    // Scoping fix: always declare input target
                    codegen_write("int %s; ", name); 
                    add_symbol(name, "int"); 
                    codegen_newline(); codegen_indent_internal();
                    parse_input(name);

                } else if (current_token.type == TOKEN_LBRACKET) {
                    match(TOKEN_LBRACKET);
                    if (!is_declared(name)) { codegen_write("int %s[100] = {", name); add_symbol(name, "int[]"); }
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
                    char inferred_type[32] = "int";
                    if (current_token.type == TOKEN_STRING) {
                        if (current_token.text[0] == '\'') strcpy(inferred_type, "char");
                        else strcpy(inferred_type, "char*");
                    }
                    else if (current_token.type == TOKEN_NUMBER && strchr(current_token.text, '.')) strcpy(inferred_type, "float");
                    
                    is_slice = 0;
                    char expr[128] = {0}; parse_expression(expr, 0);
                    if (last_expr_is_float) strcpy(inferred_type, "float");
                    
                    if (is_slice) {
                        if (!is_declared(name)) { add_symbol(name, "int[]"); codegen_write("PythonList %s;", name); codegen_newline(); codegen_indent_internal(); }
                        codegen_write("%s.len = %s - (%s);", name, slice_end, slice_start);
                        codegen_newline(); codegen_indent_internal();
                        codegen_write("for (int i = 0; i < %s.len; i++) { %s.data[i] = %s.data[(%s) + i]; }", name, name, slice_arr, slice_start);
                    } else if (strcmp(inferred_type, "int[]") == 0) {
                        if (!is_declared(name)) { codegen_write("PythonList %s = %s;", name, expr); add_symbol(name, "int[]"); }
                        else codegen_write("%s = %s;", name, expr);
                    } else if (strcmp(inferred_type, "char*") == 0) {
                        int str_len_val = strlen(expr) > 2 ? strlen(expr) - 2 : 0;
                        if (!is_declared(name)) {
                            codegen_write("%s %s = %s;", inferred_type, name, expr); add_symbol(name, inferred_type);
                            if (needs_len_var(name)) { codegen_newline(); codegen_indent_internal(); codegen_write("int %s_len = %d;", name, str_len_val); }
                        } else {
                            if (needs_len_var(name)) codegen_write("%s = %s; %s_len = %d;", name, expr, name, str_len_val);
                            else codegen_write("%s = %s;", name, expr);
                        }
                    } else {
                        if (!is_declared(name)) { codegen_write("%s %s = %s;", inferred_type, name, expr); add_symbol(name, inferred_type); }
                        else codegen_write("%s = %s;", name, expr);
                    }
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

static void parse_block() {
    match(TOKEN_COLON);
    parse_block_internal();
}

static void parse_statement() {
    if (current_token.type == TOKEN_NEWLINE) { advance_token(); return; }
    
    codegen_indent_internal();
    if (current_token.type == TOKEN_KEYWORD) {
        if (strcmp(current_token.text, "import") == 0) {
            parse_import();
            codegen_newline();
            return;
        }
        if (strcmp(current_token.text, "def") == 0) {
            match(TOKEN_KEYWORD);
            char name[64]; strcpy(name, current_token.text); match(TOKEN_IDENTIFIER);
            strcpy(current_func_name, name);
            match(TOKEN_LPAREN);
            if (strcmp(name, "main") == 0) {
                codegen_write("int main(");
            } else {
                // Determine return type based on usage (default to PythonList if generic, or primitive if math-heavy)
                const char *ret_type = "PythonList";
                if (func_has_return(name)) {
                    if (func_has_double_param(name)) ret_type = "double";
                    else if (func_has_float_param(name)) ret_type = "float";
                } else {
                    ret_type = "void";
                }
                codegen_write("%s %s(", ret_type, name);
            }
            int param_idx = 0;
            while (current_token.type != TOKEN_RPAREN) {
                const char *param_type = get_func_param_type(name, param_idx);
                codegen_write("%s %s", param_type, current_token.text);
                // Add parameter to symbol table so its type is known inside the body
                add_symbol(current_token.text, param_type);
                advance_token();
                if (current_token.type == TOKEN_COMMA) { advance_token(); codegen_write(", "); }
                param_idx++;
            }
            match(TOKEN_RPAREN);
            codegen_write(")");
            parse_block();
        } else if (strcmp(current_token.text, "print") == 0) {
            match(TOKEN_KEYWORD);
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; }
            parse_print(); codegen_newline();
        } else if (strcmp(current_token.text, "if") == 0) {
            match(TOKEN_KEYWORD);
            if (strcmp(current_token.text, "__name__") == 0) {
                // if __name__ == "__main__": 
                while (current_token.type != TOKEN_COLON) advance_token();
                match(TOKEN_COLON);
                match(TOKEN_NEWLINE);
                match(TOKEN_INDENT);
                if (strcmp(current_token.text, "main") == 0) {
                    // skip redundant main() call
                    while (current_token.type != TOKEN_DEDENT && current_token.type != TOKEN_EOF) advance_token();
                    if (current_token.type == TOKEN_DEDENT) advance_token();
                    codegen_write("// Redundant entry point skipped");
                } else {
                    codegen_write("// Entry point logic");
                    parse_block_internal(); // manual block parsing without matching colon
                }
            } else {
                if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; }
                codegen_write("if (");
                char cond[128] = {0}; parse_expression(cond, 0);
                const char *type = get_symbol_type(cond);
                if (strstr(type, "[]")) {
                    codegen_write("%s.len > 0)", cond);
                } else {
                    codegen_write("%s)", cond);
                }
                parse_block();
            }
        } else if (strcmp(current_token.text, "try") == 0) {
            match(TOKEN_KEYWORD);
            codegen_write("// try block start");
            parse_block();
        } else if (strcmp(current_token.text, "except") == 0) {
            match(TOKEN_KEYWORD);
            while (current_token.type != TOKEN_COLON) advance_token();
            match(TOKEN_COLON); // We consume it here for except because we skip the block
            // Ignore the block
            match(TOKEN_NEWLINE);
            match(TOKEN_INDENT);
            int depth = 1;
            while (depth > 0) {
                if (current_token.type == TOKEN_INDENT) depth++;
                else if (current_token.type == TOKEN_DEDENT) depth--;
                else if (current_token.type == TOKEN_EOF) break;
                advance_token();
            }
            codegen_write("// except block skipped");
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
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; }
            match(TOKEN_KEYWORD);
            codegen_write("while (");
            char cond[128] = {0}; parse_expression(cond, 0);
            codegen_write("%s)", cond);
            parse_block();
        } else if (strcmp(current_token.text, "for") == 0) {
            if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; }
            match(TOKEN_KEYWORD);
            char var[64]; strcpy(var, current_token.text); advance_token();
            match(TOKEN_KEYWORD); match(TOKEN_KEYWORD);
            match(TOKEN_LPAREN);
            char start[32] = "", end[32] = "", step[32] = "";
            char arg1[32] = {0}; parse_expression(arg1, 1);
            if (current_token.type == TOKEN_COMMA) {
                advance_token(); strcpy(start, arg1); parse_expression(end, 1);
                if (current_token.type == TOKEN_COMMA) { advance_token(); parse_expression(step, 1); }
            } else { strcpy(start, "0"); strcpy(end, arg1); }
            if (strlen(step) == 0) strcpy(step, "1");
            match(TOKEN_RPAREN);
            codegen_write("for (int %s = %s; %s < %s; %s += %s)", var, start, var, end, var, step);
            parse_block();
        } else { advance_token(); }
    } else if (current_token.type == TOKEN_IDENTIFIER) {
        if (!main_started) { codegen_write("int main() {"); codegen_newline(); main_started = 1; current_indent++; }
        char name[64]; strcpy(name, current_token.text); advance_token();
        if (current_token.type == TOKEN_ASSIGN) {
            match(TOKEN_ASSIGN);
            if (current_token.type == TOKEN_KEYWORD && (strcmp(current_token.text, "int") == 0 || strcmp(current_token.text, "input") == 0)) {
                // Scoping fix: always declare input target in main to avoid collisions
                codegen_write_raw("    int ");
                codegen_write_raw(name);
                codegen_write_raw(";");
                codegen_newline();
                add_symbol(name, "int"); 
                parse_input(name);
                while (current_token.type == TOKEN_RPAREN) advance_token();

            } else if (current_token.type == TOKEN_LBRACKET) {
                match(TOKEN_LBRACKET);
                if (!is_declared(name)) { codegen_write("PythonList %s; ", name); add_symbol(name, "int[]"); codegen_newline(); codegen_indent_internal(); }
                int elem_count = 0;
                while (current_token.type != TOKEN_RBRACKET) {
                    codegen_write("%s.data[%d] = %s; ", name, elem_count, current_token.text); 
                    advance_token();
                    elem_count++;
                    if (current_token.type == TOKEN_COMMA) { advance_token(); }
                }
                match(TOKEN_RBRACKET); 
                codegen_write("%s.len = %d;", name, elem_count);
            } else {
                char inferred_type[32] = "int";
                if (current_token.type == TOKEN_STRING) {
                    if (current_token.text[0] == '\'') strcpy(inferred_type, "char");
                    else strcpy(inferred_type, "char*");
                }
                else if (current_token.type == TOKEN_NUMBER && strchr(current_token.text, '.')) strcpy(inferred_type, "float");

                is_slice = 0;
                char expr[128] = {0}; parse_expression(expr, 0);

                if (last_expr_is_double) strcpy(inferred_type, "double");
                else if (last_expr_is_float) strcpy(inferred_type, "float");
                else if (strstr(expr, "tmp_list_") || (strstr(expr, "(") && !strstr(expr, "math.") && !strstr(expr, "input("))) {
                     strcpy(inferred_type, "int[]"); 
                }
                
                if (is_slice) {
                    if (!is_declared(name)) { add_symbol(name, "int[]"); codegen_write("PythonList %s;", name); codegen_newline(); codegen_indent_internal(); }
                    codegen_write("%s.len = %s - (%s);", name, slice_end, slice_start);
                    codegen_newline(); codegen_indent_internal();
                    codegen_write("for (int i = 0; i < %s.len; i++) { %s.data[i] = %s.data[(%s) + i]; }", name, name, slice_arr, slice_start);
                } else if (strcmp(inferred_type, "int[]") == 0) {
                    if (!is_declared(name)) { codegen_write("PythonList %s = %s;", name, expr); add_symbol(name, "int[]"); }
                    else codegen_write("%s = %s;", name, expr);
                } else if (strcmp(inferred_type, "char*") == 0) {
                    int str_len_val = strlen(expr) > 2 ? strlen(expr) - 2 : 0;
                    if (!is_declared(name)) { codegen_write("%s %s = %s;", inferred_type, name, expr); add_symbol(name, inferred_type); codegen_newline(); codegen_indent_internal(); codegen_write("int %s_len = %d;", name, str_len_val); }
                    else codegen_write("%s = %s; %s_len = %d;", name, expr, name, str_len_val);
                } else {
                    if (!is_declared(name)) { codegen_write("%s %s = %s;", inferred_type, name, expr); add_symbol(name, inferred_type); }
                    else codegen_write("%s = %s;", name, expr);
                }
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

static void pre_scan_imports(const char *source) {
    lexer_init(source);
    Token t = lexer_next_token();
    while (t.type != TOKEN_EOF) {
        if (t.type == TOKEN_KEYWORD && strcmp(t.text, "import") == 0) {
            t = lexer_next_token();
            if (t.type == TOKEN_IDENTIFIER) {
                if (strcmp(t.text, "math") == 0) math_used = 1;
                if (strcmp(t.text, "string") == 0) string_used = 1;
            }
        }
        t = lexer_next_token();
    }
}

static void pre_scan_len_usage(const char *source) {
    lexer_init(source);
    Token t = lexer_next_token();
    while (t.type != TOKEN_EOF) {
        if (t.type == TOKEN_IDENTIFIER && strcmp(t.text, "len") == 0) {
            Token next = lexer_next_token();
            if (next.type == TOKEN_LPAREN) {
                Token arg = lexer_next_token();
                if (arg.type == TOKEN_IDENTIFIER && len_needed_count < MAX_LEN_VARS) {
                    int already = 0;
                    for (int i = 0; i < len_needed_count; i++) {
                        if (strcmp(len_needed_vars[i], arg.text) == 0) { already = 1; break; }
                    }
                    if (!already) strcpy(len_needed_vars[len_needed_count++], arg.text);
                }
                t = lexer_next_token();
            } else {
                t = next;
            }
            continue;
        }
        t = lexer_next_token();
    }
}

static void pre_scan_functions(const char *source) {
    lexer_init(source);
    Token t = lexer_next_token();
    while (t.type != TOKEN_EOF) {
        if (t.type == TOKEN_KEYWORD && strcmp(t.text, "def") == 0) {
            t = lexer_next_token();
            if (t.type == TOKEN_IDENTIFIER && func_info_count < MAX_FUNCS) {
                strncpy(func_info[func_info_count].name, t.text, sizeof(func_info[func_info_count].name) - 1);
                func_info[func_info_count].name[sizeof(func_info[func_info_count].name) - 1] = '\0';
                func_info[func_info_count].has_return = 0;
                int current_func = func_info_count;
                func_info_count++;
                // Scan tokens inside this function body tracking INDENT/DEDENT depth
                int depth = 0;
                t = lexer_next_token();
                while (t.type != TOKEN_EOF) {
                    if (t.type == TOKEN_INDENT) {
                        depth++;
                    } else if (t.type == TOKEN_DEDENT) {
                        depth--;
                        if (depth <= 0) break;
                    } else if (t.type == TOKEN_KEYWORD && strcmp(t.text, "return") == 0 && depth > 0) {
                        func_info[current_func].has_return = 1;
                    }
                    t = lexer_next_token();
                }
                continue;
            }
        }
        t = lexer_next_token();
    }
}

// Pre-scan: collect variable types from top-level literal assignments
static void pre_scan_variable_types(const char *source) {
    lexer_init(source);
    Token t = lexer_next_token();
    while (t.type != TOKEN_EOF) {
        if (t.type == TOKEN_IDENTIFIER) {
            char var_name[64];
            strncpy(var_name, t.text, sizeof(var_name) - 1);
            var_name[sizeof(var_name) - 1] = '\0';
            Token next = lexer_next_token();
            if (next.type == TOKEN_ASSIGN) {
                Token val = lexer_next_token();
                if (val.type == TOKEN_NUMBER) {
                    add_pre_var(var_name, strchr(val.text, '.') ? "float" : "int");
                } else if (val.type == TOKEN_STRING) {
                    add_pre_var(var_name, val.text[0] == '\'' ? "char" : "char*");
                }
                t = val;
            } else {
                t = next;
            }
            continue;
        }
        t = lexer_next_token();
    }
}

// Pre-scan: determine function parameter types from call sites using pre_vars
static void pre_scan_func_call_param_types(const char *source) {
    lexer_init(source);
    Token t = lexer_next_token();
    while (t.type != TOKEN_EOF) {
        if (t.type == TOKEN_IDENTIFIER) {
            char func_name[64];
            strncpy(func_name, t.text, sizeof(func_name) - 1);
            func_name[sizeof(func_name) - 1] = '\0';
            Token next = lexer_next_token();
            if (next.type == TOKEN_LPAREN) {
                // Check if this is a known user-defined function call
                int is_known_func = 0;
                for (int i = 0; i < func_info_count; i++) {
                    if (strcmp(func_info[i].name, func_name) == 0) { is_known_func = 1; break; }
                }
                if (is_known_func) {
                    // Find or create entry
                    int idx = -1;
                    for (int i = 0; i < func_call_param_types_count; i++) {
                        if (strcmp(func_call_param_types[i].func_name, func_name) == 0) {
                            idx = i; break;
                        }
                    }
                    if (idx == -1 && func_call_param_types_count < MAX_FUNCS) {
                        idx = func_call_param_types_count;
                        strncpy(func_call_param_types[idx].func_name, func_name,
                                sizeof(func_call_param_types[0].func_name) - 1);
                        func_call_param_types[idx].func_name[sizeof(func_call_param_types[0].func_name) - 1] = '\0';
                        func_call_param_types[idx].param_count = 0;
                        func_call_param_types_count++;
                    }
                    if (idx >= 0) {
                        int param_idx = 0;
                        Token arg = lexer_next_token();
                        while (arg.type != TOKEN_RPAREN && arg.type != TOKEN_EOF
                               && arg.type != TOKEN_NEWLINE) {
                            if (arg.type == TOKEN_COMMA) {
                                param_idx++;
                                arg = lexer_next_token();
                                continue;
                            }
                            if (param_idx < MAX_PARAMS) {
                                const char *inferred = "int";
                                if (arg.type == TOKEN_IDENTIFIER) {
                                    inferred = get_pre_var_type(arg.text);
                                } else if (arg.type == TOKEN_NUMBER) {
                                    inferred = strchr(arg.text, '.') ? "float" : "int";
                                }
                                // Once a parameter is seen as float, keep it float
                                if (param_idx >= func_call_param_types[idx].param_count) {
                                    strncpy(func_call_param_types[idx].param_types[param_idx],
                                            inferred, sizeof(func_call_param_types[0].param_types[0]) - 1);
                                    func_call_param_types[idx].param_types[param_idx]
                                        [sizeof(func_call_param_types[0].param_types[0]) - 1] = '\0';
                                    func_call_param_types[idx].param_count = param_idx + 1;
                                } else if (strcmp(inferred, "float") == 0) {
                                    strncpy(func_call_param_types[idx].param_types[param_idx],
                                            "float", sizeof(func_call_param_types[0].param_types[0]) - 1);
                                }
                            }
                            arg = lexer_next_token();
                        }
                    }
                }
                t = next;
            } else {
                t = next;
            }
            continue;
        }
        t = lexer_next_token();
    }
}

void parser_run(const char *source) {
    parser_init();
    pre_scan_imports(source);
    pre_scan_len_usage(source);
    pre_scan_functions(source);
    pre_scan_variable_types(source);
    pre_scan_func_call_param_types(source);
    lexer_init(source);
    advance_token();
    
    codegen_init();
    
    // Header writing - now the flags are set!
    codegen_write("#include <stdio.h>\n");
    if (math_used) codegen_write("#include <math.h>\n");
    if (string_used) codegen_write("#include <string.h>\n");
    codegen_write("\n");

    while (current_token.type != TOKEN_EOF) {
        parse_statement();
    }
    
    if (main_started) {
        codegen_indent_internal();
        codegen_write("return 0;\n}\n");
    }
    
    codegen_finalize();
}
