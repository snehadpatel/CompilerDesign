#include "codegen.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static FILE *out_file = NULL;
static int current_indent = 0;
static char headers[10][64];
static int header_count = 0;

void codegen_init() {
    out_file = fopen("outputs/output.c", "w");
    if (!out_file) {
        perror("Failed to open outputs/output.c");
        exit(1);
    }
    header_count = 0;
    codegen_add_header("stdio.h");
    codegen_add_header("stdlib.h");
    codegen_add_header("string.h");
    codegen_add_header("stdbool.h");

    // Project Boilerplate: Python-like List for C
    codegen_write_raw("#include <stdio.h>\n");
    codegen_write_raw("#include <stdbool.h>\n\n");
    codegen_write_raw("typedef struct {\n");
    codegen_write_raw("    int data[100];\n");
    codegen_write_raw("    int len;\n");
    codegen_write_raw("} PythonList;\n\n");
    
    codegen_write_raw("void print_list(PythonList list) {\n");
    codegen_write_raw("    printf(\"[\");\n");
    codegen_write_raw("    for (int i = 0; i < list.len; i++) {\n");
    codegen_write_raw("        printf(\"%d\", list.data[i]);\n");
    codegen_write_raw("        if (i < list.len - 1) printf(\", \");\n");
    codegen_write_raw("    }\n");
    codegen_write_raw("    printf(\"]\\n\");\n");
    codegen_write_raw("}\n\n");
}

void codegen_add_header(const char *header) {
    for (int i = 0; i < header_count; i++) {
        if (strcmp(headers[i], header) == 0) return;
    }
    strcpy(headers[header_count++], header);
}

void codegen_indent() {
    for (int i = 0; i < current_indent; i++) fprintf(out_file, "    ");
}

void codegen_write(const char *format, ...) {
    va_list args;
    va_start(args, format);
    if (out_file) vfprintf(out_file, format, args);
    va_end(args);
}

void codegen_write_raw(const char *text) {
    if (out_file) fputs(text, out_file);
}

void codegen_newline() {
    if (out_file) fprintf(out_file, "\n");
    codegen_indent();
}

void codegen_start_block() {
    fprintf(out_file, " {\n");
    current_indent++;
    codegen_indent();
}

void codegen_end_block() {
    current_indent--;
    fprintf(out_file, "\n");
    codegen_indent();
    fprintf(out_file, "}");
}

void codegen_finalize() {
    // This is a bit hacky - real compilers do this with a buffer
    // For this lab, we'll just write headers and main manually in parser or here
    fclose(out_file);
}
