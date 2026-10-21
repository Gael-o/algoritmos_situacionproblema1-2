/*
 * Descripcion: estructura de grafo no dirigido y lector tolerante desde JSON,
 * compartido por MST, TSP y flujo maximo. Los nombres de campo aceptados son
 * supuestos que deben ajustarse cuando se conozca el esquema oficial del autograder.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 20/10/2026
 */

#ifndef GRAFO_H
#define GRAFO_H

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "json.h"

const std::vector<std::string> CLAVES_ARISTAS = {"edges", "links", "aristas", "enlaces"};
const std::vector<std::string> CLAVES_NODOS = {"nodes", "nodos", "vertices"};
const std::vector<std::string> CLAVES_ORIGEN = {"u", "from", "source", "origen", "src", "a"};
const std::vector<std::string> CLAVES_DESTINO = {"v", "to", "target", "destino", "dst", "b"};
const std::vector<std::string> CLAVES_COSTO = {"w", "weight", "cost", "costo", "distance", "distancia"};
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
 * Retorno: apuntador al valor o nullptr si ninguno existe.
 */
const JsonValue* buscarPrimerCampo(const JsonValue& objeto, const std::vector<std::string>& nombres) {
	const JsonValue* encontrado = nullptr;
	for (size_t i = 0; i < nombres.size(); i++) {
		if (encontrado == nullptr) {
			encontrado = buscarCampo(objeto, nombres[i]);
		}
	}
	return encontrado;
}

/*
 * Convierte el identificador de un nodo (numero o texto) en una llave de texto.
 * Parametro identificador: valor JSON que identifica al nodo.
 * Retorno: llave de texto unica para ese identificador.
 */
std::string identificadorATexto(const JsonValue& identificador) {
	std::string clave = "";
	if (identificador.tipo == JsonTipo::TEXTO) {
		clave = identificador.texto;
	} else if (identificador.tipo == JsonTipo::NUMERO) {
		clave = formatearNumero(identificador.numero);
	} else {
		throw std::runtime_error("identificador de nodo invalido: debe ser numero o texto");
	}
	return clave;
}

/*
 * Obtiene el indice interno de un nodo, registrandolo si es la primera vez que aparece.
 * Parametro grafo: grafo donde se guardan los identificadores originales.
 * Parametro indices: tabla de identificador a indice interno.
 * Parametro identificador: valor JSON original del nodo.
 * Retorno: indice interno del nodo (0 a n-1).
 */
int registrarNodo(Grafo& grafo, std::map<std::string, int>& indices, const JsonValue& identificador) {
	std::string clave = identificadorATexto(identificador);
	std::map<std::string, int>::iterator encontrado = indices.find(clave);
	int indice = 0;
	if (encontrado == indices.end()) {
		indice = static_cast<int>(grafo.identificadores.size());
		indices[clave] = indice;
		grafo.identificadores.push_back(identificador);
	} else {
		indice = encontrado->second;
	}
	return indice;
}

/*
 * Lee una arista, escrita como objeto o como arreglo [origen, destino, costo].
 * Parametro elemento: valor JSON de la arista.
 * Parametro grafo: grafo al que se agrega.
 * Parametro indices: tabla de identificador a indice interno.
 */
void leerArista(const JsonValue& elemento, Grafo& grafo, std::map<std::string, int>& indices) {
	const JsonValue* origen = nullptr;
	const JsonValue* destino = nullptr;
	const JsonValue* costo = nullptr;
	if (elemento.tipo == JsonTipo::ARREGLO && elemento.arreglo.size() >= 2) {
		origen = &elemento.arreglo[0];
		destino = &elemento.arreglo[1];
		if (elemento.arreglo.size() >= 3) {
			costo = &elemento.arreglo[2];
		}
	} else if (elemento.tipo == JsonTipo::OBJETO) {
		origen = buscarPrimerCampo(elemento, CLAVES_ORIGEN);
		destino = buscarPrimerCampo(elemento, CLAVES_DESTINO);
		costo = buscarPrimerCampo(elemento, CLAVES_COSTO);
	}
	if (origen == nullptr || destino == nullptr) {
		throw std::runtime_error("arista invalida: faltan el origen o el destino");
	}
	Arista arista;
	arista.origen = registrarNodo(grafo, indices, *origen);
	arista.destino = registrarNodo(grafo, indices, *destino);
	arista.costo = 1.0;
	if (costo != nullptr && costo->tipo == JsonTipo::NUMERO) {
		arista.costo = costo->numero;
	}
	grafo.aristas.push_back(arista);
}

/*
 * Extrae el identificador de un elemento de la lista de nodos (valor simple u objeto con id).
 * Parametro nodo: elemento de la lista de nodos.
 * Retorno: valor JSON que identifica al nodo.
 */
JsonValue extraerIdentificador(const JsonValue& nodo) {
	JsonValue identificador = nodo;
	if (nodo.tipo == JsonTipo::OBJETO) {
		const JsonValue* campo = buscarPrimerCampo(nodo, CLAVES_IDENTIFICADOR);
		if (campo == nullptr) {
			throw std::runtime_error("nodo invalido: falta el campo id");
		}
		identificador = *campo;
	}
	return identificador;
}

/*
 * Construye un grafo a partir de la entrada JSON de un modo.
 * Parametro entrada: objeto con la lista de aristas y, opcionalmente, la lista de nodos.
 * Retorno: grafo con identificadores originales y aristas indexadas de 0 a n-1.
 */
Grafo leerGrafo(const JsonValue& entrada) {
	Grafo grafo;
	std::map<std::string, int> indices;
	const JsonValue* nodos = buscarPrimerCampo(entrada, CLAVES_NODOS);
	if (nodos != nullptr && nodos->tipo == JsonTipo::ARREGLO) {
		for (size_t i = 0; i < nodos->arreglo.size(); i++) {
			registrarNodo(grafo, indices, extraerIdentificador(nodos->arreglo[i]));
		}
	}
	const JsonValue* aristas = buscarPrimerCampo(entrada, CLAVES_ARISTAS);
	if (aristas == nullptr || aristas->tipo != JsonTipo::ARREGLO) {
		throw std::runtime_error("la entrada no trae la lista de aristas (edges)");
	}
	for (size_t i = 0; i < aristas->arreglo.size(); i++) {
		leerArista(aristas->arreglo[i], grafo, indices);
	}
	return grafo;
}

#endif
