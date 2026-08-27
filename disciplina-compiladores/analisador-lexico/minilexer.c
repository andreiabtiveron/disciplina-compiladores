#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//CONSTANTES E TIPOS
 
#define MAX_LEXEMA 200        // tamanho maximo do buffer de lexema   
#define MAX_IDENTIFICADOR 31  // tamanho maximo permitido p/ id.      
 
typedef enum {
    TOKEN_PALAVRA_RESERVADA,
    TOKEN_IDENTIFICADOR,
    TOKEN_NUMERO_INTEIRO,
    TOKEN_NUMERO_REAL,
    TOKEN_LITERAL_CARACTERE,
    TOKEN_OPERADOR,
    TOKEN_DELIMITADOR
} TipoToken;
 
typedef struct {
    TipoToken tipo;
    char lexema[MAX_LEXEMA];
    int linha;
    int coluna;
} Token;
 
//ESTADO GLOBAL DO SCANNER
 static FILE *fonte;           /* arquivo-fonte sendo lido             */
static int linhaAtual = 1;    /* linha do proximo caractere a ler     */
static int colunaAtual = 1;   /* coluna do proximo caractere a ler    */
static int caractereEspiado = -2; /* -2 = nada espiado ainda          */
 
static long totalTokens = 0;
static long totalErros = 0;
 

//Leitura de caracteres com controle de linha/coluna e lookahead  
// Observa o proximo caractere sem consumi-lo (nao move linha/coluna). 
static int espiar(void) {
    if (caractereEspiado == -2) {
        caractereEspiado = fgetc(fonte);
    }
    return caractereEspiado;
}
 
/* Consome o proximo caractere (usando o que foi espiado, se houver)
 * e atualiza corretamente a linha e a coluna. */
static int avancar(void) {
    int c = espiar();
    caractereEspiado = -2;
 
    if (c == '\n') {
        linhaAtual++;
        colunaAtual = 1;
    } else if (c != EOF) {
        colunaAtual++;
    }
    return c;
}
 

// Funcoes auxiliares de classificacao de caracteres                    
static int ehInicioIdentificador(char c) {
    return isalpha((unsigned char) c) || c == '_';
}
 
static int ehParteIdentificador(char c) {
    return isalnum((unsigned char) c) || c == '_';
}
 
static int ehDelimitador(char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' ||
           c == '[' || c == ']' || c == ';' || c == ',';
}
 
// Lista de palavras reservadas da linguagem MiniC
static const char *PALAVRAS_RESERVADAS[] = {
    "int", "float", "char", "if", "else", "while", "return", "print"
};
static const int QTD_PALAVRAS_RESERVADAS =
    (int) (sizeof(PALAVRAS_RESERVADAS) / sizeof(PALAVRAS_RESERVADAS[0]));
 
