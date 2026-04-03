/*
Consome tokens de lex e implementa a gramática da SAL.
Cada não‑terminal corresponde a uma função específica.
É o responsável por criar e encerrar escopos durante a análise.
Interface: parse_program
*/

#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "parser.h"

static TInfoAtomo token_atual;

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
    case IDENTIFICADOR:
        return "IDENTIFICADOR";
    case PONTO_E_VIRGULA:
        return "PONTO_E_VIRGULA";
    case ABRE_PAR:
        return "ABRE_PAR";
    case FECHA_PAR:
        return "FECHA_PAR";
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

static void parse_block(void)
{
    consumir(TK_START);
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

    parse_block();

    consumir(FIM_ARQUIVO);
}