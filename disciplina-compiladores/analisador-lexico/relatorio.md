 Relatório — Analisador Léxico da linguagem MiniC
 
**Disciplina:** Compiladores
**Atividade:** Construção de um analisador léxico em C
**Linguagem-alvo:** MiniC
 
---
 
## 1. Análise léxica: resumo
 
A análise léxica é a primeira etapa de um compilador. Sua função é ler o
código-fonte caractere por caractere e agrupá-los em unidades maiores
chamadas *tokens* — palavras reservadas, identificadores, números,
operadores, delimitadores e literais. Cada token carrega, além do seu
lexema (o texto original), uma categoria e a posição (linha e coluna)
em que começou no arquivo-fonte.
 
O analisador léxico não se preocupa com a estrutura gramatical do
programa (isso é tarefa do analisador sintático) nem com o significado
dos comandos (analisador semântico). Sua única responsabilidade é
transformar um fluxo bruto de caracteres em um fluxo de tokens bem
definidos, sinalizando eventuais símbolos que não pertencem ao
alfabeto da linguagem.
 
## 2. Estratégia e algoritmo utilizado
 
O `minilexer` foi implementado como um scanner de leitura única
(*single-pass*), sem o uso de expressões regulares ou geradores como
Lex/Flex, conforme exigido.
 
O núcleo do algoritmo é um laço principal (`analisar`) que repete os
seguintes passos até o fim do arquivo:
 
1. Ignora espaços em branco (espaço, tabulação, `\r`, `\n`), atualizando
   linha e coluna a cada caractere consumido.
2. Guarda a posição (linha, coluna) do próximo caractere não consumido
   — essa é a posição de início do próximo token.
3. Consome um caractere e decide, com base nele (e, quando necessário,
   em um único caractere de *lookahead*), qual regra léxica se aplica:
   identificador/palavra reservada, número inteiro/real, literal de
   caractere, comentário, operador de um ou dois caracteres,
   delimitador, ou símbolo inválido.
4. Emite o token correspondente (ou um erro léxico) e volta ao passo 1.
A leitura de caracteres é feita por duas funções: `espiar()`, que
consulta o próximo caractere do arquivo sem consumi-lo, e `avancar()`,
que consome esse caractere e atualiza a linha/coluna. Como toda decisão
que depende de mais de um caractere (operadores de dois caracteres,
comentários `//`, parte decimal de números) é resolvida **olhando à
frente antes de consumir**, o programa nunca precisa devolver um
caractere já lido ao fluxo de entrada.
 
Erros léxicos (identificador longo demais, número mal formado, literal
de caractere inválido, símbolo desconhecido) são reportados no ponto em
que ocorrem, e a análise sempre continua a partir do caractere seguinte
ao problema — nunca há interrupção do processo.
 
## 3. Exemplos de entrada e saída
 
**Entrada** (`testes/exemplo.mc`, igual ao do enunciado):
 
```
int idade = 18;
float media = 8.5;
 
// Verifica a aprovação
if (media >= 7.0) {
    print(media);
}
 
return 0;
```
 
**Saída produzida pelo programa:**
 
```
1:1    | PALAVRA_RESERVADA  | int
1:5    | IDENTIFICADOR      | idade
1:11   | OPERADOR           | =
1:13   | NUMERO_INTEIRO     | 18
1:15   | DELIMITADOR        | ;
2:1    | PALAVRA_RESERVADA  | float
2:7    | IDENTIFICADOR      | media
2:13   | OPERADOR           | =
2:15   | NUMERO_REAL        | 8.5
2:18   | DELIMITADOR        | ;
5:1    | PALAVRA_RESERVADA  | if
5:4    | DELIMITADOR        | (
5:5    | IDENTIFICADOR      | media
5:11   | OPERADOR           | >=
5:14   | NUMERO_REAL        | 7.0
5:17   | DELIMITADOR        | )
5:19   | DELIMITADOR        | {
6:5    | PALAVRA_RESERVADA  | print
6:10   | DELIMITADOR        | (
6:11   | IDENTIFICADOR      | media
6:16   | DELIMITADOR        | )
6:17   | DELIMITADOR        | ;
7:1    | DELIMITADOR        | }
9:1    | PALAVRA_RESERVADA  | return
9:8    | NUMERO_INTEIRO     | 0
9:9    | DELIMITADOR        | ;
 
Total de tokens: 26
Total de erros lexicos: 0
```
 
