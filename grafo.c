#include "grafo.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_LINHA 2047

// como so e usado dentro da struct vertice, so guarda a outra ponta e o peso
typedef struct {
	char *nome_ponta;
	unsigned int peso_aresta;
} aresta;

// vertice tem um nome e seus vizinhos
typedef struct {
	char *nome;
	unsigned int grau;
	unsigned int estado;
	unsigned int componente;
	aresta *arestas;
} vertice;

// grafo guarda o nome e seus vertices
struct grafo {
	char *nome;
	unsigned int num_vertices;
	unsigned int num_arestas;
	vertice *vertices;	
};

// dados da DFS - para encontrar vertices de corte
typedef struct {
	unsigned int *tempo_descoberta;
	unsigned int *low;
	int *pai;
	unsigned int *eh_corte;
	unsigned int tempo_atual;
} dados_dfs_vertice;

typedef struct {
	char *u;
	char *v;
} aresta_corte;

// dados da DFS - para encontrar arestas de corte
typedef struct {
	unsigned int *tempo_descoberta;
	unsigned int *low;
	int *pai;
	unsigned int tempo_atual;
} dados_dfs_aresta;

/* -------------------------- DECLARAÇÃO DE FUNÇÕES (evitar problemas com o compilador) -------------------------- */
void remove_quebra_linha(char *str);
vertice *busca_ou_cria_vertice(grafo *g, const char *nome);
void adiciona_aresta(vertice *vert, const char *destino, unsigned int peso);
char *copia_str(const char *str);
unsigned int indice_do_vertice(grafo *g, const char *nome);
unsigned int *djikstra(grafo *g, unsigned int origem);
unsigned int diametro_componente(grafo *g, unsigned int *vertices_componente, unsigned int tamanho_componente);
void dfs_corte_vertices(grafo *g, unsigned int u, dados_dfs_vertice *dados);
int compara_nome_vertices(const void *a, const void *b);
void dfs_corte_arestas(grafo *g, unsigned int u, dados_dfs_aresta *dados, aresta_corte *arestas, unsigned int *contador);
int compara_nome_arestas(const void *a, const void *b);
int compara_uint(const void *a, const void *b);

/* -------------------------- FUNÇÕES AUXILIARES -------------------------- */
// Remove o \n de str
void remove_quebra_linha(char *str) {
	char *quebra = strchr(str, '\n');
	
	if (quebra)
		*quebra = '\0';
}

// Se vertice com o nome ja existe, o retorna. Se não existe, cria um novo com esse nome e o retorna.
vertice *busca_ou_cria_vertice(grafo *g, const char *nome) {
	// Se encontra o vertice, o retorna
	for (unsigned int i = 0; i < g->num_vertices; i++) {
		if (strcmp(g->vertices[i].nome, nome) == 0)
			return &g->vertices[i];
	}

	// Se nao encontra o vertice, aumenta tamanho da lista de vertices e cria 
	vertice *realocacao_vert;
	realocacao_vert = realloc(g->vertices, (g->num_vertices+1) * sizeof(vertice)); 
	if (!realocacao_vert) {
		exit(-1);
	}
	g->vertices = realocacao_vert;

	vertice *vert = &g->vertices[g->num_vertices];
	vert->nome = copia_str(nome);
	vert->grau = 0;
	vert->arestas = NULL;
	g->num_vertices++;

	return vert;
}

// Adiciona uma aresta vert -- destino com o peso passado como parametro
void adiciona_aresta(vertice *vert, const char *destino, unsigned int peso) {
	aresta *realocacao_arestas;

	realocacao_arestas = realloc(vert->arestas, (size_t)(vert->grau+1) * sizeof(aresta)); 
	if (!realocacao_arestas) {
		exit(-1);
	}

	vert->arestas = realocacao_arestas;
	vert->arestas[vert->grau].nome_ponta = copia_str(destino);
	vert->arestas[vert->grau].peso_aresta = peso;
	vert->grau++;
}

