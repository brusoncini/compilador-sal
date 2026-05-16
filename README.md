# SALc - Compilador da Linguagem SAL

<p align="center">
  <img src="https://img.shields.io/badge/C-99-blue.svg">
  <img src="https://img.shields.io/badge/GCC-Linux-green.svg">
  <img src="https://img.shields.io/badge/status-em%20desenvolvimento-yellow">
  <img src="https://img.shields.io/badge/tipo-projeto%20acadêmico-purple">
</p>

Compilador desenvolvido em C para a linguagem **SAL (Simple Academic Language)**, criado para a disciplina de Compiladores.

O projeto implementa:

- Análise Léxica
- Análise Sintática (ASDR)
- Tabela de Símbolos
- Controle de Escopos
- Geração de Logs
- Interface CLI

## 📚 Sobre a Linguagem SAL

A SAL é uma linguagem didática utilizada no estudo de compiladores.

Ela possui:

- Tipos primitivos (`int`, `bool`, `char`)
- Estruturas condicionais (`if`, `match`)
- Estruturas de repetição (`for`, `loop while`, `loop until`)
- Funções e procedimentos
- Variáveis globais e locais
- Vetores
- Expressões aritméticas e lógicas

## 🛠️ Tecnologias

- Linguagem C
- GCC
- Makefile
- Linux / WSL


## ⚙️ Compilação

```bash
make
```

O binário gerado será: `./salc`


## ▶️ Execução

Compilar um arquivo SAL:

```bash
./salc arquivo.sal
```

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


## 📄 Exemplo

Um exemplo de programa em SAL, reconhecido pelo compilador:

```sal
module Exemplo;

proc main()
start
    print("Olá mundo!");
end
```


## 👥 Feito por

|Nome               | RA         |
|-------------------|------------|
|Bruna Soncini      | 10428267   |
|Felipe Nakandakari | 10395160   |

Universidade Presbiteriana Mackenzie - Curso de Ciência da Computação


## 🚧 Status

Projeto em desenvolvimento!!!