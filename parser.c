#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diag.h"
#include "gerador.h"
#include "lex.h"
#include "parser.h"
#include "symtab.h"

#define TIPO_INT "int"
#define TIPO_BOOL "bool"
#define TIPO_CHAR "char"
#define TIPO_STRING "string"
#define TIPO_VOID "void"
#define TIPO_ERRO "erro"

static TInfoAtomo token_atual;

static int geracao_ativa = 1;
static int proximo_endereco_mepa = 0;
static int total_variaveis_mepa = 0;

static void erro_sintatico(const char *esperado);
static void erro_semantico(const char *mensagem);
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
static const char *parse_parametro(void);
static void parse_lista_parametros(int *quantidade_parametros, char parametros[]);
static void parse_proc_decl_resto(void);
static void parse_funcao_decl(void);

static int parse_indice_vetor(void);
static void parse_chamada_resto(Simbolo *subrotina);
static const char *parse_expr(void);
static const char *parse_and(void);
static const char *parse_relacional(void);
static const char *parse_soma_sub(void);
static const char *parse_mult_div(void);
static const char *parse_fator(void);

static int eh_relacional(TAtomo atomo);

static int eh_ou_logico(void)
{
    return token_atual.atomo == OU_LOGICO ||
           (token_atual.atomo == IDENTIFICADOR && strcmp(token_atual.texto, "v") == 0);
}

