#include "transformer/normalizador.hpp"
#include "transformer/util.hpp"

namespace tf {

ResultadoNormalizacao normalizar(const std::string& entrada) {
    ResultadoNormalizacao r;
    r.original = entrada;
    r.normalizado = colapsarEspacos(entrada);
    r.chaveBusca = removerPontuacao(paraMinusculasAscii(r.normalizado));

    r.regrasAplicadas = {
        "Espacos duplicados removidos (\"  \" -> \" \")",
        "Espacos nas extremidades removidos",
        "Capitalizacao original preservada (Qual != qual na exibicao)",
        "Pontuacao original mantida no texto exibido",
        "Chave de busca interna gerada em minusculas e sem pontuacao "
        "(usada apenas para localizar respostas cadastradas)",
    };
    return r;
}

}  // namespace tf
