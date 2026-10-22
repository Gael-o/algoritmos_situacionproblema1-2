/*
 * Descripcion: estructura de grafo no dirigido y lector tolerante desde JSON,
 * compartido por MST, TSP y flujo maximo. Los nombres de campo aceptados son
 * supuestos que deben ajustarse cuando se conozca el esquema oficial del autograder.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), Damian Alejandro Gomez Mena (A01385923), David Rangel Monsivais (A01385671)
 * Fecha: 21/10/2026
 */

#ifndef GRAFO_H
#define GRAFO_H

#include <map>
#include <string>
#include <vector>

#include "json.h"

const std::vector<std::string> CLAVES_ARISTAS = {"edges", "links", "aristas", "enlaces"};
const std::vector<std::string> CLAVES_NODOS = {"nodes", "nodos", "vertices", "n", "N", "numNodes", "nodeCount", "num_nodes", "node_count"};
const std::vector<std::string> CLAVES_ORIGEN = {"u", "from", "source", "origen", "src", "a"};
const std::vector<std::string> CLAVES_DESTINO = {"v", "to", "target", "destino", "dst", "b"};
const std::vector<std::string> CLAVES_COSTO = {"w", "weight", "cost", "costo", "distance", "distancia", "capacity", "cap", "capacidad", "c"};
const std::vector<std::string> CLAVES_IDENTIFICADOR = {"id", "name", "nombre"};

struct Arista {
	int origen = 0;
	int destino = 0;
	double costo = 0.0;
};

struct Grafo {
	std::vector<JsonValue> identificadores;
	std::vector<Arista> aristas;
};

/*
 * Busca en un objeto el primer campo cuyo nombre aparezca en la lista dada.
 * Parametro objeto: valor donde se busca.
 * Parametro nombres: nombres alternativos del campo, en orden de preferencia.
 * Parametro resultado: recibe la copia del valor encontrado.
 * Retorno: true si alguno de los nombres existe.
 */
bool buscarPrimerCampo(const JsonValue& objeto, const std::vector<std::string>& nombres, JsonValue& resultado) {
	bool encontrado = false;
	for (size_t i = 0; i < nombres.size(); i++) {
		if (!encontrado) {
			encontrado = buscarCampo(objeto, nombres[i], resultado);
		}
	}
	return encontrado;
}

/*
 * Obtiene el indice interno de un nodo, registrandolo si es la primera vez que aparece.
 * Parametro grafo: grafo donde se guardan los identificadores originales.
 * Parametro indices: tabla de identificador a indice interno.
 * Parametro identificador: valor JSON original del nodo (numero o texto).
 * Parametro indice: recibe el indice interno del nodo (0 a n-1).
 * Retorno: false si el identificador no es numero ni texto.
 */
bool registrarNodo(Grafo& grafo, std::map<std::string, int>& indices, const JsonValue& identificador, int& indice) {
	bool valido = identificador.tipo == JsonTipo::TEXTO || identificador.tipo == JsonTipo::NUMERO;
	if (valido) {
		std::string clave = identificador.texto;
		if (identificador.tipo == JsonTipo::NUMERO) {
			clave = formatearNumero(identificador.numero);
		}
		std::map<std::string, int>::iterator encontrado = indices.find(clave);
		if (encontrado == indices.end()) {
			indice = static_cast<int>(grafo.identificadores.size());
			indices[clave] = indice;
			grafo.identificadores.push_back(identificador);
		} else {
			indice = encontrado->second;
		}
	}
	return valido;
}

/*
 * Lee una arista, escrita como objeto o como arreglo [origen, destino, costo].
 * Parametro elemento: valor JSON de la arista.
 * Parametro grafo: grafo al que se agrega.
 * Parametro indices: tabla de identificador a indice interno.
 * Parametro mensaje: recibe la descripcion del problema si la arista no se reconoce.
 * Retorno: true si la arista se leyo correctamente.
 */
