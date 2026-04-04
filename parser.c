#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "parser.h"

static TInfoAtomo token_atual;

static void parse_print(void);
static void parse_scan(void);
static void parse_atribuicao(void);
static void parse_comando(void);
static void parse_bloco(void);

static void parse_expr(void);
static void parse_and(void);
static void parse_relacional(void);
static void parse_soma_sub(void);
static void parse_mult_div(void);
static void parse_fator(void);

static int eh_relacional(TAtomo atomo);

/*
 As expressoes sao analisadas na seguinte ordem:
    - parse_expr: v
    - parse_and: ^
    - parse_relacional: > < >= <= = <>
    - parse_soma_sub: + -
    - parse_mult_div: * /
    - parse_fator: id, constantes, parenteses e unarios
*/

// Funcao apenas para debug: usa nomes mais claros
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
    case TK_TRUE:
        return "TK_TRUE";
    case TK_FALSE:
        return "TK_FALSE";
    case IDENTIFICADOR:
        return "IDENTIFICADOR";
    case CONST_INT:
        return "CONST_INT";
    case CONST_CHAR:
        return "CONST_CHAR";
    case STRING:
        return "STRING";
    case ATRIBUICAO:
        return "ATRIBUICAO";
    case SOMA:
        return "SOMA";
    case SUBTRACAO:
        return "SUBTRACAO";
    case MULTIPLICACAO:
        return "MULTIPLICACAO";
    case DIVISAO:
        return "DIVISAO";
    case IGUAL:
        return "IGUAL";
    case DIFERENTE:
        return "DIFERENTE";
    case MAIOR:
        return "MAIOR";
    case MENOR:
        return "MENOR";
    case MAIOR_IGUAL:
        return "MAIOR_IGUAL";
    case MENOR_IGUAL:
        return "MENOR_IGUAL";
    case E_LOGICO:
        return "E_LOGICO";
    case OU_LOGICO:
        return "OU_LOGICO";
    case NEGACAO:
        return "NEGACAO";
    case ABRE_PAR:
        return "ABRE_PAR";
    case FECHA_PAR:
        return "FECHA_PAR";
    case VIRGULA:
        return "VIRGULA";
    case PONTO_E_VIRGULA:
        return "PONTO_E_VIRGULA";
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

static int eh_relacional(TAtomo atomo)
{
    return atomo == IGUAL ||
           atomo == DIFERENTE ||
           atomo == MAIOR ||
           atomo == MENOR ||
           atomo == MAIOR_IGUAL ||
           atomo == MENOR_IGUAL;
}

// O fator é a menor unidade da expressão
static void parse_fator(void)
{
    if (token_atual.atomo == NEGACAO)
    {
        consumir(NEGACAO);
        parse_fator();
    }
    else if (token_atual.atomo == SUBTRACAO)
    {
        consumir(SUBTRACAO);
        parse_fator();
    }
    else if (token_atual.atomo == IDENTIFICADOR)
    {
        consumir(IDENTIFICADOR);
    }
    else if (token_atual.atomo == CONST_INT)
    {
        consumir(CONST_INT);
    }
    else if (token_atual.atomo == CONST_CHAR)
    {
        consumir(CONST_CHAR);
    }
    else if (token_atual.atomo == STRING)
    {
        consumir(STRING);
    }
    else if (token_atual.atomo == TK_TRUE)
    {
        consumir(TK_TRUE);
    }
    else if (token_atual.atomo == TK_FALSE)
    {
        consumir(TK_FALSE);
    }
    else if (token_atual.atomo == ABRE_PAR)
    {
        consumir(ABRE_PAR);
        parse_expr();
        consumir(FECHA_PAR);
    }
    else
    {
        printf("Erro de sintaxe na linha %d: fator invalido (%s)\n",
               token_atual.linha,
               token_atual.texto);
        exit(1);
    }
}

static void parse_mult_div(void)
{
    parse_fator();

    while (token_atual.atomo == MULTIPLICACAO || token_atual.atomo == DIVISAO)
    {
        if (token_atual.atomo == MULTIPLICACAO)
        {
            consumir(MULTIPLICACAO);
        }
        else
        {
            consumir(DIVISAO);
        }

        parse_fator();
    }
}

static void parse_soma_sub(void)
{
    parse_mult_div();

    while (token_atual.atomo == SOMA || token_atual.atomo == SUBTRACAO)
    {
        if (token_atual.atomo == SOMA)
        {
            consumir(SOMA);
        }
        else
        {
            consumir(SUBTRACAO);
        }

        parse_mult_div();
    }
}

static void parse_relacional(void)
{
    parse_soma_sub();

    while (eh_relacional(token_atual.atomo))
    {
        TAtomo operador = token_atual.atomo;
        consumir(operador);
        parse_soma_sub();
    }
}

static void parse_and(void)
{
    parse_relacional();

    while (token_atual.atomo == E_LOGICO)
    {
        consumir(E_LOGICO);
        parse_relacional();
    }
}

static void parse_expr(void)
{
    parse_and();

    while (token_atual.atomo == OU_LOGICO)
    {
        consumir(OU_LOGICO);
        parse_and();
    }
}

static void parse_print(void)
{
    consumir(TK_PRINT);
    consumir(ABRE_PAR);

    parse_expr();

    while (token_atual.atomo == VIRGULA)
    {
        consumir(VIRGULA);
        parse_expr();
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

static void parse_atribuicao(void)
{
    consumir(IDENTIFICADOR);
    consumir(ATRIBUICAO);
    parse_expr();
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
    else if (token_atual.atomo == IDENTIFICADOR)
    {
        parse_atribuicao();
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