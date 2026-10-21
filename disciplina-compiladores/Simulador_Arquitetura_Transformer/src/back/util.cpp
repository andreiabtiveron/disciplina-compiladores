#include "transformer/util.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace tf {

unsigned long hashString(const std::string& s) {
    unsigned long h = 5381;
    for (unsigned char c : s) {
        h = ((h << 5) + h) + c;  // h * 33 + c
    }
    return h;
}

double produtoInterno(const Vetor& a, const Vetor& b) {
    double soma = 0.0;
    std::size_t n = std::min(a.size(), b.size());
    for (std::size_t i = 0; i < n; ++i) soma += a[i] * b[i];
    return soma;
}

Vetor somar(const Vetor& a, const Vetor& b) {
    Vetor r(a.size(), 0.0);
    for (std::size_t i = 0; i < a.size(); ++i) r[i] = a[i] + (i < b.size() ? b[i] : 0.0);
    return r;
}

Vetor escalar(const Vetor& a, double k) {
    Vetor r = a;
    for (double& v : r) v *= k;
    return r;
}

Vetor multiplicarVetorMatriz(const Vetor& x, const Matriz& w) {
    if (w.empty()) return {};
    std::size_t dimSaida = w[0].size();
    Vetor r(dimSaida, 0.0);
    for (std::size_t i = 0; i < x.size() && i < w.size(); ++i) {
        for (std::size_t j = 0; j < dimSaida; ++j) {
            r[j] += x[i] * w[i][j];
        }
    }
    return r;
}

std::vector<double> softmax(const std::vector<double>& logits) {
    if (logits.empty()) return {};
    double maior = *std::max_element(logits.begin(), logits.end());
    std::vector<double> exps(logits.size());
    double soma = 0.0;
    for (std::size_t i = 0; i < logits.size(); ++i) {
        exps[i] = std::exp(logits[i] - maior);
        soma += exps[i];
    }
    for (double& e : exps) e /= soma;
    return exps;
}

Matriz matrizDeterministica(int dim, double base, double variacao, int semente) {
    Matriz m(dim, Vetor(dim, 0.0));
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            double diag = (i == j) ? base : 0.0;
            double onda = std::sin((i + 1) * (j + 1) * 0.5 + semente * 0.9);
            m[i][j] = diag + variacao * onda;
        }
    }
    return m;
}

Vetor vetorDeterministico(int dim, int semente) {
    Vetor v(dim, 0.0);
    for (int i = 0; i < dim; ++i) {
        v[i] = 0.1 * std::sin(semente * 0.7 + i * 1.3);
    }
    return v;
}

Vetor normalizarSimplificado(const Vetor& x) {
    if (x.empty()) return x;
    double media = std::accumulate(x.begin(), x.end(), 0.0) / x.size();
    double maiorAbs = 1e-6;
    Vetor centralizado(x.size());
    for (std::size_t i = 0; i < x.size(); ++i) {
        centralizado[i] = x[i] - media;
        maiorAbs = std::max(maiorAbs, std::fabs(centralizado[i]));
    }
    for (double& v : centralizado) v /= maiorAbs;
    return centralizado;
}

Vetor relu(const Vetor& x) {
    Vetor r = x;
    for (double& v : r) v = std::max(0.0, v);
    return r;
}

std::string formatarVetor(const Vetor& v, int precisao) {
    std::ostringstream oss;
    oss << "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        oss << std::fixed << std::setprecision(precisao) << v[i];
        if (i + 1 < v.size()) oss << ", ";
    }
    oss << "]";
    return oss.str();
}

std::string formatarPercentual(double p) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << (p * 100.0) << "%";
    return oss.str();
}

std::string colapsarEspacos(const std::string& entrada) {
    std::string saida;
    saida.reserve(entrada.size());
    bool ultimoFoiEspaco = true;  // para descartar espacos iniciais
    for (char c : entrada) {
        bool ehEspaco = (c == ' ' || c == '\t' || c == '\n' || c == '\r');
        if (ehEspaco) {
            if (!ultimoFoiEspaco) saida.push_back(' ');
            ultimoFoiEspaco = true;
        } else {
            saida.push_back(c);
            ultimoFoiEspaco = false;
        }
    }
    while (!saida.empty() && saida.back() == ' ') saida.pop_back();
    return saida;
}

std::string paraMinusculasAscii(const std::string& entrada) {
    std::string saida = entrada;
    for (char& c : saida) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return saida;
}

std::string removerPontuacao(const std::string& entrada) {
    static const std::string pontuacao = "?!.,;:\"'()[]{}#@";
    std::string saida;
    saida.reserve(entrada.size());
    for (char c : entrada) {
        if (pontuacao.find(c) == std::string::npos) saida.push_back(c);
    }
    return colapsarEspacos(saida);
}

}  // namespace tf