**Exemplo com erros** (`testes/11_caracteres_invalidos.mc`), mostrando a
recuperação de erros sem interrupção:
 
```
Entrada: int x = 10 @ 2;
 
1:1    | PALAVRA_RESERVADA  | int
1:5    | IDENTIFICADOR      | x
1:7    | OPERADOR           | =
1:9    | NUMERO_INTEIRO     | 10
ERRO_LEXICO | linha 1, coluna 12 | simbolo invalido: @
1:14   | NUMERO_INTEIRO     | 2
1:15   | DELIMITADOR        | ;
```
 
Repare que, mesmo após o `@` inválido, o `2` e o `;` seguintes
continuam sendo reconhecidos normalmente.
 
**Exemplo com número real malformado** (`06_numeros_reais_malformados.mc`):
 
```
Entrada: 12.
         1.2.3
 
ERRO_LEXICO | linha 1, coluna 1 | numero real malformado (sem digitos apos o ponto): 12.
ERRO_LEXICO | linha 2, coluna 1 | numero real malformado (pontos em excesso): 1.2.3
```
 
## 4. Principais dificuldades encontradas
 
- **Controle de linha e coluna combinado com lookahead.** A maior
  dificuldade foi garantir que a posição relatada para cada token fosse
  sempre a do seu primeiro caractere, mesmo quando o reconhecimento
  precisa olhar um ou mais caracteres à frente (por exemplo, para
  decidir entre `>` e `>=`, ou entre um comentário e uma divisão). A
  solução adotada — separar claramente "espiar" (não altera posição) de
  "avançar" (consome e atualiza posição) — resolveu isso de forma
  limpa, sem precisar de `ungetc` nem de lógica para "desfazer" o avanço
  de linha/coluna.
- **Números reais malformados.** Decidir até onde consumir uma
  sequência como `1.2.3` para gerar um único erro léxico coerente (em
  vez de, por exemplo, emitir `1.2` como token válido e depois `.3`
  como outro erro) exigiu pensar no caso como uma extensão do
  reconhecimento normal de número real, e não como um caso totalmente
  à parte.
- **Literais de caractere malformados.** Diferenciar `''` (vazio),
  `'ab'` (mais de um caractere) e `'x` (não fechado) e ainda assim
  manter o scanner sincronizado com o restante do arquivo exigiu previr
  explicitamente o caso de a entrada terminar (`EOF`) ou a linha
  terminar (`\n`) antes de uma aspa de fechamento.
- **Evitar estouro de buffer** para identificadores extremamente longos,
  sem deixar de consumir o identificador inteiro do arquivo (para não
  perder a sincronia da análise). A solução foi contar o tamanho real
  do lexema separadamente do que efetivamente é armazenado no buffer de
  exibição, sempre limitado a um tamanho fixo.
## 5. Divisão do trabalho
 
Atividade realizada individualmente: especificação, implementação do
`minilexer.c`, criação dos arquivos de teste, verificação com
`valgrind`, e redação do README e deste relatório foram feitas pelo
mesmo autor.
 
---
 
## 6. Respostas às questões propostas
 
**1. Por que palavras reservadas e identificadores podem começar sendo
reconhecidos pela mesma regra?**
 
Porque, do ponto de vista puramente léxico (formação de caracteres),
uma palavra reservada é apenas um identificador que coincide com uma
das palavras da lista fixa da linguagem (`int`, `if`, `while`, etc.). A
regra de formação — começar com letra ou `_` e continuar com letras,
dígitos ou `_` — é idêntica nos dois casos. Só depois de o lexema
completo ter sido montado é que faz sentido compará-lo com a lista de
palavras reservadas para decidir a categoria final do token. Usar uma
única função para os dois casos evita duplicar a lógica de
reconhecimento de caracteres.
 
**2. Por que operadores de dois caracteres devem ser verificados antes
dos operadores de um caractere?**
 
Porque muitos operadores de um caractere são prefixo de um operador de
dois caracteres (`=` é prefixo de `==`, `<` de `<=`, `!` de `!=`, etc.).
Se o scanner decidisse pelo operador de um caractere assim que
encontrasse, por exemplo, o primeiro `=`, ele nunca chegaria a
verificar se o próximo caractere também é `=`, e `==` acabaria sendo
lido como dois tokens `=` separados, o que é semanticamente muito
diferente (atribuição vs. comparação de igualdade). Verificar primeiro
a possibilidade de dois caracteres — usando *lookahead* — garante que o
maior lexema válido possível seja sempre escolhido (essa é a regra do
"casamento mais longo", *maximal munch*, comum em analisadores
léxicos).
 
