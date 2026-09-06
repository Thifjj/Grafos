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
        cout << "3  - Remover vertice" << endl;
        cout << "4  - Adicionar conexoes" << endl;
        cout << "5  - Remover conexoes" << endl;
        cout << "6  - Mostrar matriz de adjacencia" << endl;
        cout << "7  - Busca em largura (BFS)" << endl;
        cout << "8  - Busca em profundidade (DFS)" << endl;
        cout << "9  - Fecho transitivo direto" << endl;
        cout << "10 - Fecho transitivo inverso" << endl;
        cout << "11 - Verificar conectividade / SFCMs" << endl;
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
                system_pause();
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
            system_pause();
            limpar_tela();
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
                system_pause();
                break;
            }

            cout << "===== ADICIONAR VERTICES =====" << endl;
            cout << "Digite quantos vertices deseja adicionar." << endl;
            cout << "Digite -1 para voltar ao menu." << endl;

            while (true)
            {
                cout << "\nQuantidade: ";
                cin >> comando;

                if (comando == -1)
                {
                    cout << "Voltando ao menu..." << endl;
                    break;
                }

                if (comando <= 0)
                {
                    cout << "Quantidade invalida!" << endl;
                    continue;
                }

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
            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // REMOVER VERTICE
        // ==========================================
        case 3:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                system_pause();
                break;
            }

            cout << "===== REMOVER VERTICE =====" << endl;

            while (true)
            {
                g.mostrar_grafo();

                cout << "\nVertice que deseja remover "
                     << "(-1 para voltar): ";

                cin >> vertice;

                if (vertice == -1)
                {
                    cout << "Voltando ao menu..." << endl;
                    break;
                }

                if (!g.vertice_valido(vertice))
                {
                    cout << "Vertice invalido!" << endl;
                    continue;
                }

                g.remove_vertice(vertice);
                nos--;

                cout << "Vertice removido com sucesso!" << endl;
                cout << "Total de vertices: " << nos << endl;

                if (nos == 0)
                {
                    cout << "O grafo nao possui mais vertices." << endl;
                    break;
                }
            }
            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // ADICIONAR CONEXOES
        // ==========================================
        case 4:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            if (nos == 0)
            {
                cout << "O grafo nao possui vertices!" << endl;
                break;
            }

            cout << "===== ADICIONAR CONEXOES =====" << endl;

            if (eh_dirigido)
                cout << "Formato: ORIGEM -> DESTINO" << endl;
            else
                cout << "Formato: VERTICE A - VERTICE B" << endl;

            cout << "Digite -1 como primeiro vertice "
                 << "para voltar ao menu." << endl;

            while (true)
            {
                g.mostrar_grafo();

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
            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // REMOVER CONEXOES
        // ==========================================
        case 5:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            if (nos == 0)
            {
                cout << "O grafo nao possui vertices!" << endl;
                break;
            }

            cout << "===== REMOVER CONEXOES =====" << endl;

            while (true)
            {
                cout << "Digite -1 como primeiro vertice "
                     << "para voltar ao menu." << endl;

                g.mostrar_grafo();

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
            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // MOSTRAR GRAFO
        // ==========================================
        case 6:
        {
            if (!grafo_criado)
            {
                cout << "Primeiro crie um grafo!" << endl;
                break;
            }

            g.mostrar_grafo();
            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // BFS
        // ==========================================
        case 7:
        {
            if (!grafo_criado || nos == 0)
            {
                cout << "O grafo nao possui vertices!" << endl;
                break;
            }

            cout << "===== BUSCA EM LARGURA =====" << endl;

            cout << "Vertice inicial (-1 para voltar): ";
            cin >> vertice;

            if (vertice == -1)
                break;

            g.bfs(vertice);

            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // DFS
        // ==========================================
        case 8:
        {
            if (!grafo_criado || nos == 0)
            {
                cout << "O grafo nao possui vertices!" << endl;
                break;
            }

            cout << "===== BUSCA EM PROFUNDIDADE =====" << endl;

            cout << "Vertice inicial (-1 para voltar): ";
            cin >> vertice;

            if (vertice == -1)
                break;

            g.dfs(vertice);

            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // FECHO TRANSITIVO DIRETO
        // ==========================================
        case 9:
        {
            if (!grafo_criado || nos == 0)
            {
                cout << "O grafo nao possui vertices!" << endl;
                break;
            }

            cout << "===== FECHO TRANSITIVO DIRETO =====" << endl;

            cout << "Vertice (-1 para voltar): ";
            cin >> vertice;

            if (vertice == -1)
                break;

            g.fecho_transitivo_direto(vertice);

            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // FECHO TRANSITIVO INVERSO
        // ==========================================
        case 10:
        {
            if (!grafo_criado || nos == 0)
            {
                cout << "O grafo nao possui vertices!" << endl;
                break;
            }

            cout << "===== FECHO TRANSITIVO INVERSO =====" << endl;

            cout << "Vertice (-1 para voltar): ";
            cin >> vertice;

            if (vertice == -1)
                break;

            g.fecho_transitivo_inverso(vertice);

            system_pause();
            limpar_tela();
            break;
        }

        // ==========================================
        // CONECTIVIDADE / SFCMs
        // ==========================================
        case 11:
        {
            if (!grafo_criado || nos == 0)
            {
                cout << "O grafo nao possui vertices!" << endl;
                break;
            }

            cout << "===== CONECTIVIDADE DO GRAFO =====" << endl;

            if (!eh_dirigido)
            {
                cout << "O grafo nao e dirigido." << endl;
                cout << "A verificacao atual de SFCMs "
                     << "e utilizada para grafos dirigidos." << endl;

                system_pause();
                break;
            }

            g.componentes_fortemente_conexos();

            system_pause();
            limpar_tela();
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