// Cria uma nova string com o mesmo conteudo de str 
char *copia_str(const char *str) {
	char *copia = (char*) malloc(strlen(str) + 1);

	if (copia)
		strcpy(copia, str);

	return copia;
}

// Retorna o indice do vertice na lista de vertices do grafo g
// Se vertice nao existe, retorna UINT_MAX
unsigned int indice_do_vertice(grafo *g, const char *nome) {
	for (unsigned int i = 0; i < g->num_vertices; i++) {
		if (strcmp(g->vertices[i].nome, nome) == 0) {
			return i;
		}
	}
	return UINT_MAX;
}

// aplica djikstra e retorna, em distancias, o valor da distancia de origem para cada um dos vertices do grafo
unsigned int *djikstra(grafo *g, unsigned int origem) {
	unsigned int *distancias = (unsigned int*) malloc(sizeof(unsigned int) * g->num_vertices);
	unsigned int *visitados = (unsigned int*) malloc(sizeof(unsigned int) * g->num_vertices);

	if ((!distancias) || (!visitados)) {
		free(distancias);
		free(visitados);
		return NULL;
	}

	// Inicializa todas as distancias com o maior valor possivel
	for (unsigned int i = 0; i < g->num_vertices; i++) {
		distancias[i] = UINT_MAX;
		visitados[i] = 0;
	}

	// Distancia de um vertice para ele mesmo é 0
	distancias[origem] = 0;

	// Loop principal
	for (unsigned int i = 0; i < g->num_vertices; i++) {
		unsigned int min_indice = UINT_MAX;
		unsigned int min_valor = UINT_MAX;

		for (unsigned int j = 0; j < g->num_vertices; j++) {
			if ((!visitados[j]) && (distancias[j]) < min_valor) {
				min_valor = distancias[j];
				min_indice = j;
			}
		}

		if (min_indice == UINT_MAX) {
			break;
		}

		visitados[min_indice] = 1;
		vertice *vert = &g->vertices[min_indice];

		for (unsigned int j = 0; j < vert->grau; j++) {
			char *nome_vizinho = vert->arestas[j].nome_ponta;
			unsigned int indice_vizinho = indice_do_vertice(g, nome_vizinho);
			unsigned int peso = vert->arestas[j].peso_aresta;

			if ((indice_vizinho != UINT_MAX) && (!visitados[indice_vizinho])) {
				unsigned int nova_distancia = distancias[min_indice] + peso;
				if (nova_distancia < distancias[indice_vizinho]) {
					distancias[indice_vizinho] = nova_distancia;
				}
			}
		}
	}

	free(visitados);
	return distancias;
}

// Calcula o diametro de uma componente conexa
unsigned int diametro_componente(grafo *g, unsigned int *vertices_componente, unsigned int tamanho_componente) {
	if ((tamanho_componente == 0) || (tamanho_componente == 1)) {
		return 0;
	}

	unsigned int diametro = 0;
	for (unsigned int i = 0; i < tamanho_componente; i++) {
		unsigned int origem = vertices_componente[i];
		unsigned int *distancias = djikstra(g, origem);

		if (!distancias) {
			continue;
		}

		for (unsigned int j = 0; j < tamanho_componente; j++) {
			unsigned int destino = vertices_componente[j];

			if ((destino != origem) && (distancias[destino] != UINT_MAX) && (distancias[destino] > diametro)) {
				diametro = distancias[destino];
			}
		}

		free(distancias);
	}
	return diametro;
}

