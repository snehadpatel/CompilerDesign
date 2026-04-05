#ifndef CODEGEN_H
#define CODEGEN_H

#include "lexer.h"

void codegen_init();
void codegen_add_header(const char *header);
void codegen_start_block();
void codegen_end_block();
void codegen_write(const char *format, ...);
void codegen_write_raw(const char *text);
void codegen_newline();
void codegen_finalize();
void codegen_declare_variable(const char *name, const char *type);
void codegen_indent();

#endif
