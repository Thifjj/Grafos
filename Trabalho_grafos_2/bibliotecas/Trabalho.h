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
    int nos = 0; // ALTERADO: permite sair sem inicializar o grafo.
    int arestas;
    int areas;
    bool conexo;
    bool dirigido;
    int **matriz_adjacencia = nullptr; // ALTERADO: destrutor seguro antes da criacao.

public:
    // ADICIONADO: antes a matriz nao era liberada ao destruir o grafo.
    // ADICIONADO: consulta somente leitura para desenhar o grafo na interface.
    int custo_conexao(int a, int b) const
    {
        return matriz_adjacencia[a - 1][b - 1];
    }

    // Agora libera cada linha e depois o vetor de ponteiros.
    ~grafo()
    {
        for (int i = 0; i < nos; i++)
            delete[] matriz_adjacencia[i];
        delete[] matriz_adjacencia;
    }

    // ADICIONADO: a copia superficial da matriz causaria liberacao duplicada
    // com o destrutor. O grafo passa a nao permitir copia.
    grafo() = default;
    grafo(const grafo &) = delete;
    grafo &operator=(const grafo &) = delete;

    // inicializa o grafo
    void inicializa_grafo(int nos = 0, bool dirigido = false)
    {
        // inicializando matriz
        this->matriz_adjacencia = new int *[nos];
        for (int i = 0; i < nos; i++)
        {
            this->matriz_adjacencia[i] = new int[nos];
        }
        // zerando matriz
        for (int i = 0; i < nos; i++)
        {
            for (int j = 0; j < nos; j++)
            {
                this->matriz_adjacencia[i][j] = 0; // Zera a matriz
            }
        };
        // setando quantidade de nos e se dirigido
        this->nos = nos;
        this->dirigido = dirigido;
        this->arestas = 0;
        this->areas = 0;
    }
    bool vertice_valido(int v)
    {
        return v >= 1 && v <= this->nos;
    }
    void cria_vertice()
    {
        int novo_tamanho = this->nos + 1;

        // Cria nova matriz maior
        int **nova_matriz = new int *[novo_tamanho];

        for (int i = 0; i < novo_tamanho; i++)
        {
            nova_matriz[i] = new int[novo_tamanho];
        }

        // Zera a nova matriz
        for (int i = 0; i < novo_tamanho; i++)
        {
            for (int j = 0; j < novo_tamanho; j++)
            {
                nova_matriz[i][j] = 0;
            }
        }

        // Copia a matriz antiga
        for (int i = 0; i < this->nos; i++)
        {
            for (int j = 0; j < this->nos; j++)
            {
                nova_matriz[i][j] = this->matriz_adjacencia[i][j];
            }
        }

        // Apaga matriz antiga
        for (int i = 0; i < this->nos; i++)
        {
            delete[] this->matriz_adjacencia[i];
        }

        delete[] this->matriz_adjacencia;

        // Atualiza o ponteiro
        this->matriz_adjacencia = nova_matriz;

        // Atualiza quantidade de nós
        this->nos = novo_tamanho;
    }
    void remove_vertice(int vertice)
    {
        if (!vertice_valido(vertice))
        {
            cout << "Vertice invalido!" << endl;
            return;
        }
        int indice_remocao = vertice - 1; // Ajusta para índice baseado em 0
        int novo_tamanho = this->nos - 1;

        // ALTERADO: antes removia o vertice sem descontar suas conexoes.
        // Agora desconta cada aresta incidente; em grafos dirigidos,
        // desconta tambem os arcos de entrada, sem contar lacos duas vezes.
        for (int i = 0; i < this->nos; i++)
        {
            if (this->matriz_adjacencia[indice_remocao][i] > 0)
                this->arestas--;
            if (this->dirigido && i != indice_remocao &&
                this->matriz_adjacencia[i][indice_remocao] > 0)
                this->arestas--;
        }

        // Cria nova matriz menor
        int **nova_matriz = new int *[novo_tamanho];

        for (int i = 0; i < novo_tamanho; i++)
        {
            nova_matriz[i] = new int[novo_tamanho];
        }

        // Zera a nova matriz
        for (int i = 0; i < novo_tamanho; i++)
        {
            for (int j = 0; j < novo_tamanho; j++)
            {
                nova_matriz[i][j] = 0;
            }
        }

        int nova_linha = 0;

        for (int i = 0; i < this->nos; i++)
        {
            if (i == indice_remocao)
                continue;

            int nova_coluna = 0;

            for (int j = 0; j < this->nos; j++)
            {
                if (j == indice_remocao)
                    continue;

                nova_matriz[nova_linha][nova_coluna] =
                    this->matriz_adjacencia[i][j];

                nova_coluna++;
            }

            nova_linha++;
        }

        for (int i = 0; i < this->nos; i++)
        {
            delete[] this->matriz_adjacencia[i];
        }

        delete[] this->matriz_adjacencia;

        this->matriz_adjacencia = nova_matriz;
        this->nos = novo_tamanho;
    }
    // cria conexao (aresta/arco)
    void cria_conexao(int a, int b, u_int8_t custo = 1)
    {
        if (vertice_valido(a) && vertice_valido(b))
        {
            if (this->dirigido)
            {
                this->matriz_adjacencia[a - 1][b - 1] = custo;
                this->arestas++;
            }
            else
            {
                this->matriz_adjacencia[a - 1][b - 1] = custo;
                this->matriz_adjacencia[b - 1][a - 1] = custo;
                this->arestas++;
            };
        }
    }
    // remove conexao (aresta/arco)
    void remove_conexao(int a, int b)
    {
        if (vertice_valido(a) && vertice_valido(b))
        {
            if (this->dirigido)
            {
                this->matriz_adjacencia[a - 1][b - 1] = 0;
                this->arestas--;
            }
            else
            {
                this->matriz_adjacencia[a - 1][b - 1] = 0;
                this->matriz_adjacencia[b - 1][a - 1] = 0;
                this->arestas--;
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

        // verifica a parte conexa com o vertice inicial
        while (!fila.empty())
        {
            int atual = fila.get_inicio();
            fila.unqueue();

            cout << atual + 1 << " ";

            // Procura vizinhos do vertice atual
            for (int i = 0; i < this->nos; i++)
            {
                if (this->matriz_adjacencia[atual][i] > 0 &&
                    !visitados[i])
                {
                    visitados[i] = true;
                    fila.queue(i);
                }
            }
        }
        // verifica a parte desconexa com o vertice inicial
        for (int i = 0; i < this->nos; i++)
        {
            if (!visitados[i])
            {
                fila.queue(i);
                visitados[i] = true;
                while (!fila.empty())
                {
                    int atual = fila.get_inicio();
                    fila.unqueue();
                    cout << atual + 1 << " ";
                    for (int j = 0; j < this->nos; j++)
                    {
                        if (this->matriz_adjacencia[atual][j] > 0 && !visitados[j])
                        {
                            visitados[j] = true;
                            fila.queue(j);
                        }
                    }
                }
            };
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
                    if (this->matriz_adjacencia[atual][i] > 0 &&
                        !visitados[i])
                    {
                        p.push(i);
                    }
                }
            }
        }
        for (int i = 0; i < this->nos; i++)
        {
            if (!visitados[i])
            {
                p.push(i);
                while (!p.empty())
                {
                    int atual = p.top();
                    p.pop();
                    if (!visitados[atual])
                    {
                        visitados[atual] = true;
                        cout << atual + 1 << " ";
                        for (int j = 0; j < this->nos; j++)
                        {
                            if (this->matriz_adjacencia[atual][j] > 0 && !visitados[j])
                            {
                                // ALTERADO: antes marcava visitados[j] aqui e o
                                // vertice era ignorado ao sair da pilha. Agora
                                // a marcacao ocorre ao processar o vertice.
                                p.push(j);
                            }
                        }
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
                    possui_conexao = this->matriz_adjacencia[atual][i] > 0;
                }
                else
                {
                    // Fecho inverso
                    // i -> atual
                    possui_conexao = this->matriz_adjacencia[i][atual] > 0;
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

        calcula_fecho(vertice_inicial - 1, false, nivel);

        cout << "\nFecho Transitivo Direto de " << vertice_inicial << endl;

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

        calcula_fecho(vertice_inicial - 1, true, nivel);

        cout << "\nFecho Transitivo Inverso de " << vertice_inicial << endl;

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

        cout << "COMPONENTES FORTEMENTE CONEXOS" << endl;

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

    void componentes_conexos()
    {
        if (this->dirigido)
        {
            cout << "O grafo e dirigido." << endl;
            cout << "Utilize componentes fortemente conexos." << endl;
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
            if (componente[v] != -1)
                continue;

            int *direto = new int[this->nos];

            calcula_fecho(v, false, direto);

            for (int i = 0; i < this->nos; i++)
            {
                if (direto[i] != -1)
                {
                    componente[i] = quantidade_componentes;
                }
            }

            quantidade_componentes++;

            delete[] direto;
        }

        cout << "COMPONENTES CONEXOS" << endl;

        for (int c = 0; c < quantidade_componentes; c++)
        {
            cout << "Componente " << c + 1 << ": { ";

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
            cout << "O grafo e CONEXO." << endl;
            this->conexo = true;
        }
        else
        {
            cout << "O grafo NAO e conexo." << endl;
            cout << "Quantidade de componentes conexos: " << quantidade_componentes << endl;

            this->conexo = false;
        }

        delete[] componente;
    }

    // verificar ciclo 3 existente grafo
    bool is_ciclo_3()
    {
        if (!this->dirigido)
        {
            for (int v = 0; v < this->nos; v++)
            {
                for (int i = 0; i < this->nos; i++)
                {
                    if (this->matriz_adjacencia[v][i] > 0)
                    {
                        for (int j = 0; j < this->nos; j++)
                        {
                            if (this->matriz_adjacencia[i][j] > 0 && this->matriz_adjacencia[j][v] > 0)
                            {
                                return true;
                            }
                        }
                    }
                }
            }
            return false;
        }
        else
        {
            return false;
        }
    }

    // verificacao se e planar com euler
    bool planar_euler()
    {
        if (is_ciclo_3())
        {
            if (this->arestas <= (3 * this->nos) - 6)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (this->arestas <= (2 * this->nos) - 4)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    // calcula areas do grafo
    void areas_grafo()
    {
        if (planar_euler())
        {
            this->areas = (this->arestas + 2 - this->nos);
            // ADICIONADO: antes o resultado nao aparecia no menu.
            cout << "Areas pela formula para grafo planar conexo: " << this->areas << endl;
            cout << "O resultado pressupoe que o grafo seja planar e conexo." << endl;
        }
        else
            cout << "O grafo nao passou pelos limites de Euler implementados." << endl;
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
    // colorir grafo

    void coloracao_grafo()
    {
        // ADICIONADO: coloracao disponivel somente para grafos nao dirigidos.
        if (this->dirigido)
        {
            cout << "Coloracao disponivel somente para grafos nao dirigidos." << endl;
            return;
        }
        // ADICIONADO: antes, grafo vazio acessava cor[-1].
        if (this->nos == 0)
        {
            cout << "O grafo nao possui vertices!" << endl;
            return;
        }
        // ADICIONADO: um laco impede coloracao propria do vertice.
        for (int i = 0; i < this->nos; i++)
        {
            if (this->matriz_adjacencia[i][i] > 0)
            {
                cout << "Nao e possivel colorir um grafo com lacos." << endl;
                return;
            }
        }
        int grau[this->nos];
        int saturacao[this->nos];
        int cor[this->nos];
        bool is_colorido[this->nos];
        int indice_maior_grau = -1;
        int maior_grau = -1;
        int v_cor = 1;

        for (int i = 0; i < this->nos; i++)
        {
            grau[i] = 0;
            cor[i] = 0;
            is_colorido[i] = false;
            saturacao[i] = 0;
        }

        // pegar grau de cada vertice
        for (int i = 0; i < this->nos; i++)
        {
            for (int j = 0; j < this->nos; j++)
            {
                if (this->matriz_adjacencia[i][j] > 0)
                {
                    grau[i]++;
                }
            }
        }

        // colorir todos os vertices
        int quantidade_coloridos = 0;
        // colorir primeiro vertice
        for (int i = 0; i < this->nos; i++)
        {
            if (grau[i] > maior_grau)
            {
                indice_maior_grau = i;
                maior_grau = grau[i];
            }
        }

        cor[indice_maior_grau] = v_cor;
        is_colorido[indice_maior_grau] = true;
        quantidade_coloridos++;

        // colorir resto dos vertices

        while (quantidade_coloridos < this->nos)
        {
            // calcular saturacao
            for (int i = 0; i < this->nos; i++)
            {
                if (!is_colorido[i])
                {
                    // vetor armazenar cores vizinhas
                    bool cores_vizinhas[this->nos + 1];
                    // zerar vetor cores_vizinhas com false
                    for (int k = 0; k <= this->nos; k++)
                    {
                        cores_vizinhas[k] = false;
                    }
                    // zerando saturacao antes da verificacao
                    saturacao[i] = 0;
                    // verificando adjacentes de [i]
                    for (int j = 0; j < this->nos; j++)
                    {
                        // verificacao de conexao e se tem cor
                        if (this->matriz_adjacencia[i][j] > 0 && is_colorido[j])
                        {

                            if (!cores_vizinhas[cor[j]])
                            {

                                cores_vizinhas[cor[j]] = true;
                                saturacao[i]++;
                            }
                        }
                    }
                }
            }

            // achar maior saturacao
            int maior_saturacao = -1;
            int indice_maior_saturacao = -1;
            int maior_grau_atual = -1;
            // verificar maior saturacao atual
            for (int i = 0; i < this->nos; i++)
            {
                if (!is_colorido[i])
                {
                    if (saturacao[i] > maior_saturacao)
                    {
                        maior_saturacao = saturacao[i];
                        indice_maior_saturacao = i;
                        maior_grau_atual = grau[i];
                    }
                    else if (saturacao[i] == maior_saturacao && grau[i] > maior_grau_atual)
                    {
                        maior_grau_atual = grau[i];
                        indice_maior_saturacao = i;
                    };
                }
            }

            // escolher cor para colorir
            bool cores_usadas[this->nos + 1];
            // ALTERADO: era i < nos; agora inicializa tambem a posicao nos.
            for (int i = 0; i <= this->nos; i++)
            {
                cores_usadas[i] = false;
            }

            for (int j = 0; j < this->nos; j++)
            {
                if (this->matriz_adjacencia[indice_maior_saturacao][j] > 0 && is_colorido[j])
                {
                    cores_usadas[cor[j]] = true;
                }
            }
            v_cor = 1;
            while (cores_usadas[v_cor])
            {
                v_cor++;
            }
            cor[indice_maior_saturacao] = v_cor;
            is_colorido[indice_maior_saturacao] = true;

            quantidade_coloridos++;
        }
        // ADICIONADO: antes as cores eram descartadas sem mostrar resultado.
        cout << "Coloracao do grafo:" << endl;
        for (int i = 0; i < this->nos; i++)
            cout << "Vertice " << i + 1 << ": cor " << cor[i] << endl;
    }

    void busca_djikstra(u_int8_t inicial)
    {
        if (inicial < 1 || inicial > (this->nos))
        {
            cout << "Valor inicial invalido" << endl;
            return;
        }

        u_int8_t estimativa_custo_min[this->nos];
        int8_t precedente[this->nos];
        bool fechado[this->nos];
        u_int8_t menor_custo;
        u_int8_t indice_menor_custo;
        u_int8_t origem = inicial - 1;

        // inicializa valores necessarios
        for (int i = 0; i < this->nos; i++)
        {
            estimativa_custo_min[i] = 255;
            precedente[i] = -1;
            fechado[i] = false;
        }
        // bota origem como 0
        estimativa_custo_min[origem] = 0;
        // ALTERADO: antes mantinha -1; a origem agora tem ela mesma como predecessor.
        precedente[origem] = origem;
        // comeca laco verificacao de todos nos
        for (int i = 0; i < this->nos; i++)
        {
            menor_custo = 255;
            indice_menor_custo = 255;
            // fica rodando ate fechar a origem
            while (!fechado[origem])
            {
                for (int j = 0; j < this->nos; j++)
                {
                    // existe conexao entre nos
                    if (this->matriz_adjacencia[origem][j] > 0)
                    {
                        // calcula custo da conexao
                        int novo_custo = estimativa_custo_min[origem] + this->matriz_adjacencia[origem][j];
                        // verifica se conexao ja nao esta fechada e se o custo e menor do que o atual
                        if (!fechado[j] && estimativa_custo_min[j] > novo_custo)
                        {
                            // seta os novos valores de custo e precedente novos
                            estimativa_custo_min[j] = novo_custo;
                            precedente[j] = origem;
                        }
                    }
                }
                fechado[origem] = true;
            }
            // calculo do proximo a ser passado
            for (int j = 0; j < this->nos; j++)
            {
                if (estimativa_custo_min[j] < menor_custo && !fechado[j])
                {
                    indice_menor_custo = j;
                    menor_custo = estimativa_custo_min[j];
                }
            }
            // se nao houver proximo somente sai
            if (indice_menor_custo == 255)
            {
                break;
            }
            // coloca na origem o valor do proximo
            origem = indice_menor_custo;
        }
        // imprimir vetores
        for (int j = 0; j < this->nos; j++)
        {
            cout << "No: " << j + 1 << " ";
        }
        cout << endl;
        for (int j = 0; j < this->nos; j++)
        {
            cout << "Cost" << static_cast<int>(estimativa_custo_min[j]) << " ";
        }
        cout << endl;
        for (int j = 0; j < this->nos; j++)
        {
            cout << "Pre" << static_cast<int>(precedente[j]) + 1 << " ";
        }
        cout << endl;
    };

    void a_estrela(int inicio, int destino)
    {
        if (!vertice_valido(inicio) || !vertice_valido(destino))
        {
            cout << "Vertice inicial ou destino invalido para A*!" << endl;
            return;
        }

        int infinito = numeric_limits<int>::max();

        int origem = inicio - 1;
        int objetivo = destino - 1;

        int *g_custo = new int[this->nos];
        int *heuristica = new int[this->nos];
        int *f_custo = new int[this->nos];
        int *precedente = new int[this->nos];

        // 0 = nao descoberto
        // 1 = aberto
        // 2 = fechado
        int *estado = new int[this->nos];

        // Inicializacao
        for (int i = 0; i < this->nos; i++)
        {
            g_custo[i] = infinito;
            heuristica[i] = 0;
            f_custo[i] = infinito;
            precedente[i] = -1;
            estado[i] = 0;
        }

        g_custo[origem] = 0;
        estado[origem] = 1;

        // Leitura das heuristicas
        cout << "\nDigite a heuristica de cada vertice ate o destino:\n";

        for (int i = 0; i < this->nos; i++)
        {
            if (i == objetivo)
            {
                heuristica[i] = 0;
            }
            else
            {
                cout << "h(" << i + 1 << ") = ";
                cin >> heuristica[i];

                while (heuristica[i] < 0)
                {
                    cout << "A heuristica deve ser maior ou igual a zero.\n";

                    cout << "h(" << i + 1 << ") = ";
                    cin >> heuristica[i];
                }
            }
        }

        f_custo[origem] =
            g_custo[origem] + heuristica[origem];

        bool encontrou_caminho = false;

        while (true) // algoritimo A*
        {
            int atual = -1;
            int menor_f_custo = infinito;

            // Procura o vertice aberto com menor f
            for (int i = 0; i < this->nos; i++)
            {
                if (estado[i] == 1 &&
                    f_custo[i] < menor_f_custo)
                {
                    menor_f_custo = f_custo[i];
                    atual = i;
                }
            }

            // Nao existe mais vertice aberto
            if (atual == -1)
            {
                break;
            }

            // Encontrou o destino
            if (atual == objetivo)
            {
                encontrou_caminho = true;

                cout << "Caminho encontrado!" << endl;

                break;
            }

            // Fecha o vertice atual
            estado[atual] = 2;

            // Percorre os adjacentes
            for (int j = 0; j < this->nos; j++)
            {
                if (this->matriz_adjacencia[atual][j] > 0)
                {
                    int peso =
                        this->matriz_adjacencia[atual][j];

                    int novo_g =
                        g_custo[atual] + peso;

                    if (novo_g < g_custo[j])
                    {
                        g_custo[j] = novo_g;

                        precedente[j] = atual;

                        f_custo[j] =
                            g_custo[j] + heuristica[j];

                        estado[j] = 1;
                    }
                }
            }
        }

        // mostrando o resultado

        if (encontrou_caminho)
        {
            int *caminho = new int[this->nos];

            int tamanho = 0;

            int atual = objetivo;

            while (atual != -1)
            {
                caminho[tamanho] = atual;
                tamanho++;

                if (atual == origem)
                {
                    break;
                }

                atual = precedente[atual];
            }

            cout << "\nCaminho encontrado: ";

            for (int i = tamanho - 1; i >= 0; i--)
            {
                cout << caminho[i] + 1;

                if (i > 0)
                {
                    cout << " -> ";
                }
            }

            cout << endl;

            cout << "Custo total: "
                 << g_custo[objetivo]
                 << endl;

            delete[] caminho;
        }
        else
        {
            cout << "\nNao existe caminho de "
                 << inicio
                 << " ate "
                 << destino
                 << "." << endl;
        }

        // Liberar memoria
        delete[] g_custo;
        delete[] heuristica;
        delete[] f_custo;
        delete[] precedente;
        delete[] estado;
    };
};
void system_pause()
{
    cout << "Pressione enter para continuar...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
};
void limpar_tela()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
#endif
