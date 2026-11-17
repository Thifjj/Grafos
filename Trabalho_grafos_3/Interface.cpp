// Interface grafica (Qt6) do criador de grafos.
//
// Uso do editor:
//   * clique em uma area vazia ........ cria um vertice
//   * clique em um vertice ............ inicia uma conexao (linha acompanha o mouse)
//   * clique em outro vertice ......... conclui e pede o custo da conexao
//   * arraste um vertice .............. move o vertice
//   * duplo clique em uma aresta ...... edita o custo
//   * botao direito ................... menu do vertice / da aresta
//   * Esc ............................. cancela a conexao em andamento
#include <QtWidgets>
#include <cmath>
#include <functional>
#include <memory>
#include <set>
#include <sstream>
#include "bibliotecas/Trabalho.h"

namespace
{
using Par = std::pair<int, int>;

const QColor kVerticePadrao("#4f46e5");
const QColor kAberto("#fbbf24");
const QColor kAtual("#fb923c");
const QColor kFechado("#34d399");
const QColor kCaminho("#a78bfa");
const QColor kArestaPadrao("#4b5b7c");
const QColor kArestaDestaque("#fbbf24");
const QColor kAcento("#22d3ee");

// Estado visual de um passo: cores dos vertices, arestas em destaque e rotulos.
struct Destaque
{
    QMap<int, QColor> cores;      // vertice (base 1) -> cor
    std::set<Par> arestas;        // arestas em destaque (base 1)
    QMap<int, QString> rotulos;   // texto exibido abaixo do vertice
    bool vazio() const { return cores.isEmpty() && arestas.empty() && rotulos.isEmpty(); }
};

// ============================================================
// Canvas: desenho e interacao com o grafo
// ============================================================
class Canvas : public QWidget
{
public:
    const grafo *g = nullptr;
    int n = 0;
    bool dirigido = false;
    QVector<QPointF> pos;
    Destaque destaque;

    std::function<void(QPointF)> aoCriarVertice;
    std::function<void(int, int)> aoConectar;
    std::function<void(int, QPoint)> aoMenuVertice;
    std::function<void(int, int, QPoint)> aoMenuAresta;
    std::function<void(int, int)> aoEditarAresta;
    std::function<void(const QString &)> aoDica;

    static constexpr double raio = 23.0;

    explicit Canvas(QWidget *pai = nullptr) : QWidget(pai)
    {
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        setMinimumSize(420, 320);
        setCursor(Qt::CrossCursor);
    }

    void cancelarConexao()
    {
        if (origem >= 0 && aoDica)
            aoDica(dicaPadrao);
        origem = -1;
        update();
    }
    void definirDicaPadrao(const QString &t) { dicaPadrao = t; }
    bool conectando() const { return origem >= 0; }

    int verticeEm(const QPointF &p) const
    {
        for (int i = n - 1; i >= 0; --i)
            if (QLineF(p, pos[i]).length() <= raio + 2)
                return i;
        return -1;
    }

    // Linha da aresta i->j (deslocada quando existe a aresta oposta em grafos dirigidos).
    QLineF linhaAresta(int i, int j) const
    {
        QPointF a = pos[i], b = pos[j];
        if (dirigido && i != j && g->custo_conexao(j + 1, i + 1) > 0)
        {
            QLineF l(a, b);
            if (l.length() > 1)
            {
                QPointF u = (b - a) / l.length();
                QPointF perp(-u.y(), u.x());
                a += perp * 7;
                b += perp * 7;
            }
        }
        return QLineF(a, b);
    }

    bool arestaEm(const QPointF &p, int &ai, int &aj) const
    {
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
            {
                if (g->custo_conexao(i + 1, j + 1) <= 0 || (!dirigido && j < i))
                    continue;
                if (i == j)
                {
                    if (QLineF(p, pos[i] + QPointF(0, -raio - 12)).length() < 18)
                    {
                        ai = i;
                        aj = j;
                        return true;
                    }
                    continue;
                }
                if (distanciaSegmento(p, linhaAresta(i, j)) < 8)
                {
                    ai = i;
                    aj = j;
                    return true;
                }
            }
        return false;
    }

protected:
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Escape)
            cancelarConexao();
        else
            QWidget::keyPressEvent(e);
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        setFocus();
        QPointF p = e->position();
        int v = verticeEm(p);
        if (e->button() == Qt::RightButton)
        {
            bool estavaConectando = origem >= 0;
            cancelarConexao();
            if (v >= 0 && aoMenuVertice)
                aoMenuVertice(v, e->globalPosition().toPoint());
            else
            {
                int ai, aj;
                if (!estavaConectando && arestaEm(p, ai, aj) && aoMenuAresta)
                    aoMenuAresta(ai, aj, e->globalPosition().toPoint());
            }
            return;
        }
        if (e->button() != Qt::LeftButton)
            return;

