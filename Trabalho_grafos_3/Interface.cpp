// Grafos Studio - interface grafica (Qt6) do criador de grafos.
//
// Uso do editor:
//   * clique em uma area vazia ........ cria um vertice
//   * clique em um vertice ............ inicia uma conexao (linha acompanha o mouse)
//   * clique em outro vertice ......... conclui e pede o custo da conexao
//   * arraste um vertice .............. move o vertice
//   * duplo clique em uma aresta ...... edita o custo
//   * botao direito ................... menu do vertice / da aresta
//   * Esc ............................. cancela a conexao em andamento
//   * Ctrl+N / Ctrl+O / Ctrl+S ........ novo / abrir / salvar
//   * Setas e Espaco .................. navegam no passo a passo
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

// ---------- paleta (laranja + branco, grafite como contraste) ----------
const QColor kVerticePadrao("#f97316");
const QColor kAberto("#fbbf24");
const QColor kAtual("#9a3412");
const QColor kFechado("#0f766e");
const QColor kCaminho("#292524");
const QColor kArestaPadrao("#d6c7b8");
const QColor kArestaDestaque("#ea580c");
const QColor kAcento("#c2410c");
const QColor kTexto("#292524");
const QColor kTextoSuave("#78716c");

using Legenda = QVector<QPair<QColor, QString>>;

// Estado visual de um passo: cores dos vertices, arestas em destaque e rotulos.
// Tempos de uma atividade no caminho critico (mostrados ao redor do vertice).
struct DadosCpm
{
    int duracao = 0;
    int inicioCedo = 0, fimCedo = 0;
    int inicioTarde = 0, fimTarde = 0;
    int folga = 0;
};

struct Destaque
{
    QMap<int, QColor> cores;    // vertice (base 1) -> cor
    std::set<Par> arestas;      // arestas em destaque (base 1)
    QMap<int, QString> rotulos; // texto exibido abaixo do vertice
    QMap<int, DadosCpm> cpm;    // vertice (base 1) -> tempos do caminho critico
    Legenda legenda;            // explicacao das cores
    bool vazio() const { return cores.isEmpty() && arestas.empty() && rotulos.isEmpty() && cpm.isEmpty(); }
};

// ---------- icones desenhados em codigo (sem arquivos externos) ----------
enum class Ico { Play, Pause, Anterior, Proximo, Reiniciar, Circulo, Limpar };

QPixmap desenharIcone(Ico t, const QColor &cor, int tam)
{
    const qreal dpr = 2.0;
    QPixmap pm(int(tam * dpr), int(tam * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const double s = tam;
    p.setPen(Qt::NoPen);
    p.setBrush(cor);
    switch (t)
    {
    case Ico::Play:
        p.drawPolygon(QPolygonF() << QPointF(s * .30, s * .18) << QPointF(s * .30, s * .82) << QPointF(s * .82, s * .50));
        break;
    case Ico::Pause:
        p.drawRoundedRect(QRectF(s * .24, s * .18, s * .17, s * .64), 1.5, 1.5);
        p.drawRoundedRect(QRectF(s * .59, s * .18, s * .17, s * .64), 1.5, 1.5);
        break;
    case Ico::Anterior:
        p.drawRoundedRect(QRectF(s * .20, s * .20, s * .12, s * .60), 1.5, 1.5);
        p.drawPolygon(QPolygonF() << QPointF(s * .80, s * .20) << QPointF(s * .80, s * .80) << QPointF(s * .40, s * .50));
        break;
    case Ico::Proximo:
        p.drawRoundedRect(QRectF(s * .68, s * .20, s * .12, s * .60), 1.5, 1.5);
        p.drawPolygon(QPolygonF() << QPointF(s * .20, s * .20) << QPointF(s * .20, s * .80) << QPointF(s * .60, s * .50));
        break;
    case Ico::Reiniciar:
    {
        QPen caneta(cor, s * .11, Qt::SolidLine, Qt::RoundCap);
        p.setPen(caneta);
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(s * .20, s * .22, s * .60, s * .60), 40 * 16, 280 * 16);
        p.setPen(Qt::NoPen);
        p.setBrush(cor);
        p.drawPolygon(QPolygonF() << QPointF(s * .86, s * .10) << QPointF(s * .86, s * .42) << QPointF(s * .58, s * .30));
        break;
    }
    case Ico::Circulo:
    {
        QPen caneta(cor, s * .09, Qt::SolidLine, Qt::RoundCap);
        p.setPen(caneta);
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(s * .5, s * .5), s * .30, s * .30);
        p.setPen(Qt::NoPen);
        p.setBrush(cor);
        for (int k = 0; k < 3; ++k)
        {
            double a = 6.2831853 * k / 3 - 1.5707963;
            p.drawEllipse(QPointF(s * .5 + s * .30 * std::cos(a), s * .5 + s * .30 * std::sin(a)), s * .105, s * .105);
        }
        break;
    }
    case Ico::Limpar:
    {
        QPen caneta(cor, s * .11, Qt::SolidLine, Qt::RoundCap);
        p.setPen(caneta);
        p.drawLine(QPointF(s * .28, s * .28), QPointF(s * .72, s * .72));
        p.drawLine(QPointF(s * .72, s * .28), QPointF(s * .28, s * .72));
        break;
    }
    }
    return pm;
}

QIcon icone(Ico t, const QColor &cor = QColor("#9a3412"), int tam = 18)
{
    QIcon ic;
    ic.addPixmap(desenharIcone(t, cor, tam), QIcon::Normal);
    ic.addPixmap(desenharIcone(t, QColor("#dccbb9"), tam), QIcon::Disabled);
    return ic;
}

