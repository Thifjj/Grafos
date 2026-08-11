#ifndef TRABALHO_H
#define TRABALHO_H

#include <iostream>
using namespace std;

// ==========================================
// CLASSE FILA (Para o BFS)
// ==========================================
class fifo {
private:
    int *vetor_fifo; // Renomeado para não conflitar com o nome da classe
    int inicio, fim, tam;

public:
    void inicializar_fifo(int tam_vetor) {
        tam = tam_vetor;
        vetor_fifo = new int[tam];
        for (int i = 0; i < tam; i++) {
            vetor_fifo[i] = 0;
        }
        inicio = 0;
        fim = 0;
    }

    void queue(int valor) {
        if (fim >= tam) {
            cout << "Fila cheia!" << endl;
            return;
        }
        vetor_fifo[fim] = valor;
        fim++;
    }

    void unqueue() {
        if (inicio == fim) {
            cout << "Nao existe nada na fila!" << endl;
            return;
        }
        // Otimizacao: Apenas avanca o inicio ao inves de mover todo o array
        inicio++; 
    }

    void mostrar() {
        for (int i = inicio; i < fim; i++) {
            cout << vetor_fifo[i] << " ";
        }
        cout << endl;
    }

    void deletarfila() {
        delete[] vetor_fifo;
        vetor_fifo = nullptr;
    }

    bool empty() {
        return inicio == fim;
    }

    int get_inicio() {
        return vetor_fifo[inicio];
    }
};

// ==========================================
// CLASSE PILHA (Para o DFS)
// ==========================================
class pilha {
private:
    int *vetor_pilha;
    int topo, tam;

public:
    void inicializar_pilha(int tam_vetor) {
        tam = tam_vetor;
        vetor_pilha = new int[tam];
        topo = 0;
    }

    void push(int valor) {
        if (topo >= tam) {
            cout << "Pilha cheia!" << endl;
            return;
        }
        vetor_pilha[topo] = valor;
        topo++;
    }

    void pop() {
        if (topo == 0) {
            cout << "Nao existe nada na pilha!" << endl;
            return;
        }
        topo--;
    }

    bool empty() {
        return topo == 0;
    }

    int top() {
        if (topo == 0) return -1;
        return vetor_pilha[topo - 1];
    }

    void deletarpilha() {
        delete[] vetor_pilha;
        vetor_pilha = nullptr;
    }
};

// ==========================================
// FUNCOES DO GRAFO
// ==========================================
inline void completa_matriz(int nos, bool dirigido, int **mat) {
    int n, m, relacoes;

    for (int i = 0; i < nos; i++) {
        for (int j = 0; j < nos; j++) {
            mat[i][j] = 0; // Zera a matriz, incluindo a diagonal principal
        }
    }

    cout << "Digite o numero de relacoes (arestas): ";
    cin >> relacoes;

    for (int i = 0; i < relacoes; i++) {
        cout << "Digite a relacao " << i + 1 << " (dois numeros separados por espaco, ex: 1 2): ";
        cin >> n >> m;

        if (n < 1 || n > nos || m < 1 || m > nos) {
            cout << "Vertice invalido! Os vertices devem estar entre 1 e " << nos << "." << endl;
            i--; // Volta um laco para o usuario tentar novamente
            continue;
        }

        if (!dirigido) {
            mat[n - 1][m - 1] = 1;
            mat[m - 1][n - 1] = 1;
        } else {
            mat[n - 1][m - 1] = 1;
        }
    }
}

// ==========================================
// BUSCA EM LARGURA (BFS)
// ==========================================
inline void bfs(int nos, int **mat, int vertice_inicial) {
    if (vertice_inicial < 1 || vertice_inicial > nos) {
        cout << "Vertice inicial invalido para BFS!" << endl;
        return;
    }

    bool *visitados = new bool[nos];
    fifo fila;
    
    // Tamanho da fila igual ao numero de nós é o suficiente, 
    // pois na busca em largura nenhum nó entra na fila duas vezes.
    fila.inicializar_fifo(nos);

    for (int i = 0; i < nos; i++) {
        visitados[i] = false;
    }

    int origem = vertice_inicial - 1;
    fila.queue(origem);
    visitados[origem] = true;

    cout << "Percurso BFS: ";

    while (!fila.empty()) {
        int atual = fila.get_inicio();
        fila.unqueue();

        cout << atual + 1 << " ";

        for (int i = 0; i < nos; i++) {
            if (mat[atual][i] == 1 && !visitados[i]) {
                visitados[i] = true;
                fila.queue(i);
            }
        }
    }

    cout << endl;
    delete[] visitados;
    fila.deletarfila();
}

// ==========================================
// BUSCA EM PROFUNDIDADE (DFS)
// ==========================================
inline void dfs(int nos, int **mat, int vertice_inicial) {
    if (vertice_inicial < 1 || vertice_inicial > nos) {
        cout << "Vertice inicial invalido para DFS!" << endl;
        return;
    }

    bool *visitados = new bool[nos];
    for (int i = 0; i < nos; i++) {
        visitados[i] = false;
    }

    pilha p;
    // O tamanho da pilha precisa ser maior (nos * nos) pois o mesmo nó pode
    // ser empilhado mais de uma vez antes de ser marcado como visitado.
    p.inicializar_pilha(nos * nos);

    int origem = vertice_inicial - 1;
    p.push(origem);

    cout << "Percurso DFS: ";

    while (!p.empty()) {
        int atual = p.top();
        p.pop();

        if (!visitados[atual]) {
            visitados[atual] = true;
            cout << atual + 1 << " ";

            // Percorre vizinhos de tras pra frente para manter a ordem crescente (LIFO)
            for (int i = nos - 1; i >= 0; i--) {
                if (mat[atual][i] == 1 && !visitados[i]) {
                    p.push(i);
                }
            }
        }
    }

    cout << endl;
    delete[] visitados;
    p.deletarpilha();
}

#endif