static int ehPalavraReservada(const char *lexema) {
    for (int i = 0; i < QTD_PALAVRAS_RESERVADAS; i++) {
        if (strcmp(lexema, PALAVRAS_RESERVADAS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}
 

static const char *nomeDoToken(TipoToken tipo) {
    switch (tipo) {
        case TOKEN_PALAVRA_RESERVADA: return "PALAVRA_RESERVADA";
        case TOKEN_IDENTIFICADOR:     return "IDENTIFICADOR";
        case TOKEN_NUMERO_INTEIRO:    return "NUMERO_INTEIRO";
        case TOKEN_NUMERO_REAL:       return "NUMERO_REAL";
        case TOKEN_LITERAL_CARACTERE: return "LITERAL_CARACTERE";
        case TOKEN_OPERADOR:          return "OPERADOR";
        case TOKEN_DELIMITADOR:       return "DELIMITADOR";
    }
    return "DESCONHECIDO";
}
 
 
static void imprimirToken(const Token *token) {
    char posicao[32];
    snprintf(posicao, sizeof(posicao), "%d:%d", token->linha, token->coluna);
    printf("%-7s| %-19s| %s\n", posicao, nomeDoToken(token->tipo), token->lexema);
    totalTokens++;
}
 
static void imprimirErro(int linha, int coluna, const char *mensagem) {
    printf("ERRO_LEXICO | linha %d, coluna %d | %s\n", linha, coluna, mensagem);
    totalErros++;
}
 
/* Acrescenta um caractere ao buffer de lexema, respeitando o limite
 * do vetor (evita qualquer escrita fora dos limites). */
static void acrescentarLexema(char *buffer, int *tamanho, char c) {
    if (*tamanho < MAX_LEXEMA - 1) {
        buffer[*tamanho] = c;
        (*tamanho)++;
    }
    /* Se o buffer estiver cheio, o caractere e simplesmente descartado
     * da representacao textual, mas o caractere ja foi consumido do
     * fluxo de entrada pelo chamador (avancar ja rodou antes). */
}
 

//RECONHECIMENTO DE CATEGORIAS DE BUFFER 
/* Identificadores e palavras reservadas.
 * 'primeiro' e o primeiro caractere, ja consumido pelo chamador. */
static void reconhecerIdentificador(char primeiro, int linha, int coluna) {
    char buffer[MAX_LEXEMA];
    int tamanho = 0;
    int tamanhoReal = 0; /* conta todos os caracteres, mesmo que o
                             buffer de exibicao seja truncado */
 
    acrescentarLexema(buffer, &tamanho, primeiro);
    tamanhoReal = 1;
 
    while (ehParteIdentificador((char) espiar())) {
        char c = (char) avancar();
        acrescentarLexema(buffer, &tamanho, c);
        tamanhoReal++;
    }
    buffer[tamanho] = '\0';
 
    if (tamanhoReal > MAX_IDENTIFICADOR) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "identificador excede %d caracteres: %s",
                 MAX_IDENTIFICADOR, buffer);
        imprimirErro(linha, coluna, msg);
        return;
    }
 
    Token token;
    token.linha = linha;
    token.coluna = coluna;
    strncpy(token.lexema, buffer, MAX_LEXEMA - 1);
    token.lexema[MAX_LEXEMA - 1] = '\0';
 
    if (ehPalavraReservada(buffer)) {
        token.tipo = TOKEN_PALAVRA_RESERVADA;
    } else {
        token.tipo = TOKEN_IDENTIFICADOR;
    }
    imprimirToken(&token);
}
 
/* Numeros inteiros e reais
 * 'primeiroDigito' e o primeiro algarismo, ja consumido. */
static void reconhecerNumero(char primeiroDigito, int linha, int coluna) {
    char buffer[MAX_LEXEMA];
    int tamanho = 0;
 
    acrescentarLexema(buffer, &tamanho, primeiroDigito);
    while (isdigit((unsigned char) espiar())) {
        acrescentarLexema(buffer, &tamanho, (char) avancar());
    }
 
    if (espiar() == '.') {
        acrescentarLexema(buffer, &tamanho, (char) avancar()); /* consome '.' */
        int digitosAposPonto = 0;
 
        while (isdigit((unsigned char) espiar())) {
            acrescentarLexema(buffer, &tamanho, (char) avancar());
            digitosAposPonto++;
        }
 
        if (digitosAposPonto == 0) {
            buffer[tamanho] = '\0';
            char msg[256];
            snprintf(msg, sizeof(msg),
                     "numero real malformado (sem digitos apos o ponto): %s",
                     buffer);
            imprimirErro(linha, coluna, msg);
            return;
        }
 
        if (espiar() == '.') {
            /* algo como 1.2.3 -> continua consumindo digitos/pontos
             * extras para reportar um unico erro coerente. */
            while (isdigit((unsigned char) espiar()) || espiar() == '.') {
                acrescentarLexema(buffer, &tamanho, (char) avancar());
            }
            buffer[tamanho] = '\0';
            char msg[256];
            snprintf(msg, sizeof(msg),
                     "numero real malformado (pontos em excesso): %s",
                     buffer);
            imprimirErro(linha, coluna, msg);
            return;
        }
 
        buffer[tamanho] = '\0';
        Token token;
        token.tipo = TOKEN_NUMERO_REAL;
        token.linha = linha;
        token.coluna = coluna;
        strncpy(token.lexema, buffer, MAX_LEXEMA - 1);
        token.lexema[MAX_LEXEMA - 1] = '\0';
        imprimirToken(&token);
        return;
    }
 
    buffer[tamanho] = '\0';
    Token token;
    token.tipo = TOKEN_NUMERO_INTEIRO;
    token.linha = linha;
    token.coluna = coluna;
    strncpy(token.lexema, buffer, MAX_LEXEMA - 1);
    token.lexema[MAX_LEXEMA - 1] = '\0';
    imprimirToken(&token);
}
 
/* Literais de caractere: 'x' */
static void reconhecerLiteralCaractere(int linha, int coluna) {
    char buffer[MAX_LEXEMA];
    int tamanho = 0;
    acrescentarLexema(buffer, &tamanho, '\'');
 
    int c1 = avancar();
 
    if (c1 == EOF || c1 == '\n') {
        buffer[tamanho] = '\0';
        imprimirErro(linha, coluna,
                     "literal de caractere nao fechado");
        return;
    }
 
    if (c1 == '\'') {
        /* literal vazio: '' */
        acrescentarLexema(buffer, &tamanho, '\'');
        buffer[tamanho] = '\0';
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "literal de caractere vazio: %s", buffer);
        imprimirErro(linha, coluna, msg);
        return;
    }
 
    acrescentarLexema(buffer, &tamanho, (char) c1);
 
    if (espiar() == '\'') {
        acrescentarLexema(buffer, &tamanho, (char) avancar());
        buffer[tamanho] = '\0';
 
        Token token;
        token.tipo = TOKEN_LITERAL_CARACTERE;
        token.linha = linha;
        token.coluna = coluna;
        strncpy(token.lexema, buffer, MAX_LEXEMA - 1);
        token.lexema[MAX_LEXEMA - 1] = '\0';
        imprimirToken(&token);
        return;
    }
 
    /* Mais de um caractere dentro das aspas, ou aspas nao fechadas:
     * consome ate encontrar o fechamento, quebra de linha ou EOF,
     * para poder continuar a analise a partir de um ponto coerente. */
    while (espiar() != '\'' && espiar() != '\n' && espiar() != EOF) {
        acrescentarLexema(buffer, &tamanho, (char) avancar());
    }
 
    if (espiar() == '\'') {
        acrescentarLexema(buffer, &tamanho, (char) avancar());
        buffer[tamanho] = '\0';
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "literal de caractere com mais de um caractere: %s", buffer);
        imprimirErro(linha, coluna, msg);
    } else {
        buffer[tamanho] = '\0';
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "literal de caractere nao fechado: %s", buffer);
        imprimirErro(linha, coluna, msg);
    }
}
 
