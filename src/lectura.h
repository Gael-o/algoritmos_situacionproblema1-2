/*
 * Descripcion: funciones auxiliares para leer datos tolerantes desde JSON (numeros, textos,
 * listas de textos, puntos y matrices), compartidas por los modos de TSP, Voronoi y cadenas.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 21/10/2026
 */

#ifndef LECTURA_H
#define LECTURA_H

#include <cstdlib>
#include <string>
#include <vector>

#include "json.h"

const std::vector<std::string> CLAVES_TEXTO_OBJETO = {"data", "payload", "text", "content", "hex", "value", "sequence", "string", "transmission", "signature", "pattern"};
const std::vector<std::string> CLAVES_ALGORITMO = {"algorithm", "algoritmo", "method", "metodo", "technique"};
const std::vector<std::string> CLAVES_COORDENADA_X = {"x", "lon", "lng", "longitude", "longitud"};
const std::vector<std::string> CLAVES_COORDENADA_Y = {"y", "lat", "latitude", "latitud"};

struct Punto {
	double x = 0.0;
	double y = 0.0;
};

/*
 * Busca el primer campo de un objeto cuyo nombre este en la lista dada.
 * Parametro objeto: valor donde se busca.
 * Parametro nombres: nombres alternativos del campo.
 * Parametro resultado: recibe la copia del valor encontrado.
 * Retorno: true si alguno de los nombres existe.
 */
bool buscarAlguno(const JsonValue& objeto, const std::vector<std::string>& nombres, JsonValue& resultado) {
	bool encontrado = false;
	for (size_t i = 0; i < nombres.size(); i++) {
		if (!encontrado) {
			encontrado = buscarCampo(objeto, nombres[i], resultado);
		}
	}
	return encontrado;
}

/*
 * Lee el nombre opcional de la tecnica pedida (campo algorithm) en minusculas y solo con letras y digitos.
 * Parametro entrada: objeto JSON del campo input.
 * Retorno: nombre normalizado, o texto vacio si no se pidio ninguna tecnica.
 */
std::string leerAlgoritmo(const JsonValue& entrada) {
	std::string normalizado = "";
	JsonValue campo;
	if (buscarAlguno(entrada, CLAVES_ALGORITMO, campo) && campo.tipo == JsonTipo::TEXTO) {
		std::string minusculas = aMinusculas(campo.texto);
		for (size_t i = 0; i < minusculas.size(); i++) {
			bool esLetra = minusculas[i] >= 'a' && minusculas[i] <= 'z';
			bool esDigito = minusculas[i] >= '0' && minusculas[i] <= '9';
			if (esLetra || esDigito) {
				normalizado += minusculas[i];
			}
		}
	}
	return normalizado;
}

/*
 * Convierte un valor JSON a numero (acepta numeros y textos numericos).
 * Parametro valor: valor JSON a convertir.
 * Parametro numero: recibe el numero leido.
 * Retorno: true si el valor era numerico.
 */
bool extraerNumero(const JsonValue& valor, double& numero) {
	bool correcto = false;
	if (valor.tipo == JsonTipo::NUMERO) {
		numero = valor.numero;
		correcto = true;
	} else if (valor.tipo == JsonTipo::TEXTO && !valor.texto.empty()) {
		char* cola = nullptr;
		double leido = std::strtod(valor.texto.c_str(), &cola);
		if (cola != nullptr && *cola == '\0') {
			numero = leido;
			correcto = true;
		}
	}
	return correcto;
}

/*
 * Convierte un valor JSON a texto: acepta textos, numeros, arreglos de lineas (se concatenan)
 * y objetos con un campo de contenido.
 * Parametro valor: valor JSON a convertir.
 * Parametro texto: recibe el texto leido.
 * Retorno: true si se pudo obtener un texto.
 */
bool extraerTexto(const JsonValue& valor, std::string& texto) {
	bool correcto = false;
	if (valor.tipo == JsonTipo::TEXTO) {
		texto = valor.texto;
		correcto = true;
	} else if (valor.tipo == JsonTipo::NUMERO) {
		texto = formatearNumero(valor.numero);
		correcto = true;
	} else if (valor.tipo == JsonTipo::ARREGLO && !valor.arreglo.empty()) {
		correcto = true;
		texto = "";
		for (size_t i = 0; i < valor.arreglo.size() && correcto; i++) {
			correcto = valor.arreglo[i].tipo == JsonTipo::TEXTO;
			texto += valor.arreglo[i].texto;
		}
	} else if (valor.tipo == JsonTipo::OBJETO) {
		JsonValue interno;
		if (buscarAlguno(valor, CLAVES_TEXTO_OBJETO, interno)) {
			correcto = extraerTexto(interno, texto);
		}
	}
	return correcto;
}

/*
 * Normaliza una secuencia hexadecimal: quita espacios y saltos de linea y pasa las letras a mayusculas.
 * Parametro texto: secuencia original.
 * Retorno: secuencia normalizada, sobre la que se miden las posiciones.
 */
