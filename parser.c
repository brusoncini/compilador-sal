#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "parser.h"

static TInfoAtomo token_atual;

static void parse_elemento(void);
static void parse_print(void);
static void parse_scan(void);
static void parse_comando(void);
static void parse_bloco(void);

// Consome tokens de lex e implementa a gramática da SAL.
static const char *nome_atomo(TAtomo atomo)
{
    switch (atomo)
    {
    case FIM_ARQUIVO:
        return "FIM_ARQUIVO";
    case ERRO:
        return "ERRO";
    case TK_MODULE:
        return "TK_MODULE";
    case TK_PROC:
        return "TK_PROC";
    case TK_MAIN:
        return "TK_MAIN";
    case TK_START:
        return "TK_START";
    case TK_END:
        return "TK_END";
    case TK_PRINT:
        return "TK_PRINT";
    case TK_SCAN:
        return "TK_SCAN";
    case IDENTIFICADOR:
        return "IDENTIFICADOR";
    case STRING:
        return "STRING";
    case CONST_INT:
        return "CONST_INT";
    case CONST_CHAR:
        return "CONST_CHAR";
    case PONTO_E_VIRGULA:
        return "PONTO_E_VIRGULA";
    case ABRE_PAR:
        return "ABRE_PAR";
    case FECHA_PAR:
        return "FECHA_PAR";
    case VIRGULA:
        return "VIRGULA";
    default:
        return "TOKEN_DESCONHECIDO";
    }
}

static void erro_sintatico(TAtomo esperado)
{
    printf("Erro sintatico na linha %d: esperado %s, encontrado %s (%s)\n",
           token_atual.linha,
           nome_atomo(esperado),
           nome_atomo(token_atual.atomo),
           token_atual.texto);
    exit(1);
}

static void avancar(void)
{
    token_atual = lex_next();
}

static void consumir(TAtomo esperado)
{
    if (token_atual.atomo != esperado)
    {
        erro_sintatico(esperado);
    }

    avancar();
}

// Cria e encerra escopos durante a análise.
static void parse_elemento(void)
{
    if (token_atual.atomo == STRING)
    {
        consumir(STRING);
    }
    else if (token_atual.atomo == CONST_INT)
    {
        consumir(CONST_INT);
    }
    else if (token_atual.atomo == CONST_CHAR)
    {
        consumir(CONST_CHAR);
    }
    else if (token_atual.atomo == IDENTIFICADOR)
    {
        consumir(IDENTIFICADOR);
    }
    else
    {
        printf("Erro de sintaxe na linha %d: elemento invalido (%s)\n",
               token_atual.linha,
               token_atual.texto);
        exit(1);
    }
}

static void parse_print(void)
{
    consumir(TK_PRINT);
    consumir(ABRE_PAR);

    parse_elemento();

    while (token_atual.atomo == VIRGULA)
    {
        consumir(VIRGULA);
        parse_elemento();
    }

    consumir(FECHA_PAR);
}

static void parse_scan(void)
{
    consumir(TK_SCAN);
    consumir(ABRE_PAR);
    consumir(IDENTIFICADOR);
    consumir(FECHA_PAR);
}

static void parse_comando(void)
{
    if (token_atual.atomo == TK_PRINT)
    {
        parse_print();
    }
    else if (token_atual.atomo == TK_SCAN)
    {
        parse_scan();
    }
    else if (token_atual.atomo == TK_START)
    {
        parse_bloco();
    }
    else
    {
        printf("Erro de sintaxe na linha %d: comando invalido (%s)\n",
               token_atual.linha,
               token_atual.texto);
        exit(1);
    }
}

static void parse_bloco(void)
{
    consumir(TK_START);

    while (token_atual.atomo != TK_END)
    {
        parse_comando();
        consumir(PONTO_E_VIRGULA);
    }

    consumir(TK_END);
}

void parser_init(void)
{
    avancar();
}

void parse_program(void)
{
    consumir(TK_MODULE);
    consumir(IDENTIFICADOR);
    consumir(PONTO_E_VIRGULA);

    consumir(TK_PROC);
    consumir(TK_MAIN);
    consumir(ABRE_PAR);
    consumir(FECHA_PAR);

    parse_bloco();

    consumir(FIM_ARQUIVO);
}