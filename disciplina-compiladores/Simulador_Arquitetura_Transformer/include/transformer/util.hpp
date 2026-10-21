#pragma once

#include <string>
#include <vector>

// Utilitarios matematicos e de formatacao usados por todas as etapas do
// simulador. Todas as funcoes aqui sao deterministicas: a mesma entrada
// sempre produz a mesma saida, o que garante reprodutibilidade da simulacao.
namespace tf {

using Vetor = std::vector<double>;
using Matriz = std::vector<Vetor>;

// Hash simples (djb2) usada para gerar IDs e embeddings deterministicos.
unsigned long hashString(const std::string& s);

double produtoInterno(const Vetor& a, const Vetor& b);
Vetor somar(const Vetor& a, const Vetor& b);
Vetor escalar(const Vetor& a, double k);

// Trata x como vetor-linha: resultado[j] = soma_i x[i] * w[i][j]
Vetor multiplicarVetorMatriz(const Vetor& x, const Matriz& w);

// softmax(logits): normaliza os valores para que a soma seja ~1.0
std::vector<double> softmax(const std::vector<double>& logits);

// Gera uma matriz quadrada (dim x dim) de forma deterministica a partir de
// uma "semente" (numero inteiro). Serve como substituto didatico para
// matrizes de pesos que, em uma LLM real, seriam aprendidas por treinamento.
Matriz matrizDeterministica(int dim, double base, double variacao, int semente);

// Gera um vetor deterministico (bias) a partir de uma semente.
Vetor vetorDeterministico(int dim, int semente);

// Normalizacao simplificada (nao e o LayerNorm completo de um Transformer
// real): centraliza pela media e limita a amplitude, apenas para manter os
// vetores em uma faixa numerica estavel entre camadas.
Vetor normalizarSimplificado(const Vetor& x);

Vetor relu(const Vetor& x);

std::string formatarVetor(const Vetor& v, int precisao = 3);
std::string formatarPercentual(double p);

// Remove espacos nas pontas e colapsa espacos internos multiplos em um so.
std::string colapsarEspacos(const std::string& entrada);

// Converte para minusculas apenas os bytes ASCII 'A'-'Z' (preserva bytes
// UTF-8 de acentuacao, que tem o bit mais significativo ligado).
std::string paraMinusculasAscii(const std::string& entrada);

// Remove pontuacao ASCII comum (usado para montar a chave de busca de
// respostas cadastradas, nao para exibicao).
std::string removerPontuacao(const std::string& entrada);

}  // namespace tf