**3. Qual é a diferença entre um erro léxico e um erro sintático?**
 
Um erro léxico ocorre quando uma sequência de caracteres não forma
nenhum token válido da linguagem — por exemplo, um símbolo como `@`
que não pertence a nenhuma categoria, ou um número mal formado como
`12.`. Já um erro sintático ocorre quando os tokens, individualmente
válidos, aparecem em uma ordem ou combinação que não respeita a
gramática da linguagem — por exemplo, `int = 10;`, em que `int`,
`=`, `10` e `;` são todos tokens perfeitamente válidos, mas a sequência
não forma um comando válido (falta o identificador da variável). O
analisador léxico só é capaz de detectar o primeiro tipo de erro; o
segundo é responsabilidade do analisador sintático, que atua em uma
etapa posterior, sobre o fluxo de tokens já reconhecido.
 
**4. Por que o analisador deve continuar trabalhando depois de
encontrar um símbolo inválido?**
 
Porque interromper a análise no primeiro erro tornaria o processo de
depuração muito mais lento: o programador só descobriria um erro por
vez, teria que corrigi-lo, recompilar e rodar de novo o analisador para
descobrir o próximo. Continuando a análise após reportar o erro léxico,
o compilador consegue listar de uma só vez todos os problemas
encontrados no arquivo (ou, pelo menos, todos os erros léxicos), o que
é muito mais útil na prática. Essa é também a razão pela qual, mesmo
diante de um identificador longo demais ou de um literal de caractere
malformado, o `minilexer` sempre consome o texto problemático por
inteiro antes de seguir em frente, evitando que o mesmo erro seja
relatado várias vezes ou que a análise perca a sincronia com o restante
do arquivo.
 
**5. Qual é o risco de não verificar o limite do vetor utilizado para
armazenar um lexema?**
 
Sem essa verificação, uma entrada com um lexema muito longo (por
exemplo, um identificador de centenas de caracteres) provocaria escrita
fora dos limites do vetor (*buffer overflow*). Em C, isso é um
comportamento indefinido: na melhor das hipóteses o programa trava; na
pior, ele sobrescreve silenciosamente outras variáveis, o endereço de
retorno da função ou outras regiões da memória, o que pode levar a
resultados incorretos difíceis de depurar ou até a vulnerabilidades de
segurança exploráveis (é uma das falhas mais clássicas e mais graves em
código C). Por isso o `minilexer` sempre verifica o espaço disponível
no buffer antes de gravar cada caractere do lexema.
 
**6. Em qual etapa seria detectado o problema em `int = 10;`,
considerando que todos os caracteres formam tokens válidos?**
 
Na análise sintática. Léxicamente, `int`, `=`, `10` e `;` são todos
tokens perfeitamente válidos (uma palavra reservada, um operador, um
número inteiro e um delimitador, respectivamente), então o analisador
léxico não identifica nenhum problema. O erro só aparece quando se
tenta encaixar essa sequência de tokens na gramática da linguagem — que
exige um identificador entre a palavra reservada de tipo e o operador
de atribuição (`int <identificador> = <expressão>;`) — e é o analisador
sintático que detecta essa violação da estrutura esperada.
 
**7. Qual seria a vantagem de armazenar os identificadores encontrados
em uma tabela de símbolos?**
 
Uma tabela de símbolos permitiria, entre outras coisas: (a) evitar
comparações repetidas de string, associando a cada identificador um
índice ou referência única, o que agiliza etapas posteriores do
compilador; (b) centralizar informações adicionais sobre cada
identificador conforme elas forem descobertas em etapas seguintes —
tipo, escopo, se é uma variável ou uma função, valor inicial etc.; (c)
detectar, já durante ou logo após a análise léxica/sintática, o uso de
identificadores não declarados ou declarações duplicadas; e (d) servir
de base para a geração de código, quando for necessário saber onde
cada variável deverá ser alocada. Embora não seja exigida nesta
atividade, a tabela de símbolos é um dos primeiros passos naturais para
evoluir o `minilexer` em direção a um compilador mais completo.
 