// Busca em profundidade auxiliar para analise dos vertices de corte
void dfs_corte_vertices(grafo *g, unsigned int u, dados_dfs_vertice *dados) {
	unsigned int filhos = 0;
	dados->tempo_descoberta[u] = dados->tempo_atual;
	dados->low[u] = dados->tempo_atual;
	dados->tempo_atual++;

	vertice *vert_u = &g->vertices[u];

	for (unsigned int i = 0; i < vert_u->grau; i++) {
		char *nome_vizinho = vert_u->arestas[i].nome_ponta;
		unsigned int indice_vizinho = indice_do_vertice(g, nome_vizinho);

		if (indice_vizinho == UINT_MAX) {
			continue;
		}

		if (dados->tempo_descoberta[indice_vizinho] == UINT_MAX) {
			filhos++;
			dados->pai[indice_vizinho] = (int) u;
			dfs_corte_vertices(g, indice_vizinho, dados);

			if (dados->low[indice_vizinho] < dados->low[u]) {
				dados->low[u] = dados->low[indice_vizinho];
			}

			if ((dados->pai[u] != -1) && (dados->low[indice_vizinho] >= dados->tempo_descoberta[u])) {
				dados->eh_corte[u] = 1;
			}
		} else if ((int)indice_vizinho != dados->pai[u]) {
			if (dados->tempo_descoberta[indice_vizinho] < dados->low[u]) {
				dados->low[u] = dados->tempo_descoberta[indice_vizinho];
			}
		}
	}

	if ((dados->pai[u] == -1) && (filhos >= 2)) {
		dados->eh_corte[u] = 1;
	} 
}

// Função de comparação para ordenação alfabética
int compara_nome_vertices(const void *a, const void *b) {
	return strcmp(*(const char * const*)a, *(const char * const *)b);
}

// Busca em profundidade auxiliar para analise das arestas de corte
void dfs_corte_arestas(grafo *g, unsigned int u, dados_dfs_aresta *dados, aresta_corte *arestas, unsigned int *contador) {
	dados->tempo_descoberta[u] = dados->tempo_atual;
	dados->low[u] = dados->tempo_atual;
	dados->tempo_atual++;

	vertice *vert_u = &g->vertices[u];

	for (unsigned int i = 0; i < vert_u->grau; i++) {
		char *nome_vizinho = vert_u->arestas[i].nome_ponta;
		int indice_vizinho = (int) indice_do_vertice(g, nome_vizinho);

		if (indice_vizinho == -1) { 
			continue;
		}

		// Vizinho não visitado
		if (dados->tempo_descoberta[indice_vizinho] == UINT_MAX) {
			dados->pai[indice_vizinho] = (int) u;
			dfs_corte_arestas(g, (unsigned int)indice_vizinho, dados, arestas, contador);
			
			// Atualiza low de u
			if (dados->low[indice_vizinho] < dados->low[u]) {
				dados->low[u] = dados->low[indice_vizinho];
			}

			// Verifica se a aresta é ponte
			if (dados->low[indice_vizinho] > dados->tempo_descoberta[u]) {
				// Armazena os nomes ordenados alfabeticamente
				char *nome_u = g->vertices[u].nome;
				char *nome_v = g->vertices[indice_vizinho].nome;

				// Ordena alfabeticamente
				if (strcmp(nome_u, nome_v) < 0) {
					arestas[*contador].u = copia_str(nome_u);
					arestas[*contador].v = copia_str(nome_v);
				} else {
					arestas[*contador].u = copia_str(nome_v);
					arestas[*contador].v = copia_str(nome_u);
				}

				(*contador)++;
			}
		} else if (indice_vizinho != dados->pai[u]) {
			if (dados->tempo_descoberta[indice_vizinho] < dados->low[u]) {
		 	 dados->low[u] = dados->tempo_descoberta[indice_vizinho];
			}
		}
	}
}

// Função de comparação para ordenação de arestas
int compara_nome_arestas(const void *a, const void *b) {
	const aresta_corte *aa = (const aresta_corte *)a;
	const aresta_corte *ab = (const aresta_corte *)b;

	// Compara primeiro vértice
	int cmp_u = strcmp(aa->u, ab->u);
	if (cmp_u != 0) { 
		return cmp_u;
	} 

	// Se primeiro igual, compara segundo
	return strcmp(aa->v, ab->v);
}

