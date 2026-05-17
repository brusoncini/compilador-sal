#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "diag.h"

static int trace_ativo = 0;
static FILE *arquivo_trace = NULL;
static int trace_indentacao = 0;

void diag_set_trace(int ativo)
{
    trace_ativo = ativo;
}

void diag_set_trace_file(const char *caminho)
{
    if (arquivo_trace != NULL)
    {
        fclose(arquivo_trace);
        arquivo_trace = NULL;
    }

    if (caminho != NULL)
    {
        arquivo_trace = fopen(caminho, "w");
        if (arquivo_trace == NULL)
        {
            printf("Erro: nao foi possivel criar arquivo de trace '%s'.\n", caminho);
            exit(1);
        }
    }
}

void diag_close(void)
{
    if (arquivo_trace != NULL)
    {
        fclose(arquivo_trace);
        arquivo_trace = NULL;
    }
}

static void diag_escrever_indentacao(void)
{
    int i;

    if (arquivo_trace == NULL)
    {
        return;
    }

    for (i = 0; i < trace_indentacao; i++)
    {
        fprintf(arquivo_trace, "  ");
    }
}

void diag_info(const char *fmt, ...)
{
    va_list args;
    char mensagem[512];

    if (!trace_ativo)
    {
        return;
    }

    va_start(args, fmt);
    vsnprintf(mensagem, sizeof(mensagem), fmt, args);
    va_end(args);

    if (arquivo_trace != NULL)
    {
        if (strncmp(mensagem, "SAI", 3) == 0 && trace_indentacao > 0)
        {
            trace_indentacao--;
        }

        diag_escrever_indentacao();
        fprintf(arquivo_trace, "%s\n", mensagem);

        if (strncmp(mensagem, "ENTRA", 5) == 0)
        {
            trace_indentacao++;
        }

        fflush(arquivo_trace);
    }
}

void diag_error_lexico(int linha, const char *mensagem, const char *lexema)
{
    printf("Erro lexico na linha %d: %s (%s)\n", linha, mensagem, lexema != NULL ? lexema : "");
    diag_close();
    exit(1);
}

void diag_error_sintatico(int linha, const char *esperado, const char *encontrado, const char *lexema)
{
    printf("Erro sintatico na linha %d: esperado %s, encontrado %s (%s)\n",
           linha,
           esperado != NULL ? esperado : "?",
           encontrado != NULL ? encontrado : "?",
           lexema != NULL ? lexema : "");
    diag_close();
    exit(1);
}

void diag_error_semantico(int linha, const char *mensagem, const char *lexema)
{
    printf("Erro semantico na linha %d: %s (%s)\n",
           linha,
           mensagem != NULL ? mensagem : "erro semantico",
           lexema != NULL ? lexema : "");
    diag_close();
    exit(1);
}
