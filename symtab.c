#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.h"

typedef struct Escopo
{
    char nome[TAM_ESCOPO];
    struct Escopo *anterior;
} Escopo;

static Simbolo *lista_simbolos = NULL;
static Escopo *pilha_escopos = NULL;

static Escopo *criar_escopo(const char *nome)
{
    Escopo *novo = (Escopo *)malloc(sizeof(Escopo));
    if (novo == NULL)
    {
        printf("Erro ao alocar memoria para escopo.\n");
        exit(1);
    }
    strncpy(novo->nome, nome, TAM_ESCOPO - 1);
    novo->nome[TAM_ESCOPO - 1] = '\0';
    novo->anterior = NULL;
    return novo;
}

static Simbolo *criar_simbolo(const char *lexema, const char *categoria, const char *tipo, const char *escopo, int extra)
{
    Simbolo *novo = (Simbolo *)malloc(sizeof(Simbolo));
    if (novo == NULL)
    {
        printf("Erro ao alocar memoria para simbolo.\n");
        exit(1);
    }

    strncpy(novo->lexema, lexema, TAM_LEXEMA - 1);
    novo->lexema[TAM_LEXEMA - 1] = '\0';
    strncpy(novo->categoria, categoria, TAM_CATEGORIA - 1);
    novo->categoria[TAM_CATEGORIA - 1] = '\0';
    strncpy(novo->tipo, tipo, TAM_TIPO - 1);
    novo->tipo[TAM_TIPO - 1] = '\0';
    strncpy(novo->escopo, escopo, TAM_ESCOPO - 1);
    novo->escopo[TAM_ESCOPO - 1] = '\0';
    novo->extra = extra;
    novo->prox = NULL;
    return novo;
}

void ts_init(void)
{
    lista_simbolos = NULL;
    pilha_escopos = NULL;
    ts_enter_scope("global");
}

void ts_free(void)
{
    Simbolo *sim = lista_simbolos;
    while (sim != NULL)
    {
        Simbolo *prox = sim->prox;
        free(sim);
        sim = prox;
    }
    lista_simbolos = NULL;

    while (pilha_escopos != NULL)
    {
        Escopo *prox = pilha_escopos->anterior;
        free(pilha_escopos);
        pilha_escopos = prox;
    }
}

void ts_enter_scope(const char *nome)
{
    Escopo *novo = criar_escopo(nome);
    novo->anterior = pilha_escopos;
    pilha_escopos = novo;
}

void ts_exit_scope(void)
{
    if (pilha_escopos != NULL)
    {
        Escopo *removido = pilha_escopos;
        pilha_escopos = pilha_escopos->anterior;
        free(removido);
    }
}

const char *ts_current_scope(void)
{
    if (pilha_escopos == NULL)
    {
        return "sem_escopo";
    }
    return pilha_escopos->nome;
}

Simbolo *ts_lookup_in_scope(const char *lexema, const char *escopo)
{
    Simbolo *atual = lista_simbolos;
    while (atual != NULL)
    {
        if (strcmp(atual->lexema, lexema) == 0 && strcmp(atual->escopo, escopo) == 0)
        {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;
}

int ts_insert(const char *lexema, const char *categoria, const char *tipo, int extra)
{
    Simbolo *novo;
    Simbolo *atual;

    if (ts_lookup_in_scope(lexema, ts_current_scope()) != NULL)
    {
        return 0;
    }

    novo = criar_simbolo(lexema, categoria, tipo, ts_current_scope(), extra);
    if (lista_simbolos == NULL)
    {
        lista_simbolos = novo;
        return 1;
    }

    atual = lista_simbolos;
    while (atual->prox != NULL)
    {
        atual = atual->prox;
    }
    atual->prox = novo;
    return 1;
}

Simbolo *ts_lookup(const char *lexema)
{
    Escopo *esc = pilha_escopos;
    while (esc != NULL)
    {
        Simbolo *encontrado = ts_lookup_in_scope(lexema, esc->nome);
        if (encontrado != NULL)
        {
            return encontrado;
        }
        esc = esc->anterior;
    }
    return NULL;
}

static void ts_imprimir_em(FILE *saida)
{
    Simbolo *atual = lista_simbolos;
    while (atual != NULL)
    {
        fprintf(saida, "SCOPE=%s  id=\"%s\"  cat=%s  tipo=%s  extra=%d\n",
                atual->escopo,
                atual->lexema,
                atual->categoria,
                atual->tipo,
                atual->extra);
        atual = atual->prox;
    }
}

void ts_print(void)
{
    printf("\n=== TABELA DE SIMBOLOS ===\n");
    ts_imprimir_em(stdout);
}

void ts_dump_to_file(const char *caminho)
{
    FILE *arquivo = fopen(caminho, "w");
    if (arquivo == NULL)
    {
        printf("Erro ao criar arquivo de tabela de simbolos.\n");
        exit(1);
    }
    ts_imprimir_em(arquivo);
    fclose(arquivo);
}
