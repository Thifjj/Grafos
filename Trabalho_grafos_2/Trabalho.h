#ifndef TRABALHO_H
#define TRABALHO_H

#include <limits>
#include <iostream>
using namespace std;

// ==========================================
// CLASSE FILA (Para o BFS)
// ==========================================
class fifo
{
private:
    int *vetor_fifo; // Renomeado para não conflitar com o nome da classe
    int inicio, fim, tam;

public:
    void inicializar_fifo(int tam_vetor)
    {
        tam = tam_vetor;
        vetor_fifo = new int[tam];
        for (int i = 0; i < tam; i++)
        {
            vetor_fifo[i] = 0;
        }
        inicio = 0;
        fim = 0;
    }

    void queue(int valor)
    {
        if (fim >= tam)
        {
            cout << "Fila cheia!" << endl;
            return;
        }
        vetor_fifo[fim] = valor;
        fim++;
    }

    void unqueue()
    {
        if (inicio == fim)
        {
            cout << "Nao existe nada na fila!" << endl;
            return;
        }
        // Otimizacao: Apenas avanca o inicio ao inves de mover todo o array
        inicio++;
    }

    void mostrar()
    {
        for (int i = inicio; i < fim; i++)
        {
            cout << vetor_fifo[i] << " ";
        }
        cout << endl;
    }

    void deletarfila()
    {
        delete[] vetor_fifo;
        vetor_fifo = nullptr;
    }

    bool empty()
    {
        return inicio == fim;
    }

    int get_inicio()
    {
        return vetor_fifo[inicio];
    }
};

// ==========================================
// CLASSE PILHA (Para o DFS)
// ==========================================
class pilha
{
private:
    int *vetor_pilha;
    int topo, tam;

public:
    void inicializar_pilha(int tam_vetor)
    {
        tam = tam_vetor;
        vetor_pilha = new int[tam];
        topo = 0;
    }

    void push(int valor)
    {
        if (topo >= tam)
        {
            cout << "Pilha cheia!" << endl;
            return;
        }
        vetor_pilha[topo] = valor;
        topo++;
    }

    void pop()
    {
        if (topo == 0)
        {
            cout << "Nao existe nada na pilha!" << endl;
            return;
        }
        topo--;
    }

    bool empty()
    {
        return topo == 0;
    }

    int top()
    {
        if (topo == 0)
            return -1;
        return vetor_pilha[topo - 1];
    }

    void deletarpilha()
    {
        delete[] vetor_pilha;
        vetor_pilha = nullptr;
    }
};

