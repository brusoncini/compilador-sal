#ifndef DIAG_H
#define DIAG_H

void diag_set_trace(int ativo);
void diag_set_trace_file(const char *caminho);
void diag_close(void);

void diag_info(const char *fmt, ...);
void diag_error_lexico(int linha, const char *mensagem, const char *lexema);
void diag_error_sintatico(int linha, const char *esperado, const char *encontrado, const char *lexema);
void diag_error_semantico(int linha, const char *mensagem, const char *lexema);

#endif
