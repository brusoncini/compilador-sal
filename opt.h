#ifndef OPT_H
#define OPT_H

typedef struct
{
    const char *arquivo_fonte;
    int gerar_tokens;
    int gerar_symtab;
    int gerar_trace;
} Opcoes;

int opts_parse(int argc, char *argv[], Opcoes *opcoes);
void opts_print_uso(void);

#endif