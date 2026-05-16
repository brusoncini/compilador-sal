#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "parser.h"
#include "symtab.h"

static TInfoAtomo token_atual;

// Comandos
static void parse_print(void);
static void parse_scan(void);
static void parse_if(void);
static void parse_ret(void);
static void parse_loop(void);
static void parse_for(void);
static void parse_match(void);
static void parse_when(void);
static void parse_condicao_when(void);
static void parse_item_when(void);
static void parse_comando(void);
static void parse_comando_identificador(void);
static void parse_bloco(void);

// Declaracoes
static void parse_tipo(void);
static const char *tipo_atual_para_texto(TAtomo atomo);
static void parse_identificador_declaracao(char nome[], int *extra);
static void parse_declaracao(void);
static void parse_globals(void);
static void parse_locals(void);
static void parse_parametro(void);
static void parse_lista_parametros(int *quantidade_parametros);
static void parse_proc_decl_resto(void);
static void parse_funcao_decl(void);
static void parse_indice_vetor(void);
static void parse_chamada_resto(void);

/*
 As expressoes sao analisadas na seguinte ordem:
    - parse_expr: v
    - parse_and: ^
    - parse_relacional: > < >= <= = <>
    - parse_soma_sub: + -
    - parse_mult_div: * /
    - parse_fator: id, constantes, parenteses e unarios
*/

// Expressoes
static void parse_expr(void);
static void parse_and(void);
static void parse_relacional(void);
static void parse_soma_sub(void);
static void parse_mult_div(void);
static void parse_fator(void);

static int eh_relacional(TAtomo atomo);
static int eh_ou_logico(void);
static void consumir_ou_logico(void);

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
    case TK_FN:
        return "TK_FN";
    case TK_MAIN:
        return "TK_MAIN";
    case TK_GLOBALS:
        return "TK_GLOBALS";
    case TK_LOCALS:
        return "TK_LOCALS";
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
    case TK_INT:
        return "TK_INT";
    case TK_BOOL:
        return "TK_BOOL";
    case TK_CHAR:
        return "TK_CHAR";
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
    case ABRE_COL:
        return "ABRE_COL";
    case FECHA_COL:
        return "FECHA_COL";
    case VIRGULA:
        return "VIRGULA";
    case PONTO_E_VIRGULA:
        return "PONTO_E_VIRGULA";
    case DOIS_PONTOS:
        return "DOIS_PONTOS";
    case TK_IF:
        return "TK_IF";
    case TK_ELSE:
        return "TK_ELSE";
    case TK_RET:
        return "TK_RET";
    case TK_LOOP:
        return "TK_LOOP";
    case TK_WHILE:
        return "TK_WHILE";
    case TK_UNTIL:
        return "TK_UNTIL";
    case TK_FOR:
        return "TK_FOR";
    case TK_TO:
        return "TK_TO";
    case TK_STEP:
        return "TK_STEP";
    case TK_DO:
        return "TK_DO";
    case TK_MATCH:
        return "TK_MATCH";
    case TK_WHEN:
        return "TK_WHEN";
    case TK_OTHERWISE:
        return "TK_OTHERWISE";
    case IMPLICA:
        return "IMPLICA";
    case INTERVALO:
        return "INTERVALO";
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

// Diferencia v (OU lógico) de v como identificador
static int eh_ou_logico(void)
{
    return token_atual.atomo == OU_LOGICO ||
           (token_atual.atomo == IDENTIFICADOR && strcmp(token_atual.texto, "v") == 0);
}

static void consumir_ou_logico(void)
{
    if (token_atual.atomo == OU_LOGICO)
    {
        consumir(OU_LOGICO);
    }
    else if (token_atual.atomo == IDENTIFICADOR && strcmp(token_atual.texto, "v") == 0)
    {
        avancar();
    }
    else
    {
        printf("Erro de sintaxe na linha %d: esperado operador logico v, encontrado %s (%s)\n",
               token_atual.linha,
               nome_atomo(token_atual.atomo),
               token_atual.texto);
        exit(1);
    }
}