// Função de comparação para ordenação não decrescente de diametros
int compara_uint(const void *a, const void *b) {
	unsigned int da = *(const unsigned int *)a;
	unsigned int db = *(const unsigned int *)b;

	return ((da > db) - (da < db));
}
/* -------------------------- FUNÇÕES DA BIBLIOTECA -------------------------- */
// lê um grafo de f e o devolve
grafo *le_grafo(FILE *f) {
	char linha[MAX_LINHA];
	
	grafo *grafo_lido = malloc(sizeof(grafo));
	if (!grafo_lido) {
		printf("[le_grafo] erro em malloc.\n");
		return NULL;
	}

	grafo_lido->nome = NULL;
	grafo_lido->num_vertices = 0;
	grafo_lido->num_arestas = 0;
	grafo_lido->vertices = NULL;

	while (fgets(linha, MAX_LINHA, f)) {
		remove_quebra_linha(linha);

		// Se é comentario, ignora
		if ((linha[0] == '/') && (linha[1] == '/'))
			continue;

		// Se começar com caractere nulo (era só \n antes da remocao), ignora
		if (linha[0] == '\0')
			continue;

		// Se a linha atual é a do nome, le e pula a iteracao
		if (!grafo_lido->nome) {
			grafo_lido->nome = copia_str(linha);
			continue;
		}

		char nome_vertice1[MAX_LINHA], nome_vertice2[MAX_LINHA];
		unsigned int peso = 1;
		// Se é linha de aresta
		if (sscanf(linha, "%s -- %s %u", nome_vertice1, nome_vertice2, &peso) >= 2) {
			// Cria os dois vertices antes da atribuicao para evitar problemas com ponteiros
			busca_ou_cria_vertice(grafo_lido, nome_vertice1);
			busca_ou_cria_vertice(grafo_lido, nome_vertice2);

			vertice *vertice1 = busca_ou_cria_vertice(grafo_lido, nome_vertice1);
			vertice *vertice2 = busca_ou_cria_vertice(grafo_lido, nome_vertice2);

			adiciona_aresta(vertice1, nome_vertice2, peso);
			adiciona_aresta(vertice2, nome_vertice1, peso);		
		} else { // Se é linha de definicao de vertice
			busca_ou_cria_vertice(grafo_lido, linha);
		}
	}

	for (unsigned int i = 0; i < grafo_lido->num_vertices; i++) {
		grafo_lido->num_arestas += grafo_lido->vertices[i].grau;
	}

	grafo_lido->num_arestas /= 2;

	return grafo_lido;
}

// desaloca toda a estrutura de dados alocada em g
unsigned int destroi_grafo(grafo *g) {
	free(g->nome);
	for (unsigned int i = 0; i < g->num_vertices; i++) {
		free(g->vertices[i].nome);
		for (unsigned int j = 0; j < g->vertices[i].grau; j++) {
			free(g->vertices[i].arestas[j].nome_ponta);
		}
		free(g->vertices[i].arestas);
	}
	free(g->vertices);
	free(g);
	return 1;
}

// devolve o nome de g
char *nome(grafo *g) {
	return g->nome;
}

// devolve 1 se g é bipartido e 0 caso contrário
unsigned int bipartido(grafo *g) {
	if (g->num_vertices == 0) {
		return 1;
	}

	// cores[i] = cor do vertice g->vertices[i]
	int *cores = (int*)malloc(sizeof(int) * g->num_vertices);
	if (!cores) {
		return 0;
	}

	// No inicio, nenhum vertice tem cor
	for (unsigned int i = 0; i < g->num_vertices; i++) {
		cores[i] = -1;
	}

	// Cria a fila de vertices usada para a busca
	unsigned int *fila = (unsigned int*)malloc(g->num_vertices * sizeof(unsigned int));
	if (!fila) {
		free(cores);
		return 0;
	}

	// Passa por todos os vertices, tentando pintar seus vizinhos com uma cor diferente da dele
	// Se um vizinho ja esta pintado com a mesma cor da dele, retorna que o grafo nao é bipartido
	for (unsigned int inicio = 0; inicio < g->num_vertices; inicio++) {
		if (cores[inicio] != -1) {
			continue;
		}

		unsigned int frente, tras;
		frente = 0;
		tras = 0;
		fila[tras++] = inicio;
		cores[inicio] = 0;

		while (frente < tras) {
			unsigned int u = fila[frente++];
			vertice *vert_u = &g->vertices[u];

			for (unsigned int j = 0; j < vert_u->grau; j++) {
				char *nome_vizinho = vert_u->arestas[j].nome_ponta;
				unsigned int indice_vizinho = indice_do_vertice(g, nome_vizinho);

				if (cores[indice_vizinho] == -1) {
					cores[indice_vizinho] = 1 - cores[u];
					fila[tras++] = indice_vizinho;
				} else if (cores[indice_vizinho] == cores[u]) {
					free(cores);
					free(fila);
					return 0;
				}
			}
		}
	}

	free(cores);
	free(fila);
	return 1;
}