class grafo
{
private:
    int nos;
    bool conexo;
    bool dirigido;
    int **matriz_adjacencia;
public:
//inicializa o grafo
void inicializa_grafo(int nos = 0 ,bool dirigido = false){
    //inicializando matriz
    this->matriz_adjacencia = new int*[nos];
    for (int i = 0; i < nos; i++) {
        this->matriz_adjacencia[i] = new int[nos];
    }
    //zerando matriz
    for (int i = 0; i < nos; i++)
    {
        for (int j = 0; j < nos; j++)
        {
            this->matriz_adjacencia[i][j] = 0; // Zera a matriz
        }
    };
    //setando quantidade de nos e se dirigido
    this->nos = nos;
    this->dirigido = dirigido;
}
bool vertice_valido(int v)
{
    return v >= 1 && v <= this->nos;
}
void cria_vertice() {
    int novo_tamanho = this->nos + 1;

    // Cria nova matriz maior
    int **nova_matriz = new int*[novo_tamanho];

    for (int i = 0; i < novo_tamanho; i++) {
        nova_matriz[i] = new int[novo_tamanho];
    }

    // Zera a nova matriz
    for (int i = 0; i < novo_tamanho; i++) {
        for (int j = 0; j < novo_tamanho; j++) {
            nova_matriz[i][j] = 0;
        }
    }

    // Copia a matriz antiga
    for (int i = 0; i < this->nos; i++) {
        for (int j = 0; j < this->nos; j++) {
            nova_matriz[i][j] = this->matriz_adjacencia[i][j];
        }
    }

    // Apaga matriz antiga
    for (int i = 0; i < this->nos; i++) {
        delete[] this->matriz_adjacencia[i];
    }

    delete[] this->matriz_adjacencia;

    // Atualiza o ponteiro
    this->matriz_adjacencia = nova_matriz;

    // Atualiza quantidade de nós
    this->nos = novo_tamanho;
}
//cria conexao (aresta/arco)
void cria_conexao(int a, int b){
    if(vertice_valido(a) && vertice_valido(b)){
        if(this->dirigido){
            this->matriz_adjacencia[a - 1][b - 1] = 1;
        }else{
            this->matriz_adjacencia[a - 1][b - 1] = 1;
            this->matriz_adjacencia[b - 1][a - 1] = 1;
        };
    }
}
//remove conexao (aresta/arco)
void remove_conexao(int a, int b){
    if(vertice_valido(a) && vertice_valido(b)){
        if(this->dirigido){
            this->matriz_adjacencia[a - 1][b - 1] = 0;
        }else{
            this->matriz_adjacencia[a - 1][b - 1] = 0;
            this->matriz_adjacencia[b - 1][a - 1] = 0;
        };
    }
}
// ==========================================
// BUSCA EM LARGURA (BFS)
// ==========================================
void bfs(int vertice_inicial)
{
    if (!vertice_valido(vertice_inicial))
    {
        cout << "Vertice inicial invalido para BFS!" << endl;
        return;
    }

    bool *visitados = new bool[this->nos];
    fifo fila;

    fila.inicializar_fifo(this->nos);

    // Inicializa todos como não visitados
    for (int i = 0; i < this->nos; i++)
    {
        visitados[i] = false;
    }

    // Como o usuario usa vertices 1, 2, 3...
    // convertemos para indices 0, 1, 2...
    int origem = vertice_inicial - 1;

    fila.queue(origem);
    visitados[origem] = true;

    cout << "Percurso BFS: ";

    while (!fila.empty())
    {
        int atual = fila.get_inicio();
        fila.unqueue();

        cout << atual + 1 << " ";

        // Procura vizinhos do vertice atual
        for (int i = 0; i < this->nos; i++)
        {
            if (this->matriz_adjacencia[atual][i] == 1 &&
                !visitados[i])
            {
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
void dfs(int vertice_inicial)
{
    if (!vertice_valido(vertice_inicial))
    {
        cout << "Vertice inicial invalido para DFS!" << endl;
        return;
    }

    bool *visitados = new bool[this->nos];

    for (int i = 0; i < this->nos; i++)
    {
        visitados[i] = false;
    }

    pilha p;

    p.inicializar_pilha(this->nos * this->nos);

    int origem = vertice_inicial - 1;

    p.push(origem);

    cout << "Percurso DFS: ";

    while (!p.empty())
    {
        int atual = p.top();
        p.pop();

        if (!visitados[atual])
        {
            visitados[atual] = true;

            cout << atual + 1 << " ";

            // Vai de tras para frente porque a pilha é LIFO.
            // Assim os vertices menores tendem a sair primeiro.
            for (int i = this->nos - 1; i >= 0; i--)
            {
                if (this->matriz_adjacencia[atual][i] == 1 &&
                    !visitados[i])
                {
                    p.push(i);
                }
            }
        }
    }

    cout << endl;

    delete[] visitados;
    p.deletarpilha();
}

void calcula_fecho(int origem, bool inverso, int *nivel)
{
    for (int i = 0; i < this->nos; i++)
    {
        nivel[i] = -1;
    }

    fifo fila;
    fila.inicializar_fifo(this->nos);

    nivel[origem] = 0;
    fila.queue(origem);

    while (!fila.empty())
    {
        int atual = fila.get_inicio();
        fila.unqueue();

        for (int i = 0; i < this->nos; i++)
        {
            bool possui_conexao;

            if (!inverso)
            {
                // Fecho direto
                // atual -> i
                possui_conexao =
                    this->matriz_adjacencia[atual][i] == 1;
            }
            else
            {
                // Fecho inverso
                // i -> atual
                possui_conexao =
                    this->matriz_adjacencia[i][atual] == 1;
            }

            if (possui_conexao && nivel[i] == -1)
            {
                nivel[i] = nivel[atual] + 1;

                fila.queue(i);
            }
        }
    }

    fila.deletarfila();
}

void fecho_transitivo_direto(int vertice_inicial)
{
    if (vertice_inicial < 1 || vertice_inicial > this->nos)
    {
        cout << "Vertice invalido!" << endl;
        return;
    }

    int *nivel = new int[this->nos];

    calcula_fecho(
        vertice_inicial - 1,
        false,
        nivel
    );

    cout << "\nFecho Transitivo Direto de "
         << vertice_inicial << endl;

    cout << "Vertice\tNivel" << endl;

    for (int i = 0; i < this->nos; i++)
    {
        cout << i + 1 << "\t";

        if (nivel[i] == -1)
            cout << "-";
        else
            cout << nivel[i];

        cout << endl;
    }

    delete[] nivel;
}

void fecho_transitivo_inverso(int vertice_inicial)
{
    if (vertice_inicial < 1 || vertice_inicial > this->nos)
    {
        cout << "Vertice invalido!" << endl;
        return;
    }

    int *nivel = new int[this->nos];

    calcula_fecho(
        vertice_inicial - 1,
        true,
        nivel
    );

    cout << "\nFecho Transitivo Inverso de "
         << vertice_inicial << endl;

    cout << "Vertice\tNivel" << endl;

    for (int i = 0; i < this->nos; i++)
    {
        cout << i + 1 << "\t";

        if (nivel[i] == -1)
            cout << "-";
        else
            cout << nivel[i];

        cout << endl;
    }

    delete[] nivel;
}

void componentes_fortemente_conexos()
{
    if (!this->dirigido)
    {
        cout << "O grafo nao e dirigido." << endl;
        return;
    }

    int *componente = new int[this->nos];

    for (int i = 0; i < this->nos; i++)
    {
        componente[i] = -1;
    }

    int quantidade_componentes = 0;

    for (int v = 0; v < this->nos; v++)
    {
        // Vertice ja pertence a algum componente
        if (componente[v] != -1)
            continue;

        int *direto = new int[this->nos];
        int *inverso = new int[this->nos];

        calcula_fecho(v, false, direto);
        calcula_fecho(v, true, inverso);

        /*
            Se o vertice aparece nos dois fechamentos:

            v consegue chegar em i
                    E
            i consegue chegar em v

            Portanto pertencem ao mesmo
            componente fortemente conexo.
        */

        for (int i = 0; i < this->nos; i++)
        {
            if (direto[i] != -1 &&
                inverso[i] != -1)
            {
                componente[i] = quantidade_componentes;
            }
        }

        quantidade_componentes++;

        delete[] direto;
        delete[] inverso;
    }

    cout << "\n==============================" << endl;
    cout << "COMPONENTES FORTEMENTE CONEXOS" << endl;
    cout << "==============================" << endl;

    for (int c = 0; c < quantidade_componentes; c++)
    {
        cout << "SFCM " << c + 1 << ": { ";

        for (int i = 0; i < this->nos; i++)
        {
            if (componente[i] == c)
            {
                cout << i + 1 << " ";
            }
        }

        cout << "}" << endl;
    }

    cout << endl;

    if (quantidade_componentes == 1)
    {
        cout << "O grafo e FORTEMENTE CONEXO." << endl;
        this->conexo = true;
    }
    else
    {
        cout << "O grafo NAO e fortemente conexo." << endl;
        cout << "Quantidade de SFCM: "
             << quantidade_componentes << endl;

        this->conexo = false;
    }

    delete[] componente;
}

void mostrar_grafo()
{
    cout << "\nMatriz de adjacencia:\n\n";

    cout << "   ";
    for (int i = 0; i < nos; i++)
        cout << i + 1 << " ";

    cout << endl;

    for (int i = 0; i < nos; i++)
    {
        cout << i + 1 << "  ";

        for (int j = 0; j < nos; j++)
        {
            cout << matriz_adjacencia[i][j] << " ";
        }

        cout << endl;
    }
}

};

void system_pause()
{
    cout << "Pressione enter para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
};
#endif