#ifndef GERADOR_H
#define GERADOR_H

void gerador_init(const char *caminho_fonte);
void gerador_close(void);
const char *gerador_saida(void);

void gera_instr_mepa(const char *rotulo, const char *mnemonico, const char *parametro1, const char *parametro2);
char *novo_rotulo(void);

#endif
