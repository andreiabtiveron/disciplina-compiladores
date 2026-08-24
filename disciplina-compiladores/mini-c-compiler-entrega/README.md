# Mini C Compiler — Análise Individual (Compiladores)

> Este repositório é uma **entrega acadêmica individual** para a disciplina de Compiladores, baseada no projeto original **[ironrinox/mini-c-compiler](https://github.com/ironrinox/mini-c-compiler)**, de autoria de **Rino Di Paola**, distribuído sob licença MIT (ver `LICENSE`). Todos os créditos do projeto original são preservados.

Este repositório contém:

- `src/` e `include/` — código-fonte original, com uma modificação individual (correção de precedência/associatividade no parser — ver `RELATORIO.md`, Seção 15).
- `examples/` — programas de exemplo originais.
- `testes/` — casos de teste léxicos, sintáticos, de execução e de precedência/associatividade criados para esta atividade.
- `evidencias/` — saída bruta capturada da execução de cada teste.
- `RELATORIO.md` — relatório técnico completo da análise, exigido pela atividade.

## Como compilar

```
gcc src/main.c src/lexer.c src/parser.c src/interpreter.c src/utils.c -Iinclude -o mini-c
```

## Como executar

```
./mini-c examples/test.txt
```

ou qualquer um dos arquivos em `testes/`:

```
./mini-c testes/09_precedencia_teste_a.txt
```

## Sobre o projeto original

Compilador educacional escrito em C puro (sem dependências externas), que implementa a leitura de um arquivo-fonte, análise léxica, análise sintática (construção de AST) e interpretação direta da AST para uma linguagem mínima com `let`, `print`, inteiros, identificadores e operadores `+ - * /`. Como detalhado no `RELATORIO.md`, o projeto funciona atualmente como um **interpretador**, não um compilador (não gera assembly, bytecode ou código de máquina).

Veja `RELATORIO.md` para a análise técnica completa, incluindo o mapeamento de arquitetura, testes obrigatórios e a melhoria individual implementada.
