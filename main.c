#include <stdio.h>
#include <stdlib.h>

#include "diag.h"
#include "lex.h"
#include "log.h"
#include "opt.h"
#include "parser.h"
#include "symtab.h"
#include "gerador.h"

int main(int argc, char *argv[])
{
    FILE *arquivo;
    const Opcoes *opcoes;
    char caminho_trace[520];

    if (!opts_parse(argc, argv))
    {
        return 1;
    }

    opcoes = opts_get();
    arquivo = fopen(opcoes->arquivo_fonte, "r");
    if (arquivo == NULL)
    {
        printf("Nao foi possivel abrir o arquivo '%s'.\n", opcoes->arquivo_fonte);
        return 1;
    }

    if (opcoes->gerar_tokens)
    {
        log_tokens_from_source(opcoes->arquivo_fonte);
    }

    if (opcoes->gerar_trace)
    {
        log_trace_from_source(opcoes->arquivo_fonte, caminho_trace, sizeof(caminho_trace));
        diag_set_trace(1);
        diag_set_trace_file(caminho_trace);
    }

    if (!lex_init(arquivo))
    {
        fclose(arquivo);
        return 1;
    }

    ts_init();
    
    gerador_init(opcoes->arquivo_fonte);
    parser_init();
    parse_program();

    gerador_close();

    printf("Analise concluida com sucesso.\n");
    printf("Arquivo MEPA gerado: %s\n", gerador_saida());

    if (opcoes->gerar_symtab)
    {
        log_symtab_from_source(opcoes->arquivo_fonte);
    }
    else
    {
        ts_print();
    }

    log_token_close();
    diag_close();
    ts_free();
    fclose(arquivo);
    return 0;
}
