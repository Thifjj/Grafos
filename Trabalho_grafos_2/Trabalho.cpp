#include <iostream>
#include "Trabalho.h"

using namespace std;

int main()
{
    grafo g;

    int opcao;
    int nos = 0;
    int a, b;
    int vertice;
    int comando;

    char dirigido;

    bool grafo_criado = false;
    bool eh_dirigido = false;

    do
    {
        cout << "\n";
        cout << "==========================================" << endl;
        cout << "            CRIADOR DE GRAFOS             " << endl;
        cout << "==========================================" << endl;

        cout << "1  - Criar grafo" << endl;
        cout << "2  - Adicionar vertices" << endl;
        cout << "3  - Adicionar conexoes" << endl;
        cout << "4  - Remover conexoes" << endl;
        cout << "5  - Mostrar matriz de adjacencia" << endl;
        cout << "6  - Busca em largura (BFS)" << endl;
        cout << "7  - Busca em profundidade (DFS)" << endl;
        cout << "8  - Fecho transitivo direto" << endl;
        cout << "9  - Fecho transitivo inverso" << endl;
        cout << "10 - Componentes fortemente conexos" << endl;
        cout << "0  - Sair" << endl;

        cout << "\nEscolha uma opcao: ";
        cin >> opcao;

        cout << endl;

        switch (opcao)
        {
        // ==========================================
        // CRIAR GRAFO
        // ==========================================
        case 1:
        {
            if (grafo_criado)
            {
                cout << "Ja existe um grafo criado!" << endl;
                cout << "Adicione ou remova vertices/conexoes "
                     << "utilizando o menu." << endl;
                break;
            }

            cout << "===== CRIAR NOVO GRAFO =====" << endl;

            cout << "Numero inicial de vertices: ";
            cin >> nos;

            while (nos < 0)
            {
                cout << "Quantidade invalida." << endl;
                cout << "Numero inicial de vertices: ";
                cin >> nos;
            }

            cout << "O grafo sera dirigido? (s/n): ";
            cin >> dirigido;

            eh_dirigido =
                (dirigido == 's' || dirigido == 'S');

            g.inicializa_grafo(nos, eh_dirigido);

            grafo_criado = true;

            cout << "\nGrafo criado com sucesso!" << endl;

            if (eh_dirigido)
                cout << "Tipo: dirigido" << endl;
            else
                cout << "Tipo: nao dirigido" << endl;

            cout << "Vertices: " << nos << endl;

            break;
        }

        // ==========================================
        // ADICIONAR VERTICES
        // ==========================================
        case 2:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            cout << "===== ADICIONAR VERTICES =====" << endl;
            cout << "Digite quantos vertices deseja adicionar." << endl;
            cout << "Digite -1 para voltar ao menu." << endl;

            while (true)
            {
                cout << "\nQuantidade: ";
                cin >> comando;

                // Sai da opcao
                if (comando == -1)
                {
                    cout << "Voltando ao menu..." << endl;
                    break;
                }

                // Nao permite zero ou valores negativos
                if (comando <= 0)
                {
                    cout << "Quantidade invalida!" << endl;
                    continue;
                }

                // Cria a quantidade solicitada
                for (int i = 0; i < comando; i++)
                {
                    g.cria_vertice();
                    nos++;
                }

                cout << comando
                     << " vertice(s) adicionado(s) com sucesso!" << endl;

                cout << "Total de vertices: "
                     << nos << endl;
            }

            break;
        }

        // ==========================================
        // ADICIONAR CONEXOES
        // ==========================================
        case 3:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            cout << "===== ADICIONAR CONEXOES =====" << endl;

            if (eh_dirigido)
            {
                cout << "Formato: ORIGEM -> DESTINO" << endl;
            }
            else
            {
                cout << "Formato: VERTICE A - VERTICE B" << endl;
            }

            cout << "Digite -1 como primeiro vertice "
                 << "para voltar ao menu." << endl;

            while (true)
            {
                cout << "\nPrimeiro vertice: ";
                cin >> a;

                if (a == -1)
                {
                    cout << "Voltando ao menu..." << endl;
                    break;
                }

                if (!g.vertice_valido(a))
                {
                    cout << "Vertice invalido!" << endl;
                    continue;
                }

                cout << "Segundo vertice: ";
                cin >> b;

                if (b == -1)
                {
                    cout << "Voltando ao menu..." << endl;
                    break;
                }

                if (!g.vertice_valido(b))
                {
                    cout << "Vertice invalido!" << endl;
                    continue;
                }

                g.cria_conexao(a, b);

                if (eh_dirigido)
                {
                    cout << "Conexao "
                         << a << " -> " << b
                         << " criada!" << endl;
                }
                else
                {
                    cout << "Conexao "
                         << a << " - " << b
                         << " criada!" << endl;
                }
            }

            break;
        }

        // ==========================================
        // REMOVER CONEXOES
        // ==========================================
        case 4:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            cout << "===== REMOVER CONEXOES =====" << endl;

            cout << "Digite -1 como primeiro vertice "
                 << "para voltar ao menu." << endl;

            while (true)
            {
                cout << "\nPrimeiro vertice: ";
                cin >> a;

                if (a == -1)
                {
                    cout << "Voltando ao menu..." << endl;
                    break;
                }

                if (!g.vertice_valido(a))
                {
                    cout << "Vertice invalido!" << endl;
                    continue;
                }

                cout << "Segundo vertice: ";
                cin >> b;

                if (b == -1)
                {
                    cout << "Voltando ao menu..." << endl;
                    break;
                }

                if (!g.vertice_valido(b))
                {
                    cout << "Vertice invalido!" << endl;
                    continue;
                }

                g.remove_conexao(a, b);

                cout << "Conexao removida!" << endl;
            }

            break;
        }

        // ==========================================
        // MOSTRAR GRAFO
        // ==========================================
        case 5:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            g.mostrar_grafo();

            break;
        }

        // ==========================================
        // BFS
        // ==========================================
        case 6:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            cout << "===== BUSCA EM LARGURA =====" << endl;

            cout << "Vertice inicial (-1 para voltar): ";
            cin >> vertice;

            if (vertice == -1)
                break;

            g.bfs(vertice);

            break;
        }

        // ==========================================
        // DFS
        // ==========================================
        case 7:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            cout << "===== BUSCA EM PROFUNDIDADE =====" << endl;

            cout << "Vertice inicial (-1 para voltar): ";
            cin >> vertice;

            if (vertice == -1)
                break;

            g.dfs(vertice);

            break;
        }

        // ==========================================
        // FECHO TRANSITIVO DIRETO
        // ==========================================
        case 8:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            cout << "===== FECHO TRANSITIVO DIRETO =====" << endl;

            cout << "Vertice (-1 para voltar): ";
            cin >> vertice;

            if (vertice == -1)
                break;

            g.fecho_transitivo_direto(vertice);

            break;
        }

        // ==========================================
        // FECHO TRANSITIVO INVERSO
        // ==========================================
        case 9:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            cout << "===== FECHO TRANSITIVO INVERSO =====" << endl;

            cout << "Vertice (-1 para voltar): ";
            cin >> vertice;

            if (vertice == -1)
                break;

            g.fecho_transitivo_inverso(vertice);

            break;
        }

        // ==========================================
        // COMPONENTES FORTEMENTE CONEXOS
        // ==========================================
        case 10:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            if (!eh_dirigido)
            {
                cout << "O grafo nao e dirigido." << endl;
                cout << "Componentes fortemente conexos "
                     << "sao utilizados em grafos dirigidos."
                     << endl;

                break;
            }

            g.componentes_fortemente_conexos();

            break;
        }

        // ==========================================
        // SAIR
        // ==========================================
        case 0:
        {
            cout << "Encerrando programa..." << endl;
            break;
        }

        default:
        {
            cout << "Opcao invalida!" << endl;
            break;
        }
        }

    } while (opcao != 0);

    return 0;
}