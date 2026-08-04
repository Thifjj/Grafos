#ifndef TRABALHO_H
#define TRABALHO_H

#include <iostream>
using namespace std;

inline void ler_matriz(int elementos, int **mat) {
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
inline void depth_first_search(int elementos, int **mat, int vertice_inicial) {
}

// Função para realizar a busca em largura (BFS)
inline void breadth_first_search(int elementos, int **mat, int vertice_inicial) {
}

#endif