// devolve o número de vértices em g
unsigned int n_vertices(grafo *g) {
	return g->num_vertices;
}

// devolve o número de arestas em g
unsigned int n_arestas(grafo *g) {
	return g->num_arestas;
}

// devolve o número de componentes em g
unsigned int n_componentes(grafo *g) {
	if (g->num_vertices == 0) {
		return 0;
	}

	unsigned int *visitados = (unsigned int*) malloc(sizeof(unsigned int) * g->num_vertices);
	if (!visitados)
		return 0;

	for (unsigned int i = 0; i < g->num_vertices; i++) {
		visitados[i] = 0;
	}

	unsigned int componentes = 0;

	for (unsigned int i = 0; i < g->num_vertices; i++) {
		if (!visitados[i]) {
			componentes++;

			unsigned int frente, tras;
			frente = 0;
			tras = 0;
			unsigned int *fila = (unsigned int*)malloc(g->num_vertices * sizeof(unsigned int));

			visitados[i] = 1;
			fila[tras++] = i;

			while (frente < tras) {
				unsigned int u = fila[frente++];
				vertice *vert_u = &g->vertices[u];

				for (unsigned int j = 0; j < vert_u->grau; j++) {
					char *nome_vizinho = vert_u->arestas[j].nome_ponta;
					unsigned int indice_vizinho = indice_do_vertice(g, nome_vizinho);
					
					if ((indice_vizinho != UINT_MAX) && (!visitados[indice_vizinho])) {
						visitados[indice_vizinho] = 1;
						fila[tras++] = indice_vizinho;
					}
				}
			}
			free(fila);
		}
	}

	free(visitados);
	return componentes;
}

// devolve uma "string" com os diâmetros dos componentes de g separados por brancos
// em ordem não decrescente
char *diametros(grafo *g) {
	if (g->num_vertices == 0) {
		char *resposta = malloc(1);
		if (resposta) {
			resposta[0] = '\0';
		}
		return resposta; 
	}

	unsigned int *visitados = (unsigned int*) malloc(sizeof(unsigned int) * g->num_vertices);
	unsigned int *diametros_componentes = (unsigned int*) malloc(sizeof(unsigned int) * g->num_vertices);
	unsigned int num_componente = 0;

	if ((!visitados) || (!diametros_componentes)) {
		free(visitados);
		free(diametros_componentes);
		return NULL;
	}

	for (unsigned int i = 0; i < g->num_vertices; i++) {
		visitados[i] = 0;
	}
	
	unsigned int *fila = (unsigned int*) malloc(sizeof(unsigned int) * g->num_vertices);
	if (!fila) {
		free(visitados);
		free(diametros_componentes);
		return NULL;
	}

	for (unsigned int i = 0; i < g->num_vertices; i++) {
		if (visitados[i]) {
			continue;
		}

		unsigned int frente, tras;
		frente = 0;
		tras = 0;
		unsigned int tamanho_componente = 0;
		unsigned int *vertices_componente = (unsigned int*) malloc(sizeof(unsigned int) * g->num_vertices);

		if (!vertices_componente) {
			continue;
		}

		fila[tras++] = i;
		visitados[i] = 1;
		vertices_componente[tamanho_componente++] = i;

		while (frente < tras) {
			unsigned int u = fila[frente++];
			vertice *vert_u = &g->vertices[u];

			for (unsigned int j = 0; j < vert_u->grau; j++) {
				char *nome_vizinho = vert_u->arestas[j].nome_ponta;
				unsigned int indice_vizinho = indice_do_vertice(g, nome_vizinho);

				if ((indice_vizinho != UINT_MAX) && (!visitados[indice_vizinho])) {
					visitados[indice_vizinho] = 1;
					fila[tras++] = indice_vizinho;
					vertices_componente[tamanho_componente++] = indice_vizinho;
				} 
			}
		}

		diametros_componentes[num_componente++] = diametro_componente(g, vertices_componente, tamanho_componente);
		free(vertices_componente);
	}

	free(visitados);
	free(fila);

	qsort(diametros_componentes, num_componente, sizeof(unsigned int), compara_uint);
	
	char *resultado = NULL;
	size_t tamanho_total = 0;

	for (unsigned int i = 0; i < num_componente; i++) {
		char buffer[20];
		int tamanho = snprintf(buffer, sizeof(buffer), "%u", diametros_componentes[i]);
	
		if (tamanho > 0) {
			char *novo_resultado = realloc(resultado, tamanho_total + (size_t)tamanho + 1 + (i > 0 ? 1 : 0));

			if (!novo_resultado) {
				free(resultado);
				free(diametros_componentes);
				return NULL;
			}

			resultado = novo_resultado;

			if (i > 0) {
				resultado[tamanho_total++] = ' ';
			}

			strcpy(resultado + tamanho_total, buffer);
			tamanho_total += (size_t)tamanho;
		}
	}

	free(diametros_componentes);
	return resultado ? resultado : copia_str("");
}

