/*
 * Descripcion: arbol de expansion minima (MST) con Kruskal (Union-Find) y Prim
 * (cola de prioridad), mas el adaptador JSON del modo "mst".
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 20/10/2026
 */

#ifndef MST_H
#define MST_H

#include <algorithm>
#include <queue>
#include <string>
#include <vector>

#include "grafo.h"
#include "json.h"

struct ResultadoMst {
	std::vector<Arista> enlaces;
	double costoTotal = 0.0;
	bool conectado = false;
};

class ConjuntoDisjunto {
private:
	std::vector<int> padre;
	std::vector<int> rango;

public:
	/*
	 * Crea n conjuntos de un solo elemento.
	 * Parametro cantidad: numero de elementos.
	 */
	explicit ConjuntoDisjunto(int cantidad) : padre(cantidad, 0), rango(cantidad, 0) {
		for (int i = 0; i < cantidad; i++) {
			padre[i] = i;
		}
	}

	/*
	 * Encuentra el representante de un elemento y comprime el camino recorrido.
	 * Parametro elemento: elemento a buscar.
	 * Retorno: representante del conjunto.
	 */
	int encontrar(int elemento) {
		int raiz = elemento;
		while (padre[raiz] != raiz) {
			raiz = padre[raiz];
		}
		int actual = elemento;
		while (padre[actual] != raiz) {
			int siguiente = padre[actual];
			padre[actual] = raiz;
			actual = siguiente;
		}
		return raiz;
	}

	/*
	 * Une los conjuntos de dos elementos usando el rango para mantener el arbol bajo.
	 * Parametro primero: un elemento.
	 * Parametro segundo: otro elemento.
	 * Retorno: true si estaban en conjuntos distintos y se unieron.
	 */
	bool unir(int primero, int segundo) {
		int raizPrimero = encontrar(primero);
		int raizSegundo = encontrar(segundo);
		bool distintos = raizPrimero != raizSegundo;
		if (distintos) {
			if (rango[raizPrimero] < rango[raizSegundo]) {
				padre[raizPrimero] = raizSegundo;
			} else if (rango[raizPrimero] > rango[raizSegundo]) {
				padre[raizSegundo] = raizPrimero;
			} else {
				padre[raizSegundo] = raizPrimero;
				rango[raizPrimero]++;
			}
		}
		return distintos;
	}
};

/*
 * Compara dos aristas por costo; ante empate usa origen y destino para dar un orden estable.
 * Parametro primera: arista izquierda.
 * Parametro segunda: arista derecha.
 * Retorno: true si la primera debe ir antes que la segunda.
 */
bool compararPorCosto(const Arista& primera, const Arista& segunda) {
	bool menor = false;
	if (primera.costo != segunda.costo) {
		menor = primera.costo < segunda.costo;
	} else if (primera.origen != segunda.origen) {
		menor = primera.origen < segunda.origen;
	} else {
		menor = primera.destino < segunda.destino;
	}
	return menor;
}

struct MayorCosto {
	/*
	 * Comparador para que la cola de prioridad entregue primero la arista mas barata.
	 * Parametro primera: arista izquierda.
	 * Parametro segunda: arista derecha.
	 * Retorno: true si la primera tiene menor prioridad que la segunda.
	 */
	bool operator()(const Arista& primera, const Arista& segunda) const {
		return compararPorCosto(segunda, primera);
	}
};

/*
 * Calcula el MST (o bosque de expansion minima) con Kruskal.
 * Parametro numeroNodos: cantidad de nodos del grafo.
 * Parametro aristas: aristas no dirigidas con su costo.
 * Retorno: enlaces elegidos, costo total y si el grafo es conexo.
 */
ResultadoMst calcularMstKruskal(int numeroNodos, const std::vector<Arista>& aristas) {
	ResultadoMst resultado;
	std::vector<Arista> ordenadas = aristas;
	std::sort(ordenadas.begin(), ordenadas.end(), compararPorCosto);
	ConjuntoDisjunto conjuntos(numeroNodos);
	size_t objetivo = 0;
	if (numeroNodos > 1) {
		objetivo = static_cast<size_t>(numeroNodos - 1);
	}
	size_t indice = 0;
	while (indice < ordenadas.size() && resultado.enlaces.size() < objetivo) {
		const Arista& actual = ordenadas[indice];
		if (conjuntos.unir(actual.origen, actual.destino)) {
			resultado.enlaces.push_back(actual);
			resultado.costoTotal += actual.costo;
		}
		indice++;
	}
	resultado.conectado = resultado.enlaces.size() == objetivo;
	return resultado;
}

