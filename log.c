#include <stdio.h>
#include <string.h>

#include "log.h"

static FILE *arquivo_tokens = NULL;
static FILE *arquivo_symtab = NULL;

static void montar_nome_tk(const char *arquivo_fonte, char *saida, int tamanho)
{
    int i = 0;
    int ultima_barrinha = -1;
    int ultimo_ponto = -1;

    while (arquivo_fonte[i] != '\0' && i < tamanho - 4)
    {
        saida[i] = arquivo_fonte[i];

        if (arquivo_fonte[i] == '/' || arquivo_fonte[i] == '\\')
        {
            ultima_barrinha = i;
        }

        if (arquivo_fonte[i] == '.')
        {
            ultimo_ponto = i;
        }

        i++;
    }

    saida[i] = '\0';

    if (ultimo_ponto > ultima_barrinha)
    {
        saida[ultimo_ponto] = '\0';
    }

    strcat(saida, ".tk");
}

static void trocar_extensao(char *nome, const char *extensao)
{
    int i;
    int ultimo_ponto = -1;

    for (i = 0; nome[i] != '\0'; i++)
    {
        if (nome[i] == '.')
        {
            ultimo_ponto = i;
        }
    }

    if (ultimo_ponto != -1)
    {
        nome[ultimo_ponto] = '\0';
    }

    strcat(nome, extensao);
}

int log_tokens_init(const char *arquivo_fonte)
{
    char nome_saida[300];

    montar_nome_tk(arquivo_fonte, nome_saida, sizeof(nome_saida));

    arquivo_tokens = fopen(nome_saida, "w");

    if (arquivo_tokens == NULL)
    {
        printf("Nao foi possivel criar o arquivo de tokens.\n");
        return 0;
    }

    return 1;
}

void log_token(TInfoAtomo token)
{
    if (arquivo_tokens == NULL)
    {
        return;
    }

    fprintf(
        arquivo_tokens,
        "%d  %s  \"%s\"\n",
        token.linha,
        lex_nome_atomo(token.atomo),
        token.texto
    );
}

void log_tokens_close(void)
{
    if (arquivo_tokens != NULL)
    {
        fclose(arquivo_tokens);
        arquivo_tokens = NULL;
    }
}

int log_symtab_init(const char *arquivo_fonte)
{
    char nome_saida[300];

    strcpy(nome_saida, arquivo_fonte);

    trocar_extensao(nome_saida, ".ts");

    arquivo_symtab = fopen(nome_saida, "w");

    if (arquivo_symtab == NULL)
    {
        printf("Nao foi possivel criar o arquivo de tabela de simbolos.\n");
        return 0;
    }

    return 1;
}

void log_symtab_line(
    const char *escopo,
    const char *lexema,
    const char *categoria,
    const char *tipo,
    int extra
)
{
    if (arquivo_symtab == NULL)
    {
        return;
    }

    fprintf(
        arquivo_symtab,
        "SCOPE=%s  id=\"%s\"  cat=%s  tipo=%s  extra=%d\n",
        escopo,
        lexema,
        categoria,
        tipo,
        extra
    );
}

void log_symtab_close(void)
{
    if (arquivo_symtab != NULL)
    {
        fclose(arquivo_symtab);
        arquivo_symtab = NULL;
    }
}