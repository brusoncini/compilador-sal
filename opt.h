#ifndef OPT_H
#define OPT_H

typedef struct
{
    char arquivo_fonte[512];
    int gerar_tokens;
    int gerar_symtab;
    int gerar_trace;
} Opcoes;

int opts_parse(int argc, char *argv[]);
const Opcoes *opts_get(void);

#endif