        if (origem >= 0)
        {
            int de = origem;
            origem = -1;
            update();
            if (v >= 0 && v != de)
            {
                if (aoDica)
                    aoDica(dicaPadrao);
                if (aoConectar)
                    aoConectar(de, v);
            }
            else if (aoDica)
                aoDica(dicaPadrao);
            return;
        }
        if (v >= 0)
        {
            arrastando = v;
            pressionado = p;
            moveu = false;
            deslocamento = pos[v] - p;
            return;
        }
        int ai, aj;
        if (arestaEm(p, ai, aj))
            return;
        if (aoCriarVertice)
            aoCriarVertice(p);
    }

    void mouseMoveEvent(QMouseEvent *e) override
    {
        QPointF p = e->position();
        cursor = p;
        if (arrastando >= 0)
        {
            if (!moveu && (p - pressionado).manhattanLength() > 6)
                moveu = true;
            if (moveu)
            {
                QPointF novo = p + deslocamento;
                novo.setX(qBound(raio + 4, novo.x(), width() - raio - 4));
                novo.setY(qBound(raio + 4, novo.y(), height() - raio - 4));
                pos[arrastando] = novo;
                setCursor(Qt::ClosedHandCursor);
            }
        }
        else
        {
            hover = verticeEm(p);
            setCursor(hover >= 0 ? Qt::PointingHandCursor : Qt::CrossCursor);
        }
        update();
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (e->button() != Qt::LeftButton || arrastando < 0)
            return;
        int v = arrastando;
        arrastando = -1;
        setCursor(Qt::PointingHandCursor);
        if (!moveu)
        {
            origem = v;
            if (aoDica)
                aoDica("Clique em outro vertice para conectar a " + QString::number(v + 1) + "  |  Esc cancela");
        }
        update();
    }

    void mouseDoubleClickEvent(QMouseEvent *e) override
    {
        int ai, aj;
        if (e->button() == Qt::LeftButton && verticeEm(e->position()) < 0 && arestaEm(e->position(), ai, aj) && aoEditarAresta)
            aoEditarAresta(ai, aj);
    }

    void leaveEvent(QEvent *) override
    {
        hover = -1;
        update();
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QLinearGradient fundo(0, 0, 0, height());
        fundo.setColorAt(0, QColor("#0b1020"));
        fundo.setColorAt(1, QColor("#111a33"));
        p.fillRect(rect(), fundo);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 26));
        for (int x = 18; x < width(); x += 28)
            for (int y = 18; y < height(); y += 28)
                p.drawEllipse(QPointF(x, y), 1.1, 1.1);

        if (n == 0)
        {
            p.setPen(QColor("#64748b"));
            QFont f = font();
            f.setPointSize(15);
            f.setBold(true);
            p.setFont(f);
            p.drawText(rect().adjusted(0, -20, 0, 0), Qt::AlignCenter, "Clique em qualquer lugar para criar o primeiro vertice");
            f.setPointSize(11);
            f.setBold(false);
            p.setFont(f);
            p.setPen(QColor("#475569"));
            p.drawText(rect().adjusted(0, 40, 0, 0), Qt::AlignCenter, "Depois clique em um vertice e em outro para conectar");
            return;
        }

        desenharArestas(p);

        // linha acompanhando o mouse durante a criacao de uma conexao
        if (origem >= 0 && origem < n)
        {
            QPointF destino = cursor;
            int h = verticeEm(cursor);
            if (h >= 0 && h != origem)
                destino = pos[h];
            QPen tracejado(kAcento, 2.5, Qt::DashLine);
            tracejado.setCapStyle(Qt::RoundCap);
            p.setPen(tracejado);
            p.drawLine(QLineF(pos[origem], destino));
        }
        desenharVertices(p);

        if (origem >= 0)
        {
            QString t = "Clique em outro vertice para conectar   -   Esc cancela";
            QFont f = font();
            f.setPointSize(11);
            f.setBold(true);
            p.setFont(f);
            QFontMetrics fm(f);
            QRectF caixa(0, 0, fm.horizontalAdvance(t) + 36, 32);
            caixa.moveCenter(QPointF(width() / 2.0, height() - 30));
            p.setPen(QPen(kAcento, 1.2));
            p.setBrush(QColor(15, 23, 42, 235));
            p.drawRoundedRect(caixa, 16, 16);
            p.setPen(QColor("#e2e8f0"));
            p.drawText(caixa, Qt::AlignCenter, t);
        }
    }

private:
    int origem = -1;
    int arrastando = -1;
    int hover = -1;
    bool moveu = false;
    QPointF cursor, pressionado, deslocamento;
    QString dicaPadrao;

    static double distanciaSegmento(const QPointF &p, const QLineF &l)
    {
        QPointF a = l.p1(), b = l.p2();
        QPointF ab = b - a;
        double comprimento2 = ab.x() * ab.x() + ab.y() * ab.y();
        if (comprimento2 < 1e-6)
            return QLineF(p, a).length();
        double t = qBound(0.0, ((p.x() - a.x()) * ab.x() + (p.y() - a.y()) * ab.y()) / comprimento2, 1.0);
        return QLineF(p, a + ab * t).length();
    }

    bool arestaEmDestaque(int i, int j) const
    {
        Par par(i + 1, j + 1);
        if (!dirigido && par.first > par.second)
            std::swap(par.first, par.second);
        return destaque.arestas.count(par) > 0;
    }

    void desenharBadge(QPainter &p, QPointF centro, const QString &texto, const QColor &cor, bool forte)
    {
        QFont f = font();
        f.setPointSize(10);
        f.setBold(true);
        p.setFont(f);
        QFontMetrics fm(f);
        QRectF caixa(0, 0, qMax(24, fm.horizontalAdvance(texto) + 14), 20);
        caixa.moveCenter(centro);
        p.setPen(QPen(cor, 1.4));
        p.setBrush(QColor("#0f172a"));
        p.drawRoundedRect(caixa, 10, 10);
        p.setPen(forte ? QColor("#fde68a") : QColor("#e2e8f0"));
        p.drawText(caixa, Qt::AlignCenter, texto);
    }

    // Etiqueta de custo ainda por desenhar (feita depois de todas as linhas).
    struct Etiqueta
    {
        QLineF linha;
        QPointF fixo;
        bool temLinha;
        double t;
        QString texto;
        QColor cor;
        bool forte;
    };

    void desenharEtiquetas(QPainter &p, const QVector<Etiqueta> &etiquetas)
    {
        // Evita sobreposicao: tenta posicoes ao longo da linha ate achar espaco livre.
        QVector<QRectF> ocupados;
        for (const auto &e : etiquetas)
        {
            QPointF centro = e.fixo;
            if (e.temLinha)
            {
                const double candidatos[] = {e.t, 0.5, 0.35, 0.65, 0.27, 0.73, 0.2, 0.8};
                centro = e.linha.pointAt(candidatos[0]);
                for (double t : candidatos)
                {
                    QPointF c = e.linha.pointAt(t);
                    QRectF caixa(0, 0, 34, 24);
                    caixa.moveCenter(c);
                    bool livre = true;
                    for (const auto &o : ocupados)
                        if (o.intersects(caixa))
                        {
                            livre = false;
                            break;
                        }
                    if (livre)
                    {
                        centro = c;
                        break;
                    }
                }
            }
            QRectF caixa(0, 0, 34, 24);
            caixa.moveCenter(centro);
            ocupados.append(caixa);
            desenharBadge(p, centro, e.texto, e.cor, e.forte);
        }
    }

    void desenharArestas(QPainter &p)
    {
        QVector<Etiqueta> etiquetas;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
            {
                int peso = g->custo_conexao(i + 1, j + 1);
                if (peso <= 0 || (!dirigido && j < i))
                    continue;
                bool forte = arestaEmDestaque(i, j);
                QColor cor = forte ? kArestaDestaque : kArestaPadrao;
                QPen caneta(cor, forte ? 4.2 : 2.4);
                caneta.setCapStyle(Qt::RoundCap);
                p.setPen(caneta);
                p.setBrush(Qt::NoBrush);
                if (i == j)
                {
                    QPointF c = pos[i] + QPointF(0, -raio - 12);
                    p.drawEllipse(c, 15, 15);
                    etiquetas.append({QLineF(), c + QPointF(0, -26), false, 0.5, QString::number(peso), cor, forte});
                    continue;
                }
                QLineF l = linhaAresta(i, j);
                p.drawLine(l);
                if (dirigido && l.length() > 1)
                {
                    QPointF u = (l.p2() - l.p1()) / l.length();
                    QPointF perp(-u.y(), u.x());
                    QPointF ponta = l.p2() - u * (raio + 1);
                    QPolygonF seta;
                    seta << ponta << ponta - u * 13 + perp * 6.5 << ponta - u * 13 - perp * 6.5;
                    p.setPen(Qt::NoPen);
                    p.setBrush(cor);
                    p.drawPolygon(seta);
                }
                // em arestas dirigidas de ida e volta, o custo fica perto da origem
                bool oposta = dirigido && g->custo_conexao(j + 1, i + 1) > 0;
                etiquetas.append({l, QPointF(), true, oposta ? 0.32 : 0.5, QString::number(peso), cor, forte});
            }
        desenharEtiquetas(p, etiquetas);
    }

    void desenharVertices(QPainter &p)
    {
        QFont f = font();
        f.setPointSize(13);
        f.setBold(true);
        for (int i = 0; i < n; ++i)
        {
            QPointF c = pos[i];
            QColor cor = destaque.cores.value(i + 1, kVerticePadrao);
            bool ativo = hover == i || origem == i || arrastando == i;
            bool colorido = destaque.cores.contains(i + 1);
            if (ativo || colorido)
            {
                QRadialGradient brilho(c, raio * 2.0);
                QColor halo = ativo ? kAcento : cor;
                halo.setAlpha(ativo ? 120 : 90);
                brilho.setColorAt(0.45, halo);
                halo.setAlpha(0);
                brilho.setColorAt(1.0, halo);
                p.setPen(Qt::NoPen);
                p.setBrush(brilho);
                p.drawEllipse(c, raio * 2.0, raio * 2.0);
            }
            QRadialGradient g2(c - QPointF(7, 8), raio * 1.7);
            g2.setColorAt(0, cor.lighter(140));
            g2.setColorAt(1, cor.darker(125));
            p.setPen(QPen(origem == i ? kAcento : cor.lighter(165), origem == i ? 3.2 : 2.2));
            p.setBrush(g2);
            p.drawEllipse(c, raio, raio);
            p.setFont(f);
            p.setPen(cor.lightness() > 150 ? QColor("#0f172a") : QColor("#ffffff"));
            p.drawText(QRectF(c.x() - raio, c.y() - raio, raio * 2, raio * 2), Qt::AlignCenter, QString::number(i + 1));

            QString rotulo = destaque.rotulos.value(i + 1);
            if (!rotulo.isEmpty())
            {
                QFont fr = font();
                fr.setPointSize(10);
                fr.setBold(true);
                p.setFont(fr);
                QFontMetrics fm(fr);
                QRectF pilula(0, 0, fm.horizontalAdvance(rotulo) + 14, 18);
                pilula.moveCenter(QPointF(c.x(), c.y() + raio + 13));
                p.setPen(QPen(QColor("#334155"), 1));
                p.setBrush(QColor(15, 23, 42, 240));
                p.drawRoundedRect(pilula, 9, 9);
                p.setPen(QColor("#e2e8f0"));
                p.drawText(pilula, Qt::AlignCenter, rotulo);
            }
        }
    }
};