QPixmap criarLogo(int tam)
{
    const qreal dpr = 2.0;
    QPixmap pm(int(tam * dpr), int(tam * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient gr(0, 0, tam, tam);
    gr.setColorAt(0, QColor("#fdba74"));
    gr.setColorAt(0.5, QColor("#f97316"));
    gr.setColorAt(1, QColor("#c2410c"));
    p.setPen(Qt::NoPen);
    p.setBrush(gr);
    p.drawRoundedRect(QRectF(0, 0, tam, tam), tam * 0.30, tam * 0.30);
    QPointF a(tam * .27, tam * .70), b(tam * .50, tam * .28), c(tam * .76, tam * .64);
    p.setPen(QPen(QColor(255, 255, 255, 235), tam * 0.065, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(a, b);
    p.drawLine(b, c);
    p.drawLine(a, c);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    for (const QPointF &q : {a, b, c})
    {
        p.drawEllipse(q, tam * .095, tam * .095);
    }
    return pm;
}

// ============================================================
// Canvas: desenho e interacao com o grafo
// ============================================================
class Canvas : public QWidget
{
public:
    const grafo *g = nullptr;
    int n = 0;
    int arestasTotal = 0;
    bool dirigido = false;
    QVector<QPointF> pos;
    Destaque destaque;

    // widgets sobrepostos (criados pela janela)
    QWidget *flutuante = nullptr;
    QWidget *btnExemplo = nullptr;

    std::function<void(QPointF)> aoCriarVertice;
    std::function<void(int, int)> aoConectar;
    std::function<void(int, QPoint)> aoMenuVertice;
    std::function<void(int, int, QPoint)> aoMenuAresta;
    std::function<void(int, int)> aoEditarAresta;

    static constexpr double raio = 23.0;

    explicit Canvas(QWidget *pai = nullptr) : QWidget(pai)
    {
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        setMinimumSize(460, 340);
        setCursor(Qt::CrossCursor);
        relogio.start();
    }

    void cancelarConexao()
    {
        origem = -1;
        dica = dicaPadrao;
        update();
    }
    void definirDicaPadrao(const QString &t)
    {
        dicaPadrao = t;
        if (origem < 0)
            dica = t;
    }
    bool conectando() const { return origem >= 0; }
    void resetarHover()
    {
        hover = -1;
        hoverA = hoverB = -1;
    }
    void animarCriacao(int i)
    {
        criado = i;
        tCriacao = relogio.elapsed();
        update();
    }
    void mostrarToast(const QString &t)
    {
        toast = t;
        update();
        QTimer::singleShot(2800, this, [this, t] {
            if (toast == t)
            {
                toast.clear();
                update();
            }
        });
    }

    int verticeEm(const QPointF &p) const
    {
        for (int i = n - 1; i >= 0; --i)
        {
            if (QLineF(p, pos[i]).length() <= raio + 2)
                return i;
        }
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
        {
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
        }
        return false;
    }

protected:
    void resizeEvent(QResizeEvent *e) override
    {
        QWidget::resizeEvent(e);
        const QSize antigo = e->oldSize();
        if (antigo.width() > 0 && antigo.height() > 0)
        {
            double sx = double(width()) / antigo.width(), sy = double(height()) / antigo.height();
            for (auto &q : pos)
            {
                q.setX(qBound(raio + 4, q.x() * sx, qMax(raio + 4, width() - raio - 4.0)));
                q.setY(qBound(raio + 4, q.y() * sy, qMax(raio + 4, height() - raio - 4.0)));
            }
        }
        posicionarOverlays();
    }
    void showEvent(QShowEvent *e) override
    {
        QWidget::showEvent(e);
        posicionarOverlays();
    }

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
            dica = dicaPadrao;
            update();
            if (v >= 0 && v != de && aoConectar)
                aoConectar(de, v);
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
            hoverA = hoverB = -1;
            if (hover < 0)
            {
                int ai, aj;
                if (arestaEm(p, ai, aj))
                {
                    hoverA = ai;
                    hoverB = aj;
                }
            }
            setCursor((hover >= 0 || hoverA >= 0) ? Qt::PointingHandCursor : Qt::CrossCursor);
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
            dica = "Clique em outro vértice para conectar ao " + QString::number(v + 1) + "   ·   Esc cancela";
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
        resetarHover();
        update();
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        precisaAnimar = false;

        QLinearGradient fundo(0, 0, width(), height());
        fundo.setColorAt(0, QColor("#fffdfa"));
        fundo.setColorAt(1, QColor("#ffeedd"));
        p.fillRect(rect(), fundo);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(194, 65, 12, 36));
        for (int x = 18; x < width(); x += 28)
        {
            for (int y = 18; y < height(); y += 28)
            {
                p.drawEllipse(QPointF(x, y), 1.1, 1.1);
            }
        }

        if (n == 0)
        {
            desenharVazio(p);
            desenharToast(p);
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
            QPen tracejado(kAcento, 2.6, Qt::DashLine);
            tracejado.setCapStyle(Qt::RoundCap);
            p.setPen(tracejado);
            p.drawLine(QLineF(pos[origem], destino));
            precisaAnimar = true;
        }
        desenharVertices(p);
        desenharHud(p);
        desenharLegenda(p);
        desenharDica(p);
        desenharToast(p);

        if (precisaAnimar)
            QTimer::singleShot(30, this, [this] { update(); });
    }

private:
    int origem = -1;
    int arrastando = -1;
    int hover = -1;
    int hoverA = -1, hoverB = -1;
    int criado = -1;
    qint64 tCriacao = 0;
    bool moveu = false;
    bool precisaAnimar = false;
    QElapsedTimer relogio;
    QPointF cursor, pressionado, deslocamento;
    QString dicaPadrao, dica, toast;

    void posicionarOverlays()
    {
        if (flutuante)
        {
            flutuante->adjustSize();
            flutuante->move(width() - flutuante->width() - 16, 16);
        }
        if (btnExemplo)
        {
            btnExemplo->resize(240, 40);
            QRectF c = cartaoVazio();
            btnExemplo->move(int(c.center().x() - 120), int(c.bottom() - 62));
        }
    }

    QRectF cartaoVazio() const
    {
        QRectF r(0, 0, 500, 310);
        r.moveCenter(rect().center());
        return r;
    }

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

    static void sombra(QPainter &p, const QRectF &r, double raioCanto, int forca = 14)
    {
        p.setPen(Qt::NoPen);
        for (int k = 1; k <= 5; ++k)
        {
            p.setBrush(QColor(124, 45, 18, forca / k + 2));
            p.drawRoundedRect(r.adjusted(-k * 1.4, -k * 0.6 + 3, k * 1.4, k * 1.8 + 3), raioCanto + k, raioCanto + k);
        }
    }

    QFont fonte(int pt, bool negrito) const
    {
        QFont f = font();
        f.setPointSize(pt);
        f.setBold(negrito);
        return f;
    }

    // ---------- elementos de interface sobre o desenho ----------
    void desenharVazio(QPainter &p)
    {
        QRectF c = cartaoVazio();
        sombra(p, c, 22, 18);
        p.setPen(QPen(QColor("#fde4cc"), 1.2));
        p.setBrush(QColor(255, 255, 255, 248));
        p.drawRoundedRect(c, 22, 22);

        p.setPen(kTexto);
        p.setFont(fonte(18, true));
        p.drawText(QRectF(c.left(), c.top() + 22, c.width(), 34), Qt::AlignCenter, "Comece o seu grafo");
        p.setPen(kTextoSuave);
        p.setFont(fonte(10, false));
        p.drawText(QRectF(c.left(), c.top() + 56, c.width(), 22), Qt::AlignCenter, "Desenhe com o mouse e rode os algoritmos em segundos");

        const QStringList passos = {"Clique no espaço vazio para criar vértices",
                                    "Clique em um vértice e depois em outro para conectar",
                                    "Escolha um algoritmo na barra lateral"};
        double y = c.top() + 98;
        for (int i = 0; i < passos.size(); ++i)
        {
            QRectF bola(0, 0, 26, 26);
            bola.moveCenter(QPointF(c.left() + 54, y + 13));
            p.setPen(Qt::NoPen);
            p.setBrush(i == 2 ? kTexto : kVerticePadrao);
            p.drawEllipse(bola);
            p.setPen(Qt::white);
            p.setFont(fonte(10, true));
            p.drawText(bola, Qt::AlignCenter, QString::number(i + 1));
            p.setPen(kTexto);
            p.setFont(fonte(11, false));
            p.drawText(QRectF(c.left() + 80, y, c.width() - 100, 26), Qt::AlignVCenter | Qt::AlignLeft, passos[i]);
            y += 38;
        }
    }

    // pilula branca com um ponto colorido; devolve a largura
    double desenharChip(QPainter &p, QPointF topo, const QString &texto, const QColor &ponto)
    {
        QFont f = fonte(10, true);
        p.setFont(f);
        QFontMetrics fm(f);
        QRectF r(topo.x(), topo.y(), fm.horizontalAdvance(texto) + 38, 30);
        sombra(p, r, 15, 10);
        p.setPen(QPen(QColor("#fde4cc"), 1));
        p.setBrush(QColor(255, 255, 255, 245));
        p.drawRoundedRect(r, 15, 15);
        p.setPen(Qt::NoPen);
        p.setBrush(ponto);
        p.drawEllipse(QPointF(r.left() + 16, r.center().y()), 4.5, 4.5);
        p.setPen(kTexto);
        p.drawText(QRectF(r.left() + 28, r.top(), r.width() - 32, r.height()), Qt::AlignVCenter | Qt::AlignLeft, texto);
        return r.width();
    }

    void desenharHud(QPainter &p)
    {
        double x = 16;
        x += desenharChip(p, QPointF(x, 16), QString("%1 %2").arg(n).arg(n == 1 ? "vértice" : "vértices"), kVerticePadrao) + 8;
        x += desenharChip(p, QPointF(x, 16), QString("%1 %2").arg(arestasTotal).arg(arestasTotal == 1 ? "aresta" : "arestas"), kAberto) + 8;
        desenharChip(p, QPointF(x, 16), dirigido ? "Dirigido" : "Não dirigido", kCaminho);
    }

    void desenharLegenda(QPainter &p)
    {
        if (destaque.legenda.isEmpty())
            return;
        QFont f = fonte(10, false);
        QFontMetrics fm(f);
        double larg = 0;
        for (const auto &it : destaque.legenda)
        {
            larg = qMax(larg, double(fm.horizontalAdvance(it.second)));
        }
        QRectF r(16, 58, larg + 58, 20 + destaque.legenda.size() * 24 + 8);
        sombra(p, r, 14, 10);
        p.setPen(QPen(QColor("#fde4cc"), 1));
        p.setBrush(QColor(255, 255, 255, 245));
        p.drawRoundedRect(r, 14, 14);
        p.setFont(fonte(8, true));
        p.setPen(QColor("#a8998a"));
        p.drawText(QRectF(r.left() + 16, r.top() + 8, r.width(), 14), Qt::AlignVCenter | Qt::AlignLeft, "LEGENDA");
        double y = r.top() + 30;
        p.setFont(f);
        for (const auto &it : destaque.legenda)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(it.first);
            p.drawEllipse(QPointF(r.left() + 20, y), 6, 6);
            p.setPen(kTexto);
            p.drawText(QRectF(r.left() + 34, y - 10, r.width() - 40, 20), Qt::AlignVCenter | Qt::AlignLeft, it.second);
            y += 24;
        }
    }

    void desenharDica(QPainter &p)
    {
        if (dica.isEmpty())
            return;
        bool ativa = origem >= 0;
        QFont f = fonte(10, ativa);
        p.setFont(f);
        QFontMetrics fm(f);
        QRectF r(0, 0, fm.horizontalAdvance(dica) + 40, 34);
        r.moveCenter(QPointF(width() / 2.0, height() - 30));
        sombra(p, r, 17, 12);
        p.setPen(QPen(ativa ? kAcento : QColor("#fde4cc"), 1.2));
        p.setBrush(ativa ? kVerticePadrao : QColor(255, 255, 255, 240));
        p.drawRoundedRect(r, 17, 17);
        p.setPen(ativa ? QColor("#ffffff") : kTextoSuave);
        p.drawText(r, Qt::AlignCenter, dica);
    }

    void desenharToast(QPainter &p)
    {
        if (toast.isEmpty())
            return;
        QFont f = fonte(10, true);
        p.setFont(f);
        QFontMetrics fm(f);
        QRectF r(0, 0, fm.horizontalAdvance(toast) + 44, 38);
        r.moveCenter(QPointF(width() / 2.0, 52));
        sombra(p, r, 19, 22);
        p.setPen(Qt::NoPen);
        p.setBrush(kCaminho);
        p.drawRoundedRect(r, 19, 19);
        p.setBrush(kVerticePadrao);
        p.drawEllipse(QPointF(r.left() + 20, r.center().y()), 4.5, 4.5);
        p.setPen(QColor("#fff7ed"));
        p.drawText(QRectF(r.left() + 32, r.top(), r.width() - 36, r.height()), Qt::AlignVCenter | Qt::AlignLeft, toast);
    }

    void desenharBadge(QPainter &p, QPointF centro, const QString &texto, const QColor &cor, bool forte)
    {
        QFont f = fonte(10, true);
        p.setFont(f);
        QFontMetrics fm(f);
        QRectF caixa(0, 0, qMax(24, fm.horizontalAdvance(texto) + 14), 20);
        caixa.moveCenter(centro);
        p.setPen(QPen(cor, 1.4));
        p.setBrush(QColor("#ffffff"));
        p.drawRoundedRect(caixa, 10, 10);
        p.setPen(forte ? QColor("#c2410c") : QColor("#44403c"));
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
                    {
                        if (o.intersects(caixa))
                        {
                            livre = false;
                            break;
                        }
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
        {
            for (int j = 0; j < n; ++j)
            {
                int peso = g->custo_conexao(i + 1, j + 1);
                if (peso <= 0 || (!dirigido && j < i))
                    continue;
                bool forte = arestaEmDestaque(i, j);
                bool sobre = hoverA == i && hoverB == j;
                QColor cor = forte ? kArestaDestaque : (sobre ? QColor("#fb923c") : kArestaPadrao);
                double largura = forte ? 4.2 : (sobre ? 3.6 : 2.4);
                p.setBrush(Qt::NoBrush);
                if (i == j)
                {
                    QPointF c = pos[i] + QPointF(0, -raio - 12);
                    if (forte)
                    {
                        QColor brilho = cor;
                        brilho.setAlpha(55);
                        p.setPen(QPen(brilho, largura + 7));
                        p.drawEllipse(c, 15, 15);
                    }
                    QPen caneta(cor, largura);
                    caneta.setCapStyle(Qt::RoundCap);
                    p.setPen(caneta);
                    p.drawEllipse(c, 15, 15);
                    etiquetas.append({QLineF(), c + QPointF(0, -26), false, 0.5, QString::number(peso), cor, forte});
                    continue;
                }
                QLineF l = linhaAresta(i, j);
                if (forte)
                {
                    QColor brilho = cor;
                    brilho.setAlpha(55);
                    QPen halo(brilho, largura + 7);
                    halo.setCapStyle(Qt::RoundCap);
                    p.setPen(halo);
                    p.drawLine(l);
                }
                QPen caneta(cor, largura);
                caneta.setCapStyle(Qt::RoundCap);
                p.setPen(caneta);
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
        }
        desenharEtiquetas(p, etiquetas);
    }

    static double easeOutBack(double t)
    {
        const double c1 = 1.70158, c3 = c1 + 1;
        return 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
    }

    void desenharVertices(QPainter &p)
    {
        QFont f = fonte(13, true);
        const qint64 agora = relogio.elapsed();
        for (int i = 0; i < n; ++i)
        {
            QPointF c = pos[i];
            QColor cor = destaque.cores.value(i + 1, kVerticePadrao);
            bool ativo = hover == i || origem == i || arrastando == i;
            bool colorido = destaque.cores.contains(i + 1);

            double escala = 1.0;
            if (i == criado)
            {
                double t = (agora - tCriacao) / 340.0;
                if (t >= 1.0)
                    criado = -1;
                else
                {
                    escala = qMax(0.05, easeOutBack(t));
                    precisaAnimar = true;
                }
            }
            if (ativo)
                escala *= 1.07;
            const double r = raio * escala;

            // pulso do vertice em processamento
            if (colorido && cor == kAtual)
            {
                double fase = std::fmod(agora / 1100.0, 1.0);
                QColor anel = kVerticePadrao;
                anel.setAlpha(int(170 * (1.0 - fase)));
                p.setPen(QPen(anel, 3));
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(c, r + 5 + fase * 15, r + 5 + fase * 15);
                precisaAnimar = true;
            }
            if (ativo || colorido)
            {
                QRadialGradient brilho(c, r * 2.0);
                QColor halo = ativo ? kVerticePadrao : cor;
                halo.setAlpha(ativo ? 110 : 80);
                brilho.setColorAt(0.45, halo);
                halo.setAlpha(0);
                brilho.setColorAt(1.0, halo);
                p.setPen(Qt::NoPen);
                p.setBrush(brilho);
                p.drawEllipse(c, r * 2.0, r * 2.0);
            }
            // sombra suave
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(124, 45, 18, 38));
            p.drawEllipse(c + QPointF(0, 4), r + 1, r + 1);

            QRadialGradient g2(c - QPointF(7, 8) * escala, r * 1.7);
            g2.setColorAt(0, cor.lighter(125));
            g2.setColorAt(1, cor.darker(112));
            p.setPen(QPen(origem == i ? kAcento : QColor(255, 255, 255), origem == i ? 3.4 : 3.0));
            p.setBrush(g2);
            p.drawEllipse(c, r, r);
            p.setFont(f);
            p.setPen(cor.lightness() > 150 ? QColor("#292524") : QColor("#ffffff"));
            p.drawText(QRectF(c.x() - r, c.y() - r, r * 2, r * 2), Qt::AlignCenter, QString::number(i + 1));

            // caminho critico: duracao no alto; inicio/fim mais cedo em cima,
            // mais tarde embaixo; folga abaixo do vertice
            if (destaque.cpm.contains(i + 1))
            {
                const DadosCpm &d = destaque.cpm[i + 1];
                QFont ft = fonte(10, true);
                p.setFont(ft);
                const QColor vermelho("#dc2626");
                auto escrever = [&](QPointF centro, const QString &t, const QColor &cor) {
                    p.setPen(cor);
                    p.drawText(QRectF(centro.x() - 40, centro.y() - 9, 80, 18), Qt::AlignCenter, t);
                };
                const double dx = raio + 9, dy = raio * 0.8;
                escrever(c + QPointF(0, -raio - 12), QString::number(d.duracao), QColor("#292524"));
                escrever(c + QPointF(-dx, -dy), QString::number(d.inicioCedo), vermelho);
                escrever(c + QPointF(dx, -dy), QString::number(d.fimCedo), vermelho);
                escrever(c + QPointF(-dx, dy), QString::number(d.inicioTarde), vermelho);
                escrever(c + QPointF(dx, dy), QString::number(d.fimTarde), vermelho);
                escrever(c + QPointF(0, raio + 14), QString("Folga: %1").arg(d.folga), d.folga == 0 ? QColor("#1d4ed8") : QColor("#15803d"));
            }

            QString rotulo = destaque.rotulos.value(i + 1);
            if (!rotulo.isEmpty())
            {
                QFont fr = fonte(10, true);
                p.setFont(fr);
                QFontMetrics fm(fr);
                QRectF pilula(0, 0, fm.horizontalAdvance(rotulo) + 16, 20);
                pilula.moveCenter(QPointF(c.x(), c.y() + raio + 15));
                sombra(p, pilula, 10, 8);
                p.setPen(QPen(QColor("#fdba74"), 1));
                p.setBrush(QColor(255, 255, 255, 250));
                p.drawRoundedRect(pilula, 10, 10);
                p.setPen(QColor("#44403c"));
                p.drawText(pilula, Qt::AlignCenter, rotulo);
            }
        }
    }
};

// ============================================================
// Cartao de algoritmo (barra lateral)
// ============================================================
class Cartao : public QFrame
{
public:
    std::function<void()> aoClicar;
    QString dicaNormal;

    Cartao(const QString &titulo, const QString &descricao, bool comPassos, QWidget *pai = nullptr) : QFrame(pai), dicaNormal(descricao)
    {
        setObjectName("operacao");
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover);
        auto l = new QVBoxLayout(this);
        l->setContentsMargins(14, 11, 14, 12);
        l->setSpacing(3);
        auto linha = new QHBoxLayout;
        linha->setSpacing(6);
        auto t = new QLabel(titulo);
        t->setObjectName("opTitulo");
        t->setWordWrap(true);
        t->setAttribute(Qt::WA_TransparentForMouseEvents);
        linha->addWidget(t, 1);
        if (comPassos)
        {
            auto tag = new QLabel("PASSOS");
            tag->setObjectName("opTag");
            tag->setAttribute(Qt::WA_TransparentForMouseEvents);
            linha->addWidget(tag, 0, Qt::AlignTop);
        }
        l->addLayout(linha);
        auto d = new QLabel(descricao);
        d->setObjectName("opDesc");
        d->setWordWrap(true);
        d->setAttribute(Qt::WA_TransparentForMouseEvents);
        l->addWidget(d);
        setToolTip(descricao);
    }

    void definirIndisponivel(const QString &motivo)
    {
        bool off = !motivo.isEmpty();
        setProperty("indisp", off);
        setToolTip(off ? "Indisponível: " + motivo : dicaNormal);
        style()->unpolish(this);
        style()->polish(this);
        for (auto *lb : findChildren<QLabel *>())
        {
            lb->style()->unpolish(lb);
            lb->style()->polish(lb);
        }
    }

protected:
    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && rect().contains(e->position().toPoint()) && aoClicar)
            aoClicar();
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
    Caixeiro,
    CaminhoCritico
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

struct ItemOp
{
    Op op;
    Cartao *cartao;
    QLabel *grupo;
    QString busca;
};

struct JanelaPrincipal : QMainWindow
{
    std::unique_ptr<grafo> g;
    int n = 0;
    bool dirigido = false;

    Canvas *canvas = nullptr;
    QPlainTextEdit *saida = nullptr;
    QTableWidget *tabela = nullptr;
    QTabWidget *abas = nullptr;
    QCheckBox *modoPassos = nullptr;
    QPushButton *btnDirigido = nullptr, *btnNaoDirigido = nullptr;
    QFrame *barraPassos = nullptr;
    QLabel *rotuloPasso = nullptr, *contadorPasso = nullptr;
    QSlider *controlePasso = nullptr;
    QPushButton *btnAnterior = nullptr, *btnProximo = nullptr, *btnReiniciar = nullptr, *btnPlay = nullptr, *btnVelocidade = nullptr;
    QTimer *relogioPlay = nullptr;
    QVector<Passo> passos;
    int passoAtual = -1;
    int velocidade = 1; // indice em kVelocidades
    QVector<OpInfo> operacoes;
    QVector<ItemOp> itens;
    QLineEdit *busca = nullptr;

    static constexpr int kIntervalos[4] = {1500, 900, 450, 180};
    static const char *rotuloVelocidade(int i)
    {
        static const char *r[4] = {"0.5×", "1×", "2×", "4×"};
        return r[i];
    }

    static Par par(int a, int b, bool dir)
    {
        if (!dir && a > b)
            std::swap(a, b);
        return Par(a, b);
    }

    JanelaPrincipal()
    {
        setWindowTitle("Grafos Studio");
        setWindowIcon(QIcon(criarLogo(64)));
        resize(1440, 900);
        setMinimumSize(1120, 720);
        g = std::make_unique<grafo>();
        g->inicializa_grafo(0, false);

        operacoes = {
            {Op::BFS, "Busca em largura (BFS)", "Percorre o grafo por níveis a partir de um vértice.", true, false, false, true},
            {Op::DFS, "Busca em profundidade (DFS)", "Percorre o grafo mergulhando em cada ramo.", true, false, false, true},
            {Op::FechoDireto, "Fecho transitivo direto", "Vértices alcançáveis a partir de um vértice.", true, false, false, false},
            {Op::FechoInverso, "Fecho transitivo inverso", "Vértices que conseguem alcançar um vértice.", true, false, false, false},
            {Op::Componentes, "Componentes conexos / SFCMs", "Separa o grafo em componentes (fortemente) conexos.", false, false, false, false},
            {Op::Dijkstra, "Dijkstra", "Menores caminhos a partir de um vértice.", true, false, false, true},
            {Op::AEstrela, "A* (heurística zero)", "Menor caminho entre dois vértices.", true, true, false, true},
            {Op::Coloracao, "Coloração de vértices", "Colore sem vizinhos iguais (não dirigido).", false, false, true, true},
            {Op::Ciclo3, "Ciclo de tamanho 3", "Procura triângulos no grafo.", false, false, true, false},
            {Op::Euler, "Limites de Euler (planaridade)", "Verifica os limites de Euler para planaridade.", false, false, true, false},
            {Op::Areas, "Áreas do grafo", "Calcula as áreas (faces) do grafo.", false, false, true, false},
            {Op::Caixeiro, "Vizinho mais próximo", "Heurística do caixeiro viajante (grafo completo, não dirigido).", true, false, true, true},
            {Op::CaminhoCritico, "Caminho crítico (CPM)", "Mesmo cálculo com 1 início e 1 fim; usa as durações informadas (grafo dirigido).", false, false, false, false},
        };

        construirInterface();
        atualizar();
    }

    static QString dicaPadrao()
    {
        return "Clique no espaço vazio para criar um vértice   ·   clique em dois vértices para conectar   ·   botão direito para opções";
    }

    // -------------------------------------------------------
    // Construcao da interface
    // -------------------------------------------------------
    QWidget *criarCabecalho()
    {
        auto cab = new QFrame;
        cab->setObjectName("cabecalho");
        auto l = new QHBoxLayout(cab);
        l->setContentsMargins(22, 12, 22, 12);
        l->setSpacing(10);

        auto logo = new QLabel;
        logo->setPixmap(criarLogo(42));
        l->addWidget(logo);
        auto textos = new QVBoxLayout;
        textos->setSpacing(0);
        auto t1 = new QLabel("Grafos <span style='color:#ea580c'>Studio</span>");
        t1->setObjectName("logoTitulo");
        auto t2 = new QLabel("Crie, edite e visualize algoritmos em grafos");
        t2->setObjectName("logoSub");
        textos->addWidget(t1);
        textos->addWidget(t2);
        l->addLayout(textos);
        l->addSpacing(26);

        auto acao = [&](const QString &texto, const QString &dica, const QKeySequence &atalho, std::function<void()> fn) {
            auto b = new QPushButton(texto);
            b->setObjectName("ghost");
            b->setToolTip(dica + (atalho.isEmpty() ? "" : "  (" + atalho.toString(QKeySequence::NativeText) + ")"));
            b->setCursor(Qt::PointingHandCursor);
            if (!atalho.isEmpty())
                b->setShortcut(atalho);
            connect(b, &QPushButton::clicked, this, [fn] { fn(); });
            l->addWidget(b);
            return b;
        };
        acao("Novo", "Começa um grafo vazio", QKeySequence::New, [this] { novoGrafo(); });
        acao("Abrir", "Abre um grafo salvo", QKeySequence::Open, [this] { abrirArquivo(); });
        acao("Salvar", "Salva o grafo em arquivo", QKeySequence::Save, [this] { salvarArquivo(); });
        l->addStretch();

        // seletor segmentado: nao dirigido / dirigido
        auto seg = new QFrame;
        seg->setObjectName("segmentoWrap");
        auto sl = new QHBoxLayout(seg);
        sl->setContentsMargins(4, 4, 4, 4);
        sl->setSpacing(2);
        btnNaoDirigido = new QPushButton("Não dirigido");
        btnDirigido = new QPushButton("Dirigido");
        for (auto *b : {btnNaoDirigido, btnDirigido})
        {
            b->setObjectName("segmento");
            b->setCheckable(true);
            b->setAutoExclusive(true);
            b->setCursor(Qt::PointingHandCursor);
            sl->addWidget(b);
        }
        btnNaoDirigido->setChecked(true);
        btnDirigido->setToolTip("Alterna o tipo do grafo mantendo as conexões");
        connect(btnDirigido, &QPushButton::toggled, this, [this](bool v) { alternarDirigido(v); });
        l->addWidget(seg);
        l->addSpacing(8);

        auto ajuda = new QPushButton("?");
        ajuda->setObjectName("ajuda");
        ajuda->setFixedSize(34, 34);
        ajuda->setToolTip("Como usar (F1)");
        ajuda->setCursor(Qt::PointingHandCursor);
        ajuda->setShortcut(QKeySequence(Qt::Key_F1));
        connect(ajuda, &QPushButton::clicked, this, [this] { mostrarAjuda(); });
        l->addWidget(ajuda);
        return cab;
    }

    QWidget *criarLateral()
    {
        auto lateral = new QWidget;
        lateral->setObjectName("lateral");
        lateral->setFixedWidth(340);
        auto ll = new QVBoxLayout(lateral);
        ll->setContentsMargins(18, 18, 10, 0);
        ll->setSpacing(12);

        busca = new QLineEdit;
        busca->setObjectName("busca");
        busca->setPlaceholderText("Buscar algoritmo...");
        busca->setClearButtonEnabled(true);
        connect(busca, &QLineEdit::textChanged, this, [this](const QString &t) { filtrar(t); });
        ll->addWidget(busca);

        auto modo = new QFrame;
        modo->setObjectName("cartaoModo");
        auto ml = new QVBoxLayout(modo);
        ml->setContentsMargins(14, 10, 14, 10);
        ml->setSpacing(2);
        modoPassos = new QCheckBox("Executar passo a passo");
        modoPassos->setObjectName("interruptor");
        modoPassos->setChecked(true);
        modoPassos->setCursor(Qt::PointingHandCursor);
        modoPassos->setToolTip("Mostra a execução do algoritmo em etapas sobre o desenho do grafo");
        auto md = new QLabel("Veja cada decisão do algoritmo no grafo, com controle de velocidade.");
        md->setObjectName("opDesc");
        md->setWordWrap(true);
        ml->addWidget(modoPassos);
        ml->addWidget(md);
        ll->addWidget(modo);

        auto lista = new QWidget;
        lista->setObjectName("listaOps");
        auto lv = new QVBoxLayout(lista);
        lv->setContentsMargins(0, 4, 8, 20);
        lv->setSpacing(8);

        auto grupo = [&](const QString &nome, std::initializer_list<Op> ops) {
            auto rotulo = new QLabel(nome.toUpper());
            rotulo->setObjectName("grupo");
            lv->addSpacing(8);
            lv->addWidget(rotulo);
            for (Op op : ops)
            {
                for (const auto &info : operacoes)
                {
                    if (info.op == op)
                    {
                        auto c = new Cartao(info.nome, info.dica, info.passos);
                        Op o = info.op;
                        c->aoClicar = [this, o] { acionarOperacao(o); };
                        lv->addWidget(c);
                        itens.append({o, c, rotulo, (info.nome + " " + info.dica + " " + nome).toLower()});
                    }
                }
            }
        };
        grupo("Percursos", {Op::BFS, Op::DFS, Op::FechoDireto, Op::FechoInverso, Op::Componentes});
        grupo("Caminhos mínimos", {Op::Dijkstra, Op::AEstrela});
        grupo("Propriedades", {Op::Coloracao, Op::Ciclo3, Op::Euler, Op::Areas});
        grupo("Caixeiro viajante", {Op::Caixeiro});
        grupo("Planejamento", {Op::CaminhoCritico});
        lv->addStretch();

        auto scroll = new QScrollArea;
        scroll->setWidget(lista);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setObjectName("scrollEsquerdo");
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ll->addWidget(scroll, 1);
        return lateral;
    }

    QWidget *criarBarraPassos()
    {
        barraPassos = new QFrame;
        barraPassos->setObjectName("barraPassos");
        auto v = new QVBoxLayout(barraPassos);
        v->setContentsMargins(20, 12, 20, 14);
        v->setSpacing(8);

        auto topo = new QHBoxLayout;
        rotuloPasso = new QLabel("Passo");
        rotuloPasso->setObjectName("passoTitulo");
        contadorPasso = new QLabel("1 / 1");
        contadorPasso->setObjectName("contador");
        topo->addWidget(rotuloPasso, 1);
        topo->addWidget(contadorPasso);
        v->addLayout(topo);

        auto linha = new QHBoxLayout;
        linha->setSpacing(6);
        btnReiniciar = botaoIcone(Ico::Reiniciar, "Reiniciar do primeiro passo");
        btnAnterior = botaoIcone(Ico::Anterior, "Passo anterior  (←)");
        btnPlay = botaoIcone(Ico::Play, "Reproduzir automaticamente  (Espaço)", QColor("#ffffff"));
        btnPlay->setObjectName("play");
        btnPlay->setFixedSize(42, 42);
        btnPlay->setIconSize(QSize(20, 20));
        btnProximo = botaoIcone(Ico::Proximo, "Próximo passo  (→)");
        controlePasso = new QSlider(Qt::Horizontal);
        controlePasso->setCursor(Qt::PointingHandCursor);
        btnVelocidade = new QPushButton(rotuloVelocidade(velocidade));
        btnVelocidade->setObjectName("ghost");
        btnVelocidade->setFixedWidth(54);
        btnVelocidade->setToolTip("Velocidade da reprodução");
        btnVelocidade->setCursor(Qt::PointingHandCursor);
        auto btnConcluir = new QPushButton("Encerrar");
        btnConcluir->setObjectName("ghost");
        btnConcluir->setCursor(Qt::PointingHandCursor);
        linha->addWidget(btnReiniciar);
        linha->addWidget(btnAnterior);
        linha->addWidget(btnPlay);
        linha->addWidget(btnProximo);
        linha->addSpacing(10);
        linha->addWidget(controlePasso, 1);
        linha->addSpacing(10);
        linha->addWidget(btnVelocidade);
        linha->addWidget(btnConcluir);
        v->addLayout(linha);
        barraPassos->hide();

        relogioPlay = new QTimer(this);
        connect(relogioPlay, &QTimer::timeout, this, [this] {
            if (passoAtual + 1 < passos.size())
                irParaPasso(passoAtual + 1);
            else
                pararPlay();
        });
        connect(btnProximo, &QPushButton::clicked, this, [this] { pararPlay(); irParaPasso(passoAtual + 1); });
        connect(btnAnterior, &QPushButton::clicked, this, [this] { pararPlay(); irParaPasso(passoAtual - 1); });
        connect(btnReiniciar, &QPushButton::clicked, this, [this] { pararPlay(); irParaPasso(0); });
        connect(btnPlay, &QPushButton::clicked, this, [this] { alternarPlay(); });
        connect(btnConcluir, &QPushButton::clicked, this, [this] { limparResultado(true); });
        connect(controlePasso, &QSlider::valueChanged, this, [this](int i) {
            if (i != passoAtual)
            {
                pararPlay();
                irParaPasso(i);
            }
        });
        connect(btnVelocidade, &QPushButton::clicked, this, [this] {
            velocidade = (velocidade + 1) % 4;
            btnVelocidade->setText(rotuloVelocidade(velocidade));
            if (relogioPlay->isActive())
                relogioPlay->start(kIntervalos[velocidade]);
        });

        // atalhos de teclado do passo a passo
        auto atalho = [this](Qt::Key k, std::function<void()> fn) {
            auto s = new QShortcut(QKeySequence(k), this);
            connect(s, &QShortcut::activated, this, [this, fn] {
                if (barraPassos->isVisible())
                    fn();
            });
        };
        atalho(Qt::Key_Right, [this] { pararPlay(); irParaPasso(passoAtual + 1); });
        atalho(Qt::Key_Left, [this] { pararPlay(); irParaPasso(passoAtual - 1); });
        atalho(Qt::Key_Space, [this] { alternarPlay(); });
        return barraPassos;
    }

    QPushButton *botaoIcone(Ico ic, const QString &dica, const QColor &cor = QColor("#9a3412"))
    {
        auto b = new QPushButton;
        b->setObjectName("icone");
        b->setIcon(icone(ic, cor, 20));
        b->setIconSize(QSize(18, 18));
        b->setFixedSize(38, 38);
        b->setToolTip(dica);
        b->setCursor(Qt::PointingHandCursor);
        return b;
    }

    void construirInterface()
    {
        auto central = new QWidget;
        auto raiz = new QVBoxLayout(central);
        raiz->setContentsMargins(0, 0, 0, 0);
        raiz->setSpacing(0);
        raiz->addWidget(criarCabecalho());

        // area do canvas
        canvas = new Canvas;
        canvas->g = g.get();
        canvas->definirDicaPadrao(dicaPadrao());
        canvas->aoCriarVertice = [this](QPointF p) { adicionarVertice(p); };
        canvas->aoConectar = [this](int a, int b) { conectar(a, b); };
        canvas->aoEditarAresta = [this](int a, int b) { conectar(a, b); };
        canvas->aoMenuVertice = [this](int v, QPoint gp) { menuVertice(v, gp); };
        canvas->aoMenuAresta = [this](int a, int b, QPoint gp) { menuAresta(a, b, gp); };

        // ferramentas flutuantes (canto superior direito do canvas)
        auto flut = new QFrame(canvas);
        flut->setObjectName("flutuante");
        auto fl = new QHBoxLayout(flut);
        fl->setContentsMargins(6, 6, 6, 6);
        fl->setSpacing(4);
        auto fbtn = [&](Ico ic, const QString &dica, std::function<void()> fn) {
            auto b = new QToolButton;
            b->setObjectName("flutuanteBtn");
            b->setIcon(icone(ic, QColor("#9a3412"), 20));
            b->setIconSize(QSize(18, 18));
            b->setToolTip(dica);
            b->setCursor(Qt::PointingHandCursor);
            connect(b, &QToolButton::clicked, this, [fn] { fn(); });
            fl->addWidget(b);
        };
        fbtn(Ico::Circulo, "Organizar vértices em círculo", [this] { organizarCirculo(); });
        fbtn(Ico::Limpar, "Limpar destaques e resultado", [this] { limparResultado(true); });
        canvas->flutuante = flut;

        // botao do estado vazio
        auto ex = new QPushButton("Carregar grafo de exemplo", canvas);
        ex->setObjectName("primario");
        ex->setCursor(Qt::PointingHandCursor);
        connect(ex, &QPushButton::clicked, this, [this] { carregarExemplo(); });
        canvas->btnExemplo = ex;

        auto areaCanvas = new QWidget;
        auto acl = new QVBoxLayout(areaCanvas);
        acl->setContentsMargins(0, 0, 0, 0);
        acl->setSpacing(0);
        acl->addWidget(canvas, 1);
        acl->addWidget(criarBarraPassos());

        // abas inferiores
        abas = new QTabWidget;
        abas->setDocumentMode(true);
        saida = new QPlainTextEdit;
        saida->setReadOnly(true);
        saida->setPlaceholderText("O resultado dos algoritmos aparece aqui.\nEscolha um algoritmo na barra lateral para começar.");
        saida->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        tabela = new QTableWidget;
        tabela->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tabela->setSelectionMode(QAbstractItemView::NoSelection);
        tabela->setShowGrid(true);
        tabela->horizontalHeader()->setDefaultSectionSize(46);
        tabela->verticalHeader()->setDefaultSectionSize(32);
        abas->addTab(saida, "Resultado");
        abas->addTab(tabela, "Matriz de adjacência");

        auto divisor = new QSplitter(Qt::Vertical);
        divisor->setHandleWidth(3);
        divisor->addWidget(areaCanvas);
        divisor->addWidget(abas);
        divisor->setStretchFactor(0, 4);
        divisor->setStretchFactor(1, 1);
        divisor->setSizes({640, 200});
        divisor->setCollapsible(0, false);

        auto corpo = new QHBoxLayout;
        corpo->setContentsMargins(0, 0, 0, 0);
        corpo->setSpacing(0);
        corpo->addWidget(criarLateral());
        corpo->addWidget(divisor, 1);
        raiz->addLayout(corpo, 1);
        setCentralWidget(central);
        statusBar()->hide();

        setStyleSheet(estilo());
    }

    static QString estilo()
    {
        // marca de selecao do checkbox, gerada em tempo de execucao
        QString caminhoCheck = QDir::temp().filePath("grafos_studio_check.png");
        {
            QPixmap pm(40, 40);
            pm.fill(Qt::transparent);
            QPainter p(&pm);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(QPen(Qt::white, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.drawPolyline(QPolygonF() << QPointF(10, 21) << QPointF(17, 28) << QPointF(30, 12));
            pm.save(caminhoCheck);
        }
        QString css = R"(
QWidget { font-family: "Inter", "Segoe UI", "Noto Sans", "Ubuntu", "Helvetica Neue", sans-serif; font-size: 13px; color: #292524; }
QMainWindow, QWidget#lateral, QScrollArea#scrollEsquerdo, QWidget#listaOps { background: #fff8f1; }
QToolTip { background: #1c1917; color: #fff7ed; border: none; padding: 6px 8px; }

/* ---------- cabecalho ---------- */
QFrame#cabecalho { background: #ffffff; border-bottom: 1px solid #f5e6d8; }
QLabel#logoTitulo { font-size: 19px; font-weight: 800; color: #1c1917; }
QLabel#logoSub { font-size: 12px; color: #a8998a; }
QPushButton#ghost { background: transparent; border: 1px solid transparent; border-radius: 10px; padding: 8px 14px; color: #57534e; font-weight: 700; }
QPushButton#ghost:hover { background: #ffedd5; color: #9a3412; }
QPushButton#ghost:pressed { background: #fed7aa; }
QFrame#segmentoWrap { background: #fff1e3; border: 1px solid #fde4cc; border-radius: 12px; }
QPushButton#segmento { background: transparent; border: none; border-radius: 9px; padding: 7px 16px; color: #78716c; font-weight: 700; }
QPushButton#segmento:hover { color: #9a3412; }
QPushButton#segmento:checked { background: #f97316; color: #ffffff; }
QPushButton#ajuda { background: #ffffff; border: 1px solid #fde4cc; border-radius: 17px; color: #ea580c; font-weight: 800; font-size: 15px; padding: 0; }
QPushButton#ajuda:hover { background: #f97316; color: #ffffff; border-color: #f97316; }

/* ---------- lateral ---------- */
QLineEdit#busca { background: #ffffff; border: 1px solid #f1dfcf; border-radius: 12px; padding: 10px 14px; }
QLineEdit#busca:focus { border: 1.5px solid #f97316; }
QFrame#cartaoModo { background: #ffffff; border: 1px solid #f5e6d8; border-radius: 14px; }
QCheckBox { spacing: 10px; font-weight: 700; color: #292524; }
QCheckBox::indicator { width: 20px; height: 20px; border-radius: 6px; border: 1.5px solid #e7cfb8; background: #ffffff; }
QCheckBox::indicator:hover { border-color: #f97316; }
QCheckBox::indicator:checked { background: #f97316; border-color: #ea580c; }
QLabel#grupo { font-size: 11px; font-weight: 800; letter-spacing: 2px; color: #ea580c; padding-left: 2px; }
QFrame#operacao { background: #ffffff; border: 1px solid #f5e6d8; border-radius: 14px; }
QFrame#operacao:hover { background: #fff3e6; border: 1px solid #f97316; }
QFrame#operacao[indisp="true"] { background: #fbf6f0; border: 1px dashed #ecdccb; }
QFrame#operacao[indisp="true"]:hover { background: #fbf6f0; border: 1px dashed #d9c3ac; }
QLabel#opTitulo { background: transparent; font-weight: 800; font-size: 13px; color: #1c1917; }
QLabel#opDesc { background: transparent; font-size: 12px; color: #78716c; }
QLabel#opTag { background: #ffedd5; color: #c2410c; border-radius: 7px; padding: 2px 7px; font-size: 9px; font-weight: 800; letter-spacing: 1px; }
QFrame#operacao[indisp="true"] QLabel#opTitulo { color: #b8a898; }
QFrame#operacao[indisp="true"] QLabel#opDesc { color: #c9bcae; }
QFrame#operacao[indisp="true"] QLabel#opTag { background: #f3eadf; color: #c9bcae; }

/* ---------- canvas: ferramentas flutuantes ---------- */
QFrame#flutuante { background: rgba(255,255,255,245); border: 1px solid #fde4cc; border-radius: 14px; }
QToolButton#flutuanteBtn { background: transparent; border: none; border-radius: 10px; padding: 7px; }
QToolButton#flutuanteBtn:hover { background: #ffedd5; }
QToolButton#flutuanteBtn:pressed { background: #fed7aa; }

/* ---------- botoes ---------- */
QPushButton { background: #ffffff; border: 1px solid #f1dfcf; border-radius: 10px; padding: 8px 16px; color: #44403c; font-weight: 700; }
QPushButton:hover { background: #fff1e3; border-color: #fdba74; color: #9a3412; }
QPushButton:pressed { background: #ffedd5; }
QPushButton:disabled { color: #c9bcae; background: #faf4ed; border-color: #f3e8dc; }
QPushButton#primario { background: #f97316; border: 1px solid #ea580c; color: #ffffff; font-weight: 800; padding: 10px 18px; }
QPushButton#primario:hover { background: #ea580c; color: #ffffff; }
QPushButton#primario:pressed { background: #c2410c; }
QPushButton#icone { background: #ffffff; border: 1px solid #f1dfcf; border-radius: 19px; padding: 0; }
QPushButton#icone:hover { background: #fff1e3; border-color: #fdba74; }
QPushButton#icone:disabled { background: #faf4ed; border-color: #f3e8dc; }
QPushButton#play { background: #f97316; border: none; border-radius: 21px; padding: 0; }
QPushButton#play:hover { background: #ea580c; }

/* ---------- passo a passo ---------- */
QFrame#barraPassos { background: #ffffff; border-top: 1px solid #f5e6d8; }
QLabel#passoTitulo { font-size: 14px; font-weight: 800; color: #9a3412; }
QLabel#contador { background: #ffedd5; color: #c2410c; border-radius: 10px; padding: 3px 12px; font-weight: 800; }
QSlider::groove:horizontal { height: 6px; background: #f5e6d8; border-radius: 3px; }
QSlider::sub-page:horizontal { background: #f97316; border-radius: 3px; }
QSlider::handle:horizontal { width: 16px; height: 16px; margin: -6px 0; border-radius: 9px; background: #ffffff; border: 3px solid #f97316; }
QSlider::handle:horizontal:hover { border-color: #c2410c; }

/* ---------- abas e tabela ---------- */
QTabWidget::pane { border: none; border-top: 1px solid #f5e6d8; background: #ffffff; }
QTabBar { background: #ffffff; }
QTabBar::tab { background: transparent; color: #a8998a; padding: 10px 22px; border: none; font-weight: 700; }
QTabBar::tab:hover { color: #ea580c; }
QTabBar::tab:selected { color: #1c1917; border-bottom: 3px solid #f97316; }
QPlainTextEdit { font-family: "JetBrains Mono", "DejaVu Sans Mono", "Consolas", monospace; font-size: 12px; }
QPlainTextEdit, QTableWidget { background: #ffffff; border: none; color: #292524; selection-background-color: #fdba74; selection-color: #1c1917; gridline-color: #f5e6d8; padding: 6px; }
QHeaderView::section { background: #fff8f1; color: #c2410c; border: none; border-right: 1px solid #f5e6d8; border-bottom: 1px solid #f5e6d8; padding: 5px; font-weight: 800; }
QTableCornerButton::section { background: #fff8f1; border: none; }
QSplitter::handle { background: #f5e6d8; }
QSplitter::handle:hover { background: #fdba74; }

/* ---------- barras de rolagem ---------- */
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: #fdba74; border-radius: 4px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #f97316; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }
QScrollBar::handle:horizontal { background: #fdba74; border-radius: 4px; min-width: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

/* ---------- menus e dialogos ---------- */
QMenu { background: #ffffff; border: 1px solid #fed7aa; border-radius: 12px; padding: 6px; }
QMenu::item { padding: 8px 24px; border-radius: 7px; }
QMenu::item:selected { background: #f97316; color: #ffffff; }
QMenu::item:disabled { color: #ea580c; font-weight: 800; }
QMenu::separator { height: 1px; background: #f5e6d8; margin: 5px 8px; }
QDialog, QMessageBox, QInputDialog { background: #fff8f1; }
QLabel#dlgTitulo { font-size: 17px; font-weight: 800; color: #1c1917; }
QLabel#dlgDesc { color: #78716c; }
QLabel#dlgDestaque { font-size: 22px; font-weight: 800; color: #ea580c; }
QSpinBox { background: #ffffff; border: 1px solid #f1dfcf; border-radius: 10px; padding: 8px 10px; selection-background-color: #fdba74; font-weight: 700; }
QSpinBox:focus { border: 1.5px solid #f97316; }
QCheckBox::indicator:checked { image: url(%1); }
)";
        return css.arg(caminhoCheck);
    }

    // -------------------------------------------------------
    // Atualizacao da tela
    // -------------------------------------------------------
    int contarArestas() const
    {
        int total = 0;
        for (int i = 0; i < n; ++i)
        {
            for (int j = dirigido ? 0 : i; j < n; ++j)
            {
                if (g->custo_conexao(i + 1, j + 1) > 0)
                    ++total;
            }
        }
        return total;
    }

    void atualizar()
    {
        canvas->g = g.get();
        canvas->n = n;
        canvas->dirigido = dirigido;
        canvas->arestasTotal = contarArestas();
        canvas->resetarHover();
        canvas->btnExemplo->setVisible(n == 0);
        canvas->flutuante->setVisible(n > 0);
        if (n > 0)
        {
            canvas->flutuante->adjustSize();
            canvas->flutuante->move(canvas->width() - canvas->flutuante->width() - 16, 16);
        }

        tabela->setRowCount(n);
        tabela->setColumnCount(n);
        QStringList nomes;
        for (int i = 0; i < n; ++i)
        {
            nomes << QString::number(i + 1);
        }
        tabela->setHorizontalHeaderLabels(nomes);
        tabela->setVerticalHeaderLabels(nomes);
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                int w = g->custo_conexao(i + 1, j + 1);
                auto item = new QTableWidgetItem(w > 0 ? QString::number(w) : "·");
                item->setTextAlignment(Qt::AlignCenter);
                if (w > 0)
                {
                    item->setBackground(QColor("#ffedd5"));
                    item->setForeground(QColor("#9a3412"));
                    QFont f = item->font();
                    f.setBold(true);
                    item->setFont(f);
                }
                else
                    item->setForeground(QColor("#d6c7b8"));
                tabela->setItem(i, j, item);
            }
        }
        atualizarDisponibilidade();
        canvas->update();
    }

    // Motivo pelo qual a operacao nao pode ser executada agora ("" = disponivel).
    QString motivoIndisponivel(Op op) const
    {
        const OpInfo &oi = info(op);
        if (n == 0)
            return "crie ao menos um vértice na área de desenho.";
        if (dirigido && oi.soNaoDirigido)
            return "requer um grafo não dirigido.";
        if (op == Op::CaminhoCritico && !dirigido)
            return "requer um grafo dirigido: cada aresta A→B indica que A precede B.";
        if (op == Op::Caixeiro)
        {
            if (n < 2)
                return "requer ao menos 2 vértices.";
            if (!grafoCompleto())
                return "requer um grafo completo: todos os pares de vértices conectados.";
        }
        return QString();
    }

    void atualizarDisponibilidade()
    {
        for (auto &it : itens)
        {
            it.cartao->definirIndisponivel(motivoIndisponivel(it.op));
        }
    }

    void filtrar(const QString &texto)
    {
        QString t = texto.trimmed().toLower();
        QMap<QLabel *, int> visiveis;
        for (auto &it : itens)
        {
            bool mostrar = t.isEmpty() || it.busca.contains(t);
            it.cartao->setVisible(mostrar);
            visiveis[it.grupo] += mostrar ? 1 : 0;
        }
        for (auto it = visiveis.begin(); it != visiveis.end(); ++it)
        {
            it.key()->setVisible(it.value() > 0);
        }
    }

    void limparResultado(bool limparTexto)
    {
        pararPlay();
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

    void mostrarAjuda()
    {
        QMessageBox caixa(this);
        caixa.setWindowTitle("Como usar o Grafos Studio");
        caixa.setIconPixmap(criarLogo(56));
        caixa.setTextFormat(Qt::RichText);
        caixa.setText(
            "<h3 style='color:#1c1917'>Como usar</h3>"
            "<table cellspacing='6'>"
            "<tr><td><b style='color:#ea580c'>Criar vértice</b></td><td>clique no espaço vazio</td></tr>"
            "<tr><td><b style='color:#ea580c'>Conectar</b></td><td>clique em um vértice e depois em outro</td></tr>"
            "<tr><td><b style='color:#ea580c'>Mover</b></td><td>arraste um vértice</td></tr>"
            "<tr><td><b style='color:#ea580c'>Editar custo</b></td><td>duplo clique na aresta</td></tr>"
            "<tr><td><b style='color:#ea580c'>Mais opções</b></td><td>botão direito em vértice ou aresta</td></tr>"
            "<tr><td><b style='color:#ea580c'>Cancelar conexão</b></td><td>Esc</td></tr>"
            "<tr><td><b style='color:#ea580c'>Passo a passo</b></td><td>← → navegam, Espaço reproduz</td></tr>"
            "<tr><td><b style='color:#ea580c'>Arquivo</b></td><td>Ctrl+N novo · Ctrl+O abrir · Ctrl+S salvar</td></tr>"
            "</table>");
        caixa.exec();
    }

    // -------------------------------------------------------
    // Edicao do grafo
    // -------------------------------------------------------
    void adicionarVertice(QPointF p)
    {
        if (n >= 100)
        {
            aviso("Limite atingido", "A interface suporta no máximo 100 vértices.");
            return;
        }
        limparResultado(false);
        g->cria_vertice();
        ++n;
        canvas->pos.append(p);
        atualizar();
        canvas->animarCriacao(n - 1);
    }

    void removerVertice(int v)
    {
        limparResultado(true);
        g->remove_vertice(v + 1);
        --n;
        canvas->pos.removeAt(v);
        canvas->animarCriacao(-1);
        atualizar();
    }

    // Dialogo de custo com pre-visualizacao da conexao.
    bool pedirCusto(int a, int b, int atual, int &custo)
    {
        QDialog dlg(this);
        dlg.setWindowTitle(atual > 0 ? "Editar conexão" : "Nova conexão");
        dlg.setMinimumWidth(360);
        auto v = new QVBoxLayout(&dlg);
        v->setContentsMargins(24, 22, 24, 20);
        v->setSpacing(10);
        auto titulo = new QLabel(atual > 0 ? "Editar custo da conexão" : "Nova conexão");
        titulo->setObjectName("dlgTitulo");
        QString seta = dirigido ? "  →  " : "  —  ";
        auto destaque = new QLabel(QString("%1%2%3").arg(a + 1).arg(seta).arg(b + 1));
        destaque->setObjectName("dlgDestaque");
        auto desc = new QLabel("Informe o custo (de 1 a 254). Ele é usado por Dijkstra, A* e pelo caixeiro viajante.");
        desc->setObjectName("dlgDesc");
        desc->setWordWrap(true);
        auto spin = new QSpinBox;
        spin->setRange(1, 254);
        spin->setValue(atual > 0 ? atual : 1);
        spin->selectAll();
        auto botoes = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        botoes->button(QDialogButtonBox::Ok)->setText("Salvar");
        botoes->button(QDialogButtonBox::Ok)->setObjectName("primario");
        botoes->button(QDialogButtonBox::Cancel)->setText("Cancelar");
        v->addWidget(titulo);
        v->addWidget(destaque);
        v->addWidget(desc);
        v->addSpacing(4);
        v->addWidget(spin);
        v->addSpacing(8);
        v->addWidget(botoes);
        connect(botoes, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(botoes, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
        if (dlg.exec() != QDialog::Accepted)
            return false;
        custo = spin->value();
        return true;
    }

    void conectar(int a, int b)
    {
        int atual = g->custo_conexao(a + 1, b + 1);
        int custo = 1;
        if (!pedirCusto(a, b, atual, custo))
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
        auto cab = menu.addAction(QString("Vértice %1").arg(v + 1));
        cab->setEnabled(false);
        menu.addSeparator();
        auto remover = menu.addAction("Remover vértice");
        menu.addSeparator();
        QMap<QAction *, Op> mapa;
        for (const auto &info : operacoes)
        {
            if (info.inicio)
            {
                auto act = menu.addAction("Iniciar aqui: " + info.nome);
                act->setEnabled(motivoIndisponivel(info.op).isEmpty());
                mapa[act] = info.op;
            }
        }
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
        QString seta = dirigido ? " → " : " — ";
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
        double r = qMax(40.0, qMin(canvas->width(), canvas->height()) / 2.0 - 90);
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

    // Grafo completo K5 com custos variados: bom para testar o caixeiro viajante.
    void carregarExemplo()
    {
        auto novo = std::make_unique<grafo>();
        novo->inicializa_grafo(5, false);
        for (int i = 1; i <= 5; ++i)
        {
            for (int j = i + 1; j <= 5; ++j)
            {
                novo->cria_conexao(i, j, 1 + (i * 7 + j * 3) % 9);
            }
        }
        substituirGrafo(std::move(novo), 5, false);
        canvas->mostrarToast("Exemplo carregado: grafo completo com 5 vértices");
    }

    void definirSeletorTipo(bool dir)
    {
        QSignalBlocker b1(btnDirigido), b2(btnNaoDirigido);
        btnDirigido->setChecked(dir);
        btnNaoDirigido->setChecked(!dir);
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
        canvas->animarCriacao(-1);
        definirSeletorTipo(dir);
        organizarCirculo();
        atualizar();
    }

    void alternarDirigido(bool novoDirigido)
    {
        auto novo = std::make_unique<grafo>();
        novo->inicializa_grafo(n, novoDirigido);
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                int w = g->custo_conexao(i + 1, j + 1);
                if (novoDirigido)
                {
                    // vindo de um grafo nao dirigido (aresta simetrica), mantem so um sentido
                    // (do menor para o maior vertice) para nao duplicar i->j e j->i
                    if (!dirigido && j < i)
                        continue;
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
            QMessageBox::warning(this, "Erro", "Não foi possível abrir o arquivo.");
            return;
        }
        std::istringstream dados(arquivo.readAll().toStdString());
        int vertices, tipo;
        if (!(dados >> vertices >> tipo) || vertices < 0 || vertices > 100 || (tipo != 0 && tipo != 1))
        {
            QMessageBox::warning(this, "Arquivo inválido", "Primeira linha: quantidade de vértices (0 a 100) e tipo (0 não dirigido, 1 dirigido).");
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
                QMessageBox::warning(this, "Arquivo inválido", "Cada conexão deve conter: origem destino custo (1 a 254).");
                return;
            }
            if (novo->custo_conexao(a, b) > 0)
                novo->remove_conexao(a, b);
            novo->cria_conexao(a, b, peso);
        }
        substituirGrafo(std::move(novo), vertices, tipo == 1);
        saida->setPlainText("Grafo importado: " + caminho);
        canvas->mostrarToast("Grafo aberto: " + QFileInfo(caminho).fileName());
    }

    void salvarArquivo()
    {
        QString caminho = QFileDialog::getSaveFileName(this, "Salvar grafo", "grafo.txt", "Grafos (*.txt)");
        if (caminho.isEmpty())
            return;
        QFile arquivo(caminho);
        if (!arquivo.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::warning(this, "Erro", "Não foi possível salvar o arquivo.");
            return;
        }
        QTextStream out(&arquivo);
        out << n << " " << (dirigido ? 1 : 0) << "\n";
        for (int i = 0; i < n; ++i)
        {
            for (int j = dirigido ? 0 : i; j < n; ++j)
            {
                int w = g->custo_conexao(i + 1, j + 1);
                if (w > 0)
                    out << i + 1 << " " << j + 1 << " " << w << "\n";
            }
        }
        canvas->mostrarToast("Grafo salvo: " + QFileInfo(caminho).fileName());
    }

    // -------------------------------------------------------
    // Execucao dos algoritmos
    // -------------------------------------------------------
    const OpInfo &info(Op op) const
    {
        for (const auto &i : operacoes)
        {
            if (i.op == op)
                return i;
        }
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
        dialogo.setMinimumWidth(380);
        auto v = new QVBoxLayout(&dialogo);
        v->setContentsMargins(24, 22, 24, 20);
        v->setSpacing(10);
        auto titulo = new QLabel(oi.nome);
        titulo->setObjectName("dlgTitulo");
        auto desc = new QLabel(oi.dica);
        desc->setObjectName("dlgDesc");
        desc->setWordWrap(true);
        v->addWidget(titulo);
        v->addWidget(desc);
        auto campos = new QFormLayout;
        campos->setVerticalSpacing(10);
        QSpinBox sa, sb;
        sa.setRange(1, n);
        sb.setRange(1, n);
        sa.setValue(a > 0 ? a : 1);
        sb.setValue(n);
        if (pedeInicio)
            campos->addRow("Vértice inicial", &sa);
        if (oi.fim)
            campos->addRow("Vértice destino", &sb);
        v->addLayout(campos);
        if (oi.op == Op::AEstrela)
        {
            auto nota = new QLabel("Nesta versão o A* usa heurística zero (equivale a Dijkstra até o destino).");
            nota->setObjectName("dlgDesc");
            nota->setWordWrap(true);
            v->addWidget(nota);
        }
        v->addSpacing(6);
        auto botoes = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        botoes->button(QDialogButtonBox::Ok)->setText("Executar");
        botoes->button(QDialogButtonBox::Ok)->setObjectName("primario");
        botoes->button(QDialogButtonBox::Cancel)->setText("Cancelar");
        v->addWidget(botoes);
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
        {
            for (int j = 1; j <= n; ++j)
            {
                if (i != j && g->custo_conexao(i, j) <= 0)
                    return false;
            }
        }
        return true;
    }

    void acionarOperacao(Op op, int inicio = 0)
    {
        const OpInfo &oi = info(op);
        QString motivo = motivoIndisponivel(op);
        if (!motivo.isEmpty())
        {
            motivo[0] = motivo[0].toUpper();
            aviso(oi.nome, motivo);
            return;
        }
        int a = inicio, b = 0;
        if (!pedirVertices(oi, a, b))
            return;
        limparResultado(false);
        abas->setCurrentIndex(0);
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
        {
            zeros += "0 ";
        }
        std::istringstream entrada(zeros);
        auto cout_antigo = cout.rdbuf(resultado.rdbuf());
        auto cin_antigo = cin.rdbuf(entrada.rdbuf());
        f();
        cout.rdbuf(cout_antigo);
        cin.rdbuf(cin_antigo);
        return QString::fromStdString(resultado.str());
    }

    static QColor corDaClasse(int c) { return QColor::fromHsv((c - 1) * 137 % 360, 190, 225); }

    static Legenda legendaPadrao()
    {
        return {{kAberto, "Descoberto"}, {kAtual, "Atual"}, {kFechado, "Processado"}, {kCaminho, "Caminho"}};
    }

    // Vertices alcancaveis (direto: seguindo as arestas; inverso: contra elas).
    QVector<bool> alcancaveis(int origem, bool inverso, std::set<Par> *arvore = nullptr) const
    {
        QVector<bool> vis(n, false);
        QList<int> fila;
        vis[origem] = true;
        fila.append(origem);
        while (!fila.isEmpty())
        {
            int v = fila.takeFirst();
            for (int u = 0; u < n; ++u)
            {
                int peso = inverso ? g->custo_conexao(u + 1, v + 1) : g->custo_conexao(v + 1, u + 1);
                if (peso > 0 && !vis[u])
                {
                    vis[u] = true;
                    fila.append(u);
                    if (arvore)
                        arvore->insert(inverso ? par(u + 1, v + 1, dirigido) : par(v + 1, u + 1, dirigido));
                }
            }
        }
        return vis;
    }

    // Pede a duracao de cada atividade (vertice). O valor inicial e o custo dos arcos
    // que saem do vertice (convencao da biblioteca: arco guarda a duracao da origem).
    bool pedirDuracoes(QVector<int> &duracao)
    {
        QDialog dialogo(this);
        dialogo.setWindowTitle("Caminho crítico");
        dialogo.setMinimumWidth(380);
        auto v = new QVBoxLayout(&dialogo);
        v->setContentsMargins(24, 22, 24, 20);
        v->setSpacing(10);
        auto titulo = new QLabel("Duração das atividades");
        titulo->setObjectName("dlgTitulo");
        auto desc = new QLabel("Cada vértice é uma atividade e cada aresta A→B indica que A precisa terminar antes de B.");
        desc->setObjectName("dlgDesc");
        desc->setWordWrap(true);
        v->addWidget(titulo);
        v->addWidget(desc);

        auto conteudo = new QWidget;
        auto campos = new QFormLayout(conteudo);
        campos->setVerticalSpacing(8);
        QVector<QSpinBox *> caixas;
        for (int i = 0; i < n; ++i)
        {
            int inicial = 1;
            for (int j = 0; j < n; ++j)
            {
                inicial = qMax(inicial, g->custo_conexao(i + 1, j + 1));
            }
            auto caixa = new QSpinBox;
            caixa->setRange(1, 9999);
            caixa->setValue(inicial);
            campos->addRow(QString("Atividade %1").arg(i + 1), caixa);
            caixas.append(caixa);
        }
        auto rolagem = new QScrollArea;
        rolagem->setWidget(conteudo);
        rolagem->setWidgetResizable(true);
        rolagem->setFrameShape(QFrame::NoFrame);
        rolagem->setMaximumHeight(360);
        v->addWidget(rolagem);

        auto botoes = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        botoes->button(QDialogButtonBox::Ok)->setText("Calcular");
        botoes->button(QDialogButtonBox::Ok)->setObjectName("primario");
        botoes->button(QDialogButtonBox::Cancel)->setText("Cancelar");
        v->addWidget(botoes);
        connect(botoes, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
        connect(botoes, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
        if (dialogo.exec() != QDialog::Accepted)
            return false;
        duracao.resize(n);
        for (int i = 0; i < n; ++i)
        {
            duracao[i] = caixas[i]->value();
        }
        return true;
    }

    // Caminho critico: o calculo e feito pela biblioteca (caminho_critico_manual).
    // O grafo tem 1 unico inicio (sem precedentes) e 1 unico fim (sem sucessoras).
    void executarCaminhoCritico()
    {
        QVector<int> duracao;
        if (!pedirDuracoes(duracao))
            return;
        int inicial = -1, fim = -1;
        for (int i = 1; i <= n; ++i)
        {
            bool semEntrada = true, semSaida = true;
            for (int j = 1; j <= n; ++j)
            {
                if (g->custo_conexao(j, i) > 0) semEntrada = false;
                if (g->custo_conexao(i, j) > 0) semSaida = false;
            }
            if (semEntrada) inicial = i;
            if (semSaida) fim = i;
        }
        if (inicial == -1 || fim == -1)
        {
            aviso("Caminho crítico", "É preciso um vértice sem precedentes (início) e um sem sucessores (fim).");
            return;
        }
        QString texto = capturar([&] { g->caminho_critico_manual(inicial, fim, duracao.data()); });

        Destaque d;
        QVector<int> caminho;
        QRegularExpression padraoCaminho("Caminho critico:\\s*(.*)");
        auto mc = padraoCaminho.match(texto);
        if (mc.hasMatch())
            for (const QString &parte : mc.captured(1).split("->"))
            {
                caminho.append(parte.trimmed().toInt());
            }
        // linhas da tabela: vertice, dur, ES, EF, LS, LF, folga, critico
        for (const QString &linha : texto.split('\n'))
        {
            QStringList c = linha.split('\t');
            if (c.size() < 8) continue;
            bool ok = false;
            int v = c[0].toInt(&ok);
            if (!ok) continue;
            int folga = c[6].toInt();
            d.cpm[v] = {c[1].toInt(), c[2].toInt(), c[3].toInt(), c[4].toInt(), c[5].toInt(), folga};
            if (folga == 0)
                d.cores[v] = kCaminho;
        }
        for (int k = 0; k + 1 < caminho.size(); ++k)
        {
            d.arestas.insert(par(caminho[k], caminho[k + 1], true));
        }
        d.legenda = {{kCaminho, "Atividade crítica (folga 0)"}, {kVerticePadrao, "Atividade com folga"}};
        canvas->destaque = d;
        saida->setPlainText(texto + "\nNo grafo: duração em cima, início/fim mais cedo (acima) e mais tarde (abaixo).");
        canvas->update();
    }

    void executarDireto(Op op, int a, int b)
    {
        if (op == Op::CaminhoCritico)
        {
            executarCaminhoCritico();
            return;
        }
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
            case Op::CaminhoCritico: break; // tratado em executarCaminhoCritico
            }
        });
        Destaque d;
        if (op == Op::Coloracao)
        {
            QRegularExpression padrao("Vertice\\s+(\\d+):\\s+cor\\s+(\\d+)");
            auto it = padrao.globalMatch(texto);
            QSet<int> classes;
            while (it.hasNext())
            {
                auto m = it.next();
                int cls = m.captured(2).toInt();
                d.cores[m.captured(1).toInt()] = corDaClasse(cls);
                classes.insert(cls);
            }
            QList<int> ordenadas = classes.values();
            std::sort(ordenadas.begin(), ordenadas.end());
            for (int c : ordenadas)
            {
                if (d.legenda.size() < 8)
                    d.legenda.append({corDaClasse(c), QString("Cor %1").arg(c)});
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
            d.legenda = {{kAtual, "Início"}, {kCaminho, "Percurso"}};
        }
        else if (op == Op::FechoDireto || op == Op::FechoInverso)
        {
            bool inverso = op == Op::FechoInverso;
            QVector<bool> vis = alcancaveis(a - 1, inverso, &d.arestas);
            for (int v = 0; v < n; ++v)
            {
                if (vis[v])
                    d.cores[v + 1] = kFechado;
            }
            d.cores[a] = kAtual;
            d.legenda = {{kAtual, "Origem"}, {kFechado, inverso ? "Alcança a origem" : "Alcançável"}};
        }
        else if (op == Op::Componentes)
        {
            // fechamento transitivo (n <= 100): mesma classe quando ha caminho nos dois sentidos
            QVector<QVector<bool>> alc(n);
            for (int v = 0; v < n; ++v)
            {
                alc[v] = alcancaveis(v, false);
            }
            QVector<int> classe(n, 0);
            int total = 0;
            for (int v = 0; v < n; ++v)
            {
                if (!classe[v])
                {
                    ++total;
                    for (int u = 0; u < n; ++u)
                    {
                        if (alc[v][u] && alc[u][v])
                            classe[u] = total;
                    }
                }
            }
            for (int v = 0; v < n; ++v)
            {
                d.cores[v + 1] = corDaClasse(classe[v]);
            }
            for (int c = 1; c <= total && c <= 8; ++c)
            {
                d.legenda.append({corDaClasse(c), QString("Componente %1").arg(c)});
            }
        }
        else if (op == Op::Ciclo3)
        {
            for (int i = 0; i < n; ++i)
            {
                for (int j = i + 1; j < n; ++j)
                {
                    for (int k = j + 1; k < n; ++k)
                    {
                        if (d.cores.isEmpty() && g->custo_conexao(i + 1, j + 1) > 0 && g->custo_conexao(j + 1, k + 1) > 0 && g->custo_conexao(i + 1, k + 1) > 0)
                        {
                            d.cores[i + 1] = d.cores[j + 1] = d.cores[k + 1] = kAtual;
                            d.arestas.insert(par(i + 1, j + 1, dirigido));
                            d.arestas.insert(par(j + 1, k + 1, dirigido));
                            d.arestas.insert(par(i + 1, k + 1, dirigido));
                        }
                    }
                }
            }
            if (!d.cores.isEmpty())
                d.legenda = {{kAtual, "Triângulo encontrado"}};
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
        {
            caminho.append(parte.trimmed().toInt());
        }
        return caminho;
    }

    // -------------------------------------------------------
    // Passo a passo
    // -------------------------------------------------------
    void pararPlay()
    {
        if (relogioPlay && relogioPlay->isActive())
            relogioPlay->stop();
        if (btnPlay)
            btnPlay->setIcon(icone(Ico::Play, QColor("#ffffff"), 20));
    }

    void alternarPlay()
    {
        if (passos.isEmpty())
            return;
        if (relogioPlay->isActive())
        {
            pararPlay();
            return;
        }
        if (passoAtual + 1 >= passos.size())
            irParaPasso(0);
        btnPlay->setIcon(icone(Ico::Pause, QColor("#ffffff"), 20));
        relogioPlay->start(kIntervalos[velocidade]);
    }

    void irParaPasso(int i)
    {
        if (i < 0 || i >= passos.size())
            return;
        passoAtual = i;
        canvas->destaque = passos[i].destaque;
        saida->setPlainText(passos[i].texto);
        rotuloPasso->setText(passos[i].texto.section('\n', 0, 0));
        contadorPasso->setText(QString("%1 / %2").arg(i + 1).arg(passos.size()));
        {
            QSignalBlocker bloqueio(controlePasso);
            controlePasso->setRange(0, passos.size() - 1);
            controlePasso->setValue(i);
        }
        btnAnterior->setEnabled(i > 0);
        btnProximo->setEnabled(i + 1 < passos.size());
        btnReiniciar->setEnabled(i > 0);
        barraPassos->show();
        canvas->update();
    }

    void gerarPassos(Op op, int inicio, int fim)
    {
        Destaque d;
        d.legenda = legendaPadrao();
        QVector<Passo> lista;
        auto registrar = [&](const QString &texto) { lista.append({d, texto}); };
        registrar("Estado inicial.\nAmarelo: descoberto  |  marrom: atual  |  verde: processado.");

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
                registrar(QString("Descobrir vértice %1.").arg(raiz + 1));
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
            {
                if (!visitado[v])
                    componente(v);
            }
            registrar("Percurso concluído: " + percurso.join(" -> "));
        }
        else if (op == Op::Coloracao)
        {
            d.legenda.clear();
            QVector<int> cor(n, 0), grau(n, 0);
            bool laco = false;
            for (int v = 0; v < n && !laco; ++v)
            {
                if (g->custo_conexao(v + 1, v + 1) > 0)
                    laco = true;
                for (int u = 0; u < n; ++u)
                {
                    if (g->custo_conexao(v + 1, u + 1) > 0)
                        ++grau[v];
                }
            }
            if (laco)
                registrar("Grafo com laço: coloração própria impossível.");
            else
            {
                for (int k = 0; k < n; ++k)
                {
                    int melhor = -1, sat = -1;
                    for (int v = 0; v < n; ++v)
                    {
                        if (!cor[v])
                        {
                            QSet<int> vizinhas;
                            for (int u = 0; u < n; ++u)
                            {
                                if (g->custo_conexao(v + 1, u + 1) > 0 && cor[u])
                                    vizinhas.insert(cor[u]);
                            }
                            if (melhor == -1 || vizinhas.size() > sat || (vizinhas.size() == sat && grau[v] > grau[melhor]))
                            {
                                melhor = v;
                                sat = vizinhas.size();
                            }
                        }
                    }
                    d.cores[melhor + 1] = kAtual;
                    registrar(QString("Selecionar %1: saturação %2, grau %3.").arg(melhor + 1).arg(sat).arg(grau[melhor]));
                    QSet<int> usadas;
                    for (int u = 0; u < n; ++u)
                    {
                        if (g->custo_conexao(melhor + 1, u + 1) > 0 && cor[u])
                            usadas.insert(cor[u]);
                    }
                    int c = 1;
                    while (usadas.contains(c))
                        ++c;
                    cor[melhor] = c;
                    d.cores[melhor + 1] = corDaClasse(c);
                    bool nova = true;
                    for (const auto &l : d.legenda)
                    {
                        if (l.first == corDaClasse(c))
                            nova = false;
                    }
                    if (nova && d.legenda.size() < 8)
                        d.legenda.append({corDaClasse(c), QString("Cor %1").arg(c)});
                    registrar(QString("Atribuir cor %1 ao vértice %2.").arg(c).arg(melhor + 1));
                }
                registrar("Coloração concluída.");
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
                {
                    d.rotulos[v + 1] = distancia[v] == infinito ? QString("∞") : QString("d=%1").arg(distancia[v]);
                }
            };
            auto tabelaTexto = [&] {
                QString t = "\nVertice | Custo | Antecessor\n";
                for (int v = 0; v < n; ++v)
                {
                    t += QString("%1 | %2 | %3\n").arg(v + 1).arg(distancia[v] == infinito ? "inf" : QString::number(distancia[v])).arg(anterior[v] < 0 ? "-" : QString::number(anterior[v] + 1));
                }
                return t;
            };
            rotular();
            d.cores[inicio] = kAberto;
            registrar((dij ? "Dijkstra: iniciar custos (limite original: 254)." : "A*: h=0 para todos os vértices.") + tabelaTexto());
            bool achou = false;
            for (int k = 0; k < n; ++k)
            {
                int v = -1;
                for (int u = 0; u < n; ++u)
                {
                    if (!pronto[u] && distancia[u] < infinito && (v == -1 || distancia[u] < distancia[v]))
                        v = u;
                }
                if (v == -1)
                    break;
                d.cores[v + 1] = kAtual;
                registrar(QString("Selecionar vértice %1 de menor custo.").arg(v + 1) + tabelaTexto());
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
                registrar(QString("Fechar vértice %1.").arg(v + 1) + tabelaTexto());
            }
            if (!dij && !achou)
                registrar("Destino inalcançável." + tabelaTexto());
            else if (dij)
                registrar("Busca concluída." + tabelaTexto());
        }
        else if (op == Op::Caixeiro)
        {
            d.legenda = {{kAtual, "Atual"}, {kAberto, "Próximo"}, {kCaminho, "Percorrido"}};
            // O percurso vem da propria funcao da biblioteca; os passos apenas o revelam.
            QString texto = capturar([&] { g->caixeiro_viajante_VADJPROX(inicio); });
            QVector<int> caminho = lerCaminho(texto);
            int acumulado = 0;
            d.cores[inicio] = kAtual;
            registrar(QString("Partir do vértice %1.").arg(inicio));
            for (int k = 1; k < caminho.size(); ++k)
            {
                int custoAresta = g->custo_conexao(caminho[k - 1], caminho[k]);
                acumulado += custoAresta;
                d.cores[caminho[k - 1]] = kCaminho;
                d.cores[caminho[k]] = (k + 1 == caminho.size()) ? kAtual : kAberto;
                d.arestas.insert(par(caminho[k - 1], caminho[k], dirigido));
                QString texto2 = (k + 1 == caminho.size())
                                     ? QString("Voltar ao vértice inicial %1 (custo %2).").arg(caminho[k]).arg(custoAresta)
                                     : QString("Vizinho mais próximo de %1: vértice %2 (custo %3).").arg(caminho[k - 1]).arg(caminho[k]).arg(custoAresta);
                d.rotulos[caminho[k]] = QString("+%1").arg(custoAresta);
                registrar(texto2 + QString("\nCusto acumulado: %1").arg(acumulado));
            }
            registrar("Ciclo concluído.\n" + texto);
        }

        passos = lista;
        irParaPasso(0);
    }

    // Uso interno: captura de tela para conferir o visual (--captura arquivo.png).
    void prepararDemonstracao()
    {
        carregarExemplo();
        gerarPassos(Op::Dijkstra, 1, 0);
        irParaPasso(7);
    }
};

constexpr int JanelaPrincipal::kIntervalos[4];
} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    app.setApplicationName("Grafos Studio");
    JanelaPrincipal janela;
    janela.show();

    QString captura;
    bool demo = false, vazio = false;
    for (int i = 1; i < argc; ++i)
    {
        QString a = argv[i];
        if (a == "--captura" && i + 1 < argc)
            captura = argv[++i];
        else if (a == "--demo")
            demo = true;
        else if (a == "--vazio")
            vazio = true;
    }
    if (!captura.isEmpty())
    {
        if (demo)
            janela.prepararDemonstracao();
        QTimer::singleShot(600, [&] {
            janela.grab().save(captura);
            app.quit();
        });
        Q_UNUSED(vazio);
    }
    return app.exec();
}
