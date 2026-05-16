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

int main(void)
{
    FILE *arquivo;

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

    ts_init();

    parser_init();
    parse_program();

    printf("Analise sintatica concluida com sucesso.\n");
    ts_print();

    ts_free();
    fclose(arquivo);
    return 0;
}