bool leerArista(const JsonValue& elemento, Grafo& grafo, std::map<std::string, int>& indices, std::string& mensaje) {
	JsonValue origen;
	JsonValue destino;
	JsonValue costo;
	bool tieneOrigen = false;
	bool tieneDestino = false;
	bool tieneCosto = false;
	if (elemento.tipo == JsonTipo::ARREGLO && elemento.arreglo.size() >= 2) {
		origen = elemento.arreglo[0];
		destino = elemento.arreglo[1];
		tieneOrigen = true;
		tieneDestino = true;
		if (elemento.arreglo.size() >= 3) {
			costo = elemento.arreglo[2];
			tieneCosto = true;
		}
	} else if (elemento.tipo == JsonTipo::OBJETO) {
		tieneOrigen = buscarPrimerCampo(elemento, CLAVES_ORIGEN, origen);
		tieneDestino = buscarPrimerCampo(elemento, CLAVES_DESTINO, destino);
		tieneCosto = buscarPrimerCampo(elemento, CLAVES_COSTO, costo);
	}
	bool correcto = tieneOrigen && tieneDestino;
	if (!correcto) {
		mensaje = "arista no reconocida, campos recibidos: " + listarClaves(elemento);
	} else {
		Arista arista;
		arista.costo = 1.0;
		correcto = registrarNodo(grafo, indices, origen, arista.origen) && registrarNodo(grafo, indices, destino, arista.destino);
		if (!correcto) {
			mensaje = "identificador de nodo invalido: debe ser numero o texto";
		} else {
			if (tieneCosto && costo.tipo == JsonTipo::NUMERO) {
				arista.costo = costo.numero;
			}
			grafo.aristas.push_back(arista);
		}
	}
	return correcto;
}

/*
 * Construye un grafo a partir de la entrada JSON de un modo.
 * Parametro entrada: objeto con la lista de aristas y, opcionalmente, la lista de nodos.
 * Parametro grafo: recibe identificadores originales y aristas indexadas de 0 a n-1.
 * Parametro mensaje: recibe la descripcion del problema si la entrada no se reconoce.
 * Retorno: true si la entrada se leyo correctamente.
 */
bool leerGrafo(const JsonValue& entrada, Grafo& grafo, std::string& mensaje) {
	std::map<std::string, int> indices;
	bool correcto = true;
	JsonValue nodos;
	bool hayNodos = buscarPrimerCampo(entrada, CLAVES_NODOS, nodos);
	if (hayNodos && nodos.tipo == JsonTipo::NUMERO && nodos.numero >= 0 && nodos.numero < 100000000) {
		int cantidad = static_cast<int>(nodos.numero);
		for (int i = 0; i < cantidad; i++) {
			int indice = 0;
			registrarNodo(grafo, indices, crearNumero(static_cast<double>(i)), indice);
		}
	}
	if (hayNodos && nodos.tipo == JsonTipo::ARREGLO) {
		for (size_t i = 0; i < nodos.arreglo.size() && correcto; i++) {
			JsonValue identificador = nodos.arreglo[i];
			int indice = 0;
			if (nodos.arreglo[i].tipo == JsonTipo::OBJETO) {
				correcto = buscarPrimerCampo(nodos.arreglo[i], CLAVES_IDENTIFICADOR, identificador);
			}
			correcto = correcto && registrarNodo(grafo, indices, identificador, indice);
			if (!correcto) {
				mensaje = "nodo no reconocido, debe ser numero, texto u objeto con id";
			}
		}
	}
	JsonValue aristas;
	if (correcto) {
		correcto = buscarPrimerCampo(entrada, CLAVES_ARISTAS, aristas) && aristas.tipo == JsonTipo::ARREGLO;
		if (!correcto) {
			mensaje = "la entrada no trae la lista de aristas, campos recibidos: " + listarClaves(entrada);
		}
	}
	for (size_t i = 0; i < aristas.arreglo.size() && correcto; i++) {
		correcto = leerArista(aristas.arreglo[i], grafo, indices, mensaje);
	}
	return correcto;
}

#endif
