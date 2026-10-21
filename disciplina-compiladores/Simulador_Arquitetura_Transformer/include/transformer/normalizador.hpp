#pragma once

#include <string>
#include <vector>

// Etapa 2 do fluxo: normalizacao do texto de entrada.
namespace tf {

struct ResultadoNormalizacao {
    std::string original;
    std::string normalizado;   // exibido ao usuario (preserva capitalizacao)
    std::string chaveBusca;    // minusculo + sem pontuacao (uso interno)
    std::vector<std::string> regrasAplicadas;
};

// Regra adotada e documentada: preserva a capitalizacao original do texto
// (nao converte para minusculas), remove espacos duplicados e espacos nas
// extremidades, e mantem a pontuacao original. Uma chave de busca auxiliar
// (minuscula e sem pontuacao) e gerada separadamente para uso interno do
// sistema de respostas cadastradas.
ResultadoNormalizacao normalizar(const std::string& entrada);

}  // namespace tf