/*
 * Expande el arbol de Prim desde un nodo inicial dentro de su componente conexa.
 * Parametro inicio: nodo desde el que se empieza.
 * Parametro adyacencia: lista de aristas salientes por nodo.
 * Parametro visitado: marcas de nodos ya incluidos (se modifica).
 * Parametro resultado: acumula enlaces y costo (se modifica).
 */
void expandirPrim(int inicio, const std::vector<std::vector<Arista>>& adyacencia, std::vector<bool>& visitado, ResultadoMst& resultado) {
	std::priority_queue<Arista, std::vector<Arista>, MayorCosto> frontera;
	visitado[inicio] = true;
	for (size_t i = 0; i < adyacencia[inicio].size(); i++) {
		frontera.push(adyacencia[inicio][i]);
	}
	while (!frontera.empty()) {
		Arista actual = frontera.top();
		frontera.pop();
		if (!visitado[actual.destino]) {
			visitado[actual.destino] = true;
			resultado.enlaces.push_back(actual);
			resultado.costoTotal += actual.costo;
			for (size_t i = 0; i < adyacencia[actual.destino].size(); i++) {
				frontera.push(adyacencia[actual.destino][i]);
			}
		}
	}
}

/*
 * Calcula el MST (o bosque de expansion minima) con Prim.
 * Parametro numeroNodos: cantidad de nodos del grafo.
 * Parametro aristas: aristas no dirigidas con su costo.
 * Retorno: enlaces elegidos, costo total y si el grafo es conexo.
 */
ResultadoMst calcularMstPrim(int numeroNodos, const std::vector<Arista>& aristas) {
	ResultadoMst resultado;
	std::vector<std::vector<Arista>> adyacencia(numeroNodos);
	for (size_t i = 0; i < aristas.size(); i++) {
		Arista inversa;
		inversa.origen = aristas[i].destino;
		inversa.destino = aristas[i].origen;
		inversa.costo = aristas[i].costo;
		adyacencia[aristas[i].origen].push_back(aristas[i]);
		adyacencia[aristas[i].destino].push_back(inversa);
	}
	std::vector<bool> visitado(numeroNodos, false);
	int componentes = 0;
	for (int inicio = 0; inicio < numeroNodos; inicio++) {
		if (!visitado[inicio]) {
			componentes++;
			expandirPrim(inicio, adyacencia, visitado, resultado);
		}
	}
	resultado.conectado = componentes <= 1;
	return resultado;
}

/*
 * Convierte el resultado del MST a la respuesta JSON del modo "mst".
 * Parametro grafo: grafo con los identificadores originales de los nodos.
 * Parametro resultado: resultado calculado.
 * Parametro algoritmo: nombre del algoritmo usado.
 * Retorno: objeto JSON con algorithm, edges, totalCost y connected.
 */
JsonValue construirRespuestaMst(const Grafo& grafo, const ResultadoMst& resultado, const std::string& algoritmo) {
	JsonValue enlaces = crearArreglo();
	for (size_t i = 0; i < resultado.enlaces.size(); i++) {
		JsonValue enlace = crearObjeto();
		agregarCampo(enlace, "u", grafo.identificadores[resultado.enlaces[i].origen]);
		agregarCampo(enlace, "v", grafo.identificadores[resultado.enlaces[i].destino]);
		agregarCampo(enlace, "w", crearNumero(resultado.enlaces[i].costo));
		agregarElemento(enlaces, enlace);
	}
	JsonValue respuesta = crearObjeto();
	agregarCampo(respuesta, "algorithm", crearTexto(algoritmo));
	agregarCampo(respuesta, "edges", enlaces);
	agregarCampo(respuesta, "totalCost", crearNumero(resultado.costoTotal));
	agregarCampo(respuesta, "connected", crearBooleano(resultado.conectado));
	return respuesta;
}

/*
 * Resuelve el modo "mst": lee el grafo, elige Prim o Kruskal (por defecto Kruskal) y responde.
 * Parametro entrada: objeto JSON del campo input; acepta el campo opcional "algorithm".
 * Retorno: respuesta JSON del modo.
 */
JsonValue resolverMst(const JsonValue& entrada) {
	Grafo grafo = leerGrafo(entrada);
	int numeroNodos = static_cast<int>(grafo.identificadores.size());
	std::string algoritmo = "kruskal";
	const JsonValue* campo = buscarCampo(entrada, "algorithm");
	if (campo != nullptr && campo->tipo == JsonTipo::TEXTO && aMinusculas(campo->texto) == "prim") {
		algoritmo = "prim";
	}
	ResultadoMst resultado;
	if (algoritmo == "prim") {
		resultado = calcularMstPrim(numeroNodos, grafo.aristas);
	} else {
		resultado = calcularMstKruskal(numeroNodos, grafo.aristas);
	}
	return construirRespuestaMst(grafo, resultado, algoritmo);
}

#endif