static const char *tipo_atual_para_texto(TAtomo atomo)
{
    if (atomo == TK_INT)
        return "int";
    if (atomo == TK_BOOL)
        return "bool";
    if (atomo == TK_CHAR)
        return "char";

    return "desconhecido";
}

// Declaracoes de tipo: int, bool, char
static void parse_tipo(void)
{
    if (token_atual.atomo == TK_INT)
    {
        consumir(TK_INT);
    }
    else if (token_atual.atomo == TK_BOOL)
    {
        consumir(TK_BOOL);
    }
    else if (token_atual.atomo == TK_CHAR)
    {
        consumir(TK_CHAR);
    }
    else
    {
        printf("Erro de sintaxe na linha %d: tipo invalido (%s)\n",
               token_atual.linha,
               token_atual.texto);
        exit(1);
    }
}

static void parse_identificador_declaracao(char nome[], int *extra)
{
    strcpy(nome, token_atual.texto);
    consumir(IDENTIFICADOR);

    *extra = 0;

    if (token_atual.atomo == ABRE_COL)
    {
        consumir(ABRE_COL);
        *extra = atoi(token_atual.texto);
        consumir(CONST_INT);
        consumir(FECHA_COL);
    }
}

// Declaracao: x, y, z: int; ou v[10]: int;
static void parse_declaracao(void)
{
    char nomes[50][100];
    int extras[50];
    int quantidade = 0;
    const char *tipo_texto;
    int i;

    parse_identificador_declaracao(nomes[quantidade], &extras[quantidade]);
    quantidade++;

    while (token_atual.atomo == VIRGULA)
    {
        consumir(VIRGULA);
        parse_identificador_declaracao(nomes[quantidade], &extras[quantidade]);
        quantidade++;
    }

    consumir(DOIS_PONTOS);
    tipo_texto = tipo_atual_para_texto(token_atual.atomo);
    parse_tipo();
    consumir(PONTO_E_VIRGULA);

    for (i = 0; i < quantidade; i++)
    {
        if (!ts_insert(nomes[i], "variavel", tipo_texto, extras[i]))
        {
            printf("Erro semantico: identificador '%s' ja declarado no escopo %s\n",
                   nomes[i], ts_current_scope());
            exit(1);
        }
    }
}

static void parse_globals(void)
{
    consumir(TK_GLOBALS);

    while (token_atual.atomo == IDENTIFICADOR)
    {
        parse_declaracao();
    }
}

static void parse_locals(void)
{
    consumir(TK_LOCALS);

    while (token_atual.atomo == IDENTIFICADOR)
    {
        parse_declaracao();
    }
}

static void parse_parametro(void)
{
    char nome[100];
    const char *tipo_texto;

    strcpy(nome, token_atual.texto);
    consumir(IDENTIFICADOR);
    consumir(DOIS_PONTOS);

    tipo_texto = tipo_atual_para_texto(token_atual.atomo);
    parse_tipo();

    if (!ts_insert(nome, "parametro", tipo_texto, 0))
    {
        printf("Erro semantico: parametro '%s' ja declarado no escopo %s\n",
               nome, ts_current_scope());
        exit(1);
    }
}

static void parse_lista_parametros(int *quantidade_parametros)
{
    *quantidade_parametros = 0;

    if (token_atual.atomo == IDENTIFICADOR)
    {
        parse_parametro();
        (*quantidade_parametros)++;

        while (token_atual.atomo == VIRGULA)
        {
            consumir(VIRGULA);
            parse_parametro();
            (*quantidade_parametros)++;
        }
    }
}

static void parse_proc_decl_resto(void)
{
    char nome[100];
    int quantidade_parametros = 0;

    strcpy(nome, token_atual.texto);

    if (!ts_insert(nome, "procedimento", "-", 0))
    {
        printf("Erro semantico: procedimento '%s' ja declarado no escopo %s\n",
               nome, ts_current_scope());
        exit(1);
    }

    consumir(IDENTIFICADOR);
    consumir(ABRE_PAR);

    ts_enter_scope(nome);
    parse_lista_parametros(&quantidade_parametros);
    consumir(FECHA_PAR);

    if (token_atual.atomo == TK_LOCALS)
    {
        parse_locals();
    }

    parse_bloco();
    ts_exit_scope();

    // atualiza extra do procedimento com quantidade de parametros
    {
        Simbolo *s = ts_lookup(nome);
        if (s != NULL)
        {
            s->extra = quantidade_parametros;
        }
    }
}

