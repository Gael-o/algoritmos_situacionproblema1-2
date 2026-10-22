/*
 * Descripcion: flujo maximo entre un nodo fuente y un nodo sumidero con el algoritmo de Dinic
 * (busqueda por niveles + caminos bloqueantes), mas el adaptador JSON del modo de flujo maximo.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 21/10/2026
 */

#ifndef FLUJO_H
#define FLUJO_H

#include <algorithm>
#include <queue>
#include <string>
#include <vector>

#include "grafo.h"
#include "json.h"
#include "lectura.h"

const double CAPACIDAD_MINIMA = 1.0e-9;
const double CAPACIDAD_INFINITA = 1.0e18;
const std::vector<std::string> CLAVES_FUENTE = {"source", "s", "src", "from", "origin", "start", "fuente", "origen"};
const std::vector<std::string> CLAVES_SUMIDERO = {"sink", "t", "target", "dst", "to", "destination", "end", "sumidero", "destino"};
const std::vector<std::string> CLAVES_DIRIGIDO = {"directed", "dirigido"};

struct ArcoFlujo {
	int destino = 0;
	double capacidad = 0.0;
};

struct RedFlujo {
	std::vector<ArcoFlujo> arcos;
	std::vector<std::vector<int>> adyacencia;
};

/*
 * Agrega un arco con su arco inverso (de capacidad dada) a la red residual.
 * Parametro red: red que se modifica.
 * Parametro origen: nodo de salida.
 * Parametro destino: nodo de llegada.
 * Parametro capacidad: capacidad del arco.
 * Parametro capacidadInversa: capacidad del arco contrario (0 si el grafo es dirigido).
 */
void agregarArcoFlujo(RedFlujo& red, int origen, int destino, double capacidad, double capacidadInversa) {
	ArcoFlujo directo;
	directo.destino = destino;
	directo.capacidad = capacidad;
	ArcoFlujo inverso;
	inverso.destino = origen;
	inverso.capacidad = capacidadInversa;
	red.adyacencia[static_cast<size_t>(origen)].push_back(static_cast<int>(red.arcos.size()));
	red.arcos.push_back(directo);
	red.adyacencia[static_cast<size_t>(destino)].push_back(static_cast<int>(red.arcos.size()));
	red.arcos.push_back(inverso);
}

/*
 * Calcula el nivel (distancia en arcos con capacidad residual) de cada nodo desde la fuente.
 * Parametro red: red residual.
 * Parametro fuente: nodo de origen del flujo.
 * Parametro nivel: recibe el nivel de cada nodo (-1 si no es alcanzable).
 */
void calcularNiveles(const RedFlujo& red, int fuente, std::vector<int>& nivel) {
	nivel.assign(red.adyacencia.size(), -1);
	std::queue<int> pendientes;
	nivel[static_cast<size_t>(fuente)] = 0;
	pendientes.push(fuente);
	while (!pendientes.empty()) {
		int actual = pendientes.front();
		pendientes.pop();
		for (size_t i = 0; i < red.adyacencia[static_cast<size_t>(actual)].size(); i++) {
			size_t indiceArco = static_cast<size_t>(red.adyacencia[static_cast<size_t>(actual)][i]);
			int vecino = red.arcos[indiceArco].destino;
			if (red.arcos[indiceArco].capacidad > CAPACIDAD_MINIMA && nivel[static_cast<size_t>(vecino)] < 0) {
				nivel[static_cast<size_t>(vecino)] = nivel[static_cast<size_t>(actual)] + 1;
				pendientes.push(vecino);
			}
		}
	}
}

/*
 * Empuja flujo por todos los caminos bloqueantes del grafo de niveles (busqueda iterativa).
 * Parametro red: red residual, se modifica.
 * Parametro fuente: nodo de origen del flujo.
 * Parametro sumidero: nodo de destino del flujo.
 * Parametro nivel: nivel de cada nodo.
 * Retorno: flujo total enviado en esta fase.
 */
