# Biblioteca de grafos

Este projeto consiste em uma biblioteca de funções relacionadas a grafos. As funções que compõem a biblioteca são:

* **bipartido**: verifica se o grafo é bipartido
* **n_arestas**: retorna o número de arestas do grafo
* **n_vertices**: retorna o número de vertices do grafo
* **n_componentes**: retorna o número de componentes do grafo
* **diametros**: retorna o diametro de cada componente do grafo
* **vertices_corte**: retorna o nome dos vertices de corte do grafo
* **arestas_corte**: retorna o nome das arestas de corte do grafo

Informações sobre o formato de entrada e de saída de cada uma das funções, assim como funções e estruturas auxiliares, estão melhores descritas em comentários no arquivo *grafo.h*.

## Importante
Para otimização de desempenho, poderiam ser feitas uma única função de busca em largura e uma de busca em profundidade, das quais todas as outras informações poderiam ser extraídas. Por exemplo, a quantidade de componentes conexas, a verificação de bipartição e o cálculo do diâmetro do grafo poderiam ser obtidos com uma única execução da busca em largura, evitando múltiplas passagens pelo grafo. Esta abordagem não foi usada apenas pela complexidade que adicionaria ao desenvolvimento.