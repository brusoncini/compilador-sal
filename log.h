#ifndef LOG_H
#define LOG_H

#include "lex.h"

int log_tokens_init(const char *arquivo_fonte);
void log_token(TInfoAtomo token);
void log_tokens_close(void);

#endif