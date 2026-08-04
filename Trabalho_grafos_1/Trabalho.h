#ifndef TRABALHO_H
#define TRABALHO_H

#include <iostream>
#include <stack>
#include <queue>
#include <vector>
using namespace std;

inline void ler_matriz_adjacencia(int elementos, int **mat) {
    for (int i = 0; i < elementos; i++) {
        for (int j = 0; j < elementos; j++) {
            cin >> mat[i][j];
        }
    }
}

inline void completa_matriz(int elementos, bool dirigido, int **mat) {
    int n, m, relacoes;
    char separador;

    for (int i = 0; i < elementos; i++) {
        for (int j = 0; j < elementos; j++) {
            if (i == j) {
                mat[i][j] = 0;
            }
        }
    }
    cout << "Digite o numero de relacoes: " << endl;
    cin >> relacoes;
    for (int i = 0; i < relacoes; i++) {
        cout << "Digite uma relacao dos vertices ex: (1,2)" << endl;
        cin >> n >> separador >> m;
        if (separador == ',') {
            if (!dirigido) {
                mat[n - 1][m - 1] = 1;
                mat[m - 1][n - 1] = 1;
            } else {
                mat[n - 1][m - 1] = 1;
            }
        }
    }
}

// Função para realizar a busca em profundidade (DFS)
// O DFS usa uma pilha (stack), porque ele segue o caminho mais fundo
// possível e volta somente quando não há mais vizinhos não visitados.
inline void depth_first_search(int elementos, int **mat, int vertice_inicial) {
    // vetor de visitados: false = ainda não foi explorado
    vector<bool> visitado(elementos, false);

    // Pilha para armazenar os vértices a serem explorados.
    stack<int> pilha;

    // Ajusta o vértice inicial para o índice da matriz (0-based).
    int origem = vertice_inicial - 1;

    // Empilha o primeiro vértice.
    pilha.push(origem);

    cout << "Percurso DFS iniciando no vertice " << vertice_inicial << ": ";

    while (!pilha.empty()) {
        // Pega o vértice do topo da pilha.
        int atual = pilha.top();
        pilha.pop();

        // Se o vértice ainda não foi visitado, ele deve ser explorado agora.
        if (!visitado[atual]) {
            visitado[atual] = true;
            cout << atual + 1 << " ";

            // Percorre todos os vizinhos do vértice atual.
            // Como a estrutura é uma pilha, os vizinhos são empilhados e
            // o último empilhado é processado primeiro (comportamento LIFO).
            for (int i = elementos - 1; i >= 0; i--) {
                if (mat[atual][i] == 1 && !visitado[i]) {
                    pilha.push(i);
                }
            }
        }
    }

    cout << endl;
}

// Função para realizar a busca em largura (BFS)
// O BFS usa uma fila (queue), porque ele explora primeiro os vizinhos
// do vértice atual, nível por nível, em ordem de amplitude.
inline void breadth_first_search(int elementos, int **mat, int vertice_inicial) {
    // vetor de visitados: false = ainda não foi explorado
    vector<bool> visitado(elementos, false);

    // Fila para armazenar os vértices em ordem de descoberta.
    queue<int> fila;

    // Ajusta o vértice inicial para o índice da matriz (0-based).
    int origem = vertice_inicial - 1;

    // Enfileira a origem.
    fila.push(origem);
    visitado[origem] = true;

    cout << "Percurso BFS iniciando no vertice " << vertice_inicial << ": ";

    while (!fila.empty()) {
        // Remove o primeiro vértice da fila.
        int atual = fila.front();
        fila.pop();

        // Exibe o vértice atual na ordem em que foi descoberto.
        cout << atual + 1 << " ";

        // Explora todos os vizinhos do vértice atual.
        // Se um vizinho ainda não foi visitado, ele entra na fila.
        for (int i = 0; i < elementos; i++) {
            if (mat[atual][i] == 1 && !visitado[i]) {
                visitado[i] = true;
                fila.push(i);
            }
        }
    }

    cout << endl;
}

#endif