// ============================================================
// Janela principal
// ============================================================
enum class Op
{
    BFS, DFS, FechoDireto, FechoInverso, Componentes,
    Dijkstra, AEstrela,
    Coloracao, Ciclo3, Euler, Areas,
    Caixeiro
};

struct OpInfo
{
    Op op;
    QString nome;
    QString dica;
    bool inicio;
    bool fim;
    bool soNaoDirigido;
    bool passos;
};

struct Passo
{
    Destaque destaque;
    QString texto;
};

struct JanelaPrincipal : QMainWindow
{
    std::unique_ptr<grafo> g;
    int n = 0;
    bool dirigido = false;

    Canvas *canvas = nullptr;
    QPlainTextEdit *saida = nullptr;
    QTableWidget *tabela = nullptr;
    QLabel *estatistica = nullptr;
    QCheckBox *modoPassos = nullptr;
    QAction *acaoDirigido = nullptr;
    QFrame *barraPassos = nullptr;
    QLabel *rotuloPasso = nullptr;
    QPushButton *btnAnterior = nullptr, *btnProximo = nullptr, *btnReiniciar = nullptr;
    QVector<Passo> passos;
    int passoAtual = -1;
    QVector<OpInfo> operacoes;

    static Par par(int a, int b, bool dir)
    {
        if (!dir && a > b)
            std::swap(a, b);
        return Par(a, b);
    }

    JanelaPrincipal()
    {
        setWindowTitle("Grafos Studio");
        resize(1360, 860);
        g = std::make_unique<grafo>();
        g->inicializa_grafo(0, false);

        operacoes = {
            {Op::BFS, "Busca em largura (BFS)", "Percorre o grafo por niveis a partir de um vertice", true, false, false, true},
            {Op::DFS, "Busca em profundidade (DFS)", "Percorre o grafo em profundidade a partir de um vertice", true, false, false, true},
            {Op::FechoDireto, "Fecho transitivo direto", "Vertices alcancaveis a partir de um vertice", true, false, false, false},
            {Op::FechoInverso, "Fecho transitivo inverso", "Vertices que alcancam um vertice", true, false, false, false},
            {Op::Componentes, "Componentes conexos / SFCMs", "Componentes conexos (ou fortemente conexos)", false, false, false, false},
            {Op::Dijkstra, "Dijkstra", "Menores caminhos a partir de um vertice", true, false, false, true},
            {Op::AEstrela, "A* (heuristica zero)", "Menor caminho entre dois vertices", true, true, false, true},
            {Op::Coloracao, "Coloracao de vertices", "Coloracao propria (grafos nao dirigidos)", false, false, true, true},
            {Op::Ciclo3, "Ciclo de tamanho 3", "Verifica a existencia de triangulos", false, false, true, false},
            {Op::Euler, "Limites de Euler (planaridade)", "Verifica os limites de Euler", false, false, true, false},
            {Op::Areas, "Areas do grafo", "Calcula as areas (faces)", false, false, true, false},
            {Op::Caixeiro, "Vertice adjacente mais proximo", "Heuristica do vizinho mais proximo (grafo completo, nao dirigido)", true, false, true, true},
        };

        construirInterface();
        statusBar()->showMessage(dicaPadrao());
        atualizar();
    }

    static QString dicaPadrao()
    {
        return "Clique no espaco vazio para criar um vertice  |  clique em um vertice e depois em outro para conectar  |  arraste para mover  |  botao direito para opcoes";
    }

