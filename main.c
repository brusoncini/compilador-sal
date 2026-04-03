/*
Coordena a execução: processa parâmetros,
abre arquivos, inicializa lex, parser, symtab e diag,
aciona o processo de análise e, ao final,
encerra todos os módulos de forma ordenada. Não realiza análise direta.
*/

#include <stdio.h>
#include <stdlib.h>

#include "lex.h"

// imprime os nomes dos enums pra debug
const char *nome_categoria(TAtomo atomo)
{
    switch (atomo)
    {
    case CONST_INT:
        return "CONST_INT";
    case CONST_CHAR:
        return "CONST_CHAR";
    case STRING:
        return "STRING";
    case IDENTIFICADOR:
        return "IDENTIFICADOR";
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
    case TK_IF:
        return "TK_IF";
    case TK_ELSE:
        return "TK_ELSE";
    case TK_MATCH:
        return "TK_MATCH";
    case TK_WHEN:
        return "TK_WHEN";
    case TK_OTHERWISE:
        return "TK_OTHERWISE";
    case TK_FOR:
        return "TK_FOR";
    case TK_TO:
        return "TK_TO";
    case TK_STEP:
        return "TK_STEP";
    case TK_DO:
        return "TK_DO";
    case TK_LOOP:
        return "TK_LOOP";
    case TK_WHILE:
        return "TK_WHILE";
    case TK_UNTIL:
        return "TK_UNTIL";
    case TK_PRINT:
        return "TK_PRINT";
    case TK_SCAN:
        return "TK_SCAN";
    case TK_RET:
        return "TK_RET";
    case TK_INT:
        return "TK_INT";
    case TK_BOOL:
        return "TK_BOOL";
    case TK_CHAR:
        return "TK_CHAR";
    case TK_TRUE:
        return "TK_TRUE";
    case TK_FALSE:
        return "TK_FALSE";
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
    case INTERVALO:
        return "INTERVALO";
    case IMPLICA:
        return "IMPLICA";
    case ERRO:
        return "ERRO";
    case FIM_ARQUIVO:
        return "FIM_ARQUIVO";
    default:
        return "DESCONHECIDO";
    }
}

int main()
{
    // 1. O main processa a linha de comando via opt e inicializa os módulos.
    FILE *arquivo;
    TInfoAtomo token;

    arquivo = fopen("arquivo.sal", "r");

    if (arquivo == NULL)
    {
        printf("Nao foi possivel abrir o arquivo.\n");
        return 1;
    }

    if (!lex_init(arquivo))
    {
        fclose(arquivo);
        return 1;
    }

    do
    {
        token = lex_next();
        printf("Token lido: %s\n", token.texto);
        printf("Linha: %d\n", token.linha);
        printf("Categoria: %s\n", nome_categoria(token.atomo));
    } while (token.atomo != FIM_ARQUIVO);

    // 2. Concluída a análise (com sucesso ou erro), o main finaliza os módulos, fecha os arquivos e encerra a execução.
    fclose(arquivo);
}
