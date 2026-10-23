/*
 * Descripcion: punto de entrada de Sentinela del Norte. Lee un objeto JSON por stdin
 * ({"mode": ..., "input": {...}}), ejecuta el algoritmo pedido y escribe un objeto JSON por stdout.
 * Los errores no usan excepciones: se responden como {"error": "..."}.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), Damian Alejandro Gomez Mena (A01385923), David Rangel Monsivais (A01385671)
 * Fecha: 21/10/2026
 */

#include <iostream>
#include <iterator>
#include <string>

#include "cadenas.h"
#include "flujo.h"
#include "json.h"
#include "mst.h"
#include "propagacion.h"
#include "tsp.h"
#include "voronoi.h"

/*
 * Lee todo stdin como texto.
 * Retorno: contenido completo de la entrada estandar.
 */
std::string leerEntradaCompleta() {
	std::istreambuf_iterator<char> inicio(std::cin);
	std::istreambuf_iterator<char> fin;
	return std::string(inicio, fin);
}

/*
 * Normaliza el nombre de un modo: minusculas y solo letras o digitos
 * (asi "Max flow", "max_flow" y "maxflow" son el mismo modo).
 * Parametro modo: nombre recibido.
 * Retorno: nombre normalizado.
 */
std::string normalizarModo(const std::string& modo) {
	std::string normalizado = "";
	std::string minusculas = aMinusculas(modo);
	for (size_t i = 0; i < minusculas.size(); i++) {
		bool esLetra = minusculas[i] >= 'a' && minusculas[i] <= 'z';
		bool esDigito = minusculas[i] >= '0' && minusculas[i] <= '9';
		if (esLetra || esDigito) {
			normalizado += minusculas[i];
		}
	}
	return normalizado;
}

/*
 * Indica si un texto contiene a otro.
 * Parametro texto: texto donde se busca.
 * Parametro parte: fragmento buscado.
 * Retorno: true si el fragmento aparece en el texto.
 */
bool contiene(const std::string& texto, const std::string& parte) {
	return texto.find(parte) != std::string::npos;
}

/*
 * Traduce el nombre recibido de un modo (ya normalizado) al nombre interno, aceptando variantes.
 * Parametro clave: nombre normalizado (minusculas, solo letras y digitos).
 * La propagacion se revisa al final para que no le quite nombres a los siete modos del reto.
 * Retorno: mst, tsp, maxflow, voronoi, matching, palindrome, commonsubstr, propagation, o texto
 * vacio si no se reconoce.
 */
std::string identificarModo(const std::string& clave) {
	std::string modo = "";
	if (contiene(clave, "mst") || contiene(clave, "spanning")) {
		modo = "mst";
	} else if (contiene(clave, "tsp") || contiene(clave, "salesman") || contiene(clave, "viajante")) {
		modo = "tsp";
	} else if (contiene(clave, "flow") || contiene(clave, "flujo")) {
		modo = "maxflow";
	} else if (contiene(clave, "voronoi")) {
		modo = "voronoi";
	} else if (contiene(clave, "palin")) {
		modo = "palindrome";
	} else if (contiene(clave, "substr") || contiene(clave, "common") || contiene(clave, "similar") || contiene(clave, "lcs")) {
		modo = "commonsubstr";
	} else if (contiene(clave, "match") || contiene(clave, "signature") || contiene(clave, "kmp") || contiene(clave, "pattern") || contiene(clave, "firma")) {
		modo = "matching";
	} else if (contiene(clave, "propag") || contiene(clave, "route") || contiene(clave, "ruta") || contiene(clave, "dijkstra") || contiene(clave, "shortest")) {
		modo = "propagation";
	}
	return modo;
}

/*
 * Ejecuta el algoritmo que corresponde a un modo.
 * Parametro modo: nombre del modo solicitado.
 * Parametro entrada: objeto JSON del campo input.
 * Retorno: respuesta JSON del modo, o un error si el modo no existe.
 */
JsonValue ejecutarModo(const std::string& modo, const JsonValue& entrada) {
	std::string interno = identificarModo(normalizarModo(modo));
	JsonValue respuesta;
	if (interno == "mst") {
		respuesta = resolverMst(entrada);
	} else if (interno == "tsp") {
		respuesta = resolverTsp(entrada);
	} else if (interno == "maxflow") {
		respuesta = resolverFlujoMaximo(entrada);
	} else if (interno == "voronoi") {
		respuesta = resolverVoronoi(entrada);
	} else if (interno == "matching") {
		respuesta = resolverFirmas(entrada);
	} else if (interno == "palindrome") {
		respuesta = resolverPalindromo(entrada);
	} else if (interno == "commonsubstr") {
		respuesta = resolverSubcadenaComun(entrada);
	} else if (interno == "propagation") {
		respuesta = resolverPropagacion(entrada);
	} else {
		respuesta = construirError("modo desconocido: " + modo);
	}
	return respuesta;
}

/*
 * Valida la solicitud JSON y ejecuta el modo pedido.
 * Parametro solicitud: objeto con los campos mode e input.
 * Retorno: respuesta JSON del modo, o un error si la solicitud no es valida.
 */
JsonValue procesarSolicitud(const JsonValue& solicitud) {
	JsonValue respuesta;
	JsonValue modo;
	if (!buscarCampo(solicitud, "mode", modo) || modo.tipo != JsonTipo::TEXTO) {
		respuesta = construirError("la solicitud debe traer el campo mode como texto, campos recibidos: " + listarClaves(solicitud));
	} else {
		JsonValue entrada = crearObjeto();
		buscarCampo(solicitud, "input", entrada);
		respuesta = ejecutarModo(modo.texto, entrada);
	}
	return respuesta;
}

/*
 * Lee la solicitud por stdin, la procesa y escribe la respuesta por stdout.
 * Retorno: siempre 0; los errores se reportan dentro del JSON de respuesta.
 */
int main() {
	std::string textoEntrada = leerEntradaCompleta();
	JsonParser lector(textoEntrada);
	JsonValue solicitud;
	JsonValue respuesta;
	std::string mensaje = "";
	if (lector.parsear(solicitud, mensaje)) {
		respuesta = procesarSolicitud(solicitud);
	} else {
		respuesta = construirError(mensaje);
	}
	std::cout << serializar(respuesta) << "\n";
	return 0;
}
