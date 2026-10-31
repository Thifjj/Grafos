// ADICIONADO: interface Qt separada do menu de terminal.
#include <QtWidgets>
#include <cmath>
#include <memory>
#include <sstream>
#include "bibliotecas/Trabalho.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QWidget janela;
    janela.setWindowTitle("Criador de grafos");
    janela.resize(1100, 750);
    // ALTERADO: a primeira tela contem somente a criacao do grafo.
    auto raiz = new QVBoxLayout(&janela);
    auto paginas = new QStackedWidget;
    raiz->addWidget(paginas);
    auto inicial = new QWidget;
    auto inicioLayout = new QVBoxLayout(inicial);
    inicioLayout->addStretch();
    auto cartao = new QGroupBox("Criar grafo");
    cartao->setMaximumWidth(440);
    auto cadastro = new QFormLayout(cartao);
    auto quantidade = new QSpinBox;
    quantidade->setRange(0, 100);
    quantidade->setValue(5);
    auto dirigido = new QCheckBox("Grafo dirigido");
    auto criar = new QPushButton("Criar grafo");
    criar->setObjectName("criarGrafo");
    cadastro->addRow("Numero de vertices", quantidade);
    cadastro->addRow("Tipo", dirigido);
    cadastro->addRow(criar);
    // ADICIONADO: importa vertices, tipo e conexoes de um arquivo de texto.
    auto importar = new QPushButton("Abrir grafo de arquivo");
    cadastro->addRow(importar);
    inicioLayout->addWidget(cartao, 0, Qt::AlignHCenter);
    inicioLayout->addStretch();
    paginas->addWidget(inicial);
    auto trabalho = new QWidget;
    paginas->addWidget(trabalho);
    auto layout = new QHBoxLayout(trabalho);
    auto painel = new QVBoxLayout;
    layout->addLayout(painel);
    painel->addWidget(new QLabel("<h2>Operacoes do grafo</h2>"));
    auto origem = new QSpinBox;
    auto destino = new QSpinBox;
    auto custo = new QSpinBox;
    origem->setRange(1, 100);
    destino->setRange(1, 100);
    custo->setRange(1, 254);
    custo->setValue(1);
    // ALTERADO: controles internos guardam parametros; cada botao abre seu formulario.
    origem->setParent(&janela); origem->hide();
    destino->setParent(&janela); destino->hide();
    custo->setParent(&janela); custo->hide();
    auto operacao = new QComboBox(&janela);
    operacao->addItems({"Adicionar vertice", "Remover vertice", "Adicionar conexao", "Remover conexao", "Matriz de adjacencia", "BFS", "DFS", "Fecho direto", "Fecho inverso", "Componentes / SFCMs", "Dijkstra", "A estrela", "Coloracao", "Ciclo de tamanho 3", "Limites de Euler", "Areas"});
    operacao->hide();
    auto executar = new QPushButton(&janela);
    executar->hide();
    auto botoes = new QVBoxLayout;
    painel->addLayout(botoes);
    auto modoPassos = new QCheckBox("Executar passo a passo");
    modoPassos->setChecked(true);
    painel->addWidget(modoPassos);
    auto proximo = new QPushButton("Proximo passo");
    auto reiniciar = new QPushButton("Reiniciar passos");
    painel->addWidget(proximo);
    painel->addWidget(reiniciar);
    proximo->setEnabled(false); reiniciar->setEnabled(false);
    auto status = new QLabel("Crie um grafo para comecar.");
    painel->addWidget(status);
    painel->addStretch();
    auto direita = new QVBoxLayout;
    layout->addLayout(direita, 1);
    auto cena = new QGraphicsScene(&janela);
    auto vista = new QGraphicsView(cena);
    vista->setRenderHint(QPainter::Antialiasing);
    direita->addWidget(vista, 2);
    auto saida = new QPlainTextEdit;
    saida->setReadOnly(true);
    direita->addWidget(saida, 1);
    std::unique_ptr<grafo> g;
    // ADICIONADO: guarda as cores calculadas para pintar os vertices no desenho.
    QMap<int, QColor> coresVertices;
    struct Passo { QMap<int,QColor> cores; QString texto; };
    QVector<Passo> passos;
    int passoAtual = -1;
    int n = 0;
    bool eh_dirigido = false;
    // ADICIONADO: desenha arestas, custos e vertices a partir da matriz real.
    auto desenhar = [&]
    {
        cena->clear();
        QVector<QPointF> pontos;
        for (int i = 0; i < n; ++i)
        {
            double angulo = 6.283185307 * i / n;
            pontos.append(QPointF(220 * cos(angulo), 220 * sin(angulo)));
        }
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
            {
                int peso = g->custo_conexao(i + 1, j + 1);
                if (!peso || (!eh_dirigido && j < i))
                    continue;
                QPointF a = pontos[i], b = pontos[j];
                if (i == j)
                    cena->addEllipse(a.x() - 15, a.y() - 45, 30, 35);
                else
                {
                    cena->addLine(QLineF(a, b), QPen(QColor("#64748b"), 2));
                    if (eh_dirigido)
                    {
                        QLineF linha(a, b);
                        QPointF ponta = b - (b - a) * (22 / linha.length());
                        QPointF v = (b - a) / linha.length();
                        QPointF lateral(-v.y(), v.x());
                        QPolygonF seta;
                        seta << ponta << ponta - v * 12 + lateral * 6 << ponta - v * 12 - lateral * 6;
                        cena->addPolygon(seta, QPen(Qt::darkGray), QBrush(Qt::darkGray));
                    }
                }
                auto texto = cena->addText(QString::number(peso));
                texto->setPos((a + b) / 2);
            }
        for (int i = 0; i < n; ++i)
        {
            auto p = pontos[i];
            QColor preenchimento = coresVertices.value(i + 1, QColor("#dbeafe"));
            cena->addEllipse(p.x() - 20, p.y() - 20, 40, 40, QPen(Qt::darkBlue), QBrush(preenchimento));
            auto label = cena->addText(QString::number(i + 1));
            label->setPos(p - QPointF(10, 14));
        }
        vista->fitInView(cena->itemsBoundingRect().adjusted(-30, -30, 30, 30), Qt::KeepAspectRatio);
        origem->setMaximum(qMax(1, n));
        destino->setMaximum(qMax(1, n));
        status->setText(QString("%1 vertices | %2").arg(n).arg(eh_dirigido ? "Dirigido" : "Nao dirigido"));
    };
    // ADICIONADO: estados visuais independentes, sem alterar a matriz do grafo.
    auto mostrarPasso = [&] {
        if (passoAtual < 0 || passoAtual >= passos.size()) return;
        coresVertices = passos[passoAtual].cores;
        saida->setPlainText(QString("Passo %1 de %2\n").arg(passoAtual+1).arg(passos.size()) + passos[passoAtual].texto);
        proximo->setEnabled(passoAtual+1 < passos.size());
        desenhar();
    };
    auto limparPassos = [&] {
        passos.clear(); passoAtual = -1;
        proximo->setEnabled(false); reiniciar->setEnabled(false);
    };
    QObject::connect(proximo, &QPushButton::clicked, [&] { ++passoAtual; mostrarPasso(); });
    QObject::connect(reiniciar, &QPushButton::clicked, [&] { passoAtual=0; mostrarPasso(); });
    auto prepararPassos = [&](int op, int inicio, int fim) {
        limparPassos(); coresVertices.clear();
        QMap<int,QColor> cores;
        auto registrar = [&](QString texto) { passos.append({cores,texto}); };
        QColor aberto("#fde68a"), atual("#fb923c"), fechado("#86efac");
        registrar("Estado inicial. Amarelo: descoberto; laranja: atual; verde: processado.");
        if (op == 5 || op == 6) {
            QVector<bool> visitado(n,false);
            QList<int> pendentes;
            QStringList percurso;
            auto componente = [&](int raiz) {
                pendentes.append(raiz);
                if(op==5) visitado[raiz]=true;
                cores[raiz+1]=aberto;
                registrar(QString("Descobrir vertice %1.").arg(raiz+1));
                while(!pendentes.isEmpty()) {
                    int v=op==5 ? pendentes.takeFirst() : pendentes.takeLast();
                    if(op==6 && visitado[v]) continue;
                    visitado[v]=true; cores[v+1]=atual;
                    percurso << QString::number(v+1);
                    registrar(QString("Processar %1. Percurso: %2").arg(v+1).arg(percurso.join(" → ")));
                    for(int k=0;k<n;++k) {
                        int u=op==5 ? k : n-1-k;
                        if(g->custo_conexao(v+1,u+1)>0 && !visitado[u]) {
                            if(op==5) visitado[u]=true;
                            pendentes.append(u); cores[u+1]=aberto;
                            registrar(QString("%1: inserir %2. Pendentes: %3").arg(op==5 ? "Fila" : "Pilha").arg(u+1).arg(pendentes.size()));
                        }
                    }
                    cores[v+1]=fechado; registrar(QString("Finalizar %1.").arg(v+1));
                }
            };
            componente(inicio-1);
            for(int v=0;v<n;++v) if(!visitado[v]) componente(v);
            registrar("Percurso concluido: " + percurso.join(" → "));
        } else if(op==12) {
            QVector<int> cor(n,0), grau(n,0);
            for(int v=0;v<n;++v) {
                if(g->custo_conexao(v+1,v+1)>0) { registrar("Grafo com laco: coloracao propria impossivel."); passoAtual=0; reiniciar->setEnabled(true); mostrarPasso(); return; }
                for(int u=0;u<n;++u) if(g->custo_conexao(v+1,u+1)>0) ++grau[v];
            }
            for(int k=0;k<n;++k) {
                int melhor=-1, sat=-1;
                for(int v=0;v<n;++v) if(!cor[v]) {
                    QSet<int> vizinhas;
                    for(int u=0;u<n;++u) if(g->custo_conexao(v+1,u+1)>0 && cor[u]) vizinhas.insert(cor[u]);
                    if(melhor==-1 || vizinhas.size()>sat || (vizinhas.size()==sat && grau[v]>grau[melhor])) { melhor=v; sat=vizinhas.size(); }
                }
                cores[melhor+1]=atual;
                registrar(QString("Selecionar %1: saturacao %2, grau %3.").arg(melhor+1).arg(sat).arg(grau[melhor]));
                QSet<int> usadas;
                for(int u=0;u<n;++u) if(g->custo_conexao(melhor+1,u+1)>0 && cor[u]) usadas.insert(cor[u]);
                int c=1; while(usadas.contains(c)) ++c;
                cor[melhor]=c; cores[melhor+1]=QColor::fromHsv((c-1)*137%360,135,245);
                registrar(QString("Atribuir cor %1 ao vertice %2.").arg(c).arg(melhor+1));
            }
            registrar("Coloracao concluida.");
        } else {
            // A* usa h=0, como a execucao direta da interface.
            int infinito=op==10 ? 255 : std::numeric_limits<int>::max();
            QVector<int> distancia(n,infinito), anterior(n,-1);
            QVector<bool> pronto(n,false);
            distancia[inicio-1]=0; anterior[inicio-1]=inicio-1;
            auto tabela = [&] {
                QString texto="\nVertice | Custo | Antecessor\n";
                for(int v=0;v<n;++v) texto+=QString("%1 | %2 | %3\n").arg(v+1).arg(distancia[v]==infinito ? "∞" : QString::number(distancia[v])).arg(anterior[v]<0 ? "—" : QString::number(anterior[v]+1));
                return texto;
            };
            cores[inicio]=aberto;
            registrar((op==11 ? "A*: h=0 para todos os vertices." : "Dijkstra: iniciar custos (limite original: 254).") + tabela());
            for(int k=0;k<n;++k) {
                int v=-1;
                for(int u=0;u<n;++u) if(!pronto[u] && distancia[u]<infinito && (v==-1 || distancia[u]<distancia[v])) v=u;
                if(v==-1) break;
                cores[v+1]=atual;
                registrar(QString("Selecionar vertice %1 de menor custo.").arg(v+1)+tabela());
                if(op==11 && v==fim-1) {
                    QStringList caminho;
                    for(int u=v;;u=anterior[u]) { caminho.prepend(QString::number(u+1)); cores[u+1]=QColor("#a5b4fc"); if(u==inicio-1) break; }
                    registrar("Destino encontrado: "+caminho.join(" → ")+tabela());
                    break;
                }
                pronto[v]=true;
                for(int u=0;u<n;++u) {
                    int peso=g->custo_conexao(v+1,u+1);
                    if(peso>0 && !pronto[u] && distancia[v]+peso<distancia[u]) {
                        distancia[u]=distancia[v]+peso; anterior[u]=v; cores[u+1]=aberto;
                        registrar(QString("Relaxar %1 → %2: atualizar custo e antecessor.").arg(v+1).arg(u+1)+tabela());
                    }
                }
                cores[v+1]=fechado;
                registrar(QString("Fechar vertice %1.").arg(v+1)+tabela());
            }
            registrar((op==11 && distancia[fim-1]==infinito ? "Destino inalcançavel." : "Busca concluida.")+tabela());
        }
        passoAtual=0; reiniciar->setEnabled(true); mostrarPasso();
    };
    QObject::connect(criar, &QPushButton::clicked, [&]
                     {
        limparPassos();
        g = std::make_unique<grafo>(); n = quantidade->value();
        coresVertices.clear();
        eh_dirigido = dirigido->isChecked(); g->inicializa_grafo(n,eh_dirigido);
        saida->setPlainText("Grafo criado. Escolha uma operacao nos botoes ao lado."); paginas->setCurrentWidget(trabalho); desenhar(); });
    QObject::connect(importar, &QPushButton::clicked, [&] {
        QString caminho = QFileDialog::getOpenFileName(&janela, "Abrir grafo", QString(), "Grafos (*.txt);;Todos os arquivos (*)");
        if (caminho.isEmpty()) return;
        QFile arquivo(caminho);
        if (!arquivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(&janela, "Erro", "Nao foi possivel abrir o arquivo."); return;
        }
        std::istringstream dados(arquivo.readAll().toStdString());
        int vertices, tipo;
        if (!(dados >> vertices >> tipo) || vertices < 0 || vertices > 100 || (tipo != 0 && tipo != 1)) {
            QMessageBox::warning(&janela, "Arquivo invalido", "Primeira linha: quantidade de vertices (0 a 100) e tipo (0 nao dirigido, 1 dirigido)."); return;
        }
        auto novoGrafo = std::make_unique<grafo>();
        novoGrafo->inicializa_grafo(vertices, tipo == 1);
        while (true) {
            dados >> std::ws;
            if (dados.eof()) break;
            int a, b, peso;
            if (!(dados >> a >> b >> peso) || !novoGrafo->vertice_valido(a) || !novoGrafo->vertice_valido(b) || peso < 1 || peso > 254) {
                QMessageBox::warning(&janela, "Arquivo invalido", "Cada conexao deve conter: origem destino custo (1 a 254)."); return;
            }
            if (novoGrafo->custo_conexao(a,b)>0) novoGrafo->remove_conexao(a,b);
            novoGrafo->cria_conexao(a,b,peso);
        }
        limparPassos();
        g = std::move(novoGrafo); n = vertices; eh_dirigido = tipo == 1;
        coresVertices.clear(); paginas->setCurrentWidget(trabalho);
        saida->setPlainText("Grafo importado: " + caminho); desenhar();
    });
    QObject::connect(executar, &QPushButton::clicked, [&]
                     {
        if (!g) { saida->setPlainText("Primeiro crie um grafo."); return; }
        int op = operacao->currentIndex(), a = origem->value(), b = destino->value();
        if (!n && op != 0) { saida->setPlainText("O grafo esta vazio."); return; }
        if (op == 0 && n >= 100) { saida->setPlainText("Limite da interface: 100 vertices."); return; }
        if (eh_dirigido && op >= 12) { saida->setPlainText("Operacao somente para nao dirigidos."); return; }
        // ADICIONADO: A* usa heuristica zero, valida e equivalente a Dijkstra.
        std::ostringstream resultado;
        std::istringstream entrada(std::string(n*2, char(32)));
        std::string zeros;
        for (int i=0;i<n;++i) zeros += "0 ";
        entrada.str(zeros);
        auto cout_antigo = cout.rdbuf(resultado.rdbuf());
        auto cin_antigo = cin.rdbuf(entrada.rdbuf());
        switch(op) {
        case 0: g->cria_vertice(); ++n; cout << "Vertice adicionado."; break;
        case 1: g->remove_vertice(a); --n; cout << "Vertice removido."; break;
        case 2:
            // Evita incrementar o contador novamente ao atualizar um peso.
            if(g->custo_conexao(a,b)>0) g->remove_conexao(a,b);
            g->cria_conexao(a,b,custo->value()); cout << "Conexao salva."; break;
        case 3:
            if(g->custo_conexao(a,b)>0) {g->remove_conexao(a,b); cout << "Conexao removida.";}
            else cout << "A conexao nao existe.";
            break;
        case 4: g->mostrar_grafo(); break;
        case 5: g->bfs(a); break;
        case 6: g->dfs(a); break;
        case 7: g->fecho_transitivo_direto(a); break;
        case 8: g->fecho_transitivo_inverso(a); break;
        case 9: if(eh_dirigido) g->componentes_fortemente_conexos(); else g->componentes_conexos(); break;
        case 10: g->busca_djikstra(a); break;
        case 11: g->a_estrela(a,b); break;
        case 12: g->coloracao_grafo(); break;
        case 13: cout << (g->is_ciclo_3() ? "Existe ciclo de tamanho 3." : "Nao existe ciclo de tamanho 3."); break;
        case 14: cout << (g->planar_euler() ? "Satisfaz os limites de Euler; nao garante planaridade." : "Nao satisfaz os limites implementados."); break;
        case 15: g->areas_grafo(); break;
        }
        cout.rdbuf(cout_antigo); cin.rdbuf(cin_antigo);
        QString textoResultado = QString::fromStdString(resultado.str());
        // ADICIONADO: usa as mesmas classes de cor exibidas pelo algoritmo.
        // A cor e limpa ao editar o grafo, pois a coloracao anterior pode ficar invalida.
        if (op <= 3 || op == 12) coresVertices.clear();
        if (op == 12) {
            QRegularExpression padrao("Vertice\\s+(\\d+):\\s+cor\\s+(\\d+)");
            auto ocorrencias = padrao.globalMatch(textoResultado);
            while (ocorrencias.hasNext()) {
                auto item = ocorrencias.next();
                int vertice = item.captured(1).toInt();
                int cor = item.captured(2).toInt();
                // Tons claros preservam a leitura dos numeros dos vertices.
                coresVertices[vertice] = QColor::fromHsv((cor - 1) * 137 % 360, 135, 245);
            }
        }
        saida->setPlainText(textoResultado); desenhar(); });
    // ADICIONADO: botoes diretos e formularios com apenas os campos necessarios.
    for (int op = 0; op < operacao->count(); ++op) {
        auto botao = new QPushButton(operacao->itemText(op));
        botao->setObjectName(QString("operacao%1").arg(op));
        botoes->addWidget(botao);
        QObject::connect(botao, &QPushButton::clicked, [&, op] {
            if (eh_dirigido && op >= 12) {
                QMessageBox::information(&janela, "Operacao indisponivel", "Esta operacao requer um grafo nao dirigido.");
                return;
            }
            if (!n && op != 0) {
                QMessageBox::information(&janela, "Grafo vazio", "Adicione um vertice primeiro.");
                return;
            }
            bool usaOrigem = op == 1 || op == 2 || op == 3 || (op >= 5 && op <= 8) || op == 10 || op == 11;
            bool usaDestino = op == 2 || op == 3 || op == 11;
            if (usaOrigem) {
                QDialog dialogo(&janela);
                dialogo.setWindowTitle(operacao->itemText(op));
                auto campos = new QFormLayout(&dialogo);
                QSpinBox a, b, peso;
                a.setRange(1,n); b.setRange(1,n);
                a.setValue(origem->value()); b.setValue(destino->value());
                peso.setRange(1,254); peso.setValue(custo->value());
                campos->addRow(op == 1 ? "Vertice a remover" : "Vertice inicial", &a);
                if (usaDestino) campos->addRow("Destino", &b);
                if (op == 2) campos->addRow("Custo", &peso);
                if (op == 11) campos->addRow(new QLabel("A* utiliza heuristica zero nesta versao."));
                auto confirmar = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                campos->addRow(confirmar);
                QObject::connect(confirmar, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
                QObject::connect(confirmar, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
                if (dialogo.exec() != QDialog::Accepted) return;
                origem->setValue(a.value()); destino->setValue(b.value()); custo->setValue(peso.value());
            }
            if (modoPassos->isChecked() && (op==5 || op==6 || op==10 || op==11 || op==12)) {
                prepararPassos(op, origem->value(), destino->value());
                return;
            }
            limparPassos();
            operacao->setCurrentIndex(op);
            executar->click();
        });
    }
    auto novo = new QPushButton("Criar outro grafo");
    painel->addWidget(novo);
    QObject::connect(novo, &QPushButton::clicked, [&] {
        if (QMessageBox::question(&janela, "Novo grafo", "Voltar para a criacao? Ao criar, o grafo atual sera substituido.") == QMessageBox::Yes)
            paginas->setCurrentWidget(inicial);
    });
    janela.setStyleSheet("QWidget {font-size:14px;} QPushButton {padding:7px 14px; border:1px solid #94a3b8; border-radius:6px;} QPushButton:hover {background:#dbeafe; color:#172554;} QGroupBox {padding:20px; font-size:20px;} QSpinBox {padding:6px;} ");
    janela.show();
    return app.exec();
}
