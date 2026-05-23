#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gerador.h"

static FILE *arquivo_mepa = NULL;
static char caminho_saida[520];
static int contador_rotulos = 0;

static void montar_caminho_saida(const char *caminho_fonte)
{
    char *ponto;

    if (caminho_fonte == NULL) {
        snprintf(caminho_saida, sizeof(caminho_saida), "saida.mepa");
        return;
    }

    snprintf(caminho_saida, sizeof(caminho_saida), "%s", caminho_fonte);
    ponto = strrchr(caminho_saida, '.');

    if (ponto != NULL) {
        *ponto = '\0';
    }

    strncat(caminho_saida, ".mepa", sizeof(caminho_saida) - strlen(caminho_saida) - 1);
}

void gerador_init(const char *caminho_fonte)
{
    montar_caminho_saida(caminho_fonte);

    arquivo_mepa = fopen(caminho_saida, "w");
    if (arquivo_mepa == NULL) {
        printf("Erro: nao foi possivel criar o arquivo MEPA '%s'.\n", caminho_saida);
        exit(1);
    }
}

void gerador_close(void)
{
    if (arquivo_mepa != NULL) {
        fclose(arquivo_mepa);
        arquivo_mepa = NULL;
    }
}

const char *gerador_saida(void)
{
    return caminho_saida;
}

void gera_instr_mepa(const char *rotulo, const char *mnemonico, const char *parametro1, const char *parametro2)
{
    if (arquivo_mepa == NULL || mnemonico == NULL) {
        return;
    }

    if (rotulo != NULL && rotulo[0] != '\0') {
        fprintf(arquivo_mepa, "%s: ", rotulo);
    }

    fprintf(arquivo_mepa, "%s", mnemonico);

    if (parametro1 != NULL && parametro2 != NULL) {
        fprintf(arquivo_mepa, " %s,%s", parametro1, parametro2);
    } else if (parametro1 != NULL) {
        fprintf(arquivo_mepa, " %s", parametro1);
    }

    fprintf(arquivo_mepa, "\n");
}

char *novo_rotulo(void)
{
    char *rotulo = (char *)malloc(20 * sizeof(char));

    if (rotulo == NULL) {
        printf("Erro: nao foi possivel alocar rotulo MEPA.\n");
        exit(1);
    }

    contador_rotulos++;
    snprintf(rotulo, 20, "L%d", contador_rotulos);
    return rotulo;
}
