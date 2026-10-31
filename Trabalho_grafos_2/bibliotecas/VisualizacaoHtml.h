#ifndef VISUALIZACAO_HTML_H
#define VISUALIZACAO_HTML_H
#include <fstream>
#include <sstream>
#include <streambuf>
#include <cmath>
#include <cstdlib>
#include <map>
#include <cstdio>
#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

// ADICIONADO: copia a saida para a pagina mantendo a impressao no terminal.
class SaidaVisual : public std::streambuf {
    std::ostream &saida;
    std::streambuf *original;
    std::string historico;
protected:
    int_type overflow(int_type c) override {
        if (traits_type::eq_int_type(c, traits_type::eof())) return traits_type::not_eof(c);
        historico += traits_type::to_char_type(c);
        if(historico.size()>18000) historico.erase(0,6000);
        return original->sputc(traits_type::to_char_type(c));
    }
    int sync() override { return original->pubsync(); }
public:
    explicit SaidaVisual(std::ostream &s):saida(s),original(s.rdbuf(this)) {}
    ~SaidaVisual() { saida.rdbuf(original); }
    std::string texto() const { return historico; }
};
inline std::string escaparHtml(const std::string &s) {
    std::string r;for(char c:s) {
        if(c=='&')r+="&amp;";else if(c=='<')r+="&lt;";else if(c=='>')r+="&gt;";else r+=c;
    }return r;
}
// ADICIONADO: os valores sao lidos diretamente da matriz C++, sem algoritmos JS.
inline void salvarVisualizacao(const grafo &g,int n,bool dirigido,const std::string &texto) {
    std::ofstream f("grafos_visualizacao.html",std::ios::binary);
    if(!f)return;
    f<<"<!doctype html><html lang='pt-BR'><meta charset='utf-8'><meta http-equiv='refresh' content='2'><title>Visualizacao do grafo</title>"
        "<style>body{font:16px system-ui;background:#eef2f7;color:#172554;margin:30px}main{display:flex;gap:20px}section{background:white;padding:24px;border-radius:15px;flex:1}svg{width:100%;height:550px}pre{white-space:pre-wrap;max-height:550px;overflow:auto}text{font:16px system-ui}</style>"
        "<h1>Visualizacao do grafo</h1><p>Use o menu do terminal. Esta pagina atualiza automaticamente a cada 2 segundos.</p><main><section>";
    f<<n<<" vertices | "<<(dirigido?"Dirigido":"Nao dirigido")<<"<svg viewBox='-300 -300 600 600'><defs><marker id='seta' viewBox='0 0 10 10' refX='10' refY='5' markerWidth='7' markerHeight='7' orient='auto'><path d='M0 0L10 5L0 10' fill='gray'/></marker></defs>";
    auto ponto=[&](int i){double angulo=6.283185307*i/n;return std::pair<double,double>(220*cos(angulo),220*sin(angulo));};
    for(int i=0;i<n;i++)for(int j=0;j<n;j++) {
        int custo=g.custo_conexao(i+1,j+1);if(!custo||(!dirigido&&j<i))continue;
        auto [x,y]=ponto(i);auto [a,b]=ponto(j);
        if(i==j)f<<"<ellipse cx='"<<x<<"' cy='"<<y-25<<"' rx='20' ry='30' fill='none' stroke='gray'/>";
        else {double d=hypot(a-x,b-y);a-=(a-x)*22/d;b-=(b-y)*22/d;f<<"<line x1='"<<x<<"' y1='"<<y<<"' x2='"<<a<<"' y2='"<<b<<"' stroke='gray' "<<(dirigido?"marker-end='url(#seta)'":"")<<"/>";}
        f<<"<text x='"<<(x+a)/2<<"' y='"<<(y+b)/2-5<<"'>"<<custo<<"</text>";
    }
    // Usa somente a coloracao ja calculada pelo metodo C++.
    std::map<int,int> cores;
    auto inicio=texto.rfind("Coloracao do grafo:");
    if(inicio!=std::string::npos) {
        std::istringstream linhas(texto.substr(inicio));std::string linha;
        while(std::getline(linhas,linha)) {int v,c;if(std::sscanf(linha.c_str(),"Vertice %d: cor %d",&v,&c)==2)cores[v]=c;}
    }
    for(int i=0;i<n;i++) {auto [x,y]=ponto(i);std::string cor=cores.count(i+1)?"hsl("+std::to_string((cores[i+1]-1)*137%360)+",75%,76%)":"#dbeafe";
        f<<"<circle cx='"<<x<<"' cy='"<<y<<"' r='21' fill='"<<cor<<"' stroke='#172554'/><text x='"<<x<<"' y='"<<y+5<<"' text-anchor='middle'>"<<i+1<<"</text>";
    }
    f<<"</svg></section><section><h2>Saida do terminal</h2><pre>"<<escaparHtml(texto)<<"</pre></section></main></html>";
}
inline void abrirVisualizacao() {
#ifdef _WIN32
    int resultado=std::system("start \"\" grafos_visualizacao.html");(void)resultado;
#else
    // Processo intermediario evita deixar um processo filho pendente no terminal.
    pid_t filho=fork();
    if(filho==0) {if(fork()==0){execlp("xdg-open","xdg-open","grafos_visualizacao.html",static_cast<char*>(nullptr));_exit(1);}_exit(0);}
    if(filho>0)waitpid(filho,nullptr,0);
#endif
}
#endif
