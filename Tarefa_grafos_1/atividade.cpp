#include <iostream>
using namespace std;

void ler_matriz(int elementos, int **mat) {
    for (int i = 0; i < elementos; i++) {
        for (int j = 0; j < elementos; j++) {
            cin >> mat[i][j];
        }
    }
}

void completa_matriz(int elementos, bool dirigido, int **mat) {
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
        if(!dirigido){
            mat[n - 1][m - 1] = 1;
            mat[m - 1][n - 1] = 1;
        }else{
            mat[n - 1][m - 1] = 1;
        }
    }
    }
    return;
}
// Função para realizar a busca em profundidade (DFS)
void depth_first_search(int elementos, int **mat, int vertice_inicial){

}
// Função para realizar a busca em largura (BFS)
void breadth_first_search(int elementos, int **mat, int vertice_inicial){
    
}

int main() {
    int elementos;
    cout<< "Digite o numero de elemento do grafo: " << endl;
    cin >> elementos;

    int **mat = new int*[elementos];
    for (int i = 0; i < elementos; i++) {
        mat[i] = new int[elementos];
    }

    completa_matriz(elementos, true, mat);
    ler_matriz(elementos, mat);
    return 0;
}