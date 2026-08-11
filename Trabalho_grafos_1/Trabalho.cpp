#include <iostream>
#include "Trabalho.h"

using namespace std;

int main()
{
    int nos;
    int vertice_inicial;
    char dirigido;

    cout << "===== CRIACAO DO GRAFO =====" << endl;
    cout << "Digite o numero de vertices: ";
    cin >> nos;

    // Aloca a matriz de adjacencia
    int **mat = new int*[nos];
    for (int i = 0; i < nos; i++) {
        mat[i] = new int[nos];
    }

    // Pergunta se o grafo e dirigido
    cout << "O grafo e dirigido? (s/n): ";
    cin >> dirigido;

    bool eh_dirigido = (dirigido == 's' || dirigido == 'S');

    // Preenche a matriz invocando a funcao
    completa_matriz(nos, eh_dirigido, mat);

    // Mostra a Matriz de Adjacencia
    cout << "\n===== MATRIZ DE ADJACENCIA =====" << endl;
    for (int i = 0; i < nos; i++) {
        for (int j = 0; j < nos; j++) {
            cout << mat[i][j] << " ";
        }
        cout << endl;
    }

    // Escolhe vertice inicial
    cout << "\nDigite o vertice inicial para os percursos: ";
    cin >> vertice_inicial;

    cout << "\n===== RESULTADOS DOS PERCURSOS =====" << endl;
    
    // Executa BFS
    bfs(nos, mat, vertice_inicial);

    // Executa DFS
    dfs(nos, mat, vertice_inicial);

    // Libera memoria da matriz no final do programa
    for (int i = 0; i < nos; i++) {
        delete[] mat[i];
    }
    delete[] mat;

    return 0;
}