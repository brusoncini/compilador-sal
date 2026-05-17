#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diag.h"
#include "lex.h"
#include "parser.h"
#include "symtab.h"

static TInfoAtomo token_atual;

static void erro_sintatico(const char *esperado);
static void avancar(void);
static void consumir(TAtomo esperado);

static void parse_program_interno(void);
static void parse_bloco(void);
static void parse_comando(void);
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
static void parse_comando_identificador(void);
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
static void parse_expr(void);
static void parse_and(void);
static void parse_relacional(void);
static void parse_soma_sub(void);
static void parse_mult_div(void);
static void parse_fator(void);
static int eh_relacional(TAtomo atomo);

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
        erro_sintatico("sOR");
    }
}

static const char *nome_atomo(TAtomo atomo)
{
    return lex_token_name(atomo);
}

static void erro_sintatico(const char *esperado)
{
    diag_error_sintatico(token_atual.linha, esperado, nome_atomo(token_atual.atomo), token_atual.texto);
}

static void erro_semantico(const char *mensagem)
{
    diag_error_semantico(token_atual.linha, mensagem, token_atual.texto);
}

static void avancar(void)
{
    token_atual = lex_next();
}

static void consumir(TAtomo esperado)
{
    if (token_atual.atomo != esperado)
    {
        erro_sintatico(nome_atomo(esperado));
    }
    avancar();
}

static int eh_relacional(TAtomo atomo)
{
    return atomo == IGUAL || atomo == DIFERENTE || atomo == MAIOR ||
           atomo == MENOR || atomo == MAIOR_IGUAL || atomo == MENOR_IGUAL;
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

static void parse_tipo(void)
{
    if (token_atual.atomo == TK_INT)
        consumir(TK_INT);
    else if (token_atual.atomo == TK_BOOL)
        consumir(TK_BOOL);
    else if (token_atual.atomo == TK_CHAR)
        consumir(TK_CHAR);
    else
        erro_sintatico("tipo");
}

static void parse_identificador_declaracao(char nome[], int *extra)
{
    strcpy(nome, token_atual.texto);
    consumir(IDENTIFICADOR);
    *extra = 0;
    if (token_atual.atomo == ABRE_COL)
    {
        consumir(ABRE_COL);
        if (token_atual.atomo != CONST_INT)
        {
            erro_sintatico("sCTEINT");
        }
        *extra = token_atual.atributo_numero;
        consumir(CONST_INT);
        consumir(FECHA_COL);
    }
}

static void parse_declaracao(void)
{
    char nomes[64][100];
    int extras[64];
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
            token_atual.linha = token_atual.linha;
            diag_error_semantico(token_atual.linha, "identificador ja declarado no escopo", nomes[i]);
        }
    }
}

static void parse_globals(void)
{
    diag_info("ENTRA <glob>");
    consumir(TK_GLOBALS);
    while (token_atual.atomo == IDENTIFICADOR)
    {
        parse_declaracao();
    }
    diag_info("SAI <glob>");
}

