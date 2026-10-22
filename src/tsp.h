/*
 * Descripcion: problema del viajante (TSP): distancias minimas entre todos los pares
 * (Floyd-Warshall), solucion exacta (Held-Karp) para pocos nodos y aproximada
 * (vecino mas cercano + 2-opt) para instancias grandes, mas el adaptador JSON del modo "tsp".
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 21/10/2026
 */

#ifndef TSP_H
#define TSP_H

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "grafo.h"
#include "json.h"
#include "lectura.h"

typedef std::vector<std::vector<double>> Matriz;

const double INFINITO_TSP = 1.0e18;
const int LIMITE_EXACTO = 16;
const int LIMITE_FLOYD = 600;
const long long LIMITE_EVALUACIONES = 150000000LL;
const std::vector<std::string> CLAVES_MATRIZ = {"matrix", "distances", "distanceMatrix", "distance_matrix", "dist", "costs", "costMatrix", "cost_matrix", "adjacency"};
const std::vector<std::string> CLAVES_PUNTOS = {"points", "coords", "coordinates", "cities", "locations", "sites", "nodes"};
const std::vector<std::string> CLAVES_INICIO = {"start", "origin", "depot", "source", "inicio"};

struct ResultadoTsp {
	std::vector<int> ruta;
	double costo = 0.0;
	bool exacto = false;
	std::string nombre = "";
};

/*
 * Lee una matriz de distancias desde el primer campo que sea un arreglo de arreglos de numeros.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro matriz: recibe la matriz cuadrada leida.
 * Retorno: true si se encontro una matriz cuadrada valida.
 */
bool leerMatrizDistancias(const JsonValue& entrada, Matriz& matriz) {
	bool correcto = false;
	JsonValue lista;
	if (buscarAlguno(entrada, CLAVES_MATRIZ, lista) && lista.tipo == JsonTipo::ARREGLO && !lista.arreglo.empty()) {
		correcto = true;
		size_t n = lista.arreglo.size();
		for (size_t i = 0; i < n && correcto; i++) {
			correcto = lista.arreglo[i].tipo == JsonTipo::ARREGLO && lista.arreglo[i].arreglo.size() == n;
			std::vector<double> fila;
			for (size_t j = 0; j < n && correcto; j++) {
				double valor = INFINITO_TSP;
				if (!extraerNumero(lista.arreglo[i].arreglo[j], valor)) {
					valor = INFINITO_TSP;
				}
				fila.push_back(valor);
			}
			matriz.push_back(fila);
		}
	}
	return correcto;
}

/*
 * Reemplaza cada distancia por la del camino minimo entre el par (Floyd-Warshall).
 * Parametro matriz: matriz de distancias directas, se modifica.
 */
void completarCaminosMinimos(Matriz& matriz) {
	size_t n = matriz.size();
	for (size_t k = 0; k < n; k++) {
		for (size_t i = 0; i < n; i++) {
			for (size_t j = 0; j < n; j++) {
				double alterno = matriz[i][k] + matriz[k][j];
				if (alterno < matriz[i][j]) {
					matriz[i][j] = alterno;
				}
			}
		}
	}
}

/*
 * Construye la matriz de distancias directas de un grafo no dirigido.
 * Parametro grafo: grafo con sus aristas.
 * Retorno: matriz con la menor arista directa entre cada par (infinito si no hay).
 */
Matriz construirMatrizDeGrafo(const Grafo& grafo) {
	size_t n = grafo.identificadores.size();
	Matriz matriz(n, std::vector<double>(n, INFINITO_TSP));
	for (size_t i = 0; i < n; i++) {
		matriz[i][i] = 0.0;
	}
	for (size_t i = 0; i < grafo.aristas.size(); i++) {
		size_t u = static_cast<size_t>(grafo.aristas[i].origen);
		size_t v = static_cast<size_t>(grafo.aristas[i].destino);
		double costo = grafo.aristas[i].costo;
		if (costo < matriz[u][v]) {
			matriz[u][v] = costo;
			matriz[v][u] = costo;
		}
	}
	return matriz;
}

/*
 * Construye la matriz de distancias euclidianas entre puntos.
 * Parametro puntos: coordenadas de los nodos.
 * Retorno: matriz simetrica de distancias.
 */
Matriz construirMatrizDePuntos(const std::vector<Punto>& puntos) {
	size_t n = puntos.size();
	Matriz matriz(n, std::vector<double>(n, 0.0));
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			double dx = puntos[i].x - puntos[j].x;
			double dy = puntos[i].y - puntos[j].y;
			matriz[i][j] = std::sqrt(dx * dx + dy * dy);
		}
	}
	return matriz;
}

