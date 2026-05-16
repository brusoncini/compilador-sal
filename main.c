#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "parser.h"
#include "symtab.h"
#include "opt.h"
#include "log.h"

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

    if (opcoes.gerar_tokens)
    {
        if (!log_tokens_init(opcoes.arquivo_fonte))
        {
            fclose(arquivo);
            return 1;
        }
    }

    if (opcoes.gerar_symtab)
    {
        if (!log_symtab_init(opcoes.arquivo_fonte))
        {
            log_tokens_close();
            fclose(arquivo);
            return 1;
        }
    }

    if (opcoes.gerar_trace)
    {
        if (!log_trace_init(opcoes.arquivo_fonte))
        {
            log_tokens_close();
            log_symtab_close();
            fclose(arquivo);
            return 1;
        }
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
    log_tokens_close();
    log_symtab_close();
    log_trace_close();
    fclose(arquivo);

    return 0;
}