static void parse_locals(void)
{
    diag_info("ENTRA <locals>");
    consumir(TK_LOCALS);
    while (token_atual.atomo == IDENTIFICADOR)
    {
        parse_declaracao();
    }
    diag_info("SAI <locals>");
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
        diag_error_semantico(token_atual.linha, "parametro ja declarado no escopo", nome);
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
    Simbolo *s;
    strcpy(nome, token_atual.texto);

    if (!ts_insert(nome, "procedimento", "-", 0))
    {
        diag_error_semantico(token_atual.linha, "procedimento ja declarado", nome);
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

    s = ts_lookup_in_scope(nome, "global");
    if (s != NULL)
    {
        s->extra = quantidade_parametros;
    }
}

static void parse_funcao_decl(void)
{
    char nome[100];
    const char *tipo_texto;
    int quantidade_parametros = 0;
    Simbolo *s;

    consumir(TK_FN);
    strcpy(nome, token_atual.texto);

    if (!ts_insert(nome, "funcao", "pendente", 0))
    {
        diag_error_semantico(token_atual.linha, "funcao ja declarada", nome);
    }

    consumir(IDENTIFICADOR);
    consumir(ABRE_PAR);
    ts_enter_scope(nome);
    parse_lista_parametros(&quantidade_parametros);
    consumir(FECHA_PAR);
    consumir(DOIS_PONTOS);
    tipo_texto = tipo_atual_para_texto(token_atual.atomo);
    parse_tipo();

    s = ts_lookup_in_scope(nome, "global");
    if (s != NULL)
    {
        strncpy(s->tipo, tipo_texto, TAM_TIPO - 1);
        s->tipo[TAM_TIPO - 1] = '\0';
        s->extra = quantidade_parametros;
    }

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
            erro_semantico("identificador nao declarado");
        }
        consumir(IDENTIFICADOR);
    }
    else
    {
        erro_sintatico("indice de vetor");
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
            erro_semantico("identificador nao declarado");
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
        consumir(CONST_INT);
    else if (token_atual.atomo == CONST_CHAR)
        consumir(CONST_CHAR);
    else if (token_atual.atomo == STRING)
        consumir(STRING);
    else if (token_atual.atomo == TK_TRUE)
        consumir(TK_TRUE);
    else if (token_atual.atomo == TK_FALSE)
        consumir(TK_FALSE);
    else if (token_atual.atomo == ABRE_PAR)
    {
        consumir(ABRE_PAR);
        parse_expr();
        consumir(FECHA_PAR);
    }
    else
        erro_sintatico("fator");
}

static void parse_mult_div(void)
{
    parse_fator();
    while (token_atual.atomo == MULTIPLICACAO || token_atual.atomo == DIVISAO)
    {
        if (token_atual.atomo == MULTIPLICACAO)
            consumir(MULTIPLICACAO);
        else
            consumir(DIVISAO);
        parse_fator();
    }
}

static void parse_soma_sub(void)
{
    parse_mult_div();
    while (token_atual.atomo == SOMA || token_atual.atomo == SUBTRACAO)
    {
        if (token_atual.atomo == SOMA)
            consumir(SOMA);
        else
            consumir(SUBTRACAO);
        parse_mult_div();
    }
}

static void parse_relacional(void)
{
    parse_soma_sub();
    while (eh_relacional(token_atual.atomo))
    {
        TAtomo op = token_atual.atomo;
        consumir(op);
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
    if (token_atual.atomo != IDENTIFICADOR || ts_lookup(token_atual.texto) == NULL)
    {
        erro_semantico("identificador nao declarado");
    }
    consumir(IDENTIFICADOR);
    if (token_atual.atomo == ABRE_COL)
    {
        parse_indice_vetor();
    }
    consumir(FECHA_PAR);
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
    if (token_atual.atomo != IDENTIFICADOR || ts_lookup(token_atual.texto) == NULL)
    {
        erro_semantico("identificador nao declarado");
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

static void parse_comando_identificador(void)
{
    if (ts_lookup(token_atual.texto) == NULL)
    {
        erro_semantico("identificador nao declarado");
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
        parse_print();
    else if (token_atual.atomo == TK_SCAN)
        parse_scan();
    else if (token_atual.atomo == TK_IF)
        parse_if();
    else if (token_atual.atomo == TK_START)
        parse_bloco();
    else if (token_atual.atomo == TK_RET)
        parse_ret();
    else if (token_atual.atomo == TK_LOOP)
        parse_loop();
    else if (token_atual.atomo == TK_FOR)
        parse_for();
    else if (token_atual.atomo == TK_MATCH)
        parse_match();
    else if (token_atual.atomo == IDENTIFICADOR)
        parse_comando_identificador();
    else
        erro_sintatico("comando");
}

static void parse_bloco(void)
{
    diag_info("ENTRA <bco>");
    consumir(TK_START);
    while (token_atual.atomo != TK_END)
    {
        parse_comando();
        consumir(PONTO_E_VIRGULA);
    }
    consumir(TK_END);
    diag_info("SAI <bco>");
}

void parser_init(void)
{
    avancar();
}

void parse_program(void)
{
    diag_info("ENTRA <ini>");
    parse_program_interno();
    diag_info("SAI <ini>");
}

static void parse_program_interno(void)
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

    erro_sintatico("proc main");
}
