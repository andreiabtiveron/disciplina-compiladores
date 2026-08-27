# minilexer — Analisador Léxico da linguagem MiniC

Implementação em C (padrão C11) de um analisador léxico (scanner) para a
linguagem fictícia **MiniC**, desenvolvida como atividade prática da
disciplina de Compiladores.

## Como compilar

Requer apenas um compilador C compatível com C11 (testado com GCC 13).

```bash
gcc -Wall -Wextra -pedantic -std=c11 minilexer.c -o minilexer
```

A compilação não produz nenhum erro ou aviso.

## Como executar

```bash
./minilexer <arquivo-fonte>
```

Exemplo:

```bash
./minilexer testes/exemplo.mc
```

No Windows (após compilar com `gcc` ou outro compilador C11):

```bash
.\minilexer.exe testes\exemplo.mc
```

Se nenhum arquivo for informado, o programa imprime:

```
Uso: minilexer <arquivo-fonte>
```

Se o arquivo não existir ou não puder ser aberto, o programa informa o
erro e encerra com código de saída 1, sem tentar ler nada.

## Saída

Para cada token reconhecido é impressa uma linha no formato:

```
LINHA:COLUNA | CATEGORIA | LEXEMA
```

Para cada erro léxico encontrado é impressa uma linha no formato:

```
ERRO_LEXICO | linha L, coluna C | <mensagem>
```

Ao final, o programa imprime o total de tokens reconhecidos e o total de
erros léxicos encontrados. Erros não são contados como tokens — os dois
contadores são independentes, como pedido no enunciado.

## Estrutura do repositório

```
.
├── minilexer.c          # implementação do analisador léxico
├── README.md             # este arquivo
├── relatorio.md           # relatório da atividade
└── testes/                # arquivos .mc usados nos testes
    ├── exemplo.mc                          # exemplo do enunciado
    ├── 01_palavras_reservadas.mc
    ├── 02_identificadores_validos.mc
    ├── 03_identificador_muito_longo.mc
    ├── 04_numeros_inteiros.mc
    ├── 05_numeros_reais.mc
    ├── 06_numeros_reais_malformados.mc
    ├── 07_operadores.mc
    ├── 08_delimitadores.mc
    ├── 09_comentarios.mc
    ├── 10_literais_caractere.mc
    ├── 11_caracteres_invalidos.mc
    ├── 12_arquivo_vazio.mc
    ├── 13_somente_espacos_e_comentarios.mc
    └── 14_sem_espacos.mc
```

## Decisões de implementação

- **Leitura caractere a caractere com lookahead de 1 caractere.** O
  scanner usa duas funções internas: `espiar()`, que observa o próximo
  caractere sem consumi-lo e sem alterar linha/coluna, e `avancar()`,
  que consome o caractere (usando o que já foi espiado, se houver) e
  atualiza `linhaAtual`/`colunaAtual`. Essa abordagem evita a necessidade
  de "devolver" caracteres já consumidos ao fluxo (não é usado
  `ungetc`), porque toda decisão de dois caracteres (`==`, `!=`, `<=`,
  `>=`, `&&`, `||`, comentário `//` vs. divisão `/`, parte decimal de um
  número) é tomada **olhando à frente antes de consumir**.

- **Posição do token.** Antes de iniciar o reconhecimento de cada
  token, a linha e a coluna atuais (que sempre representam a posição do
  próximo caractere ainda não consumido) são copiadas para
  `linhaToken`/`colunaToken`. Essa é a posição relatada na saída,
  mesmo que o token tenha mais de um caractere.

- **Identificadores vs. palavras reservadas.** Ambos seguem a mesma
  regra léxica (começam com letra ou `_`, continuam com letras,
  dígitos ou `_`). Por isso são reconhecidos pela mesma função
  (`reconhecerIdentificador`), que ao final verifica, por comparação de
  string, se o lexema coincide com alguma palavra reservada
  (`ehPalavraReservada`). Só a partir desse ponto os dois casos se
  diferenciam.

- **Limite de 31 caracteres para identificadores.** O identificador é
  sempre consumido por completo (todos os caracteres válidos são lidos
  do arquivo), mesmo quando ultrapassa 31 caracteres — o programa apenas
  conta o tamanho real (`tamanhoReal`) e, se ultrapassar o limite,
  reporta um erro léxico em vez de emitir um token. Isso garante que a
  análise continue de forma consistente a partir do próximo caractere,
  sem quebrar o restante do arquivo.

