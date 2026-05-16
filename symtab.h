#ifndef SYMTAB_H
#define SYMTAB_H

#define TAM_LEXEMA 100
#define TAM_CATEGORIA 30
#define TAM_TIPO 30
#define TAM_ESCOPO 100

typedef struct Simbolo
{
    char lexema[TAM_LEXEMA];
    char categoria[TAM_CATEGORIA];
    char tipo[TAM_TIPO];
    char escopo[TAM_ESCOPO];
    int extra;
    struct Simbolo *prox;
} Simbolo;

void ts_init(void);
void ts_free(void);

void ts_enter_scope(const char *nome);
void ts_exit_scope(void);

const char *ts_current_scope(void);

int ts_insert(const char *lexema, const char *categoria, const char *tipo, int extra);
Simbolo *ts_lookup(const char *lexema);

void ts_print(void);

#endif