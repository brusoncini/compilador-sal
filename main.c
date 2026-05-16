/*
Coordena a execução: processa parâmetros,
abre arquivos, inicializa lex, parser, symtab e diag,
aciona o processo de análise e, ao final,
encerra todos os módulos de forma ordenada. Não realiza análise direta.
*/

#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "parser.h"
#include "symtab.h"
#include "opt.h"

int main(int argc, char *argv[])
{
    FILE *arquivo;
    Opcoes opcoes;

    if (!opts_parse(argc, argv, &opcoes))
    {
        return 1;
    }

    arquivo = fopen(opcoes.arquivo_fonte, "r");

    if (arquivo == NULL)
    {
        printf("Nao foi possivel abrir o arquivo: %s\n", opcoes.arquivo_fonte);
        return 1;
    }

    if (!lex_init(arquivo))
    {
        fclose(arquivo);
        return 1;
    }

    ts_init();

    parser_init();
    parse_program();

    printf("Analise sintatica concluida com sucesso.\n");
    ts_print();

    ts_free();
    fclose(arquivo);

    return 0;
}