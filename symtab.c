/*
    Gerencia escopos, inserção e busca de identificadores.
    Deve espelhar a estrutura de blocos da SAL (globals, locals e blocos internos de start...end) e suas regras de visibilidade.
    Interface: ts_insert, ts_lookup
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symtab.h"
#include "log.h"

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
    Simbolo *atual_simbolo = lista_simbolos;
    Simbolo *prox_simbolo;

    Escopo *atual_escopo = pilha_escopos;
    Escopo *prox_escopo;

    while (atual_simbolo != NULL)
    {
        prox_simbolo = atual_simbolo->prox;
        free(atual_simbolo);
        atual_simbolo = prox_simbolo;
    }

    while (atual_escopo != NULL)
    {
        prox_escopo = atual_escopo->anterior;
        free(atual_escopo);
        atual_escopo = prox_escopo;
    }

    lista_simbolos = NULL;
    pilha_escopos = NULL;
}

void ts_enter_scope(const char *nome)
{
    Escopo *novo = criar_escopo(nome);
    novo->anterior = pilha_escopos;
    pilha_escopos = novo;
}

void ts_exit_scope(void)
{
    Escopo *removido;

    if (pilha_escopos == NULL)
    {
        return;
    }

    removido = pilha_escopos;
    pilha_escopos = pilha_escopos->anterior;
    free(removido);
}

const char *ts_current_scope(void)
{
    if (pilha_escopos == NULL)
    {
        return "sem_escopo";
    }

    return pilha_escopos->nome;
}

int ts_insert(const char *lexema, const char *categoria, const char *tipo, int extra)
{
    Simbolo *atual = lista_simbolos;
    Simbolo *novo;

    while (atual != NULL)
    {
        if (strcmp(atual->lexema, lexema) == 0 &&
            strcmp(atual->escopo, ts_current_scope()) == 0)
        {
            return 0;
        }

        atual = atual->prox;
    }

    novo = criar_simbolo(lexema, categoria, tipo, ts_current_scope(), extra);

    if (lista_simbolos == NULL)
    {
        lista_simbolos = novo;
    }
    else
    {
        atual = lista_simbolos;
        while (atual->prox != NULL)
        {
            atual = atual->prox;
        }
        atual->prox = novo;
    }

    return 1;
}

Simbolo *ts_lookup(const char *lexema)
{
    Escopo *escopo_atual = pilha_escopos;

    while (escopo_atual != NULL)
    {
        Simbolo *atual = lista_simbolos;

        while (atual != NULL)
        {
            if (strcmp(atual->lexema, lexema) == 0 &&
                strcmp(atual->escopo, escopo_atual->nome) == 0)
            {
                return atual;
            }

            atual = atual->prox;
        }

        escopo_atual = escopo_atual->anterior;
    }

    return NULL;
}

void ts_print(void)
{
    Simbolo *atual = lista_simbolos;

    printf("\n=== TABELA DE SIMBOLOS ===\n");

    while (atual != NULL)
    {
        printf("SCOPE=%s  id=\"%s\"  cat=%s  tipo=%s  extra=%d\n",
               atual->escopo,
               atual->lexema,
               atual->categoria,
               atual->tipo,
               atual->extra);

        log_symtab_line(
            atual->escopo,
            atual->lexema,
            atual->categoria,
            atual->tipo,
            atual->extra);

        atual = atual->prox;
    }
}