- **Números reais malformados.** Depois de reconhecer os dígitos antes
  do ponto e o próprio ponto, o scanner conta quantos dígitos aparecem
  depois dele. Se nenhum dígito aparecer (`12.`), ou se houver um novo
  ponto logo em seguida (`1.2.3`), a sequência inteira é tratada como um
  único erro léxico (e todo o texto correspondente é consumido antes de
  continuar), em vez de gerar vários tokens picotados que confundiriam
  a leitura da saída.

- **Literais de caractere.** São tratados caso a caso: vazio (`''`),
  fechado corretamente com um único caractere (`'a'`), com mais de um
  caractere (`'ab'`) ou não fechado (`'x` seguido de quebra de linha ou
  fim de arquivo). Em todos os casos de erro, o scanner consome
  caracteres até encontrar uma aspa simples de fechamento, uma quebra
  de linha ou o fim do arquivo, para não perder a sincronia com o
  restante do código.

- **Comentários `//`.** Ao encontrar uma `/`, o scanner olha o próximo
  caractere: se for outra `/`, todo o restante da linha é descartado
  sem gerar nenhum token; caso contrário, a `/` isolada é emitida como
  operador de divisão.

- **Operadores de um e dois caracteres.** Para cada caractere que pode
  iniciar um operador composto (`=`, `!`, `<`, `>`, `&`, `|`), o scanner
  sempre olha o caractere seguinte antes de decidir o lexema final.
  Isso garante, por exemplo, que `==` seja emitido como um único token,
  e não como dois tokens `=` seguidos.

- **Caracteres inválidos.** Qualquer caractere que não se encaixe em
  nenhuma regra (por exemplo `@`, `#`, `$`, ou um `&`/`|` isolado, que
  não formam operador válido sozinhos) gera uma linha
  `ERRO_LEXICO | linha L, coluna C | simbolo invalido: <c>` e a análise
  continua no caractere seguinte, sem interromper o restante do
  arquivo.

- **Segurança de buffers.** Todos os lexemas são armazenados em um
  buffer de tamanho fixo (`MAX_LEXEMA = 200`, bem maior que qualquer
  token válido da linguagem). A função `acrescentarLexema` verifica o
  espaço disponível antes de escrever em cada posição, evitando
  qualquer escrita fora dos limites do vetor, mesmo em entradas
  adversariais (por exemplo, um identificador extremamente longo).

- **Organização do código.** O programa é dividido em funções curtas e
  de responsabilidade única: leitura de caracteres (`espiar`,
  `avancar`), classificação de caracteres (`ehInicioIdentificador`,
  `ehParteIdentificador`, `ehDelimitador`, `ehPalavraReservada`),
  reconhecimento de cada categoria de token
  (`reconhecerIdentificador`, `reconhecerNumero`,
  `reconhecerLiteralCaractere`, `emitirSimples`), impressão
  (`imprimirToken`, `imprimirErro`, `nomeDoToken`) e o laço principal
  (`analisar`).

## Limitações conhecidas

- Não são implementados comentários de múltiplas linhas (`/* ... */`),
  strings entre aspas duplas, sequências de escape em literais de
  caractere (`'\n'`) nem números em notação científica — nenhum desses
  itens é exigido pelo enunciado (fazem parte dos desafios opcionais).
- O programa não verifica balanceamento de parênteses/chaves, ordem dos
  comandos ou tipos — isso pertence à análise sintática/semântica, fora
  do escopo desta atividade.
- Mensagens de erro são todas em português, sem internacionalização.
- Não há tabela de símbolos (desafio opcional não implementado).

## Testes realizados

Os 14 casos de teste obrigatórios do enunciado estão na pasta
`testes/`, um arquivo por caso, cobrindo: todas as palavras reservadas;
identificadores válidos; identificador com mais de 31 caracteres;
números inteiros; números reais; números reais malformados; todos os
operadores; todos os delimitadores; comentários; literais de caractere
válidos e inválidos; caracteres inválidos; um arquivo vazio; um arquivo
só com espaços e comentários; e tokens colados sem espaço entre eles.
O arquivo `testes/exemplo.mc` reproduz o exemplo do enunciado, cuja
saída do programa foi conferida token a token (26 tokens, 0 erros,
posições de linha/coluna idênticas às apresentadas no enunciado). Todos
os testes também foram executados sob `valgrind` sem nenhum erro de
acesso à memória reportado.