// devolve uma "string" com os nomes dos vértices de corte de g em
// ordem alfabética, separados por brancos
char *vertices_corte(grafo *g) {
	if (g->num_vertices == 0) {
		char *resposta = malloc(1);
		if (resposta) {
			resposta[0]= '\0';
		}
		return resposta;
	}

	unsigned int *tempo_descoberta = (unsigned int *) malloc(sizeof(unsigned int) * g->num_vertices);
	unsigned int *low = (unsigned int *) malloc(sizeof(unsigned int) * g->num_vertices);
	int *pai = (int*) malloc(sizeof(int) * g->num_vertices);
	unsigned int *eh_corte = (unsigned int*) malloc(sizeof(unsigned int) * g->num_vertices);
	
	if ((!tempo_descoberta) || (!low) || (!pai) || (!eh_corte)) {
		free(tempo_descoberta);
		free(low);
		free(pai);
		free(eh_corte);
		return NULL;
	}

	for (unsigned int i = 0; i < g->num_vertices; i++) {
		tempo_descoberta[i] = UINT_MAX;
		low[i] = UINT_MAX;
		pai[i] = -1;
		eh_corte[i] = 0;
	}

	dados_dfs_vertice dados = {
		.tempo_descoberta = tempo_descoberta,
		.low = low,
		.pai = pai,
		.eh_corte = eh_corte,
		.tempo_atual = 0
	};

	for (unsigned int i = 0; i < g->num_vertices; i++) {
		if (tempo_descoberta[i] == UINT_MAX) {
			dfs_corte_vertices(g, i, &dados);
		}
	}

	unsigned int contador = 0;
	char **nomes_corte = malloc(g->num_vertices * sizeof(char*));

	for (unsigned int i = 0; i < g->num_vertices; i++) {
		if (eh_corte[i]) {
			nomes_corte[contador++] = g->vertices[i].nome;
		}
 	}

	qsort(nomes_corte, contador, sizeof(char*), compara_nome_vertices);

	size_t tamanho_total = 0;
	for (unsigned int i = 0; i < contador; i++) {
		tamanho_total += strlen(nomes_corte[i]) + 1; 
	}

	// Construir string de resultado
	char *resultado = malloc(tamanho_total > 0 ? tamanho_total : 1);
	if (!resultado) {
		free(nomes_corte);
		free(tempo_descoberta);
		free(low);
		free(pai);
		free(eh_corte);
		return NULL;
	}

	char *ptr = resultado;
	for (unsigned int i = 0; i < contador; i++) {
		size_t len = strlen(nomes_corte[i]);
		if (i > 0) {
			*ptr++ = ' ';
		}
		memcpy(ptr, nomes_corte[i], len);
		ptr += len;
	}
	*ptr = '\0';

	// Liberar memória auxiliar
	free(nomes_corte);
	free(tempo_descoberta);
	free(low);
	free(pai);
	free(eh_corte);
 
	return resultado;
}