double enviarFlujoBloqueante(RedFlujo& red, int fuente, int sumidero, const std::vector<int>& nivel) {
	std::vector<size_t> puntero(red.adyacencia.size(), 0);
	std::vector<int> camino;
	int nodo = fuente;
	double enviado = 0.0;
	bool terminado = false;
	while (!terminado) {
		if (nodo == sumidero) {
			double cuello = CAPACIDAD_INFINITA;
			for (size_t i = 0; i < camino.size(); i++) {
				double residual = red.arcos[static_cast<size_t>(camino[i])].capacidad;
				if (residual < cuello) {
					cuello = residual;
				}
			}
			for (size_t i = 0; i < camino.size(); i++) {
				red.arcos[static_cast<size_t>(camino[i])].capacidad -= cuello;
				red.arcos[static_cast<size_t>(camino[i]) ^ 1U].capacidad += cuello;
			}
			enviado += cuello;
			camino.clear();
			nodo = fuente;
		} else {
			bool avanzo = false;
			size_t posicion = static_cast<size_t>(nodo);
			while (!avanzo && puntero[posicion] < red.adyacencia[posicion].size()) {
				int indiceArco = red.adyacencia[posicion][puntero[posicion]];
				int vecino = red.arcos[static_cast<size_t>(indiceArco)].destino;
				bool sirve = red.arcos[static_cast<size_t>(indiceArco)].capacidad > CAPACIDAD_MINIMA && nivel[static_cast<size_t>(vecino)] == nivel[posicion] + 1;
				if (sirve) {
					camino.push_back(indiceArco);
					nodo = vecino;
					avanzo = true;
				} else {
					puntero[posicion]++;
				}
			}
			if (!avanzo && nodo == fuente) {
				terminado = true;
			} else if (!avanzo) {
				int ultimoArco = camino.back();
				camino.pop_back();
				nodo = red.arcos[static_cast<size_t>(ultimoArco) ^ 1U].destino;
				puntero[static_cast<size_t>(nodo)]++;
			}
		}
	}
	return enviado;
}

/*
 * Calcula el flujo maximo entre dos nodos con Dinic (niveles + caminos bloqueantes).
 * Parametro red: red residual, se modifica.
 * Parametro fuente: nodo de origen del flujo.
 * Parametro sumidero: nodo de destino del flujo.
 * Retorno: valor del flujo maximo.
 */
double calcularFlujoMaximo(RedFlujo& red, int fuente, int sumidero) {
	double total = 0.0;
	std::vector<int> nivel;
	if (fuente != sumidero) {
		calcularNiveles(red, fuente, nivel);
		while (nivel[static_cast<size_t>(sumidero)] >= 0) {
			total += enviarFlujoBloqueante(red, fuente, sumidero, nivel);
			calcularNiveles(red, fuente, nivel);
		}
	}
	return total;
}

/*
 * Calcula el flujo maximo entre dos nodos con Edmonds-Karp (camino aumentante mas corto por BFS).
 * Parametro red: red residual, se modifica.
 * Parametro fuente: nodo de origen del flujo.
 * Parametro sumidero: nodo de destino del flujo.
 * Retorno: valor del flujo maximo.
 */
double calcularFlujoEdmondsKarp(RedFlujo& red, int fuente, int sumidero) {
	double total = 0.0;
	size_t cantidad = red.adyacencia.size();
	bool hayCamino = fuente != sumidero;
	while (hayCamino) {
		std::vector<int> arcoPrevio(cantidad, -1);
		std::vector<bool> visitado(cantidad, false);
		std::queue<int> pendientes;
		visitado[static_cast<size_t>(fuente)] = true;
		pendientes.push(fuente);
		while (!pendientes.empty() && !visitado[static_cast<size_t>(sumidero)]) {
			int actual = pendientes.front();
			pendientes.pop();
			for (size_t i = 0; i < red.adyacencia[static_cast<size_t>(actual)].size(); i++) {
				int indiceArco = red.adyacencia[static_cast<size_t>(actual)][i];
				int vecino = red.arcos[static_cast<size_t>(indiceArco)].destino;
				if (red.arcos[static_cast<size_t>(indiceArco)].capacidad > CAPACIDAD_MINIMA && !visitado[static_cast<size_t>(vecino)]) {
					visitado[static_cast<size_t>(vecino)] = true;
					arcoPrevio[static_cast<size_t>(vecino)] = indiceArco;
					pendientes.push(vecino);
				}
			}
		}
		hayCamino = visitado[static_cast<size_t>(sumidero)];
		if (hayCamino) {
			double cuello = CAPACIDAD_INFINITA;
			int nodo = sumidero;
			while (nodo != fuente) {
				size_t indiceArco = static_cast<size_t>(arcoPrevio[static_cast<size_t>(nodo)]);
				cuello = std::min(cuello, red.arcos[indiceArco].capacidad);
				nodo = red.arcos[indiceArco ^ 1U].destino;
			}
			nodo = sumidero;
			while (nodo != fuente) {
				size_t indiceArco = static_cast<size_t>(arcoPrevio[static_cast<size_t>(nodo)]);
				red.arcos[indiceArco].capacidad -= cuello;
				red.arcos[indiceArco ^ 1U].capacidad += cuello;
				nodo = red.arcos[indiceArco ^ 1U].destino;
			}
			total += cuello;
		}
	}
	return total;
}