/*
 * Indica si todas las distancias son finitas.
 * Parametro matriz: matriz a revisar.
 * Retorno: true si no hay pares sin conexion.
 */
bool esCompleta(const Matriz& matriz) {
	bool completa = true;
	for (size_t i = 0; i < matriz.size(); i++) {
		for (size_t j = 0; j < matriz.size(); j++) {
			if (matriz[i][j] >= INFINITO_TSP) {
				completa = false;
			}
		}
	}
	return completa;
}

/*
 * Indica si la matriz es simetrica (condicion para usar 2-opt).
 * Parametro matriz: matriz a revisar.
 * Retorno: true si d(i,j) = d(j,i) para todo par.
 */
bool esSimetrica(const Matriz& matriz) {
	bool simetrica = true;
	for (size_t i = 0; i < matriz.size(); i++) {
		for (size_t j = 0; j < matriz.size(); j++) {
			if (std::fabs(matriz[i][j] - matriz[j][i]) > 1.0e-9) {
				simetrica = false;
			}
		}
	}
	return simetrica;
}

/*
 * Suma la distancia de una ruta cerrada.
 * Parametro matriz: distancias entre nodos.
 * Parametro ruta: nodos visitados; el ultimo ya es el de inicio.
 * Retorno: costo total.
 */
double calcularCostoRuta(const Matriz& matriz, const std::vector<int>& ruta) {
	double total = 0.0;
	for (size_t i = 0; i + 1 < ruta.size(); i++) {
		total += matriz[static_cast<size_t>(ruta[i])][static_cast<size_t>(ruta[i + 1])];
	}
	return total;
}

/*
 * Resuelve el TSP de forma exacta con programacion dinamica sobre subconjuntos (Held-Karp).
 * Parametro matriz: distancias entre todos los pares.
 * Parametro inicio: nodo donde empieza y termina la ruta.
 * Retorno: ruta cerrada (n + 1 nodos).
 */
std::vector<int> resolverExacto(const Matriz& matriz, int inicio) {
	int n = static_cast<int>(matriz.size());
	int m = n - 1;
	std::vector<int> otros;
	for (int i = 0; i < n; i++) {
		if (i != inicio) {
			otros.push_back(i);
		}
	}
	size_t estados = static_cast<size_t>(1) << m;
	size_t columnas = static_cast<size_t>(m);
	std::vector<double> costo(estados * columnas, INFINITO_TSP);
	std::vector<int> previo(estados * columnas, -1);
	for (int j = 0; j < m; j++) {
		size_t unico = static_cast<size_t>(1) << j;
		costo[unico * columnas + static_cast<size_t>(j)] = matriz[static_cast<size_t>(inicio)][static_cast<size_t>(otros[static_cast<size_t>(j)])];
	}
	for (size_t mascara = 1; mascara < estados; mascara++) {
		for (int j = 0; j < m; j++) {
			bool dentro = ((mascara >> j) & 1U) != 0;
			double actual = costo[mascara * columnas + static_cast<size_t>(j)];
			if (dentro && actual < INFINITO_TSP) {
				for (int k = 0; k < m; k++) {
					bool libre = ((mascara >> k) & 1U) == 0;
					if (libre) {
						size_t siguiente = mascara | (static_cast<size_t>(1) << k);
						double candidato = actual + matriz[static_cast<size_t>(otros[static_cast<size_t>(j)])][static_cast<size_t>(otros[static_cast<size_t>(k)])];
						size_t posicion = siguiente * columnas + static_cast<size_t>(k);
						if (candidato < costo[posicion]) {
							costo[posicion] = candidato;
							previo[posicion] = j;
						}
					}
				}
			}
		}
	}
	size_t completa = estados - 1;
	double mejor = INFINITO_TSP * 4.0;
	int ultimo = 0;
	for (int j = 0; j < m; j++) {
		double total = costo[completa * columnas + static_cast<size_t>(j)] + matriz[static_cast<size_t>(otros[static_cast<size_t>(j)])][static_cast<size_t>(inicio)];
		if (total < mejor) {
			mejor = total;
			ultimo = j;
		}
	}
	std::vector<int> inversa;
	size_t mascara = completa;
	int actualJ = ultimo;
	while (actualJ >= 0) {
		inversa.push_back(otros[static_cast<size_t>(actualJ)]);
		int anterior = previo[mascara * columnas + static_cast<size_t>(actualJ)];
		mascara = mascara & ~(static_cast<size_t>(1) << actualJ);
		actualJ = anterior;
	}
	std::vector<int> ruta;
	ruta.push_back(inicio);
	for (size_t i = inversa.size(); i > 0; i--) {
		ruta.push_back(inversa[i - 1]);
	}
	ruta.push_back(inicio);
	return ruta;
}