    // -------------------------------------------------------
    // Construcao da interface
    // -------------------------------------------------------
    void construirInterface()
    {
        // barra de ferramentas
        auto barra = addToolBar("Principal");
        barra->setMovable(false);
        barra->setToolButtonStyle(Qt::ToolButtonTextOnly);
        auto titulo = new QLabel("  GRAFOS <span style='color:#22d3ee'>STUDIO</span>  ");
        titulo->setObjectName("titulo");
        barra->addWidget(titulo);
        barra->addSeparator();
        barra->addAction("Novo", this, [this] { novoGrafo(); });
        barra->addAction("Abrir...", this, [this] { abrirArquivo(); });
        barra->addAction("Salvar...", this, [this] { salvarArquivo(); });
        barra->addSeparator();
        acaoDirigido = barra->addAction("Grafo dirigido");
        acaoDirigido->setCheckable(true);
        acaoDirigido->setToolTip("Alterna entre grafo dirigido e nao dirigido (mantem as conexoes)");
        connect(acaoDirigido, &QAction::toggled, this, [this](bool v) { alternarDirigido(v); });
        barra->addAction("Organizar em circulo", this, [this] { organizarCirculo(); });
        barra->addAction("Limpar destaques", this, [this] { limparResultado(true); });

        // painel esquerdo
        auto esquerda = new QWidget;
        esquerda->setObjectName("painelEsquerdo");
        auto esqLayout = new QVBoxLayout(esquerda);
        esqLayout->setContentsMargins(14, 14, 14, 14);
        esqLayout->setSpacing(6);

        modoPassos = new QCheckBox("Executar passo a passo");
        modoPassos->setChecked(true);
        modoPassos->setToolTip("Mostra a execucao do algoritmo em etapas sobre o desenho do grafo");

        auto grupo = [&](const QString &nome, std::initializer_list<Op> ops) {
            auto rotulo = new QLabel(nome);
            rotulo->setObjectName("grupo");
            esqLayout->addSpacing(6);
            esqLayout->addWidget(rotulo);
            for (Op op : ops)
                for (const auto &info : operacoes)
                    if (info.op == op)
                    {
                        auto b = new QPushButton(info.nome);
                        b->setObjectName("operacao");
                        b->setToolTip(info.dica);
                        b->setCursor(Qt::PointingHandCursor);
                        esqLayout->addWidget(b);
                        Op o = info.op;
                        connect(b, &QPushButton::clicked, this, [this, o] { acionarOperacao(o); });
                    }
        };
        auto cab = new QLabel("ALGORITMOS");
        cab->setObjectName("secao");
        esqLayout->addWidget(cab);
        esqLayout->addWidget(modoPassos);
        grupo("Percursos", {Op::BFS, Op::DFS, Op::FechoDireto, Op::FechoInverso, Op::Componentes});
        grupo("Caminhos minimos", {Op::Dijkstra, Op::AEstrela});
        grupo("Propriedades", {Op::Coloracao, Op::Ciclo3, Op::Euler, Op::Areas});
        grupo("Caixeiro viajante", {Op::Caixeiro});
        esqLayout->addStretch();
        auto scroll = new QScrollArea;
        scroll->setWidget(esquerda);
        scroll->setWidgetResizable(true);
        scroll->setFixedWidth(290);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setObjectName("scrollEsquerdo");

        // area do canvas
        canvas = new Canvas;
        canvas->g = g.get();
        canvas->definirDicaPadrao(dicaPadrao());
        canvas->aoDica = [this](const QString &t) { statusBar()->showMessage(t); };
        canvas->aoCriarVertice = [this](QPointF p) { adicionarVertice(p); };
        canvas->aoConectar = [this](int a, int b) { conectar(a, b); };
        canvas->aoEditarAresta = [this](int a, int b) { conectar(a, b); };
        canvas->aoMenuVertice = [this](int v, QPoint gp) { menuVertice(v, gp); };
        canvas->aoMenuAresta = [this](int a, int b, QPoint gp) { menuAresta(a, b, gp); };

        barraPassos = new QFrame;
        barraPassos->setObjectName("barraPassos");
        auto bl = new QHBoxLayout(barraPassos);
        bl->setContentsMargins(14, 8, 14, 8);
        rotuloPasso = new QLabel("Passo");
        rotuloPasso->setObjectName("passo");
        btnAnterior = new QPushButton("<  Anterior");
        btnProximo = new QPushButton("Proximo  >");
        btnProximo->setObjectName("primario");
        btnReiniciar = new QPushButton("Reiniciar");
        auto btnConcluir = new QPushButton("Encerrar");
        bl->addWidget(rotuloPasso, 1);
        bl->addWidget(btnAnterior);
        bl->addWidget(btnProximo);
        bl->addWidget(btnReiniciar);
        bl->addWidget(btnConcluir);
        barraPassos->hide();
        connect(btnProximo, &QPushButton::clicked, this, [this] { irParaPasso(passoAtual + 1); });
        connect(btnAnterior, &QPushButton::clicked, this, [this] { irParaPasso(passoAtual - 1); });
        connect(btnReiniciar, &QPushButton::clicked, this, [this] { irParaPasso(0); });
        connect(btnConcluir, &QPushButton::clicked, this, [this] { limparResultado(true); });

        auto areaCanvas = new QWidget;
        auto acl = new QVBoxLayout(areaCanvas);
        acl->setContentsMargins(0, 0, 0, 0);
        acl->setSpacing(0);
        acl->addWidget(canvas, 1);
        acl->addWidget(barraPassos);

        // abas inferiores
        auto abas = new QTabWidget;
        saida = new QPlainTextEdit;
        saida->setReadOnly(true);
        saida->setPlaceholderText("O resultado dos algoritmos aparece aqui.");
        saida->setFont(QFont("monospace", 11));
        tabela = new QTableWidget;
        tabela->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tabela->setSelectionMode(QAbstractItemView::NoSelection);
        tabela->horizontalHeader()->setDefaultSectionSize(44);
        tabela->verticalHeader()->setDefaultSectionSize(30);
        abas->addTab(saida, "Resultado");
        abas->addTab(tabela, "Matriz de adjacencia");

        auto divisor = new QSplitter(Qt::Vertical);
        divisor->addWidget(areaCanvas);
        divisor->addWidget(abas);
        divisor->setStretchFactor(0, 4);
        divisor->setStretchFactor(1, 1);
        divisor->setSizes({620, 200});

        auto central = new QWidget;
        auto cl = new QHBoxLayout(central);
        cl->setContentsMargins(0, 0, 0, 0);
        cl->setSpacing(0);
        cl->addWidget(scroll);
        cl->addWidget(divisor, 1);
        setCentralWidget(central);

        estatistica = new QLabel;
        estatistica->setObjectName("estatistica");
        statusBar()->addPermanentWidget(estatistica);

        setStyleSheet(estilo());
    }

