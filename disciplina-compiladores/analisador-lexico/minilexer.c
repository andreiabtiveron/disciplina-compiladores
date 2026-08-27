#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



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
