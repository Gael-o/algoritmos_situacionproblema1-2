/*
 * Descripcion: punto de entrada de Sentinela del Norte. Lee un objeto JSON por stdin
 * ({"mode": ..., "input": {...}}), ejecuta el algoritmo pedido y escribe un objeto JSON por stdout.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 20/10/2026
 */

#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "cadenas.h"
#include "flujo.h"
#include "json.h"
#include "mst.h"
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
 * Ejecuta el algoritmo que corresponde a un modo. Los nombres de modo aceptados son
 * supuestos hasta confirmar el esquema oficial del autograder.
 * Parametro modo: nombre del modo solicitado.
 * Parametro entrada: objeto JSON del campo input.
 * Retorno: respuesta JSON del modo.
 */
JsonValue ejecutarModo(const std::string& modo, const JsonValue& entrada) {
	std::string clave = normalizarModo(modo);
	JsonValue respuesta;
	if (clave == "mst") {
		respuesta = resolverMst(entrada);
	} else if (clave == "tsp") {
		respuesta = resolverTsp(entrada);
	} else if (clave == "maxflow" || clave == "flow" || clave == "flujomaximo") {
		respuesta = resolverFlujoMaximo(entrada);
	} else if (clave == "voronoi") {
		respuesta = resolverVoronoi(entrada);
	} else if (clave == "matching" || clave == "kmp" || clave == "signatures") {
		respuesta = resolverFirmas(entrada);
	} else if (clave == "palindrome" || clave == "palindromo") {
		respuesta = resolverPalindromo(entrada);
	} else if (clave == "commonsubstr" || clave == "commonsubstring" || clave == "lcs") {
		respuesta = resolverSubcadenaComun(entrada);
	} else {
		throw std::runtime_error("modo desconocido: " + modo);
	}
	return respuesta;
}

/*
 * Valida la solicitud JSON y ejecuta el modo pedido.
 * Parametro solicitud: objeto con los campos mode e input.
 * Retorno: respuesta JSON del modo.
 */
JsonValue procesarSolicitud(const JsonValue& solicitud) {
	const JsonValue* modo = buscarCampo(solicitud, "mode");
	if (modo == nullptr || modo->tipo != JsonTipo::TEXTO) {
		throw std::runtime_error("la solicitud debe traer el campo mode como texto");
	}
	JsonValue entrada = crearObjeto();
	const JsonValue* campoEntrada = buscarCampo(solicitud, "input");
	if (campoEntrada != nullptr) {
		entrada = *campoEntrada;
	}
	return ejecutarModo(modo->texto, entrada);
}

/*
 * Construye una respuesta de error con el mensaje dado.
 * Parametro mensaje: descripcion del error.
 * Retorno: objeto JSON con el campo error.
 */
JsonValue construirError(const std::string& mensaje) {
	JsonValue respuesta = crearObjeto();
	agregarCampo(respuesta, "error", crearTexto(mensaje));
	return respuesta;
}

/*
 * Lee la solicitud por stdin, la procesa y escribe la respuesta por stdout.
 * Retorno: 0 si todo salio bien, 1 si hubo un error (que tambien se escribe como JSON).
 */
int main() {
	int codigoSalida = 0;
	JsonValue respuesta;
	try {
		std::string textoEntrada = leerEntradaCompleta();
		JsonParser lector(textoEntrada);
		respuesta = procesarSolicitud(lector.parsear());
	} catch (const std::exception& error) {
		respuesta = construirError(error.what());
		codigoSalida = 1;
	}
	std::cout << serializar(respuesta) << "\n";
	return codigoSalida;
}
