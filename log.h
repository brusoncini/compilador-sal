#ifndef LOG_H
#define LOG_H

#include "lex.h"

int log_tokens_init(const char *arquivo_fonte);
void log_token(TInfoAtomo token);
void log_tokens_close(void);

int log_symtab_init(const char *arquivo_fonte);
void log_symtab_line(
    const char *escopo,
    const char *lexema,
    const char *categoria,
    const char *tipo,
    int extra);
void log_symtab_close(void);

#endif