#include <stdio.h>
#include <string.h>

#include "log.h"
#include "symtab.h"

static FILE *arquivo_tokens = NULL;

void log_base_from_source(const char *arquivo_fonte, char *base, int tamanho)
{
    int i;
    int ultimo_ponto = -1;

    strncpy(base, arquivo_fonte, tamanho - 1);
    base[tamanho - 1] = '\0';

    for (i = 0; base[i] != '\0'; i++)
    {
        if (base[i] == '.')
        {
            ultimo_ponto = i;
        }
    }

    if (ultimo_ponto >= 0)
    {
        base[ultimo_ponto] = '\0';
    }
}

void log_tokens_from_source(const char *arquivo_fonte)
{
    char base[512];
    char caminho[520];

    log_base_from_source(arquivo_fonte, base, sizeof(base));
    snprintf(caminho, sizeof(caminho), "%s.tk", base);
    arquivo_tokens = fopen(caminho, "w");
}

void log_symtab_from_source(const char *arquivo_fonte)
{
    char base[512];
    char caminho[520];

    log_base_from_source(arquivo_fonte, base, sizeof(base));
    snprintf(caminho, sizeof(caminho), "%s.ts", base);
    ts_dump_to_file(caminho);
}

void log_trace_from_source(const char *arquivo_fonte, char *caminho, int tamanho)
{
    char base[512];

    log_base_from_source(arquivo_fonte, base, sizeof(base));
    snprintf(caminho, tamanho, "%s.trc", base);
}

void log_token_write(const TInfoAtomo *token)
{
    if (arquivo_tokens == NULL || token == NULL)
    {
        return;
    }

    fprintf(arquivo_tokens, "%d  %s  \"%s\"\n",
            token->linha,
            lex_token_name(token->atomo),
            token->texto);
}

void log_token_close(void)
{
    if (arquivo_tokens != NULL)
    {
        fclose(arquivo_tokens);
        arquivo_tokens = NULL;
    }
}
