#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opt.h"

static Opcoes opcoes;

static void mostrar_uso(void)
{
    printf("Uso: salc <arquivo.sal> [--tokens] [--symtab] [--trace]\n");
}

int opts_parse(int argc, char *argv[])
{
    int i;

    memset(&opcoes, 0, sizeof(opcoes));

    if (argc < 2)
    {
        mostrar_uso();
        return 0;
    }

    strncpy(opcoes.arquivo_fonte, argv[1], sizeof(opcoes.arquivo_fonte) - 1);
    opcoes.arquivo_fonte[sizeof(opcoes.arquivo_fonte) - 1] = '\0';

    for (i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "--tokens") == 0)
        {
            opcoes.gerar_tokens = 1;
        }
        else if (strcmp(argv[i], "--symtab") == 0)
        {
            opcoes.gerar_symtab = 1;
        }
        else if (strcmp(argv[i], "--trace") == 0)
        {
            opcoes.gerar_trace = 1;
        }
        else
        {
            printf("Opcao invalida: %s\n", argv[i]);
            mostrar_uso();
            return 0;
        }
    }

    return 1;
}

const Opcoes *opts_get(void)
{
    return &opcoes;
}