/*
 * Construye una ruta cerrada con la heuristica del vecino mas cercano.
 * Parametro matriz: distancias entre todos los pares.
 * Parametro inicio: nodo donde empieza y termina la ruta.
 * Retorno: ruta cerrada (n + 1 nodos).
 */
std::vector<int> construirVecinoCercano(const Matriz& matriz, int inicio) {
	size_t n = matriz.size();
	std::vector<bool> visitado(n, false);
	std::vector<int> ruta;
	int actual = inicio;
	visitado[static_cast<size_t>(inicio)] = true;
	ruta.push_back(inicio);
	for (size_t paso = 1; paso < n; paso++) {
		int mejor = -1;
		double mejorDistancia = INFINITO_TSP * 4.0;
		for (size_t j = 0; j < n; j++) {
			double distancia = matriz[static_cast<size_t>(actual)][j];
			if (!visitado[j] && distancia < mejorDistancia) {
				mejorDistancia = distancia;
				mejor = static_cast<int>(j);
			}
		}
		visitado[static_cast<size_t>(mejor)] = true;
		ruta.push_back(mejor);
		actual = mejor;
	}
	ruta.push_back(inicio);
	return ruta;
}

/*
 * Mejora una ruta cerrada con 2-opt: invierte tramos mientras reduzcan la distancia.
 * Parametro matriz: distancias simetricas entre todos los pares.
 * Parametro ruta: ruta cerrada (n + 1 nodos), el primer nodo queda fijo; se modifica.
 */
void mejorarConDosOpt(const Matriz& matriz, std::vector<int>& ruta) {
	size_t n = ruta.size() - 1;
	long long evaluaciones = 0;
	bool mejoro = true;
	while (mejoro && evaluaciones < LIMITE_EVALUACIONES) {
		mejoro = false;
		for (size_t i = 1; i + 1 < n && evaluaciones < LIMITE_EVALUACIONES; i++) {
			for (size_t j = i + 1; j < n; j++) {
				size_t a = static_cast<size_t>(ruta[i - 1]);
				size_t b = static_cast<size_t>(ruta[i]);
				size_t c = static_cast<size_t>(ruta[j]);
				size_t d = static_cast<size_t>(ruta[j + 1]);
				double cambio = matriz[a][c] + matriz[b][d] - matriz[a][b] - matriz[c][d];
				evaluaciones++;
				if (cambio < -1.0e-9) {
					std::reverse(ruta.begin() + static_cast<long>(i), ruta.begin() + static_cast<long>(j) + 1);
					mejoro = true;
				}
			}
		}
	}
}

/*
 * Resuelve el TSP eligiendo el metodo: por defecto exacto si es pequeno y aproximado si no;
 * con la tecnica pedida se fuerza Held-Karp (solo hasta el limite exacto), vecino mas cercano + 2-opt o solo vecino mas cercano.
 * Parametro matriz: distancias entre todos los pares (finitas).
 * Parametro inicio: nodo donde empieza y termina la ruta.
 * Parametro tecnica: tecnica pedida, normalizada (puede estar vacia).
 * Retorno: ruta, costo y si la solucion es exacta.
 */
ResultadoTsp calcularTsp(const Matriz& matriz, int inicio, const std::string& tecnica) {
	ResultadoTsp resultado;
	int n = static_cast<int>(matriz.size());
	bool soloVecino = tecnica == "nn" || tecnica == "nearestneighbor";
	bool pideAproximado = tecnica == "2opt" || tecnica == "heuristic" || soloVecino;
	if (n == 1) {
		resultado.ruta.push_back(inicio);
		resultado.ruta.push_back(inicio);
		resultado.exacto = true;
		resultado.nombre = "trivial";
	} else if (n <= LIMITE_EXACTO && !pideAproximado) {
		resultado.ruta = resolverExacto(matriz, inicio);
		resultado.exacto = true;
		resultado.nombre = "held-karp";
	} else {
		resultado.ruta = construirVecinoCercano(matriz, inicio);
		resultado.nombre = "nearest-neighbor";
		if (!soloVecino && esSimetrica(matriz)) {
			mejorarConDosOpt(matriz, resultado.ruta);
			resultado.nombre = "nearest-neighbor+2-opt";
		}
	}
	resultado.costo = calcularCostoRuta(matriz, resultado.ruta);
	return resultado;
}

/*
 * Convierte el resultado del TSP a la respuesta JSON del modo "tsp".
 * Parametro identificadores: identificadores originales de los nodos.
 * Parametro resultado: ruta y costo calculados.
 * Retorno: objeto JSON con route (y alias), cost (y alias) y optimal.
 */