static void parse_funcao_decl(void)
{
    char nome[100];
    const char *tipo_texto;
    int quantidade_parametros = 0;

    consumir(TK_FN);

    strcpy(nome, token_atual.texto);
    consumir(IDENTIFICADOR);

    consumir(ABRE_PAR);
    ts_enter_scope(nome);
    parse_lista_parametros(&quantidade_parametros);
    consumir(FECHA_PAR);
    consumir(DOIS_PONTOS);

    tipo_texto = tipo_atual_para_texto(token_atual.atomo);
    parse_tipo();
    ts_exit_scope();

    if (!ts_insert(nome, "funcao", tipo_texto, quantidade_parametros))
    {
        printf("Erro semantico: funcao '%s' ja declarada no escopo %s\n",
               nome, ts_current_scope());
        exit(1);
    }

    ts_enter_scope(nome);

    if (token_atual.atomo == TK_LOCALS)
    {
        parse_locals();
    }

    parse_bloco();
    ts_exit_scope();
}

static void parse_indice_vetor(void)
{
    consumir(ABRE_COL);

    if (token_atual.atomo == CONST_INT)
    {
        consumir(CONST_INT);
    }
    else if (token_atual.atomo == IDENTIFICADOR)
    {
        if (ts_lookup(token_atual.texto) == NULL)
        {
            printf("Erro semantico: identificador '%s' nao declarado\n", token_atual.texto);
            exit(1);
        }
        consumir(IDENTIFICADOR);
    }
    else
    {
        printf("Erro de sintaxe na linha %d: indice de vetor invalido (%s)\n",
               token_atual.linha,
               token_atual.texto);
        exit(1);
    }

    consumir(FECHA_COL);
}

