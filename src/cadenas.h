/*
 * Descripcion: algoritmos de cadenas para ciberseguridad: busqueda de firmas
 * (KMP), palindromo mas largo (Manacher) y subcadena comun mas larga.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 20/10/2026
 */

#ifndef CADENAS_H
#define CADENAS_H

#include <stdexcept>

#include "json.h"

/*
 * Resuelve el modo de busqueda de firmas. Pendiente (persona C): tres firmas contra dos
 * transmisiones, seis resultados con la primera posicion (base 1) de cada coincidencia.
 * Parametro: objeto JSON del campo input.
 * Retorno: seis resultados de deteccion.
 */
JsonValue resolverFirmas(const JsonValue&) {
	throw std::runtime_error("modo de firmas pendiente de implementar");
}

/*
 * Resuelve el modo de palindromo mas largo. Pendiente (persona C): posicion inicial y final
 * (base 1) del palindromo continuo mas largo de cada transmision, de longitud par o impar.
 * Parametro: objeto JSON del campo input.
 * Retorno: posiciones inicial y final por transmision.
 */
JsonValue resolverPalindromo(const JsonValue&) {
	throw std::runtime_error("modo de palindromo pendiente de implementar");
}

/*
 * Resuelve el modo de subcadena comun mas larga. Pendiente (persona C): subcadena continua
 * (no subsecuencia) mas larga presente en ambas transmisiones, con posiciones en la primera.
 * Parametro: objeto JSON del campo input.
 * Retorno: posicion inicial y final en la primera transmision.
 */
JsonValue resolverSubcadenaComun(const JsonValue&) {
	throw std::runtime_error("modo de subcadena comun pendiente de implementar");
}

#endif