/*
 * Busca el indice interno de un nodo a partir de su identificador original.
 * Parametro grafo: grafo con los identificadores originales.
 * Parametro valor: identificador buscado (numero o texto).
 * Parametro indice: recibe el indice interno del nodo.
 * Retorno: true si el nodo existe.
 */
bool buscarIndiceNodo(const Grafo& grafo, const JsonValue& valor, int& indice) {
	bool encontrado = false;
	std::string clave = "";
	if (extraerTexto(valor, clave)) {
		for (size_t i = 0; i < grafo.identificadores.size(); i++) {
			std::string actual = "";
			extraerTexto(grafo.identificadores[i], actual);
			if (!encontrado && actual == clave) {
				indice = static_cast<int>(i);
				encontrado = true;
			}
		}
	}
	return encontrado;
}

/*
 * Determina la fuente y el sumidero: los campos de la entrada o, si faltan, el primer y ultimo nodo.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro grafo: grafo ya leido.
 * Parametro fuente: recibe el indice de la fuente.
 * Parametro sumidero: recibe el indice del sumidero.
 */
void determinarExtremos(const JsonValue& entrada, const Grafo& grafo, int& fuente, int& sumidero) {
	JsonValue valor;
	fuente = 0;
	sumidero = static_cast<int>(grafo.identificadores.size()) - 1;
	if (buscarAlguno(entrada, CLAVES_FUENTE, valor)) {
		buscarIndiceNodo(grafo, valor, fuente);
	}
	if (buscarAlguno(entrada, CLAVES_SUMIDERO, valor)) {
		buscarIndiceNodo(grafo, valor, sumidero);
	}
}

/*
 * Resuelve el modo de flujo maximo: lee la red con capacidades y calcula el flujo de fuente a sumidero.
 * Parametro entrada: objeto JSON del campo input (aristas con capacidad, fuente y sumidero).
 * Retorno: respuesta JSON con el valor del flujo maximo, o un error si la entrada no se reconoce.
 */
JsonValue resolverFlujoMaximo(const JsonValue& entrada) {
	Grafo grafo;
	std::string mensaje = "";
	JsonValue respuesta;
	if (!leerGrafo(entrada, grafo, mensaje)) {
		respuesta = construirError(mensaje);
	} else if (grafo.identificadores.size() < 2) {
		respuesta = construirError("flujo: la red necesita al menos dos nodos");
	} else {
		bool dirigido = true;
		JsonValue campo;
		if (buscarAlguno(entrada, CLAVES_DIRIGIDO, campo) && campo.tipo == JsonTipo::BOOLEANO) {
			dirigido = campo.booleano;
		}
		RedFlujo red;
		red.adyacencia.assign(grafo.identificadores.size(), std::vector<int>());
		for (size_t i = 0; i < grafo.aristas.size(); i++) {
			double inversa = 0.0;
			if (!dirigido) {
				inversa = grafo.aristas[i].costo;
			}
			agregarArcoFlujo(red, grafo.aristas[i].origen, grafo.aristas[i].destino, grafo.aristas[i].costo, inversa);
		}
		int fuente = 0;
		int sumidero = 0;
		determinarExtremos(entrada, grafo, fuente, sumidero);
		std::string tecnica = leerAlgoritmo(entrada);
		std::string nombre = "dinic";
		double valor = 0.0;
		if (tecnica == "edmondskarp" || tecnica == "ek") {
			nombre = "edmonds-karp";
			valor = calcularFlujoEdmondsKarp(red, fuente, sumidero);
		} else {
			valor = calcularFlujoMaximo(red, fuente, sumidero);
		}
		respuesta = crearObjeto();
		agregarCampo(respuesta, "algorithm", crearTexto(nombre));
		agregarCampo(respuesta, "maxFlow", crearNumero(valor));
		agregarCampo(respuesta, "max_flow", crearNumero(valor));
		agregarCampo(respuesta, "flow", crearNumero(valor));
		agregarCampo(respuesta, "value", crearNumero(valor));
	}
	return respuesta;
}

#endif
