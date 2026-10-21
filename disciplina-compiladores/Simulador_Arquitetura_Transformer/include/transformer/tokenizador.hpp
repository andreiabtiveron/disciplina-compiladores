#pragma once

#include <string>
#include <vector>

// Etapa 3 do fluxo: tokenizacao.
namespace tf {

// Tokenizador por palavras/pontuacao: separa por espacos e trata cada sinal
// de pontuacao (? ! . , ; : etc.) como um token independente. Caracteres
// UTF-8 de acentuacao (byte >= 0x80) sao tratados como parte da palavra.
std::vector<std::string> tokenizar(const std::string& textoNormalizado);

// Versao didatica que demonstra que um token nao precisa ser uma palavra
// inteira: qualquer palavra com mais de 6 caracteres (nivel ASCII) e
// dividida em duas subpartes por uma regra fixa e simples. Usada apenas
// quando o usuario pede explicitamente a demonstracao (comando
// "subpalavra"), pois nao e o tokenizador padrao do simulador.
std::vector<std::string> tokenizarSubpalavras(const std::string& textoNormalizado);

}  // namespace tf
