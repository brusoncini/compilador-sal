#ifndef LOG_H
#define LOG_H

#include "lex.h"

void log_base_from_source(const char *arquivo_fonte, char *base, int tamanho);
void log_tokens_from_source(const char *arquivo_fonte);
void log_symtab_from_source(const char *arquivo_fonte);
void log_trace_from_source(const char *arquivo_fonte, char *caminho, int tamanho);
void log_token_write(const TInfoAtomo *token);
void log_token_close(void);

#endif