    static QString estilo()
    {
        return R"(
QWidget { font-size: 13px; color: #e5e7eb; }
QMainWindow, QWidget#painelEsquerdo, QScrollArea#scrollEsquerdo { background: #0f172a; }
QToolBar { background: #0b1220; border: none; border-bottom: 1px solid #1e293b; padding: 6px 10px; spacing: 6px; }
QToolBar QLabel#titulo { font-size: 15px; font-weight: 800; letter-spacing: 2px; color: #f8fafc; }
QToolButton { background: transparent; border: 1px solid transparent; border-radius: 8px; padding: 7px 14px; color: #cbd5e1; font-weight: 600; }
QToolButton:hover { background: #1e293b; color: #ffffff; }
QToolButton:checked { background: #4338ca; color: #ffffff; border-color: #6366f1; }
QLabel#secao { font-size: 11px; font-weight: 800; letter-spacing: 2px; color: #64748b; padding-bottom: 4px; }
QLabel#grupo { font-size: 12px; font-weight: 700; color: #22d3ee; padding-top: 6px; }
QPushButton { background: #1e293b; border: 1px solid #334155; border-radius: 8px; padding: 8px 14px; color: #e2e8f0; font-weight: 600; }
QPushButton:hover { background: #273449; border-color: #475569; }
QPushButton:pressed { background: #0f172a; }
QPushButton:disabled { color: #475569; background: #172033; border-color: #1e293b; }
QPushButton#operacao { text-align: left; padding: 9px 12px; background: #131c31; }
QPushButton#operacao:hover { background: #1e2a4a; border-color: #6366f1; color: #ffffff; }
QPushButton#primario { background: #4f46e5; border-color: #6366f1; color: white; }
QPushButton#primario:hover { background: #6366f1; }
QPushButton#primario:disabled { background: #1e1b4b; color: #4b5563; border-color: #312e81; }
QCheckBox { spacing: 8px; color: #cbd5e1; padding: 4px 0 6px 0; }
QCheckBox::indicator { width: 18px; height: 18px; border-radius: 5px; border: 1px solid #475569; background: #0b1220; }
QCheckBox::indicator:checked { background: #6366f1; border-color: #818cf8; }
QFrame#barraPassos { background: #0b1220; border-top: 1px solid #1e293b; }
QLabel#passo { font-weight: 700; color: #fde68a; }
QTabWidget::pane { border: none; border-top: 1px solid #1e293b; background: #0b1220; }
QTabBar::tab { background: transparent; color: #94a3b8; padding: 9px 18px; border: none; font-weight: 600; }
QTabBar::tab:selected { color: #ffffff; border-bottom: 2px solid #22d3ee; }
QPlainTextEdit, QTableWidget { background: #0b1220; border: none; color: #e2e8f0; selection-background-color: #3730a3; gridline-color: #1e293b; }
QHeaderView::section { background: #111a2e; color: #94a3b8; border: none; border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b; padding: 4px; font-weight: 700; }
QStatusBar { background: #0b1220; color: #94a3b8; border-top: 1px solid #1e293b; }
QLabel#estatistica { color: #22d3ee; font-weight: 700; padding-right: 10px; }
QSplitter::handle { background: #1e293b; height: 2px; }
QScrollBar:vertical { background: transparent; width: 10px; }
QScrollBar::handle:vertical { background: #334155; border-radius: 5px; min-height: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; }
QMenu { background: #111a2e; border: 1px solid #334155; border-radius: 8px; padding: 6px; }
QMenu::item { padding: 7px 22px; border-radius: 5px; }
QMenu::item:selected { background: #4338ca; }
QMenu::item:disabled { color: #22d3ee; font-weight: 700; }
QMenu::separator { height: 1px; background: #1e293b; margin: 5px 8px; }
QDialog, QMessageBox, QInputDialog { background: #0f172a; }
QSpinBox { background: #0b1220; border: 1px solid #334155; border-radius: 6px; padding: 6px; }
QToolTip { background: #111a2e; color: #e2e8f0; border: 1px solid #334155; padding: 4px; }
)";
    }

    // -------------------------------------------------------
    // Atualizacao da tela
    // -------------------------------------------------------
    int contarArestas() const
    {
        int total = 0;
        for (int i = 0; i < n; ++i)
            for (int j = dirigido ? 0 : i; j < n; ++j)
                if (g->custo_conexao(i + 1, j + 1) > 0)
                    ++total;
        return total;
    }

    void atualizar()
    {
        canvas->g = g.get();
        canvas->n = n;
        canvas->dirigido = dirigido;
        estatistica->setText(QString("%1 vertices  |  %2 arestas  |  %3").arg(n).arg(contarArestas()).arg(dirigido ? "Dirigido" : "Nao dirigido"));
        tabela->setRowCount(n);
        tabela->setColumnCount(n);
        QStringList nomes;
        for (int i = 0; i < n; ++i)
            nomes << QString::number(i + 1);
        tabela->setHorizontalHeaderLabels(nomes);
        tabela->setVerticalHeaderLabels(nomes);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
            {
                int w = g->custo_conexao(i + 1, j + 1);
                auto item = new QTableWidgetItem(w > 0 ? QString::number(w) : "-");
                item->setTextAlignment(Qt::AlignCenter);
                if (w > 0)
                {
                    item->setBackground(QColor("#312e81"));
                    item->setForeground(QColor("#ffffff"));
                }
                else
                    item->setForeground(QColor("#475569"));
                tabela->setItem(i, j, item);
            }
        canvas->update();
    }

    void limparResultado(bool limparTexto)
    {
        passos.clear();
        passoAtual = -1;
        barraPassos->hide();
        canvas->destaque = Destaque();
        if (limparTexto)
            saida->clear();
        canvas->update();
    }

    void aviso(const QString &titulo, const QString &texto)
    {
        QMessageBox::information(this, titulo, texto);
    }

    // -------------------------------------------------------
    // Edicao do grafo
    // -------------------------------------------------------
    void adicionarVertice(QPointF p)
    {
        if (n >= 100)
        {
            aviso("Limite atingido", "A interface suporta no maximo 100 vertices.");
            return;
        }
        limparResultado(false);
        g->cria_vertice();
        ++n;
        canvas->pos.append(p);
        atualizar();
    }

    void removerVertice(int v)
    {
        limparResultado(true);
        g->remove_vertice(v + 1);
        --n;
        canvas->pos.removeAt(v);
        atualizar();
    }

    void conectar(int a, int b)
    {
        int atual = g->custo_conexao(a + 1, b + 1);
        bool ok = false;
        QString seta = dirigido ? " -> " : " - ";
        int custo = QInputDialog::getInt(this, atual > 0 ? "Editar conexao" : "Nova conexao",
                                         QString("Custo da conexao %1%2%3 (1 a 254):").arg(a + 1).arg(seta).arg(b + 1),
                                         atual > 0 ? atual : 1, 1, 254, 1, &ok);
        if (!ok)
            return;
        limparResultado(false);
        if (atual > 0)
            g->remove_conexao(a + 1, b + 1);
        g->cria_conexao(a + 1, b + 1, custo);
        atualizar();
    }

    void removerConexao(int a, int b)
    {
        limparResultado(false);
        g->remove_conexao(a + 1, b + 1);
        atualizar();
    }

    void menuVertice(int v, QPoint onde)
    {
        QMenu menu(this);
        auto cab = menu.addAction(QString("Vertice %1").arg(v + 1));
        cab->setEnabled(false);
        menu.addSeparator();
        auto remover = menu.addAction("Remover vertice");
        menu.addSeparator();
        QMap<QAction *, Op> mapa;
        for (const auto &info : operacoes)
            if (info.inicio)
                mapa[menu.addAction("Iniciar aqui: " + info.nome)] = info.op;
        QAction *escolhida = menu.exec(onde);
        if (!escolhida)
            return;
        if (escolhida == remover)
            removerVertice(v);
        else if (mapa.contains(escolhida))
            acionarOperacao(mapa[escolhida], v + 1);
    }

    void menuAresta(int a, int b, QPoint onde)
    {
        QMenu menu(this);
        QString seta = dirigido ? " -> " : " - ";
        auto cab = menu.addAction(QString("Aresta %1%2%3  (custo %4)").arg(a + 1).arg(seta).arg(b + 1).arg(g->custo_conexao(a + 1, b + 1)));
        cab->setEnabled(false);
        menu.addSeparator();
        auto editar = menu.addAction("Editar custo...");
        auto remover = menu.addAction("Remover aresta");
        QAction *escolhida = menu.exec(onde);
        if (escolhida == editar)
            conectar(a, b);
        else if (escolhida == remover)
            removerConexao(a, b);
    }

    void organizarCirculo()
    {
        if (n == 0)
            return;
        QPointF centro(canvas->width() / 2.0, canvas->height() / 2.0);
        double r = qMax(40.0, qMin(canvas->width(), canvas->height()) / 2.0 - 70);
        for (int i = 0; i < n; ++i)
        {
            double ang = 6.283185307 * i / n - 1.5707963;
            canvas->pos[i] = centro + QPointF(r * std::cos(ang), r * std::sin(ang));
        }
        canvas->update();
    }

    void novoGrafo()
    {
        if (n > 0 && QMessageBox::question(this, "Novo grafo", "Descartar o grafo atual?") != QMessageBox::Yes)
            return;
        substituirGrafo(std::make_unique<grafo>(), 0, dirigido);
    }

    void substituirGrafo(std::unique_ptr<grafo> novo, int vertices, bool dir)
    {
        limparResultado(true);
        g = std::move(novo);
        n = vertices;
        dirigido = dir;
        canvas->pos.clear();
        canvas->pos.resize(n);
        canvas->g = g.get();
        canvas->n = n;
        canvas->dirigido = dir;
        QSignalBlocker bloqueio(acaoDirigido);
        acaoDirigido->setChecked(dir);
        organizarCirculo();
        atualizar();
    }

    void alternarDirigido(bool novoDirigido)
    {
        auto novo = std::make_unique<grafo>();
        novo->inicializa_grafo(n, novoDirigido);
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
            {
                int w = g->custo_conexao(i + 1, j + 1);
                if (novoDirigido)
                {
                    if (w > 0)
                        novo->cria_conexao(i + 1, j + 1, w);
                }
                else
                {
                    if (j < i)
                        continue;
                    int w2 = w > 0 ? w : g->custo_conexao(j + 1, i + 1);
                    if (w2 > 0)
                        novo->cria_conexao(i + 1, j + 1, w2);
                }
            }
        limparResultado(true);
        g = std::move(novo);
        dirigido = novoDirigido;
        atualizar();
    }

    // -------------------------------------------------------
    // Arquivos (formato: "vertices tipo" e depois "origem destino custo")
    // -------------------------------------------------------
    void abrirArquivo()
    {
        QString caminho = QFileDialog::getOpenFileName(this, "Abrir grafo", QString(), "Grafos (*.txt);;Todos os arquivos (*)");
        if (caminho.isEmpty())
            return;
        QFile arquivo(caminho);
        if (!arquivo.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Erro", "Nao foi possivel abrir o arquivo.");
            return;
        }
        std::istringstream dados(arquivo.readAll().toStdString());
        int vertices, tipo;
        if (!(dados >> vertices >> tipo) || vertices < 0 || vertices > 100 || (tipo != 0 && tipo != 1))
        {
            QMessageBox::warning(this, "Arquivo invalido", "Primeira linha: quantidade de vertices (0 a 100) e tipo (0 nao dirigido, 1 dirigido).");
            return;
        }
        auto novo = std::make_unique<grafo>();
        novo->inicializa_grafo(vertices, tipo == 1);
        while (true)
        {
            dados >> std::ws;
            if (dados.eof())
                break;
            int a, b, peso;
            if (!(dados >> a >> b >> peso) || !novo->vertice_valido(a) || !novo->vertice_valido(b) || peso < 1 || peso > 254)
            {
                QMessageBox::warning(this, "Arquivo invalido", "Cada conexao deve conter: origem destino custo (1 a 254).");
                return;
            }
            if (novo->custo_conexao(a, b) > 0)
                novo->remove_conexao(a, b);
            novo->cria_conexao(a, b, peso);
        }
        substituirGrafo(std::move(novo), vertices, tipo == 1);
        saida->setPlainText("Grafo importado: " + caminho);
    }

    void salvarArquivo()
    {
        QString caminho = QFileDialog::getSaveFileName(this, "Salvar grafo", "grafo.txt", "Grafos (*.txt)");
        if (caminho.isEmpty())
            return;
        QFile arquivo(caminho);
        if (!arquivo.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Erro", "Nao foi possivel salvar o arquivo.");
            return;
        }
        QTextStream out(&arquivo);
        out << n << " " << (dirigido ? 1 : 0) << "\n";
        for (int i = 0; i < n; ++i)
            for (int j = dirigido ? 0 : i; j < n; ++j)
            {
                int w = g->custo_conexao(i + 1, j + 1);
                if (w > 0)
                    out << i + 1 << " " << j + 1 << " " << w << "\n";
            }
        statusBar()->showMessage("Grafo salvo em " + caminho, 5000);
    }

    // -------------------------------------------------------
    // Execucao dos algoritmos
    // -------------------------------------------------------
    const OpInfo &info(Op op) const
    {
        for (const auto &i : operacoes)
            if (i.op == op)
                return i;
        return operacoes[0];
    }

    // Pergunta o(s) vertice(s) necessarios; "a" ja preenchido (>0) nao e perguntado.
    bool pedirVertices(const OpInfo &oi, int &a, int &b)
    {
        bool pedeInicio = oi.inicio && a <= 0;
        if (!pedeInicio && !oi.fim)
            return true;
        QDialog dialogo(this);
        dialogo.setWindowTitle(oi.nome);
        auto campos = new QFormLayout(&dialogo);
        QSpinBox sa, sb;
        sa.setRange(1, n);
        sb.setRange(1, n);
        sa.setValue(a > 0 ? a : 1);
        sb.setValue(n);
        if (pedeInicio)
            campos->addRow("Vertice inicial", &sa);
        if (oi.fim)
            campos->addRow("Vertice destino", &sb);
        if (oi.op == Op::AEstrela)
            campos->addRow(new QLabel("A* usa heuristica zero nesta versao."));
        auto botoes = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        campos->addRow(botoes);
        connect(botoes, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
        connect(botoes, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
        if (dialogo.exec() != QDialog::Accepted)
            return false;
        if (pedeInicio)
            a = sa.value();
        b = sb.value();
        return true;
    }

    bool grafoCompleto() const
    {
        for (int i = 1; i <= n; ++i)
            for (int j = 1; j <= n; ++j)
                if (i != j && g->custo_conexao(i, j) <= 0)
                    return false;
        return true;
    }

    void acionarOperacao(Op op, int inicio = 0)
    {
        const OpInfo &oi = info(op);
        if (n == 0)
        {
            aviso("Grafo vazio", "Crie ao menos um vertice clicando na area de desenho.");
            return;
        }
        if (dirigido && oi.soNaoDirigido)
        {
            aviso("Operacao indisponivel", "Esta operacao requer um grafo nao dirigido.");
            return;
        }
        if (op == Op::Caixeiro)
        {
            if (n < 2)
            {
                aviso("Caixeiro viajante", "O grafo precisa ter ao menos 2 vertices.");
                return;
            }
            if (!grafoCompleto())
            {
                aviso("Caixeiro viajante", "A heuristica requer um grafo completo: todos os pares de vertices precisam estar conectados.");
                return;
            }
        }
        int a = inicio, b = 0;
        if (!pedirVertices(oi, a, b))
            return;
        limparResultado(false);
        if (modoPassos->isChecked() && oi.passos)
            gerarPassos(op, a, b);
        else
            executarDireto(op, a, b);
    }

    // Executa a funcao da biblioteca capturando o que ela imprime.
    QString capturar(const std::function<void()> &f)
    {
        std::ostringstream resultado;
        std::string zeros;
        for (int i = 0; i < n + 1; ++i)
            zeros += "0 ";
        std::istringstream entrada(zeros);
        auto cout_antigo = cout.rdbuf(resultado.rdbuf());
        auto cin_antigo = cin.rdbuf(entrada.rdbuf());
        f();
        cout.rdbuf(cout_antigo);
        cin.rdbuf(cin_antigo);
        return QString::fromStdString(resultado.str());
    }

    static QColor corDaClasse(int c) { return QColor::fromHsv((c - 1) * 137 % 360, 135, 245); }

    void executarDireto(Op op, int a, int b)
    {
        QString texto = capturar([&] {
            switch (op)
            {
            case Op::BFS: g->bfs(a); break;
            case Op::DFS: g->dfs(a); break;
            case Op::FechoDireto: g->fecho_transitivo_direto(a); break;
            case Op::FechoInverso: g->fecho_transitivo_inverso(a); break;
            case Op::Componentes:
                if (dirigido) g->componentes_fortemente_conexos(); else g->componentes_conexos();
                break;
            case Op::Dijkstra: g->busca_djikstra(a); break;
            case Op::AEstrela: g->a_estrela(a, b); break;
            case Op::Coloracao: g->coloracao_grafo(); break;
            case Op::Ciclo3:
                cout << (g->is_ciclo_3() ? "Existe ciclo de tamanho 3." : "Nao existe ciclo de tamanho 3.");
                break;
            case Op::Euler:
                cout << (g->planar_euler() ? "Satisfaz os limites de Euler; nao garante planaridade." : "Nao satisfaz os limites implementados.");
                break;
            case Op::Areas: g->areas_grafo(); break;
            case Op::Caixeiro: g->caixeiro_viajante_VADJPROX(a); break;
            }
        });
        Destaque d;
        if (op == Op::Coloracao)
        {
            QRegularExpression padrao("Vertice\\s+(\\d+):\\s+cor\\s+(\\d+)");
            auto it = padrao.globalMatch(texto);
            while (it.hasNext())
            {
                auto m = it.next();
                d.cores[m.captured(1).toInt()] = corDaClasse(m.captured(2).toInt());
            }
        }
        else if (op == Op::Caixeiro)
        {
            QVector<int> caminho = lerCaminho(texto);
            for (int k = 0; k < caminho.size(); ++k)
            {
                d.cores[caminho[k]] = kCaminho;
                if (k + 1 < caminho.size())
                    d.arestas.insert(par(caminho[k], caminho[k + 1], dirigido));
            }
            d.cores[a] = kAtual;
        }
        canvas->destaque = d;
        saida->setPlainText(texto);
        canvas->update();
    }

    static QVector<int> lerCaminho(const QString &texto)
    {
        QVector<int> caminho;
        QRegularExpression padrao("Caminho:\\s*(.*)");
        auto m = padrao.match(texto);
        if (!m.hasMatch())
            return caminho;
        for (const QString &parte : m.captured(1).split("->"))
            caminho.append(parte.trimmed().toInt());
        return caminho;
    }

    // -------------------------------------------------------
    // Passo a passo
    // -------------------------------------------------------
    void irParaPasso(int i)
    {
        if (i < 0 || i >= passos.size())
            return;
        passoAtual = i;
        canvas->destaque = passos[i].destaque;
        saida->setPlainText(passos[i].texto);
        rotuloPasso->setText(QString("Passo %1 de %2").arg(i + 1).arg(passos.size()));
        btnAnterior->setEnabled(i > 0);
        btnProximo->setEnabled(i + 1 < passos.size());
        btnReiniciar->setEnabled(i > 0);
        barraPassos->show();
        canvas->update();
    }

    void gerarPassos(Op op, int inicio, int fim)
    {
        Destaque d;
        QVector<Passo> lista;
        auto registrar = [&](const QString &texto) { lista.append({d, texto}); };
        registrar("Estado inicial.\nAmarelo: descoberto  |  laranja: atual  |  verde: processado.");

        if (op == Op::BFS || op == Op::DFS)
        {
            bool bfs = op == Op::BFS;
            QVector<bool> visitado(n, false);
            QList<Par> pendentes; // (vertice, pai)
            QStringList percurso;
            auto componente = [&](int raiz) {
                pendentes.append({raiz, -1});
                if (bfs)
                    visitado[raiz] = true;
                d.cores[raiz + 1] = kAberto;
                registrar(QString("Descobrir vertice %1.").arg(raiz + 1));
                while (!pendentes.isEmpty())
                {
                    Par item = bfs ? pendentes.takeFirst() : pendentes.takeLast();
                    int v = item.first;
                    if (!bfs && visitado[v])
                        continue;
                    visitado[v] = true;
                    if (!bfs && item.second >= 0)
                        d.arestas.insert(par(item.second + 1, v + 1, dirigido));
                    d.cores[v + 1] = kAtual;
                    percurso << QString::number(v + 1);
                    registrar(QString("Processar %1.\nPercurso: %2").arg(v + 1).arg(percurso.join(" -> ")));
                    for (int k = 0; k < n; ++k)
                    {
                        int u = bfs ? k : n - 1 - k;
                        if (g->custo_conexao(v + 1, u + 1) > 0 && !visitado[u])
                        {
                            if (bfs)
                            {
                                visitado[u] = true;
                                d.arestas.insert(par(v + 1, u + 1, dirigido));
                            }
                            pendentes.append({u, v});
                            d.cores[u + 1] = kAberto;
                            registrar(QString("%1: inserir %2.\nPendentes: %3").arg(bfs ? "Fila" : "Pilha").arg(u + 1).arg(pendentes.size()));
                        }
                    }
                    d.cores[v + 1] = kFechado;
                    registrar(QString("Finalizar %1.").arg(v + 1));
                }
            };
            componente(inicio - 1);
            for (int v = 0; v < n; ++v)
                if (!visitado[v])
                    componente(v);
            registrar("Percurso concluido: " + percurso.join(" -> "));
        }
        else if (op == Op::Coloracao)
        {
            QVector<int> cor(n, 0), grau(n, 0);
            bool laco = false;
            for (int v = 0; v < n && !laco; ++v)
            {
                if (g->custo_conexao(v + 1, v + 1) > 0)
                    laco = true;
                for (int u = 0; u < n; ++u)
                    if (g->custo_conexao(v + 1, u + 1) > 0)
                        ++grau[v];
            }
            if (laco)
                registrar("Grafo com laco: coloracao propria impossivel.");
            else
            {
                for (int k = 0; k < n; ++k)
                {
                    int melhor = -1, sat = -1;
                    for (int v = 0; v < n; ++v)
                        if (!cor[v])
                        {
                            QSet<int> vizinhas;
                            for (int u = 0; u < n; ++u)
                                if (g->custo_conexao(v + 1, u + 1) > 0 && cor[u])
                                    vizinhas.insert(cor[u]);
                            if (melhor == -1 || vizinhas.size() > sat || (vizinhas.size() == sat && grau[v] > grau[melhor]))
                            {
                                melhor = v;
                                sat = vizinhas.size();
                            }
                        }
                    d.cores[melhor + 1] = kAtual;
                    registrar(QString("Selecionar %1: saturacao %2, grau %3.").arg(melhor + 1).arg(sat).arg(grau[melhor]));
                    QSet<int> usadas;
                    for (int u = 0; u < n; ++u)
                        if (g->custo_conexao(melhor + 1, u + 1) > 0 && cor[u])
                            usadas.insert(cor[u]);
                    int c = 1;
                    while (usadas.contains(c))
                        ++c;
                    cor[melhor] = c;
                    d.cores[melhor + 1] = corDaClasse(c);
                    registrar(QString("Atribuir cor %1 ao vertice %2.").arg(c).arg(melhor + 1));
                }
                registrar("Coloracao concluida.");
            }
        }
        else if (op == Op::Dijkstra || op == Op::AEstrela)
        {
            bool dij = op == Op::Dijkstra;
            int infinito = dij ? 255 : std::numeric_limits<int>::max();
            QVector<int> distancia(n, infinito), anterior(n, -1);
            QVector<bool> pronto(n, false);
            distancia[inicio - 1] = 0;
            anterior[inicio - 1] = inicio - 1;
            auto rotular = [&] {
                for (int v = 0; v < n; ++v)
                    d.rotulos[v + 1] = distancia[v] == infinito ? QString("inf") : QString("d=%1").arg(distancia[v]);
            };
            auto tabelaTexto = [&] {
                QString t = "\nVertice | Custo | Antecessor\n";
                for (int v = 0; v < n; ++v)
                    t += QString("%1 | %2 | %3\n").arg(v + 1).arg(distancia[v] == infinito ? "inf" : QString::number(distancia[v])).arg(anterior[v] < 0 ? "-" : QString::number(anterior[v] + 1));
                return t;
            };
            rotular();
            d.cores[inicio] = kAberto;
            registrar((dij ? "Dijkstra: iniciar custos (limite original: 254)." : "A*: h=0 para todos os vertices.") + tabelaTexto());
            bool achou = false;
            for (int k = 0; k < n; ++k)
            {
                int v = -1;
                for (int u = 0; u < n; ++u)
                    if (!pronto[u] && distancia[u] < infinito && (v == -1 || distancia[u] < distancia[v]))
                        v = u;
                if (v == -1)
                    break;
                d.cores[v + 1] = kAtual;
                registrar(QString("Selecionar vertice %1 de menor custo.").arg(v + 1) + tabelaTexto());
                if (!dij && v == fim - 1)
                {
                    d.arestas.clear();
                    QStringList caminho;
                    for (int u = v;; u = anterior[u])
                    {
                        caminho.prepend(QString::number(u + 1));
                        d.cores[u + 1] = kCaminho;
                        if (u == inicio - 1)
                            break;
                        d.arestas.insert(par(anterior[u] + 1, u + 1, dirigido));
                    }
                    registrar("Destino encontrado: " + caminho.join(" -> ") + QString("\nCusto total: %1").arg(distancia[v]) + tabelaTexto());
                    achou = true;
                    break;
                }
                pronto[v] = true;
                for (int u = 0; u < n; ++u)
                {
                    int peso = g->custo_conexao(v + 1, u + 1);
                    if (peso > 0 && !pronto[u] && distancia[v] + peso < distancia[u])
                    {
                        if (anterior[u] >= 0 && anterior[u] != u)
                            d.arestas.erase(par(anterior[u] + 1, u + 1, dirigido));
                        distancia[u] = distancia[v] + peso;
                        anterior[u] = v;
                        d.arestas.insert(par(v + 1, u + 1, dirigido));
                        d.cores[u + 1] = kAberto;
                        rotular();
                        registrar(QString("Relaxar %1 -> %2: atualizar custo e antecessor.").arg(v + 1).arg(u + 1) + tabelaTexto());
                    }
                }
                d.cores[v + 1] = kFechado;
                registrar(QString("Fechar vertice %1.").arg(v + 1) + tabelaTexto());
            }
            if (!dij && !achou)
                registrar("Destino inalcancavel." + tabelaTexto());
            else if (dij)
                registrar("Busca concluida." + tabelaTexto());
        }
        else if (op == Op::Caixeiro)
        {
            // O percurso vem da propria funcao da biblioteca; os passos apenas o revelam.
            QString texto = capturar([&] { g->caixeiro_viajante_VADJPROX(inicio); });
            QVector<int> caminho = lerCaminho(texto);
            int acumulado = 0;
            d.cores[inicio] = kAtual;
            registrar(QString("Partir do vertice %1.").arg(inicio));
            for (int k = 1; k < caminho.size(); ++k)
            {
                int custoAresta = g->custo_conexao(caminho[k - 1], caminho[k]);
                acumulado += custoAresta;
                d.cores[caminho[k - 1]] = kCaminho;
                d.cores[caminho[k]] = (k + 1 == caminho.size()) ? kAtual : kAberto;
                d.arestas.insert(par(caminho[k - 1], caminho[k], dirigido));
                QString texto2 = (k + 1 == caminho.size())
                                     ? QString("Voltar ao vertice inicial %1 (custo %2).").arg(caminho[k]).arg(custoAresta)
                                     : QString("Vizinho mais proximo de %1: vertice %2 (custo %3).").arg(caminho[k - 1]).arg(caminho[k]).arg(custoAresta);
                d.rotulos[caminho[k]] = QString("+%1").arg(custoAresta);
                registrar(texto2 + QString("\nCusto acumulado: %1").arg(acumulado));
            }
            registrar("Ciclo concluido.\n" + texto);
        }

        passos = lista;
        irParaPasso(0);
    }
};
} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    JanelaPrincipal janela;
    janela.show();
    return app.exec();
}
