# Trabalho de Grafos

Implementação em **C++** de um sistema interativo para criação, modificação, visualização e análise de grafos utilizando uma **matriz de adjacência dinâmica**.

O projeto permite trabalhar com grafos dirigidos e não dirigidos, adicionar e remover vértices e conexões durante a execução, executar BFS e DFS, calcular fechos transitivos e identificar Componentes Fortemente Conexos Máximos.

**Repositório:** `https://github.com/Thifjj/Grafos`

Esta documentação concentra-se principalmente em:

- `Trabalho.cpp` — contém a função `main()` e toda a interface do programa;
- `Trabalho.h` — contém a classe `grafo` e sua implementação.

---

## Sumário

1. [Funcionalidades](#1-funcionalidades)
2. [Estrutura do projeto](#2-estrutura-do-projeto)
3. [Representação do grafo](#3-representação-do-grafo)
4. [Classe `grafo`](#4-classe-grafo)
5. [Atributos da classe](#5-atributos-da-classe-grafo)
6. [Métodos da classe](#6-métodos-da-classe-grafo)
7. [Busca em Largura — BFS](#7-busca-em-largura--bfs)
8. [Busca em Profundidade — DFS](#8-busca-em-profundidade--dfs)
9. [Cálculo de fechos](#9-cálculo-de-fechos)
10. [Fecho Transitivo Direto](#10-fecho-transitivo-direto)
11. [Fecho Transitivo Inverso](#11-fecho-transitivo-inverso)
12. [Componentes Fortemente Conexos](#12-componentes-fortemente-conexos-máximos)
13. [Visualização da matriz](#13-visualização-da-matriz)
14. [`Trabalho.cpp`](#14-trabalhocpp)
15. [Menu e opções](#16-menu-principal)
16. [Compilação](#21-compilação)
17. [Resumo](#22-resumo-dos-métodos-da-classe-grafo)

---

## 1. Funcionalidades

O programa disponibiliza:

- criação de grafo dirigido ou não dirigido;
- definição da quantidade inicial de vértices;
- adição e remoção de vértices;
- adição e remoção de arestas ou arcos;
- visualização da matriz de adjacência;
- Busca em Largura — BFS;
- Busca em Profundidade — DFS;
- fecho transitivo direto;
- fecho transitivo inverso;
- identificação de Componentes Fortemente Conexos Máximos — SFCMs;
- verificação de conectividade forte em grafos dirigidos.

---

## 2. Estrutura do projeto

No repositório, a implementação atual está em:

```text
Grafos/
└── Trabalho_grafos_2/
    ├── Trabalho.cpp
    ├── Trabalho.h
    ├── README.md
    ├── Criador_grafos
    └── Criador_grafos.exe
```

### `Trabalho.cpp`

Arquivo principal da aplicação. Ele contém:

```cpp
int main()
```

e é responsável por:

- criar o objeto `grafo`;
- exibir o menu;
- ler as entradas do usuário;
- validar operações;
- chamar os métodos da classe `grafo`;
- controlar o fluxo geral da aplicação.

> A função de entrada continua sendo `main()`, porém ela está definida no arquivo `Trabalho.cpp`. Portanto, os comandos de compilação devem usar `Trabalho.cpp`, não `main.cpp`.

### `Trabalho.h`

Contém a classe `grafo` e a implementação dos métodos responsáveis pela representação, edição e análise do grafo.

### Executáveis

```text
Criador_grafos      -> Linux
Criador_grafos.exe  -> Windows
```

---

# 3. Representação do grafo

O grafo é armazenado através de uma **matriz de adjacência dinâmica**.

Para um grafo com `N` vértices, é criada uma matriz:

```text
N x N
```

Cada posição:

```cpp
matriz_adjacencia[i][j]
```

indica se existe uma conexão entre os vértices representados pelos índices `i` e `j`.

Uma conexão existente é representada por:

```text
1
```

A ausência de conexão é representada por:

```text
0
```

Exemplo:

```text
    1 2 3 4
1   0 1 0 0
2   1 0 1 0
3   0 1 0 1
4   0 0 1 0
```

Neste exemplo existem conexões entre:

```text
1 - 2
2 - 3
3 - 4
```

---

## 3.1 Numeração dos vértices

Para o usuário, os vértices são apresentados começando em:

```text
1
```

Entretanto, arrays e matrizes em C++ utilizam índices iniciando em:

```text
0
```

Por isso, sempre que um vértice informado pelo usuário precisa acessar a matriz, é utilizada a conversão:

```cpp
vertice - 1
```

Exemplo:

| Vértice apresentado ao usuário | Índice interno |
|---:|---:|
| 1 | 0 |
| 2 | 1 |
| 3 | 2 |
| 4 | 3 |

---

# 4. Classe `grafo`

A classe possui a seguinte estrutura principal:

```cpp
class grafo
{
private:
    int nos;
    bool conexo;
    bool dirigido;
    int **matriz_adjacencia;
};
```

---

# 5. Atributos da classe `grafo`

## `int nos`

Armazena a quantidade atual de vértices existentes no grafo.

Esse valor é atualizado quando:

- um grafo é criado;
- um vértice é adicionado;
- um vértice é removido.

---

## `bool conexo`

Armazena o resultado da análise de conectividade forte realizada pelo método:

```cpp
componentes_fortemente_conexos()
```

Quando existe somente um SFCM:

```cpp
conexo = true;
```

Quando existem dois ou mais SFCMs:

```cpp
conexo = false;
```

---

## `bool dirigido`

Indica o tipo do grafo.

```cpp
true
```

representa um grafo dirigido.

```cpp
false
```

representa um grafo não dirigido.

Esse atributo influencia principalmente a forma como as conexões são adicionadas e removidas da matriz.

---

## `int **matriz_adjacencia`

Ponteiro utilizado para armazenar dinamicamente a matriz de adjacência.

A matriz pode aumentar ou diminuir durante a execução porque o programa permite adicionar e remover vértices.

---

# 6. Métodos da classe `grafo`

## 6.1 `inicializa_grafo`

```cpp
void inicializa_grafo(int nos = 0, bool dirigido = false)
```

Inicializa o grafo.

### Parâmetros

- `nos`: quantidade inicial de vértices;
- `dirigido`: informa se o grafo será dirigido.

### Funcionamento

O método:

1. cria uma matriz dinâmica de tamanho `nos x nos`;
2. inicializa todas as posições da matriz com `0`;
3. armazena a quantidade de vértices em `this->nos`;
4. armazena o tipo do grafo em `this->dirigido`.

No momento da criação não existem conexões, portanto toda a matriz começa zerada.

Exemplo para três vértices:

```text
    1 2 3
1   0 0 0
2   0 0 0
3   0 0 0
```

---

## 6.2 `vertice_valido`

```cpp
bool vertice_valido(int v)
```

Verifica se um número informado representa um vértice existente.

A condição utilizada é:

```cpp
v >= 1 && v <= this->nos
```

### Retorno

Retorna:

```cpp
true
```

quando o vértice existe.

Retorna:

```cpp
false
```

quando o vértice está fora do intervalo válido.

Essa função é utilizada pelo `Trabalho.cpp` e por outros métodos da classe para evitar acessos inválidos à matriz.

---

## 6.3 `cria_vertice`

```cpp
void cria_vertice()
```

Adiciona um novo vértice ao grafo.

Como a matriz de adjacência possui tamanho fixo após cada alocação, a função precisa criar uma nova matriz maior.

### Funcionamento

Se o grafo possui `N` vértices:

```cpp
int novo_tamanho = this->nos + 1;
```

É criada uma nova matriz:

```text
(N + 1) x (N + 1)
```

Em seguida:

1. a nova matriz é criada;
2. todas as suas posições são inicializadas com `0`;
3. os dados da matriz antiga são copiados;
4. a matriz antiga é desalocada;
5. o ponteiro passa a apontar para a nova matriz;
6. `nos` é incrementado.

O novo vértice começa sem conexões.

Exemplo:

```text
Antes:

    1 2
1   0 1
2   1 0
```

Após adicionar um vértice:

```text
    1 2 3
1   0 1 0
2   1 0 0
3   0 0 0
```

---

## 6.4 `remove_vertice`

```cpp
void remove_vertice(int vertice)
```

Remove um vértice específico do grafo.

### Parâmetro

- `vertice`: número do vértice informado pelo usuário.

### Funcionamento

Primeiramente é verificado se o vértice existe:

```cpp
if (!vertice_valido(vertice))
```

O número fornecido pelo usuário é convertido para índice interno:

```cpp
int indice_remocao = vertice - 1;
```

Depois é criada uma matriz com uma linha e uma coluna a menos:

```cpp
int novo_tamanho = this->nos - 1;
```

Durante a cópia da matriz antiga:

- a linha correspondente ao vértice removido é ignorada;
- a coluna correspondente ao vértice removido é ignorada.

Isso remove automaticamente todas as conexões que envolviam aquele vértice.

Depois:

1. a matriz antiga é desalocada;
2. a nova matriz é armazenada;
3. a quantidade de vértices é reduzida.

### Renumeração

Após uma remoção, os vértices posteriores são renumerados pela nova matriz.

Exemplo:

```text
Vértices originais:

1 2 3 4 5
```

Removendo o vértice `3`:

```text
Antigo 1 -> novo 1
Antigo 2 -> novo 2
Antigo 3 -> removido
Antigo 4 -> novo 3
Antigo 5 -> novo 4
```

---

## 6.5 `cria_conexao`

```cpp
void cria_conexao(int a, int b)
```

Cria uma conexão entre dois vértices.

Antes da alteração, os dois vértices são validados.

---

### Grafo dirigido

Em um grafo dirigido é criada somente a conexão:

```text
a -> b
```

através de:

```cpp
matriz_adjacencia[a - 1][b - 1] = 1;
```

Exemplo:

```text
1 -> 2
```

produz:

```text
    1 2
1   0 1
2   0 0
```

---

### Grafo não dirigido

Em um grafo não dirigido são alteradas duas posições:

```cpp
matriz_adjacencia[a - 1][b - 1] = 1;
matriz_adjacencia[b - 1][a - 1] = 1;
```

Isso representa:

```text
a - b
```

Exemplo:

```text
1 - 2
```

produz:

```text
    1 2
1   0 1
2   1 0
```

---

## 6.6 `remove_conexao`

```cpp
void remove_conexao(int a, int b)
```

Remove uma conexão existente entre dois vértices.

---

### Grafo dirigido

Remove:

```text
a -> b
```

atribuindo:

```cpp
matriz_adjacencia[a - 1][b - 1] = 0;
```

---

### Grafo não dirigido

Remove as duas direções da matriz:

```cpp
matriz_adjacencia[a - 1][b - 1] = 0;
matriz_adjacencia[b - 1][a - 1] = 0;
```

---

# 7. Busca em Largura — BFS

## `bfs`

```cpp
void bfs(int vertice_inicial)
```

Executa uma **Busca em Largura** a partir de um vértice informado pelo usuário.

BFS significa:

```text
Breadth-First Search
```

O algoritmo visita primeiro os vértices mais próximos da origem e depois avança para os níveis seguintes.

### Funcionamento

A função:

1. valida o vértice inicial;
2. cria um vetor `visitados`;
3. inicializa todos os vértices como não visitados;
4. insere o vértice inicial em uma fila;
5. marca o vértice inicial como visitado;
6. retira um vértice da fila;
7. procura seus vizinhos na matriz de adjacência;
8. adiciona os vizinhos ainda não visitados à fila;
9. continua até a fila ficar vazia.

### Vetor de visitados

O vetor:

```cpp
bool *visitados
```

impede que um mesmo vértice seja processado repetidamente.

### Exemplo

Para:

```text
1 - 2
|   |
3 - 4
```

uma possível execução iniciando em `1` é:

```text
Percurso BFS: 1 2 3 4
```

---

# 8. Busca em Profundidade — DFS

## `dfs`

```cpp
void dfs(int vertice_inicial)
```

Executa uma **Busca em Profundidade** a partir do vértice escolhido.

DFS significa:

```text
Depth-First Search
```

Ao contrário da BFS, a DFS tenta avançar o máximo possível por um caminho antes de retornar.

### Funcionamento

A função:

1. valida o vértice inicial;
2. cria o vetor `visitados`;
3. inicializa uma pilha;
4. adiciona o vértice inicial à pilha;
5. retira o elemento do topo;
6. caso ainda não tenha sido visitado, marca-o;
7. adiciona seus vizinhos ainda não visitados à pilha;
8. continua enquanto houver elementos na pilha.

Os vizinhos são analisados da maior posição para a menor:

```cpp
for (int i = this->nos - 1; i >= 0; i--)
```

Como a estrutura utilizada é uma pilha, isso faz com que vértices de menor numeração tendam a ser processados primeiro.

---

# 9. Cálculo de fechos

## 9.1 `calcula_fecho`

```cpp
void calcula_fecho(int origem, bool inverso, int *nivel)
```

É a função interna utilizada pelos métodos de fecho transitivo e pela análise de componentes fortemente conexos.

### Parâmetros

- `origem`: índice interno do vértice de origem;
- `inverso`: define o sentido da busca;
- `nivel`: vetor que recebe o nível de cada vértice encontrado.

### Inicialização

Todos os vértices começam com:

```cpp
nivel[i] = -1;
```

O valor `-1` significa:

```text
vértice não alcançado
```

O vértice de origem recebe:

```cpp
nivel[origem] = 0;
```

---

### Fecho direto

Quando:

```cpp
inverso == false
```

a função procura conexões:

```text
atual -> i
```

utilizando:

```cpp
matriz_adjacencia[atual][i]
```

---

### Fecho inverso

Quando:

```cpp
inverso == true
```

o sentido é invertido.

A função verifica:

```text
i -> atual
```

através de:

```cpp
matriz_adjacencia[i][atual]
```

---

### Níveis

Quando um novo vértice é encontrado:

```cpp
nivel[i] = nivel[atual] + 1;
```

Assim, além de indicar quais vértices pertencem ao fecho, o vetor informa a distância em níveis a partir da origem.

---

# 10. Fecho Transitivo Direto

## `fecho_transitivo_direto`

```cpp
void fecho_transitivo_direto(int vertice_inicial)
```

Mostra os vértices alcançáveis a partir do vértice escolhido seguindo o sentido normal das conexões.

A função chama:

```cpp
calcula_fecho(vertice_inicial - 1, false, nivel);
```

### Saída

É apresentada uma tabela:

```text
Vertice    Nivel
1          0
2          1
3          2
4          -
```

O símbolo:

```text
-
```

representa um vértice que não pertence ao fecho transitivo direto da origem.

---

# 11. Fecho Transitivo Inverso

## `fecho_transitivo_inverso`

```cpp
void fecho_transitivo_inverso(int vertice_inicial)
```

Determina quais vértices conseguem alcançar o vértice escolhido.

A função utiliza:

```cpp
calcula_fecho(vertice_inicial - 1, true, nivel);
```

Ou seja, durante a busca a matriz é consultada no sentido inverso.

### Interpretação

Se o vértice `4` aparece no fecho transitivo inverso do vértice `1`, significa que existe algum caminho:

```text
4 -> ... -> 1
```

---

# 12. Componentes Fortemente Conexos Máximos

## `componentes_fortemente_conexos`

```cpp
void componentes_fortemente_conexos()
```

Identifica os **Componentes Fortemente Conexos Máximos — SFCMs** de um grafo dirigido.

O método somente executa sua análise quando:

```cpp
this->dirigido == true
```

Caso contrário, informa que a operação é destinada a grafos dirigidos.

---

## 12.1 Conceito

Dois vértices pertencem ao mesmo componente fortemente conexo quando existe caminho nos dois sentidos.

Para os vértices `A` e `B`:

```text
A consegue alcançar B
```

e:

```text
B consegue alcançar A
```

---

## 12.2 Funcionamento do algoritmo

É criado um vetor:

```cpp
int *componente
```

Inicialmente todos os valores recebem:

```cpp
-1
```

Isso indica que o vértice ainda não pertence a nenhum componente identificado.

Para cada vértice ainda não classificado:

1. calcula-se seu fecho transitivo direto;
2. calcula-se seu fecho transitivo inverso;
3. verifica-se quais vértices aparecem nos dois resultados;
4. os vértices presentes nos dois fechos são atribuídos ao mesmo SFCM.

A condição utilizada é:

```cpp
if (direto[i] != -1 &&
    inverso[i] != -1)
```

---

## 12.3 Exemplo

Considere:

```text
1 <-> 2 -> 3 <-> 4
```

Os componentes podem ser apresentados como:

```text
SFCM 1: { 1 2 }
SFCM 2: { 3 4 }
```

---

## 12.4 Verificação de conectividade

Após identificar os componentes, o método verifica quantos SFCMs existem.

Se:

```cpp
quantidade_componentes == 1
```

o grafo é considerado fortemente conexo:

```text
O grafo e FORTEMENTE CONEXO.
```

e:

```cpp
this->conexo = true;
```

Caso contrário:

```text
O grafo NAO e fortemente conexo.
```

e:

```cpp
this->conexo = false;
```

A quantidade de SFCMs também é mostrada.

---

# 13. Visualização da matriz

## `mostrar_grafo`

```cpp
void mostrar_grafo()
```

Apresenta a matriz de adjacência atual do grafo.

A primeira linha mostra os números dos vértices correspondentes às colunas.

Cada linha seguinte mostra:

- o número do vértice;
- as conexões armazenadas naquela linha.

Exemplo:

```text
Matriz de adjacencia:

   1 2 3
1  0 1 0
2  1 0 1
3  0 1 0
```

Essa visualização é utilizada tanto para consultar o grafo quanto para auxiliar o usuário durante operações de adição e remoção.

---

# 14. `Trabalho.cpp`

O arquivo `Trabalho.cpp` é responsável pela interface do programa.

O fluxo geral é:

```text
Inicialização
     |
     v
Criação do objeto grafo
     |
     v
Exibição do menu
     |
     v
Leitura da opção
     |
     v
Execução da operação escolhida
     |
     v
Retorno ao menu
     |
     v
Opção 0 encerra o programa
```

---

# 15. Variáveis utilizadas no `Trabalho.cpp`

## `grafo g`

```cpp
grafo g;
```

Cria o objeto principal que representa o grafo.

Todas as operações estruturais e algoritmos são executados através desse objeto.

Exemplos:

```cpp
g.cria_vertice();
g.cria_conexao(a, b);
g.bfs(vertice);
g.dfs(vertice);
```

---

## `int opcao`

Armazena a opção escolhida no menu principal.

---

## `int nos`

Mantém no `Trabalho` a quantidade de vértices apresentada ao usuário.

Esse valor também é atualizado ao adicionar ou remover vértices.

---

## `int a, b`

Armazenam os dois vértices usados na criação ou remoção de conexões.

---

## `int vertice`

Utilizada nas operações que precisam de apenas um vértice, como:

- remover vértice;
- BFS;
- DFS;
- fecho direto;
- fecho inverso.

---

## `int comando`

Armazena a quantidade de vértices que o usuário deseja adicionar.

---

## `char dirigido`

Recebe:

```text
s
```

ou:

```text
n
```

durante a criação do grafo.

---

## `bool grafo_criado`

Indica se um grafo já foi inicializado.

Isso impede que determinadas operações sejam executadas antes da criação do grafo.

---

## `bool eh_dirigido`

Mantém no `Trabalho` a informação sobre o tipo do grafo.

É usada principalmente para:

- adaptar as mensagens da interface;
- impedir a execução da análise de SFCMs em grafos não dirigidos.

---

# 16. Menu principal

O programa utiliza um laço:

```cpp
do
{
    ...
} while (opcao != 0);
```

Isso mantém o programa em execução até o usuário escolher:

```text
0 - Sair
```

O menu atual contém:

```text
1  - Criar grafo
2  - Adicionar vertices
3  - Remover vertice
4  - Adicionar conexoes
5  - Remover conexoes
6  - Mostrar matriz de adjacencia
7  - Busca em largura (BFS)
8  - Busca em profundidade (DFS)
9  - Fecho transitivo direto
10 - Fecho transitivo inverso
11 - Verificar conectividade / SFCMs
0  - Sair
```

A opção escolhida é tratada através de:

```cpp
switch (opcao)
```

---

# 17. Opções do `Trabalho.cpp`

## Opção 1 — Criar grafo

```text
1 - Criar grafo
```

Permite criar o grafo inicial.

O usuário informa:

1. quantidade inicial de vértices;
2. se o grafo é dirigido.

A quantidade de vértices não pode ser negativa.

Depois é executado:

```cpp
g.inicializa_grafo(nos, eh_dirigido);
```

Após a criação:

```cpp
grafo_criado = true;
```

O programa impede a criação de um segundo grafo sobre o atual.

---

## Opção 2 — Adicionar vértices

```text
2 - Adicionar vertices
```

Permite informar quantos vértices serão adicionados.

Exemplo:

```text
Quantidade: 5
```

O `Trabalho` executa:

```cpp
for (int i = 0; i < comando; i++)
{
    g.cria_vertice();
    nos++;
}
```

Portanto, cinco novos vértices são criados sem exigir que o usuário execute a opção cinco vezes.

O valor:

```text
-1
```

retorna ao menu.

---

## Opção 3 — Remover vértice

```text
3 - Remover vertice
```

Mostra a matriz atual e solicita qual vértice deve ser removido.

Antes da remoção é feita a validação:

```cpp
g.vertice_valido(vertice)
```

A remoção é realizada através de:

```cpp
g.remove_vertice(vertice);
```

Depois o contador usado pelo `Trabalho` é atualizado:

```cpp
nos--;
```

Se todos os vértices forem removidos, o programa informa que o grafo ficou sem vértices.

---

## Opção 4 — Adicionar conexões

```text
4 - Adicionar conexoes
```

Permite criar arestas ou arcos.

Para um grafo dirigido, a interface apresenta:

```text
ORIGEM -> DESTINO
```

Para um grafo não dirigido:

```text
VERTICE A - VERTICE B
```

Os dois vértices são validados antes da chamada:

```cpp
g.cria_conexao(a, b);
```

O usuário pode continuar adicionando conexões até informar:

```text
-1
```

como primeiro vértice.

---

## Opção 5 — Remover conexões

```text
5 - Remover conexoes
```

Solicita dois vértices e chama:

```cpp
g.remove_conexao(a, b);
```

A função da classe determina automaticamente se deve remover:

- apenas um arco;
- ou as duas posições correspondentes a uma aresta não dirigida.

---

## Opção 6 — Mostrar matriz de adjacência

```text
6 - Mostrar matriz de adjacencia
```

Executa:

```cpp
g.mostrar_grafo();
```

A matriz permanece visível até o usuário continuar.

---

## Opção 7 — Busca em largura

```text
7 - Busca em largura (BFS)
```

Solicita o vértice inicial e executa:

```cpp
g.bfs(vertice);
```

O resultado mostra a ordem dos vértices visitados.

---

## Opção 8 — Busca em profundidade

```text
8 - Busca em profundidade (DFS)
```

Solicita o vértice inicial e executa:

```cpp
g.dfs(vertice);
```

O resultado mostra a sequência obtida pela busca em profundidade.

---

## Opção 9 — Fecho transitivo direto

```text
9 - Fecho transitivo direto
```

Solicita um vértice e chama:

```cpp
g.fecho_transitivo_direto(vertice);
```

O resultado apresenta:

- os vértices alcançados;
- o nível de cada vértice em relação à origem;
- `-` para vértices não alcançados.

---

## Opção 10 — Fecho transitivo inverso

```text
10 - Fecho transitivo inverso
```

Solicita um vértice e executa:

```cpp
g.fecho_transitivo_inverso(vertice);
```

O resultado indica quais vértices conseguem alcançar o vértice selecionado.

---

## Opção 11 — Conectividade / SFCMs

```text
11 - Verificar conectividade / SFCMs
```

No código atual, essa opção é utilizada para grafos dirigidos.

Para grafos dirigidos é executado:

```cpp
g.componentes_fortemente_conexos();
```

O programa então:

1. identifica os SFCMs;
2. apresenta os vértices de cada componente;
3. informa se o grafo é fortemente conexo;
4. informa a quantidade de componentes quando houver mais de um.

Para grafos não dirigidos, o `Trabalho` informa que a análise atual de SFCMs é destinada a grafos dirigidos e não chama o método.

---

## Opção 0 — Sair

```text
0 - Sair
```

Encerra o laço principal e finaliza o programa.

---

# 18. Validações realizadas pelo `Trabalho.cpp`

O `Trabalho.cpp` realiza diversas verificações antes de chamar os métodos do grafo.

Entre elas:

### Grafo ainda não criado

Operações que dependem de um grafo verificam:

```cpp
if (!grafo_criado)
```

---

### Grafo sem vértices

Operações como BFS, DFS e fechos verificam:

```cpp
nos == 0
```

---

### Vértice inexistente

As operações que recebem vértices utilizam:

```cpp
g.vertice_valido(...)
```

---

### Retorno ao menu

Em várias opções o valor:

```text
-1
```

é utilizado como comando de retorno ao menu sem executar a operação.

---

# 19. Fluxo entre `Trabalho.cpp` e a classe `grafo`

A interface não acessa diretamente a matriz de adjacência.

O `Trabalho.cpp` recebe a solicitação do usuário e chama um método da classe.

Exemplo de criação de conexão:

```text
Usuário escolhe opção 4
        |
        v
Trabalho.cpp lê os vértices A e B
        |
        v
Trabalho.cpp valida os vértices
        |
        v
g.cria_conexao(A, B)
        |
        v
classe grafo altera a matriz
```

Outro exemplo, para BFS:

```text
Usuário escolhe opção 7
        |
        v
Trabalho.cpp lê o vértice inicial
        |
        v
g.bfs(vertice)
        |
        v
classe grafo percorre a matriz
        |
        v
sequência é apresentada
```

Essa divisão mantém a interface separada da implementação das operações do grafo.

---

# 20. Exemplo de utilização

Um fluxo possível é:

```text
1 - Criar grafo
```

Entrada:

```text
Numero inicial de vertices: 4
O grafo sera dirigido? (s/n): s
```

Depois:

```text
4 - Adicionar conexoes
```

Criando:

```text
1 -> 2
2 -> 3
3 -> 1
3 -> 4
```

A matriz será semelhante a:

```text
    1 2 3 4
1   0 1 0 0
2   0 0 1 0
3   1 0 0 1
4   0 0 0 0
```

Executando BFS a partir de `1`:

```text
Percurso BFS: 1 2 3 4
```

Executando o fecho transitivo direto de `1`:

```text
Vertice    Nivel
1          0
2          1
3          2
4          3
```

Na análise dos SFCMs:

```text
SFCM 1: { 1 2 3 }
SFCM 2: { 4 }
```

Como existem dois componentes, o grafo não é fortemente conexo.

---

# 21. Compilação

O projeto utiliza apenas recursos da linguagem C++ e da biblioteca padrão.

## Linux

Com `g++`:

```bash
g++ -std=c++17 Trabalho.cpp -o Criador_grafos
g++ -std=c++17 Trabalho.cpp -o Grafo
```

Executar:

```bash
./Criador_grafos
```

---

## Windows

Com MinGW-w64:

```bash
g++ -std=c++17 Trabalho.cpp -o Criador_grafos.exe
g++ -std=c++17 Trabalho.cpp -o Grafo.exe
```

Executar no PowerShell:

```powershell
.\Criador_grafos.exe
```

---

## Gerando o executável Windows a partir do Linux

Com o compilador cruzado MinGW-w64:

```bash
x86_64-w64-mingw32-g++ -std=c++17 Trabalho.cpp -o Criador_grafos.exe -static -static-libgcc -static-libstdc++
x86_64-w64-mingw32-g++ -std=c++17 Trabalho.cpp -o Grafo.exe -static -static-libgcc -static-libstdc++
```

---

# 22. Resumo dos métodos da classe `grafo`

| Método | Função |
|---|---|
| `inicializa_grafo()` | Cria e inicializa a matriz de adjacência |
| `vertice_valido()` | Verifica se um vértice existe |
| `cria_vertice()` | Aumenta o grafo em um vértice |
| `remove_vertice()` | Remove um vértice, sua linha e sua coluna |
| `cria_conexao()` | Cria uma aresta ou arco |
| `remove_conexao()` | Remove uma aresta ou arco |
| `bfs()` | Executa busca em largura |
| `dfs()` | Executa busca em profundidade |
| `calcula_fecho()` | Calcula internamente o fecho direto ou inverso e seus níveis |
| `fecho_transitivo_direto()` | Exibe os vértices alcançáveis a partir de uma origem |
| `fecho_transitivo_inverso()` | Exibe os vértices que conseguem alcançar uma origem |
| `componentes_fortemente_conexos()` | Identifica os SFCMs e verifica conectividade forte |
| `mostrar_grafo()` | Mostra a matriz de adjacência |

---

# 23. Resumo das opções do programa

| Opção | Operação |
|---:|---|
| `1` | Criar grafo |
| `2` | Adicionar vértices |
| `3` | Remover vértice |
| `4` | Adicionar conexões |
| `5` | Remover conexões |
| `6` | Mostrar matriz |
| `7` | BFS |
| `8` | DFS |
| `9` | Fecho transitivo direto |
| `10` | Fecho transitivo inverso |
| `11` | Conectividade forte / SFCMs |
| `0` | Encerrar |

---

# 24. Observação sobre a implementação atual

A análise implementada pela opção de conectividade utiliza **Componentes Fortemente Conexos**, portanto a verificação existente está direcionada a **grafos dirigidos**.

No código atual, grafos não dirigidos podem utilizar normalmente:

- criação e remoção de vértices;
- criação e remoção de arestas;
- matriz de adjacência;
- BFS;
- DFS;
- fechos transitivos.

Entretanto, a opção `11` não executa uma verificação específica de conectividade para grafos não dirigidos.

---

## Conclusão

O projeto utiliza uma matriz de adjacência dinâmica para permitir que a estrutura do grafo seja modificada durante a execução.

A classe `grafo` concentra as operações sobre a estrutura e os algoritmos, enquanto o `Trabalho.cpp` controla a interação com o usuário e direciona cada comando para o método correspondente.

Essa organização permite realizar tanto operações básicas de edição do grafo quanto algoritmos de análise, incluindo BFS, DFS, fechos transitivos e identificação de Componentes Fortemente Conexos Máximos.
