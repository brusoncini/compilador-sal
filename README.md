# SALc - Compilador da Linguagem SAL

<p align="center">
  <img src="https://img.shields.io/badge/C-99-blue.svg">
  <img src="https://img.shields.io/badge/GCC-Linux-yellow.svg">
  <img src="https://img.shields.io/badge/status-concluído-green">
  <img src="https://img.shields.io/badge/tipo-projeto%20acadêmico-purple">
</p>

Compilador desenvolvido em linguagem **C** para a linguagem **SAL (Simple Academic Language)**, como projeto acadêmico da disciplina de **Compiladores**.

O projeto implementa as principais fases de um compilador, desde a leitura do código-fonte até a geração de código intermediário compatível com a **MEPA (Máquina de Execução de Pascal)**.

---

## 📌 Funcionalidades implementadas

O compilador atualmente contempla:

- Análise léxica;
- Análise sintática por **ASDR**;
- Tabela de símbolos;
- Controle de escopos;
- Análise semântica;
- Verificação de declaração prévia de identificadores;
- Verificação de duplicidade de símbolos no mesmo escopo;
- Checagem básica de tipos;
- Geração de código intermediário em **MEPA**;
- Geração de arquivo `.mepa`;
- Interface via linha de comando;
- Geração opcional de tokens, tabela de símbolos e rastreamento da análise.

---

## 📚 Sobre a linguagem SAL

A **SAL** é uma linguagem didática utilizada para o estudo das etapas de construção de compiladores.

A linguagem possui recursos como:

- Tipos primitivos: `int`, `bool` e `char`;
- Variáveis globais e locais;
- Comandos de entrada e saída: `scan` e `print`;
- Atribuições;
- Expressões aritméticas;
- Expressões relacionais;
- Expressões lógicas;
- Estruturas condicionais, como `if` e `else`;
- Estruturas de repetição, como `loop while` e `loop until`;
- Funções e procedimentos;
- Vetores.

---

## 🖥️ O que é a MEPA?

A **MEPA (Máquina de Execução de Pascal)** é uma máquina virtual didática baseada em pilha, utilizada no ensino de compiladores para executar código intermediário gerado a partir de linguagens estruturadas.

Em vez de traduzir diretamente o programa SAL para linguagem de máquina real, o compilador gera um conjunto de instruções MEPA, como `INPP`, `AMEM`, `CRCT`, `CRVL`, `ARMZ`, `SOMA`, `DSVF`, `IMPR` e `PARA`.

Essas instruções representam operações de baixo nível, como alocação de memória, carregamento de constantes, acesso a variáveis, operações aritméticas, desvios condicionais e impressão de valores. Dessa forma, a MEPA permite simular a execução do programa compilado de maneira mais simples e controlada.

## ⚙️ Recursos atualmente gerados em MEPA

A geração de código MEPA contempla o núcleo principal da linguagem, incluindo:

- Declaração de variáveis;
- Alocação de memória com `AMEM`;
- Desalocação de memória com `DMEM`;
- Leitura com `LEIT`;
- Impressão com `IMPR`;
- Atribuição com `ARMZ`;
- Carregamento de constantes com `CRCT`;
- Carregamento de variáveis com `CRVL`;
- Operações aritméticas:
  - `SOMA`
  - `SUBT`
  - `MULT`
  - `DIVI`
  - `INVR`
- Operações relacionais:
  - `CMME`
  - `CMEG`
  - `CMMA`
  - `CMAG`
  - `CMIG`
  - `CMDG`
- Operações lógicas:
  - `CONJ`
  - `DISJ`
  - `NEGA`
- Desvios e rótulos:
  - `DSVF`
  - `DSVS`
  - `NADA`
- Finalização do programa com `PARA`.

---

## 🧠 Análise semântica

Durante a análise, o compilador também realiza validações semânticas, como:

- Verificar se uma variável foi declarada antes de ser usada;
- Impedir a declaração duplicada de identificadores no mesmo escopo;
- Verificar se uma atribuição possui tipos compatíveis;
- Validar o uso de variáveis em expressões;
- Validar condições em comandos condicionais e de repetição;
- Controlar os endereços das variáveis para geração do código MEPA.

Exemplo de erro semântico detectável:

```sal
module Erro;

globals
a : int;

proc main()
start
    a := b + 1;
end
```

Nesse caso, a variável `b` não foi declarada.

---

## 🛠️ Tecnologias utilizadas

- Linguagem C;
- GCC;
- Makefile;
- Linux / WSL;
- MEPA.

---

## ⚙️ Compilação

Para compilar o projeto, execute:

```bash
make
```

Para limpar os arquivos gerados e recompilar do zero:

```bash
make clean
make
```

O binário gerado será:

```bash
./salc
```

---

## ▶️ Execução

Para compilar um arquivo SAL:

```bash
./salc arquivo.sal
```

Ao executar o compilador, será gerado automaticamente um arquivo `.mepa` com o mesmo nome do arquivo de entrada.

---

## 🔎 Opções auxiliares

Gerar lista de tokens:

```bash
./salc arquivo.sal --tokens
```

Gerar tabela de símbolos:

```bash
./salc arquivo.sal --symtab
```

Gerar rastreamento da análise:

```bash
./salc arquivo.sal --trace
```

---

## 📄 Exemplo de programa SAL

Arquivo `exemplo.sal`:

```sal
module Ex1;

globals
a,b,c : int;

proc main()
start
    scan(b);
    scan(c);
    a := b + c * 2;
    print(a);
end
```

---

## 📤 Código MEPA gerado

Para o exemplo acima, o compilador gera um arquivo `exemplo.mepa` semelhante a:

```text
INPP
AMEM 3
LEIT
ARMZ 0,1
LEIT
ARMZ 0,2
CRVL 0,1
CRVL 0,2
CRCT 2
MULT
SOMA
ARMZ 0,0
CRVL 0,0
IMPR
DMEM 3
PARA
```

Esse código pode ser executado em um interpretador MEPA compatível.

---

## 🚧 Limitações atuais

A geração de código MEPA foi implementada para o núcleo principal da linguagem SAL.

Alguns recursos da linguagem podem estar apenas reconhecidos sintaticamente ou parcialmente tratados, como:

- Vetores;
- Chamadas completas de funções e procedimentos;
- Comando `match`;
- Alguns casos mais avançados de escopo local;
- Impressão direta de strings na MEPA.

Essas limitações fazem parte do recorte adotado para a etapa de geração de código do projeto acadêmico.

---

## 👩🏻‍💻 Feito por

Desenvolvido com ❤️ por [Bruna Soncini](https://www.linkedin.com/in/brunasoncini/).

Projeto acadêmico desenvolvido para a disciplina de **Compiladores**.