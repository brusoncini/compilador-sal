#include <stdio.h>
#include <stdlib.h>

#include "diag.h"
#include "log.h"

void diag_info(const char *mensagem)
{
    printf("%s\n", mensagem);
    log_trace_line(mensagem);
}

void diag_error(const char *mensagem)
{
    printf("Erro: %s\n", mensagem);
    exit(1);
}

void diag_error_linha(int linha, const char *mensagem)
{
    printf("Erro na linha %d: %s\n", linha, mensagem);
    exit(1);
}