std::string normalizarSecuencia(const std::string& texto) {
	std::string normalizada = "";
	for (size_t i = 0; i < texto.size(); i++) {
		char actual = texto[i];
		bool esEspacio = actual == ' ' || actual == '\n' || actual == '\r' || actual == '\t';
		if (!esEspacio && actual >= 'a' && actual <= 'z') {
			actual = static_cast<char>(actual - 'a' + 'A');
		}
		if (!esEspacio) {
			normalizada += actual;
		}
	}
	return normalizada;
}

/*
 * Lee una lista de textos desde el primer campo de la entrada que sea un arreglo.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro nombres: nombres alternativos del campo.
 * Parametro textos: recibe los textos leidos.
 * Retorno: true si el campo existia y todos sus elementos eran textos.
 */
bool leerListaTextos(const JsonValue& entrada, const std::vector<std::string>& nombres, std::vector<std::string>& textos) {
	bool correcto = false;
	JsonValue lista;
	if (buscarAlguno(entrada, nombres, lista) && lista.tipo == JsonTipo::ARREGLO) {
		correcto = true;
		for (size_t i = 0; i < lista.arreglo.size() && correcto; i++) {
			std::string texto = "";
			correcto = extraerTexto(lista.arreglo[i], texto);
			textos.push_back(texto);
		}
	}
	return correcto;
}

/*
 * Reune todos los campos de texto de un objeto, en el orden en que aparecen.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro textos: recibe los textos encontrados.
 */
void recolectarCamposTexto(const JsonValue& entrada, std::vector<std::string>& textos) {
	if (entrada.tipo == JsonTipo::OBJETO) {
		for (size_t i = 0; i < entrada.valores.size(); i++) {
			if (entrada.valores[i].tipo == JsonTipo::TEXTO) {
				textos.push_back(entrada.valores[i].texto);
			}
		}
	} else if (entrada.tipo == JsonTipo::ARREGLO) {
		for (size_t i = 0; i < entrada.arreglo.size(); i++) {
			if (entrada.arreglo[i].tipo == JsonTipo::TEXTO) {
				textos.push_back(entrada.arreglo[i].texto);
			}
		}
	}
}

/*
 * Reune los textos de un objeto, tanto los campos de texto como los arreglos de textos, en orden.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro textos: recibe los textos encontrados.
 */
void recolectarTextosEnArreglos(const JsonValue& entrada, std::vector<std::string>& textos) {
	for (size_t i = 0; i < entrada.valores.size(); i++) {
		if (entrada.valores[i].tipo == JsonTipo::TEXTO) {
			textos.push_back(entrada.valores[i].texto);
		} else if (entrada.valores[i].tipo == JsonTipo::ARREGLO) {
			for (size_t j = 0; j < entrada.valores[i].arreglo.size(); j++) {
				std::string texto = "";
				if (entrada.valores[i].arreglo[j].tipo == JsonTipo::TEXTO && extraerTexto(entrada.valores[i].arreglo[j], texto)) {
					textos.push_back(texto);
				}
			}
		}
	}
}

/*
 * Lee un punto escrito como [x, y] o como objeto con campos x/y (o lon/lat).
 * Parametro elemento: valor JSON del punto.
 * Parametro punto: recibe las coordenadas.
 * Retorno: true si el punto se leyo correctamente.
 */
bool leerPunto(const JsonValue& elemento, Punto& punto) {
	bool correcto = false;
	if (elemento.tipo == JsonTipo::ARREGLO && elemento.arreglo.size() >= 2) {
		correcto = extraerNumero(elemento.arreglo[0], punto.x) && extraerNumero(elemento.arreglo[1], punto.y);
	} else if (elemento.tipo == JsonTipo::OBJETO) {
		JsonValue valorX;
		JsonValue valorY;
		correcto = buscarAlguno(elemento, CLAVES_COORDENADA_X, valorX) && buscarAlguno(elemento, CLAVES_COORDENADA_Y, valorY);
		correcto = correcto && extraerNumero(valorX, punto.x) && extraerNumero(valorY, punto.y);
	}
	return correcto;
}

/*
 * Lee una lista de puntos desde el primer campo de la entrada que sea un arreglo de puntos.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro nombres: nombres alternativos del campo.
 * Parametro puntos: recibe los puntos leidos.
 * Retorno: true si el campo existia y todos sus elementos eran puntos validos.
 */
bool leerListaPuntos(const JsonValue& entrada, const std::vector<std::string>& nombres, std::vector<Punto>& puntos) {
	bool correcto = false;
	JsonValue lista;
	if (buscarAlguno(entrada, nombres, lista) && lista.tipo == JsonTipo::ARREGLO) {
		correcto = true;
		for (size_t i = 0; i < lista.arreglo.size() && correcto; i++) {
			Punto punto;
			correcto = leerPunto(lista.arreglo[i], punto);
			puntos.push_back(punto);
		}
	}
	return correcto;
}

/*
 * Crea un arreglo JSON de numeros enteros a partir de un vector.
 * Parametro valores: valores a convertir.
 * Retorno: arreglo JSON con los numeros.
 */
JsonValue crearArregloNumeros(const std::vector<double>& valores) {
	JsonValue arreglo = crearArreglo();
	for (size_t i = 0; i < valores.size(); i++) {
		agregarElemento(arreglo, crearNumero(valores[i]));
	}
	return arreglo;
}

#endif