static void consumir_ou_logico(void)
{
    if (token_atual.atomo == OU_LOGICO) {
        consumir(OU_LOGICO);
    } else if (token_atual.atomo == IDENTIFICADOR && strcmp(token_atual.texto, "v") == 0) {
        avancar();
    } else {
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

static void erro_semantico_nome(const char *mensagem, const char *nome)
{
    diag_error_semantico(token_atual.linha, mensagem, nome);
}

static void avancar(void)
{
    token_atual = lex_next();
}

static void consumir(TAtomo esperado)
{
    if (token_atual.atomo != esperado) {
        erro_sintatico(nome_atomo(esperado));
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

static char *int_para_str(int valor)
{
    static char buffers[8][32];
    static int indice = 0;

    indice = (indice + 1) % 8;
    snprintf(buffers[indice], sizeof(buffers[indice]), "%d", valor);

    return buffers[indice];
}

static void emitir0(const char *mnemonico)
{
    if (geracao_ativa) {
        gera_instr_mepa(NULL, mnemonico, NULL, NULL);
    }
}

static void emitir1(const char *mnemonico, const char *parametro1)
{
    if (geracao_ativa) {
        gera_instr_mepa(NULL, mnemonico, parametro1, NULL);
    }
}

static void emitir2(const char *mnemonico, int parametro1, int parametro2)
{
    if (geracao_ativa) {
        gera_instr_mepa(NULL, mnemonico, int_para_str(parametro1), int_para_str(parametro2));
    }
}

static void emitir_rotulo(const char *rotulo)
{
    if (geracao_ativa) {
        gera_instr_mepa(rotulo, "NADA", NULL, NULL);
    }
}

static int tipos_iguais(const char *a, const char *b)
{
    return a != NULL && b != NULL && strcmp(a, b) == 0;
}

static void exigir_tipo(const char *tipo_obtido, const char *tipo_esperado, const char *contexto)
{
    char mensagem[180];

    if (!tipos_iguais(tipo_obtido, tipo_esperado)) {
        snprintf(mensagem, sizeof(mensagem), "tipo incompativel em %s: esperado %s", contexto, tipo_esperado);
        erro_semantico(mensagem);
    }
}

static int simbolo_e_variavel(Simbolo *simbolo)
{
    return simbolo != NULL &&
           (strcmp(simbolo->categoria, "variavel") == 0 || strcmp(simbolo->categoria, "parametro") == 0);
}

static int deve_alocar_mepa_no_escopo_atual(void)
{
    const char *escopo = ts_current_scope();

    if (!geracao_ativa) {
        return 0;
    }

    return strcmp(escopo, "global") == 0 || strcmp(escopo, "main") == 0;
}

static void adicionar_tipo_lista(char lista[], const char *tipo)
{
    if (lista[0] != '\0') {
        strncat(lista, ",", TAM_PARAMETROS - strlen(lista) - 1);
    }

    strncat(lista, tipo, TAM_PARAMETROS - strlen(lista) - 1);
}

static const char *tipo_atual_para_texto(TAtomo atomo)
{
    if (atomo == TK_INT) return TIPO_INT;
    if (atomo == TK_BOOL) return TIPO_BOOL;
    if (atomo == TK_CHAR) return TIPO_CHAR;
    return "desconhecido";
}

static void parse_tipo(void)
{
    if (token_atual.atomo == TK_INT) {
        consumir(TK_INT);
    } else if (token_atual.atomo == TK_BOOL) {
        consumir(TK_BOOL);
    } else if (token_atual.atomo == TK_CHAR) {
        consumir(TK_CHAR);
    } else {
        erro_sintatico("tipo");
    }
}

static void parse_identificador_declaracao(char nome[], int *extra)
{
    strcpy(nome, token_atual.texto);
    consumir(IDENTIFICADOR);

    *extra = 0;

    if (token_atual.atomo == ABRE_COL) {
        consumir(ABRE_COL);

        if (token_atual.atomo != CONST_INT) {
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

    while (token_atual.atomo == VIRGULA) {
        consumir(VIRGULA);
        parse_identificador_declaracao(nomes[quantidade], &extras[quantidade]);
        quantidade++;
    }

    consumir(DOIS_PONTOS);
    tipo_texto = tipo_atual_para_texto(token_atual.atomo);
    parse_tipo();
    consumir(PONTO_E_VIRGULA);

    for (i = 0; i < quantidade; i++) {
        int endereco_ou_extra = extras[i];

        if (deve_alocar_mepa_no_escopo_atual()) {
            int tamanho = extras[i] > 0 ? extras[i] : 1;
            endereco_ou_extra = proximo_endereco_mepa;
            proximo_endereco_mepa += tamanho;
            total_variaveis_mepa += tamanho;
        }

        if (!ts_insert(nomes[i], "variavel", tipo_texto, endereco_ou_extra)) {
            diag_error_semantico(token_atual.linha, "identificador ja declarado no escopo", nomes[i]);
        }
    }
}

static void parse_globals(void)
{
    diag_info("ENTRA ");
    consumir(TK_GLOBALS);

    while (token_atual.atomo == IDENTIFICADOR) {
        parse_declaracao();
    }

    diag_info("SAI ");
}

static void parse_locals(void)
{
    diag_info("ENTRA ");
    consumir(TK_LOCALS);

    while (token_atual.atomo == IDENTIFICADOR) {
        parse_declaracao();
    }

    diag_info("SAI ");
}

static const char *parse_parametro(void)
{
    char nome[100];
    const char *tipo_texto;

    strcpy(nome, token_atual.texto);
    consumir(IDENTIFICADOR);
    consumir(DOIS_PONTOS);

    tipo_texto = tipo_atual_para_texto(token_atual.atomo);
    parse_tipo();

    if (!ts_insert(nome, "parametro", tipo_texto, 0)) {
        diag_error_semantico(token_atual.linha, "parametro ja declarado no escopo", nome);
    }

    return tipo_texto;
}

static void parse_lista_parametros(int *quantidade_parametros, char parametros[])
{
    *quantidade_parametros = 0;
    parametros[0] = '\0';

    if (token_atual.atomo == IDENTIFICADOR) {
        const char *tipo = parse_parametro();
        adicionar_tipo_lista(parametros, tipo);
        (*quantidade_parametros)++;

        while (token_atual.atomo == VIRGULA) {
            consumir(VIRGULA);
            tipo = parse_parametro();
            adicionar_tipo_lista(parametros, tipo);
            (*quantidade_parametros)++;
        }
    }
}

static void parse_proc_decl_resto(void)
{
    char nome[100];
    char parametros[TAM_PARAMETROS];
    int quantidade_parametros = 0;
    Simbolo *s;

    strcpy(nome, token_atual.texto);

    if (!ts_insert(nome, "procedimento", TIPO_VOID, 0)) {
        diag_error_semantico(token_atual.linha, "procedimento ja declarado", nome);
    }

    consumir(IDENTIFICADOR);
    consumir(ABRE_PAR);

    ts_enter_scope(nome);
    parse_lista_parametros(&quantidade_parametros, parametros);
    consumir(FECHA_PAR);

    s = ts_lookup_in_scope(nome, "global");
    if (s != NULL) {
        s->extra = quantidade_parametros;
        strncpy(s->parametros, parametros, TAM_PARAMETROS - 1);
        s->parametros[TAM_PARAMETROS - 1] = '\0';
    }

    if (token_atual.atomo == TK_LOCALS) {
        parse_locals();
    }

    parse_bloco();
    ts_exit_scope();
}

static void parse_funcao_decl(void)
{
    char nome[100];
    char parametros[TAM_PARAMETROS];
    const char *tipo_texto;
    int quantidade_parametros = 0;
    Simbolo *s;

    consumir(TK_FN);

    strcpy(nome, token_atual.texto);
    if (!ts_insert(nome, "funcao", "pendente", 0)) {
        diag_error_semantico(token_atual.linha, "funcao ja declarada", nome);
    }

    consumir(IDENTIFICADOR);
    consumir(ABRE_PAR);

    ts_enter_scope(nome);
    parse_lista_parametros(&quantidade_parametros, parametros);
    consumir(FECHA_PAR);
    consumir(DOIS_PONTOS);

    tipo_texto = tipo_atual_para_texto(token_atual.atomo);
    parse_tipo();

    s = ts_lookup_in_scope(nome, "global");
    if (s != NULL) {
        strncpy(s->tipo, tipo_texto, TAM_TIPO - 1);
        s->tipo[TAM_TIPO - 1] = '\0';
        s->extra = quantidade_parametros;
        strncpy(s->parametros, parametros, TAM_PARAMETROS - 1);
        s->parametros[TAM_PARAMETROS - 1] = '\0';
    }

    if (token_atual.atomo == TK_LOCALS) {
        parse_locals();
    }

    parse_bloco();
    ts_exit_scope();
}

static int parse_indice_vetor(void)
{
    int indice = 0;

    consumir(ABRE_COL);

    if (token_atual.atomo == CONST_INT) {
        indice = token_atual.atributo_numero;
        consumir(CONST_INT);
    } else if (token_atual.atomo == IDENTIFICADOR) {
        if (ts_lookup(token_atual.texto) == NULL) {
            erro_semantico("identificador nao declarado");
        }

        if (geracao_ativa) {
            erro_semantico("indice de vetor com variavel nao suportado na geracao MEPA");
        }

        consumir(IDENTIFICADOR);
    } else {
        erro_sintatico("indice de vetor");
    }

    consumir(FECHA_COL);
    return indice;
}

static void parse_chamada_resto(Simbolo *subrotina)
{
    char tipos_argumentos[TAM_PARAMETROS];
    int quantidade_argumentos = 0;

    tipos_argumentos[0] = '\0';

    consumir(ABRE_PAR);

    if (token_atual.atomo != FECHA_PAR) {
        const char *tipo = parse_expr();
        adicionar_tipo_lista(tipos_argumentos, tipo);
        quantidade_argumentos++;

        while (token_atual.atomo == VIRGULA) {
            consumir(VIRGULA);
            tipo = parse_expr();
            adicionar_tipo_lista(tipos_argumentos, tipo);
            quantidade_argumentos++;
        }
    }

    consumir(FECHA_PAR);

    if (subrotina == NULL ||
        (strcmp(subrotina->categoria, "procedimento") != 0 && strcmp(subrotina->categoria, "funcao") != 0)) {
        erro_semantico_nome("identificador nao e sub-rotina", subrotina != NULL ? subrotina->lexema : "");
    }

    if (subrotina->extra != quantidade_argumentos) {
        erro_semantico_nome("quantidade de argumentos incompativel", subrotina->lexema);
    }

    if (subrotina->parametros[0] != '\0' && strcmp(subrotina->parametros, tipos_argumentos) != 0) {
        erro_semantico_nome("tipos dos argumentos incompativeis", subrotina->lexema);
    }

    if (geracao_ativa) {
        erro_semantico_nome("chamadas de sub-rotina nao sao suportadas nesta etapa de geracao MEPA", subrotina->lexema);
    }
}

static const char *parse_fator(void)
{
    if (token_atual.atomo == NEGACAO) {
        const char *tipo;
        consumir(NEGACAO);
        tipo = parse_fator();
        exigir_tipo(tipo, TIPO_BOOL, "negacao logica");
        emitir0("NEGA");
        return TIPO_BOOL;
    }

    if (token_atual.atomo == SUBTRACAO) {
        const char *tipo;
        consumir(SUBTRACAO);
        tipo = parse_fator();
        exigir_tipo(tipo, TIPO_INT, "menos unario");
        emitir0("INVR");
        return TIPO_INT;
    }

    if (token_atual.atomo == IDENTIFICADOR) {
        char nome[100];
        int deslocamento = 0;
        Simbolo *simbolo;

        strcpy(nome, token_atual.texto);
        simbolo = ts_lookup(nome);

        if (simbolo == NULL) {
            erro_semantico("identificador nao declarado");
        }

        consumir(IDENTIFICADOR);

        if (token_atual.atomo == ABRE_PAR) {
            parse_chamada_resto(simbolo);

            if (strcmp(simbolo->categoria, "funcao") != 0) {
                erro_semantico_nome("procedimento nao pode ser usado como expressao", nome);
            }

            return simbolo->tipo;
        }

        if (token_atual.atomo == ABRE_COL) {
            deslocamento = parse_indice_vetor();
        }

        if (!simbolo_e_variavel(simbolo)) {
            erro_semantico_nome("identificador nao e variavel", nome);
        }

        emitir2("CRVL", 0, simbolo->extra + deslocamento);
        return simbolo->tipo;
    }

    if (token_atual.atomo == CONST_INT) {
        int valor = token_atual.atributo_numero;
        consumir(CONST_INT);
        emitir1("CRCT", int_para_str(valor));
        return TIPO_INT;
    }

    if (token_atual.atomo == CONST_CHAR) {
        int valor = token_atual.atributo_numero;
        consumir(CONST_CHAR);
        emitir1("CRCT", int_para_str(valor));
        return TIPO_CHAR;
    }

    if (token_atual.atomo == STRING) {
        consumir(STRING);
        return TIPO_STRING;
    }

    if (token_atual.atomo == TK_TRUE) {
        consumir(TK_TRUE);
        emitir1("CRCT", "1");
        return TIPO_BOOL;
    }

    if (token_atual.atomo == TK_FALSE) {
        consumir(TK_FALSE);
        emitir1("CRCT", "0");
        return TIPO_BOOL;
    }

    if (token_atual.atomo == ABRE_PAR) {
        const char *tipo;
        consumir(ABRE_PAR);
        tipo = parse_expr();
        consumir(FECHA_PAR);
        return tipo;
    }

    erro_sintatico("fator");
    return TIPO_ERRO;
}

static const char *parse_mult_div(void)
{
    const char *tipo_esq = parse_fator();

    while (token_atual.atomo == MULTIPLICACAO || token_atual.atomo == DIVISAO) {
        TAtomo op = token_atual.atomo;
        const char *tipo_dir;

        if (op == MULTIPLICACAO) {
            consumir(MULTIPLICACAO);
        } else {
            consumir(DIVISAO);
        }

        tipo_dir = parse_fator();

        exigir_tipo(tipo_esq, TIPO_INT, "operacao aritmetica");
        exigir_tipo(tipo_dir, TIPO_INT, "operacao aritmetica");

        if (op == MULTIPLICACAO) {
            emitir0("MULT");
        } else {
            emitir0("DIVI");
        }

        tipo_esq = TIPO_INT;
    }

    return tipo_esq;
}

static const char *parse_soma_sub(void)
{
    const char *tipo_esq = parse_mult_div();

    while (token_atual.atomo == SOMA || token_atual.atomo == SUBTRACAO) {
        TAtomo op = token_atual.atomo;
        const char *tipo_dir;

        if (op == SOMA) {
            consumir(SOMA);
        } else {
            consumir(SUBTRACAO);
        }

        tipo_dir = parse_mult_div();

        exigir_tipo(tipo_esq, TIPO_INT, "operacao aritmetica");
        exigir_tipo(tipo_dir, TIPO_INT, "operacao aritmetica");

        if (op == SOMA) {
            emitir0("SOMA");
        } else {
            emitir0("SUBT");
        }

        tipo_esq = TIPO_INT;
    }

    return tipo_esq;
}

static const char *mepa_relacional(TAtomo op)
{
    if (op == MENOR) return "CMME";
    if (op == MENOR_IGUAL) return "CMEG";
    if (op == MAIOR) return "CMMA";
    if (op == MAIOR_IGUAL) return "CMAG";
    if (op == IGUAL) return "CMIG";
    if (op == DIFERENTE) return "CMDG";
    return "NADA";
}

static const char *parse_relacional(void)
{
    const char *tipo_esq = parse_soma_sub();

    while (eh_relacional(token_atual.atomo)) {
        TAtomo op = token_atual.atomo;
        const char *tipo_dir;

        consumir(op);
        tipo_dir = parse_soma_sub();

        if (op == MENOR || op == MENOR_IGUAL || op == MAIOR || op == MAIOR_IGUAL) {
            exigir_tipo(tipo_esq, TIPO_INT, "comparacao relacional");
            exigir_tipo(tipo_dir, TIPO_INT, "comparacao relacional");
        } else if (!tipos_iguais(tipo_esq, tipo_dir)) {
            erro_semantico("comparacao entre tipos incompativeis");
        }

        emitir0(mepa_relacional(op));
        tipo_esq = TIPO_BOOL;
    }

    return tipo_esq;
}

static const char *parse_and(void)
{
    const char *tipo_esq = parse_relacional();

    while (token_atual.atomo == E_LOGICO) {
        const char *tipo_dir;

        consumir(E_LOGICO);
        tipo_dir = parse_relacional();

        exigir_tipo(tipo_esq, TIPO_BOOL, "conjuncao logica");
        exigir_tipo(tipo_dir, TIPO_BOOL, "conjuncao logica");

        emitir0("CONJ");
        tipo_esq = TIPO_BOOL;
    }

    return tipo_esq;
}

static const char *parse_expr(void)
{
    const char *tipo_esq = parse_and();

    while (eh_ou_logico()) {
        const char *tipo_dir;

        consumir_ou_logico();
        tipo_dir = parse_and();

        exigir_tipo(tipo_esq, TIPO_BOOL, "disjuncao logica");
        exigir_tipo(tipo_dir, TIPO_BOOL, "disjuncao logica");

        emitir0("DISJ");
        tipo_esq = TIPO_BOOL;
    }

    return tipo_esq;
}

static void parse_print(void)
{
    const char *tipo;

    consumir(TK_PRINT);
    consumir(ABRE_PAR);

    tipo = parse_expr();
    if (tipos_iguais(tipo, TIPO_STRING)) {
        erro_semantico("print de string nao suportado na geracao MEPA desta etapa");
    }
    emitir0("IMPR");

    while (token_atual.atomo == VIRGULA) {
        consumir(VIRGULA);
        tipo = parse_expr();
        if (tipos_iguais(tipo, TIPO_STRING)) {
            erro_semantico("print de string nao suportado na geracao MEPA desta etapa");
        }
        emitir0("IMPR");
    }

    consumir(FECHA_PAR);
}

static void parse_scan(void)
{
    Simbolo *simbolo;
    char nome[100];
    int deslocamento = 0;

    consumir(TK_SCAN);
    consumir(ABRE_PAR);

    if (token_atual.atomo != IDENTIFICADOR) {
        erro_semantico("identificador nao declarado");
    }

    strcpy(nome, token_atual.texto);
    simbolo = ts_lookup(nome);

    if (simbolo == NULL) {
        erro_semantico("identificador nao declarado");
    }

    if (!simbolo_e_variavel(simbolo)) {
        erro_semantico_nome("scan exige variavel", nome);
    }

    consumir(IDENTIFICADOR);

    if (token_atual.atomo == ABRE_COL) {
        deslocamento = parse_indice_vetor();
    }

    emitir0("LEIT");
    emitir2("ARMZ", 0, simbolo->extra + deslocamento);

    consumir(FECHA_PAR);
}

static void parse_if(void)
{
    const char *tipo;
    char *rotulo_falso = novo_rotulo();
    char *rotulo_fim = novo_rotulo();

    consumir(TK_IF);
    consumir(ABRE_PAR);

    tipo = parse_expr();
    exigir_tipo(tipo, TIPO_BOOL, "condicao do if");

    consumir(FECHA_PAR);

    emitir1("DSVF", rotulo_falso);
    parse_comando();

    if (token_atual.atomo == TK_ELSE) {
        emitir1("DSVS", rotulo_fim);
        emitir_rotulo(rotulo_falso);

        consumir(TK_ELSE);
        parse_comando();

        emitir_rotulo(rotulo_fim);
    } else {
        emitir_rotulo(rotulo_falso);
    }
}

static void parse_ret(void)
{
    if (geracao_ativa) {
        erro_semantico("ret nao suportado dentro do programa principal MEPA");
    }

    consumir(TK_RET);
    parse_expr();
}

static void parse_loop(void)
{
    consumir(TK_LOOP);

    if (token_atual.atomo == TK_WHILE) {
        const char *tipo;
        char *rotulo_inicio = novo_rotulo();
        char *rotulo_fim = novo_rotulo();

        consumir(TK_WHILE);
        emitir_rotulo(rotulo_inicio);

        consumir(ABRE_PAR);
        tipo = parse_expr();
        exigir_tipo(tipo, TIPO_BOOL, "condicao do while");
        consumir(FECHA_PAR);

        emitir1("DSVF", rotulo_fim);
        parse_comando();
        emitir1("DSVS", rotulo_inicio);
        emitir_rotulo(rotulo_fim);
    } else {
        const char *tipo;
        char *rotulo_inicio = novo_rotulo();

        emitir_rotulo(rotulo_inicio);

        while (token_atual.atomo != TK_UNTIL) {
            parse_comando();
            consumir(PONTO_E_VIRGULA);
        }

        consumir(TK_UNTIL);
        consumir(ABRE_PAR);
        tipo = parse_expr();
        exigir_tipo(tipo, TIPO_BOOL, "condicao do until");
        consumir(FECHA_PAR);

        emitir1("DSVF", rotulo_inicio);
    }
}

static void parse_for(void)
{
    if (geracao_ativa) {
        erro_semantico("for nao suportado nesta etapa de geracao MEPA; use loop while");
    }

    consumir(TK_FOR);

    if (token_atual.atomo != IDENTIFICADOR || ts_lookup(token_atual.texto) == NULL) {
        erro_semantico("identificador nao declarado");
    }

    consumir(IDENTIFICADOR);

    if (token_atual.atomo == ABRE_COL) {
        parse_indice_vetor();
    }

    consumir(ATRIBUICAO);
    parse_expr();
    consumir(TK_TO);
    parse_expr();

    if (token_atual.atomo == TK_STEP) {
        consumir(TK_STEP);
        parse_expr();
    }

    consumir(TK_DO);
    parse_comando();
}

static void parse_item_when(void)
{
    if (token_atual.atomo == SUBTRACAO) {
        consumir(SUBTRACAO);
    }

    consumir(CONST_INT);

    if (token_atual.atomo == INTERVALO) {
        consumir(INTERVALO);

        if (token_atual.atomo == SUBTRACAO) {
            consumir(SUBTRACAO);
        }

        consumir(CONST_INT);
    }
}

static void parse_condicao_when(void)
{
    parse_item_when();

    while (token_atual.atomo == VIRGULA) {
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
    if (geracao_ativa) {
        erro_semantico("match nao suportado nesta etapa de geracao MEPA; use if/else");
    }

    consumir(TK_MATCH);
    consumir(ABRE_PAR);
    parse_expr();
    consumir(FECHA_PAR);

    while (token_atual.atomo == TK_WHEN) {
        parse_when();
    }

    if (token_atual.atomo == TK_OTHERWISE) {
        consumir(TK_OTHERWISE);
        consumir(IMPLICA);
        parse_comando();
        consumir(PONTO_E_VIRGULA);
    }

    consumir(TK_END);
}

static void parse_comando_identificador(void)
{
    char nome[100];
    int deslocamento = 0;
    Simbolo *simbolo;

    strcpy(nome, token_atual.texto);
    simbolo = ts_lookup(nome);

    if (simbolo == NULL) {
        erro_semantico("identificador nao declarado");
    }

    consumir(IDENTIFICADOR);

    if (token_atual.atomo == ABRE_PAR) {
        parse_chamada_resto(simbolo);
        return;
    }

    if (!simbolo_e_variavel(simbolo)) {
        erro_semantico_nome("lado esquerdo da atribuicao precisa ser variavel", nome);
    }

    if (token_atual.atomo == ABRE_COL) {
        deslocamento = parse_indice_vetor();
    }

    consumir(ATRIBUICAO);

    {
        const char *tipo_expr = parse_expr();
        if (!tipos_iguais(simbolo->tipo, tipo_expr)) {
            erro_semantico_nome("tipo incompativel na atribuicao", nome);
        }
    }

    emitir2("ARMZ", 0, simbolo->extra + deslocamento);
}

static void parse_comando(void)
{
    if (token_atual.atomo == TK_PRINT) {
        parse_print();
    } else if (token_atual.atomo == TK_SCAN) {
        parse_scan();
    } else if (token_atual.atomo == TK_IF) {
        parse_if();
    } else if (token_atual.atomo == TK_START) {
        parse_bloco();
    } else if (token_atual.atomo == TK_RET) {
        parse_ret();
    } else if (token_atual.atomo == TK_LOOP) {
        parse_loop();
    } else if (token_atual.atomo == TK_FOR) {
        parse_for();
    } else if (token_atual.atomo == TK_MATCH) {
        parse_match();
    } else if (token_atual.atomo == IDENTIFICADOR) {
        parse_comando_identificador();
    } else {
        erro_sintatico("comando");
    }
}

static void parse_bloco(void)
{
    diag_info("ENTRA ");
    consumir(TK_START);

    while (token_atual.atomo != TK_END) {
        parse_comando();
        consumir(PONTO_E_VIRGULA);
    }

    consumir(TK_END);
    diag_info("SAI ");
}

void parser_init(void)
{
    avancar();
}

void parse_program(void)
{
    diag_info("ENTRA ");
    parse_program_interno();
    diag_info("SAI ");
}

static void parse_program_interno(void)
{
    consumir(TK_MODULE);
    consumir(IDENTIFICADOR);
    consumir(PONTO_E_VIRGULA);

    emitir0("INPP");

    if (token_atual.atomo == TK_GLOBALS) {
        int antes = total_variaveis_mepa;
        parse_globals();

        if (total_variaveis_mepa - antes > 0) {
            emitir1("AMEM", int_para_str(total_variaveis_mepa - antes));
        }
    }

    while (token_atual.atomo == TK_FN || token_atual.atomo == TK_PROC) {
        if (token_atual.atomo == TK_FN) {
            int geracao_anterior = geracao_ativa;
            geracao_ativa = 0;
            parse_funcao_decl();
            geracao_ativa = geracao_anterior;
        } else {
            consumir(TK_PROC);

            if (token_atual.atomo == TK_MAIN) {
                consumir(TK_MAIN);
                consumir(ABRE_PAR);
                consumir(FECHA_PAR);

                ts_enter_scope("main");
                geracao_ativa = 1;

                if (token_atual.atomo == TK_LOCALS) {
                    int antes = total_variaveis_mepa;
                    parse_locals();

                    if (total_variaveis_mepa - antes > 0) {
                        emitir1("AMEM", int_para_str(total_variaveis_mepa - antes));
                    }
                }

                parse_bloco();
                ts_exit_scope();
                consumir(FIM_ARQUIVO);

                if (total_variaveis_mepa > 0) {
                    emitir1("DMEM", int_para_str(total_variaveis_mepa));
                }

                emitir0("PARA");
                return;
            }

            {
                int geracao_anterior = geracao_ativa;
                geracao_ativa = 0;
                parse_proc_decl_resto();
                geracao_ativa = geracao_anterior;
            }
        }
    }

    erro_sintatico("proc main");
}
