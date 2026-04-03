/*
    Varredura sequencial do código‑fonte, devolvendo um token por chamada.
    Mantém posição exata no arquivo e distingue corretamente todas as categorias lexicais da SAL.
    Interface: lex_next
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"

static FILE *fonte = NULL;
static int linha_atual = 1;

static int eh_letra(int c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int eh_digito(int c)
{
    return c >= '0' && c <= '9';
}

static int eh_inicio_identificador(int c)
{
    return eh_letra(c) || c == '_';
}

static int eh_corpo_identificador(int c)
{
    return eh_letra(c) || eh_digito(c) || c == '_';
}

static TInfoAtomo criar_token(TAtomo atomo, const char *texto)
{
    TInfoAtomo token;

    token.atomo = atomo;
    token.linha = linha_atual;
    token.atributo_numero = 0;
    token.texto[0] = '\0';

    if (texto != NULL)
    {
        strncpy(token.texto, texto, sizeof(token.texto) - 1);
        token.texto[sizeof(token.texto) - 1] = '\0';
    }

    return token;
}

static TAtomo reconhece_palavra(const char texto[])
{
    switch (texto[0])
    {
    case 'b':
        if (strcmp(texto, "bool") == 0)
            return TK_BOOL;
        break;

    case 'c':
        if (strcmp(texto, "char") == 0)
            return TK_CHAR;
        break;

    case 'd':
        if (strcmp(texto, "do") == 0)
            return TK_DO;
        break;

    case 'e':
        if (strcmp(texto, "else") == 0)
            return TK_ELSE;
        if (strcmp(texto, "end") == 0)
            return TK_END;
        break;

    case 'f':
        if (strcmp(texto, "false") == 0)
            return TK_FALSE;
        if (strcmp(texto, "fn") == 0)
            return TK_FN;
        if (strcmp(texto, "for") == 0)
            return TK_FOR;
        break;

    case 'g':
        if (strcmp(texto, "globals") == 0)
            return TK_GLOBALS;
        break;

    case 'i':
        if (strcmp(texto, "if") == 0)
            return TK_IF;
        if (strcmp(texto, "int") == 0)
            return TK_INT;
        break;

    case 'l':
        if (strcmp(texto, "locals") == 0)
            return TK_LOCALS;
        if (strcmp(texto, "loop") == 0)
            return TK_LOOP;
        break;

    case 'm':
        if (strcmp(texto, "main") == 0)
            return TK_MAIN;
        if (strcmp(texto, "match") == 0)
            return TK_MATCH;
        if (strcmp(texto, "module") == 0)
            return TK_MODULE;
        break;

    case 'o':
        if (strcmp(texto, "otherwise") == 0)
            return TK_OTHERWISE;
        break;

    case 'p':
        if (strcmp(texto, "print") == 0)
            return TK_PRINT;
        if (strcmp(texto, "proc") == 0)
            return TK_PROC;
        break;

    case 'r':
        if (strcmp(texto, "ret") == 0)
            return TK_RET;
        break;

    case 's':
        if (strcmp(texto, "scan") == 0)
            return TK_SCAN;
        if (strcmp(texto, "start") == 0)
            return TK_START;
        if (strcmp(texto, "step") == 0)
            return TK_STEP;
        break;

    case 't':
        if (strcmp(texto, "to") == 0)
            return TK_TO;
        if (strcmp(texto, "true") == 0)
            return TK_TRUE;
        break;

    case 'u':
        if (strcmp(texto, "until") == 0)
            return TK_UNTIL;
        break;

    case 'v':
        if (strcmp(texto, "v") == 0)
            return OU_LOGICO;
        break;

    case 'w':
        if (strcmp(texto, "when") == 0)
            return TK_WHEN;
        if (strcmp(texto, "while") == 0)
            return TK_WHILE;
        break;
    }

    return IDENTIFICADOR;
}

static int ignora_espacos(void)
{
    int c = fgetc(fonte);

    while (c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        if (c == '\n')
        {
            linha_atual++;
        }

        c = fgetc(fonte);
    }

    return c;
}

static TInfoAtomo ler_numero(int primeiro)
{
    TInfoAtomo token = criar_token(CONST_INT, NULL);
    int c = primeiro;
    int i = 0;

    while (eh_digito(c))
    {
        if (i < (int)sizeof(token.texto) - 1)
        {
            token.texto[i] = (char)c;
            i++;
        }

        c = fgetc(fonte);
    }

    token.texto[i] = '\0';

    if (c != EOF)
    {
        ungetc(c, fonte);
    }

    token.atributo_numero = atoi(token.texto);
    return token;
}

static TInfoAtomo ler_palavra(int primeiro)
{
    TInfoAtomo token = criar_token(IDENTIFICADOR, NULL);
    int c = primeiro;
    int i = 0;

    while (eh_corpo_identificador(c))
    {
        if (i < (int)sizeof(token.texto) - 1)
        {
            token.texto[i] = (char)c;
            i++;
        }

        c = fgetc(fonte);
    }

    token.texto[i] = '\0';

    if (c != EOF)
    {
        ungetc(c, fonte);
    }

    token.atomo = reconhece_palavra(token.texto);
    return token;
}

static TInfoAtomo ler_char(void)
{
    TInfoAtomo token = criar_token(ERRO, NULL);
    int meio = fgetc(fonte);
    int fecha = fgetc(fonte);

    if (meio == EOF || fecha == EOF)
    {
        strcpy(token.texto, "char_incompleto");
        return token;
    }

    if (meio == '\n')
    {
        linha_atual++;
        strcpy(token.texto, "char_invalido");
        return token;
    }

    if (fecha != '\'')
    {
        strcpy(token.texto, "char_invalido");
        return token;
    }

    token.atomo = CONST_CHAR;
    token.texto[0] = '\'';
    token.texto[1] = (char)meio;
    token.texto[2] = '\'';
    token.texto[3] = '\0';

    return token;
}

static TInfoAtomo ler_string(void)
{
    TInfoAtomo token = criar_token(STRING, NULL);
    int c = fgetc(fonte);
    int i = 0;

    while (c != '"' && c != EOF)
    {
        if (c == '\n')
        {
            linha_atual++;
        }

        if (i < (int)sizeof(token.texto) - 1)
        {
            token.texto[i] = (char)c;
            i++;
        }

        c = fgetc(fonte);
    }

    if (c == EOF)
    {
        token.atomo = ERRO;
        strcpy(token.texto, "string_nao_fechada");
        return token;
    }

    token.texto[i] = '\0';
    return token;
}

static TInfoAtomo ler_delimitador(int c)
{
    TInfoAtomo token = criar_token(ERRO, NULL);

    switch (c)
    {
    case ';':
        token.atomo = PONTO_E_VIRGULA;
        strcpy(token.texto, ";");
        return token;

    case '(':
        token.atomo = ABRE_PAR;
        strcpy(token.texto, "(");
        return token;

    case ')':
        token.atomo = FECHA_PAR;
        strcpy(token.texto, ")");
        return token;

    case '[':
        token.atomo = ABRE_COL;
        strcpy(token.texto, "[");
        return token;

    case ']':
        token.atomo = FECHA_COL;
        strcpy(token.texto, "]");
        return token;

    case ',':
        token.atomo = VIRGULA;
        strcpy(token.texto, ",");
        return token;

    case ':':
    {
        int prox = fgetc(fonte);

        if (prox == '=')
        {
            token.atomo = ATRIBUICAO;
            strcpy(token.texto, ":=");
            return token;
        }

        if (prox != EOF)
        {
            ungetc(prox, fonte);
        }

        token.atomo = DOIS_PONTOS;
        strcpy(token.texto, ":");
        return token;
    }
    }

    strcpy(token.texto, "delimitador_invalido");
    return token;
}

static TInfoAtomo ler_operador(int c)
{
    TInfoAtomo token = criar_token(ERRO, NULL);

    switch (c)
    {
    case '+':
        token.atomo = SOMA;
        strcpy(token.texto, "+");
        return token;

    case '-':
        token.atomo = SUBTRACAO;
        strcpy(token.texto, "-");
        return token;

    case '*':
        token.atomo = MULTIPLICACAO;
        strcpy(token.texto, "*");
        return token;

    case '/':
        token.atomo = DIVISAO;
        strcpy(token.texto, "/");
        return token;

    case '~':
        token.atomo = NEGACAO;
        strcpy(token.texto, "~");
        return token;

    case '^':
        token.atomo = E_LOGICO;
        strcpy(token.texto, "^");
        return token;

    case '=':
    {
        int prox = fgetc(fonte);

        if (prox == '>')
        {
            token.atomo = IMPLICA;
            strcpy(token.texto, "=>");
            return token;
        }

        if (prox != EOF)
        {
            ungetc(prox, fonte);
        }

        token.atomo = IGUAL;
        strcpy(token.texto, "=");
        return token;
    }

    case '>':
    {
        int prox = fgetc(fonte);

        if (prox == '=')
        {
            token.atomo = MAIOR_IGUAL;
            strcpy(token.texto, ">=");
            return token;
        }

        if (prox != EOF)
        {
            ungetc(prox, fonte);
        }

        token.atomo = MAIOR;
        strcpy(token.texto, ">");
        return token;
    }

    case '<':
    {
        int prox = fgetc(fonte);

        if (prox == '=')
        {
            token.atomo = MENOR_IGUAL;
            strcpy(token.texto, "<=");
            return token;
        }

        if (prox == '>')
        {
            token.atomo = DIFERENTE;
            strcpy(token.texto, "<>");
            return token;
        }

        if (prox != EOF)
        {
            ungetc(prox, fonte);
        }

        token.atomo = MENOR;
        strcpy(token.texto, "<");
        return token;
    }

    case '.':
    {
        int prox = fgetc(fonte);

        if (prox == '.')
        {
            token.atomo = INTERVALO;
            strcpy(token.texto, "..");
            return token;
        }

        if (prox != EOF)
        {
            ungetc(prox, fonte);
        }

        token.atomo = ERRO;
        strcpy(token.texto, ".");
        return token;
    }
    }

    strcpy(token.texto, "operador_invalido");
    return token;
}

static TInfoAtomo pular_comentario_linha(void)
{
    int c = fgetc(fonte);

    while (c != '\n' && c != EOF)
    {
        c = fgetc(fonte);
    }

    if (c == '\n')
    {
        linha_atual++;
    }

    return lex_next();
}

static TInfoAtomo pular_comentario_bloco(void)
{
    TInfoAtomo token = criar_token(ERRO, "comentario_bloco_nao_fechado");
    int atual;
    int anterior = 0;

    while ((atual = fgetc(fonte)) != EOF)
    {
        if (atual == '\n')
        {
            linha_atual++;
        }

        if (anterior == '}' && atual == '@')
        {
            return lex_next();
        }

        anterior = atual;
    }

    return token;
}

int lex_init(FILE *arquivo)
{
    if (arquivo == NULL)
    {
        printf("Erro ao abrir arquivo.\n");
        return 0;
    }

    fonte = arquivo;
    linha_atual = 1;
    return 1;
}

TInfoAtomo lex_next(void)
{
    int c = ignora_espacos();

    if (c == EOF)
    {
        return criar_token(FIM_ARQUIVO, "EOF");
    }

    if (eh_inicio_identificador(c))
    {
        return ler_palavra(c);
    }

    if (eh_digito(c))
    {
        return ler_numero(c);
    }

    switch (c)
    {
    case '\'':
        return ler_char();

    case '"':
        return ler_string();

    case '@':
    {
        int prox = fgetc(fonte);

        if (prox == '{')
        {
            return pular_comentario_bloco();
        }

        if (prox != EOF)
        {
            ungetc(prox, fonte);
        }

        return pular_comentario_linha();
    }

    case ';':
    case '(':
    case ')':
    case '[':
    case ']':
    case ',':
    case ':':
        return ler_delimitador(c);

    case '+':
    case '-':
    case '*':
    case '/':
    case '~':
    case '^':
    case '=':
    case '>':
    case '<':
    case '.':
        return ler_operador(c);

    default:
    {
        TInfoAtomo token = criar_token(ERRO, NULL);
        token.texto[0] = (char)c;
        token.texto[1] = '\0';
        return token;
    }
    }
}