/* Emite um token simples de operador ou delimitador (1 ou 2 chars). */
static void emitirSimples(TipoToken tipo, const char *lexema, int linha, int coluna) {
    Token token;
    token.tipo = tipo;
    token.linha = linha;
    token.coluna = coluna;
    strncpy(token.lexema, lexema, MAX_LEXEMA - 1);
    token.lexema[MAX_LEXEMA - 1] = '\0';
    imprimirToken(&token);
}
 
/* ------------------------------------------------------------------ */
/* Laco principal do analisador lexico                                 */
/* ------------------------------------------------------------------ */
 
static void analisar(void) {
    for (;;) {
        /* pula espacos em branco (espaco, tab, CR, LF) */
        int c = espiar();
        while (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            avancar();
            c = espiar();
        }
 
        if (c == EOF) {
            break;
        }
 
        int linhaToken = linhaAtual;
        int colunaToken = colunaAtual;
        c = avancar();
 
        if (c == '/') {
            if (espiar() == '/') {
                /* comentario de uma linha: ignora ate o fim da linha */
                avancar();
                while (espiar() != '\n' && espiar() != EOF) {
                    avancar();
                }
                continue;
            }
            emitirSimples(TOKEN_OPERADOR, "/", linhaToken, colunaToken);
            continue;
        }
 
        if (ehInicioIdentificador((char) c)) {
            reconhecerIdentificador((char) c, linhaToken, colunaToken);
            continue;
        }
 
        if (isdigit((unsigned char) c)) {
            reconhecerNumero((char) c, linhaToken, colunaToken);
            continue;
        }
 
        if (c == '\'') {
            reconhecerLiteralCaractere(linhaToken, colunaToken);
            continue;
        }
 
        if (ehDelimitador((char) c)) {
            char lex[2] = { (char) c, '\0' };
            emitirSimples(TOKEN_DELIMITADOR, lex, linhaToken, colunaToken);
            continue;
        }
 
        switch (c) {
            case '+': emitirSimples(TOKEN_OPERADOR, "+", linhaToken, colunaToken); continue;
            case '-': emitirSimples(TOKEN_OPERADOR, "-", linhaToken, colunaToken); continue;
            case '*': emitirSimples(TOKEN_OPERADOR, "*", linhaToken, colunaToken); continue;
            case '%': emitirSimples(TOKEN_OPERADOR, "%", linhaToken, colunaToken); continue;
 
            case '=':
                if (espiar() == '=') { avancar(); emitirSimples(TOKEN_OPERADOR, "==", linhaToken, colunaToken); }
                else                 { emitirSimples(TOKEN_OPERADOR, "=", linhaToken, colunaToken); }
                continue;
 
            case '!':
                if (espiar() == '=') { avancar(); emitirSimples(TOKEN_OPERADOR, "!=", linhaToken, colunaToken); }
                else                 { emitirSimples(TOKEN_OPERADOR, "!", linhaToken, colunaToken); }
                continue;
 
            case '<':
                if (espiar() == '=') { avancar(); emitirSimples(TOKEN_OPERADOR, "<=", linhaToken, colunaToken); }
                else                 { emitirSimples(TOKEN_OPERADOR, "<", linhaToken, colunaToken); }
                continue;
 
            case '>':
                if (espiar() == '=') { avancar(); emitirSimples(TOKEN_OPERADOR, ">=", linhaToken, colunaToken); }
                else                 { emitirSimples(TOKEN_OPERADOR, ">", linhaToken, colunaToken); }
                continue;
 
            case '&':
                if (espiar() == '&') {
                    avancar();
                    emitirSimples(TOKEN_OPERADOR, "&&", linhaToken, colunaToken);
                } else {
                    imprimirErro(linhaToken, colunaToken, "simbolo invalido: &");
                }
                continue;
 
            case '|':
                if (espiar() == '|') {
                    avancar();
                    emitirSimples(TOKEN_OPERADOR, "||", linhaToken, colunaToken);
                } else {
                    imprimirErro(linhaToken, colunaToken, "simbolo invalido: |");
                }
                continue;
 
            default: {
                char msg[64];
                snprintf(msg, sizeof(msg), "simbolo invalido: %c", (char) c);
                imprimirErro(linhaToken, colunaToken, msg);
                continue;
            }
        }
    }
}
 

//main
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: minilexer <arquivo-fonte>\n");
        return 1;
    }
 
    fonte = fopen(argv[1], "r");
    if (fonte == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo '%s'.\n", argv[1]);
        return 1;
    }
 
    analisar();
 
    fclose(fonte);
 
    printf("\nTotal de tokens: %ld\n", totalTokens);
    printf("Total de erros lexicos: %ld\n", totalErros);
 
    return 0;
}
