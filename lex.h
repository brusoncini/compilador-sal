#ifndef LEX_H
#define LEX_H

#include <stdio.h>

typedef enum
{
    CONST_INT,
    CONST_CHAR,
    STRING,
    IDENTIFICADOR,
    TK_MODULE,
    TK_PROC,
    TK_FN,
    TK_MAIN,
    TK_GLOBALS,
    TK_LOCALS,
    TK_START,
    TK_END,
    TK_IF,
    TK_ELSE,
    TK_MATCH,
    TK_WHEN,
    TK_OTHERWISE,
    TK_FOR,
    TK_TO,
    TK_STEP,
    TK_DO,
    TK_LOOP,
    TK_WHILE,
    TK_UNTIL,
    TK_PRINT,
    TK_SCAN,
    TK_RET,
    TK_INT,
    TK_BOOL,
    TK_CHAR,
    TK_TRUE,
    TK_FALSE,
    ATRIBUICAO,
    SOMA,
    SUBTRACAO,
    MULTIPLICACAO,
    DIVISAO,
    IGUAL,
    DIFERENTE,
    MAIOR,
    MENOR,
    MAIOR_IGUAL,
    MENOR_IGUAL,
    E_LOGICO,
    OU_LOGICO,
    NEGACAO,
    ABRE_PAR,
    FECHA_PAR,
    ABRE_COL,
    FECHA_COL,
    VIRGULA,
    PONTO_E_VIRGULA,
    DOIS_PONTOS,
    INTERVALO,
    IMPLICA,
    ERRO,
    FIM_ARQUIVO
} TAtomo;

typedef struct
{
    TAtomo atomo;
    int linha;
    int atributo_numero;
    char texto[256];
} TInfoAtomo;

int lex_init(FILE *arquivo);
TInfoAtomo lex_next(void);
const char *lex_token_name(TAtomo atomo);

#endif
