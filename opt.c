#include <stdio.h>
#include <string.h>

#include "opt.h"

void opts_print_uso(void)
{
    printf("Uso:\n");
    printf("  ./salc <arquivo.sal> [--tokens | --symtab | --trace]\n");
}

int opts_parse(int argc, char *argv[], Opcoes *opcoes)
{
    int i;

    opcoes->arquivo_fonte = NULL;
    opcoes->gerar_tokens = 0;
    opcoes->gerar_symtab = 0;
    opcoes->gerar_trace = 0;

    if (argc < 2)
    {
        opts_print_uso();
        return 0;
    }

    opcoes->arquivo_fonte = argv[1];

    for (i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "--tokens") == 0)
        {
            opcoes->gerar_tokens = 1;
        }
        else if (strcmp(argv[i], "--symtab") == 0)
        {
            opcoes->gerar_symtab = 1;
        }
        else if (strcmp(argv[i], "--trace") == 0)
        {
            opcoes->gerar_trace = 1;
        }
        else
        {
            printf("Opcao desconhecida: %s\n", argv[i]);
            opts_print_uso();
            return 0;
        }
    }

    return 1;
}