// devolve uma "string" com as arestas de corte de g em ordem alfabética, separadas por brancos
// cada aresta é o par de nomes de seus vértices em ordem alfabética, separadas por brancos
//
// por exemplo, se as arestas de corte são {z, a}, {x, b} e {y, c}, a resposta será a string
// "a z b x c y"
char *arestas_corte(grafo *g) {
	if (g->num_vertices == 0 || g->num_arestas == 0) {
		char *resposta = malloc(1);
		if (resposta) {
			resposta[0] = '\0';
		}

		return resposta;
	}

	// Aloca estruturas para DFS
	unsigned int *tempo_descoberta = malloc(g->num_vertices * sizeof(unsigned int));
	unsigned int *low = malloc(g->num_vertices * sizeof(unsigned int));
	int *pai = malloc(g->num_vertices * sizeof(int));

	if (!tempo_descoberta || !low || !pai) {
		free(tempo_descoberta);
		free(low);
		free(pai);
		return NULL;
	}

	// Inicializa estruturas DFS
	for (unsigned int i = 0; i < g->num_vertices; i++) {
		tempo_descoberta[i] = UINT_MAX;
		low[i] = UINT_MAX;
		pai[i] = -1;
	}

	// Aloca array para armazenar arestas de corte
	aresta_corte *arestas = malloc(g->num_arestas * sizeof(aresta_corte));
	unsigned int contador = 0;

	dados_dfs_aresta dados = {
		.tempo_descoberta = tempo_descoberta,
		.low = low,
		.pai = pai,
		.tempo_atual = 0
	};

	// Executa DFS para cada componente
	for (unsigned int i = 0; i < g->num_vertices; i++) {
		if (tempo_descoberta[i] == UINT_MAX) {
			dfs_corte_arestas(g, i, &dados, arestas, &contador);
		}
	}

	// Ordena arestas alfabeticamente
	qsort(arestas, contador, sizeof(aresta_corte), compara_nome_arestas);

	// Calcula tamanho total da string resultado
	size_t tamanho_total = 0;
	for (unsigned int i = 0; i < contador; i++) {
		tamanho_total += strlen(arestas[i].u) + 1;
		tamanho_total += strlen(arestas[i].v) + 1;
	}

	// Aloca e constrói string resultado
	char *resultado = malloc(tamanho_total > 0 ? tamanho_total : 1);
	if (!resultado) {
		// Libera memória alocada para nomes
		for (unsigned int i = 0; i < contador; i++) {
			free(arestas[i].u);
			free(arestas[i].v);
		}

		free(arestas);
		free(tempo_descoberta);
		free(low);
		free(pai);
		return NULL;
	}

	char *ptr = resultado;
	for (unsigned int i = 0; i < contador; i++) {
		// Copia primeiro nome
		size_t len_u = strlen(arestas[i].u);
		memcpy(ptr, arestas[i].u, len_u);
		ptr += len_u;
		*ptr++ = ' ';

		// Copia segundo nome
		size_t len_v = strlen(arestas[i].v);
		memcpy(ptr, arestas[i].v, len_v);
		ptr += len_v;

		// Adiciona espaço entre pares
		if (i < contador - 1) {
			*ptr++ = ' ';
		}

		// Libera nomes individuais
		free(arestas[i].u);
		free(arestas[i].v);
	}
	*ptr = '\0';

	// Libera memória auxiliar
	free(arestas);
	free(tempo_descoberta);
	free(low);
	free(pai);
	
	return resultado;
}