static void parse_chamada_resto(void)
{
    consumir(ABRE_PAR);

    if (token_atual.atomo != FECHA_PAR)
    {
        parse_expr();

        while (token_atual.atomo == VIRGULA)
        {
            consumir(VIRGULA);
            parse_expr();
        }
    }

    consumir(FECHA_PAR);
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
        if (ts_lookup(token_atual.texto) == NULL)
        {
            printf("Erro semantico: identificador '%s' nao declarado\n", token_atual.texto);
            exit(1);
        }

        consumir(IDENTIFICADOR);

        if (token_atual.atomo == ABRE_PAR)
        {
            parse_chamada_resto();
        }
        else if (token_atual.atomo == ABRE_COL)
        {
            parse_indice_vetor();
        }
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

static void parse_if(void)
{
    consumir(TK_IF);
    consumir(ABRE_PAR);
    parse_expr();
    consumir(FECHA_PAR);

    parse_comando();

    if (token_atual.atomo == TK_ELSE)
    {
        consumir(TK_ELSE);
        parse_comando();
    }
}

static void parse_ret(void)
{
    consumir(TK_RET);
    parse_expr();
}

static void parse_loop(void)
{
    consumir(TK_LOOP);

    // Caso loop while
    if (token_atual.atomo == TK_WHILE)
    {
        consumir(TK_WHILE);
        consumir(ABRE_PAR);
        parse_expr();
        consumir(FECHA_PAR);

        parse_comando();
    }
    else
    {
        // Caso loop until
        while (token_atual.atomo != TK_UNTIL)
        {
            parse_comando();
            consumir(PONTO_E_VIRGULA);
        }

        consumir(TK_UNTIL);
        consumir(ABRE_PAR);
        parse_expr();
        consumir(FECHA_PAR);
    }
}

static void parse_for(void)
{
    consumir(TK_FOR);

    if (ts_lookup(token_atual.texto) == NULL)
    {
        printf("Erro semantico: identificador '%s' nao declarado\n", token_atual.texto);
        exit(1);
    }

    consumir(IDENTIFICADOR);

    if (token_atual.atomo == ABRE_COL)
    {
        parse_indice_vetor();
    }

    consumir(ATRIBUICAO);
    parse_expr();

    consumir(TK_TO);
    parse_expr();

    if (token_atual.atomo == TK_STEP)
    {
        consumir(TK_STEP);
        parse_expr();
    }

    consumir(TK_DO);
    parse_comando();
}

// Comandos do match
static void parse_item_when(void)
{
    if (token_atual.atomo == SUBTRACAO)
    {
        consumir(SUBTRACAO);
    }

    consumir(CONST_INT);

    if (token_atual.atomo == INTERVALO)
    {
        consumir(INTERVALO);

        if (token_atual.atomo == SUBTRACAO)
        {
            consumir(SUBTRACAO);
        }

        consumir(CONST_INT);
    }
}

static void parse_condicao_when(void)
{
    parse_item_when();

    while (token_atual.atomo == VIRGULA)
    {
        consumir(VIRGULA);
        parse_item_when();
    }
}

static void parse_when(void)
{
    consumir(TK_WHEN);
    parse_condicao_when();
    consumir(IMPLICA);
    parse_comando();
    consumir(PONTO_E_VIRGULA);
}

static void parse_match(void)
{
    consumir(TK_MATCH);
    consumir(ABRE_PAR);
    parse_expr();
    consumir(FECHA_PAR);

    while (token_atual.atomo == TK_WHEN)
    {
        parse_when();
    }

    if (token_atual.atomo == TK_OTHERWISE)
    {
        consumir(TK_OTHERWISE);
        consumir(IMPLICA);
        parse_comando();
        consumir(PONTO_E_VIRGULA);
    }

    consumir(TK_END);
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

    while (eh_ou_logico())
    {
        consumir_ou_logico();
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

    if (ts_lookup(token_atual.texto) == NULL)
    {
        printf("Erro semantico: identificador '%s' nao declarado\n", token_atual.texto);
        exit(1);
    }

    consumir(IDENTIFICADOR);

    if (token_atual.atomo == ABRE_COL)
    {
        parse_indice_vetor();
    }

    consumir(FECHA_PAR);
}

static void parse_comando_identificador(void)
{
    if (ts_lookup(token_atual.texto) == NULL)
    {
        printf("Erro semantico: identificador '%s' nao declarado\n", token_atual.texto);
        exit(1);
    }

    consumir(IDENTIFICADOR);

    if (token_atual.atomo == ABRE_PAR)
    {
        parse_chamada_resto();
    }
    else
    {
        if (token_atual.atomo == ABRE_COL)
        {
            parse_indice_vetor();
        }

        consumir(ATRIBUICAO);
        parse_expr();
    }
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
    else if (token_atual.atomo == TK_IF)
    {
        parse_if();
    }
    else if (token_atual.atomo == TK_START)
    {
        parse_bloco();
    }
    else if (token_atual.atomo == TK_RET)
    {
        parse_ret();
    }
    else if (token_atual.atomo == TK_LOOP)
    {
        parse_loop();
    }
    else if (token_atual.atomo == TK_FOR)
    {
        parse_for();
    }
    else if (token_atual.atomo == TK_MATCH)
    {
        parse_match();
    }
    else if (token_atual.atomo == IDENTIFICADOR)
    {
        parse_comando_identificador();
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

    if (token_atual.atomo == TK_GLOBALS)
    {
        parse_globals();
    }

    while (token_atual.atomo == TK_FN || token_atual.atomo == TK_PROC)
    {
        if (token_atual.atomo == TK_FN)
        {
            parse_funcao_decl();
        }
        else
        {
            consumir(TK_PROC);

            if (token_atual.atomo == TK_MAIN)
            {
                consumir(TK_MAIN);
                consumir(ABRE_PAR);
                consumir(FECHA_PAR);

                ts_enter_scope("main");

                if (token_atual.atomo == TK_LOCALS)
                {
                    parse_locals();
                }

                parse_bloco();
                ts_exit_scope();

                consumir(FIM_ARQUIVO);
                return;
            }
            else
            {
                parse_proc_decl_resto();
            }
        }
    }

    printf("Erro de sintaxe na linha %d: esperado proc main, encontrado %s (%s)\n",
           token_atual.linha,
           nome_atomo(token_atual.atomo),
           token_atual.texto);
    exit(1);
}