JsonValue construirRespuestaTsp(const std::vector<JsonValue>& identificadores, const ResultadoTsp& resultado) {
	JsonValue ruta = crearArreglo();
	for (size_t i = 0; i < resultado.ruta.size(); i++) {
		agregarElemento(ruta, identificadores[static_cast<size_t>(resultado.ruta[i])]);
	}
	JsonValue respuesta = crearObjeto();
	agregarCampo(respuesta, "algorithm", crearTexto(resultado.nombre));
	agregarCampo(respuesta, "route", ruta);
	agregarCampo(respuesta, "tour", ruta);
	agregarCampo(respuesta, "cost", crearNumero(resultado.costo));
	agregarCampo(respuesta, "totalCost", crearNumero(resultado.costo));
	agregarCampo(respuesta, "distance", crearNumero(resultado.costo));
	agregarCampo(respuesta, "optimal", crearBooleano(resultado.exacto));
	return respuesta;
}

/*
 * Busca el indice del nodo de inicio indicado en la entrada (por defecto, el primero).
 * Parametro entrada: objeto JSON del campo input.
 * Parametro identificadores: identificadores originales de los nodos.
 * Retorno: indice interno del nodo de inicio.
 */
int buscarInicio(const JsonValue& entrada, const std::vector<JsonValue>& identificadores) {
	int inicio = 0;
	JsonValue pedido;
	if (buscarAlguno(entrada, CLAVES_INICIO, pedido)) {
		std::string clave = "";
		extraerTexto(pedido, clave);
		for (size_t i = 0; i < identificadores.size(); i++) {
			std::string actual = "";
			extraerTexto(identificadores[i], actual);
			if (actual == clave) {
				inicio = static_cast<int>(i);
			}
		}
	}
	return inicio;
}

/*
 * Lee los nodos de la entrada como matriz, grafo o puntos y arma la matriz de distancias.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro matriz: recibe la matriz de distancias entre todos los pares.
 * Parametro identificadores: recibe los identificadores originales de los nodos.
 * Parametro mensaje: recibe el motivo si la entrada no se reconoce.
 * Retorno: true si la entrada se reconocio.
 */
bool leerInstanciaTsp(const JsonValue& entrada, Matriz& matriz, std::vector<JsonValue>& identificadores, std::string& mensaje) {
	bool correcto = false;
	JsonValue aristas;
	std::vector<Punto> puntos;
	if (leerMatrizDistancias(entrada, matriz)) {
		for (size_t i = 0; i < matriz.size(); i++) {
			identificadores.push_back(crearNumero(static_cast<double>(i)));
		}
		correcto = true;
	} else if (buscarPrimerCampo(entrada, CLAVES_ARISTAS, aristas)) {
		Grafo grafo;
		correcto = leerGrafo(entrada, grafo, mensaje);
		if (correcto) {
			matriz = construirMatrizDeGrafo(grafo);
			identificadores = grafo.identificadores;
		}
	} else if (leerListaPuntos(entrada, CLAVES_PUNTOS, puntos)) {
		matriz = construirMatrizDePuntos(puntos);
		for (size_t i = 0; i < puntos.size(); i++) {
			identificadores.push_back(crearNumero(static_cast<double>(i)));
		}
		correcto = true;
	} else {
		mensaje = "tsp: no se reconoce la entrada, campos recibidos: " + listarClaves(entrada);
	}
	return correcto;
}

/*
 * Resuelve el modo "tsp": ruta cerrada de costo minimo que visita cada nodo una vez.
 * Parametro entrada: objeto JSON del campo input (matriz de distancias, grafo o puntos).
 * Retorno: respuesta JSON con la ruta (n + 1 nodos) y su costo, o un error si la entrada no se reconoce.
 */
JsonValue resolverTsp(const JsonValue& entrada) {
	Matriz matriz;
	std::vector<JsonValue> identificadores;
	std::string mensaje = "";
	JsonValue respuesta;
	if (!leerInstanciaTsp(entrada, matriz, identificadores, mensaje)) {
		respuesta = construirError(mensaje);
	} else if (matriz.empty()) {
		respuesta = construirError("tsp: la entrada no tiene nodos");
	} else {
		int tamano = static_cast<int>(matriz.size());
		if (!esCompleta(matriz) && tamano <= LIMITE_FLOYD) {
			completarCaminosMinimos(matriz);
		}
		if (!esCompleta(matriz)) {
			respuesta = construirError("tsp: el grafo no es conexo o es demasiado grande");
		} else {
			int inicio = buscarInicio(entrada, identificadores);
			ResultadoTsp resultado = calcularTsp(matriz, inicio, leerAlgoritmo(entrada));
			respuesta = construirRespuestaTsp(identificadores, resultado);
		}
	}
	return respuesta;
}

#endif
