/*
 * Descripcion: problema del viajante (TSP): exacto (Held-Karp) para instancias
 * pequenas y aproximado (vecino mas cercano + 2-opt) para la red GEANT completa.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 20/10/2026
 */

#ifndef TSP_H
#define TSP_H

#include <stdexcept>

#include "json.h"

/*
 * Resuelve el modo "tsp". Pendiente (persona A): calcular primero distancias minimas
 * entre todos los pares (Floyd-Warshall) porque un trayecto puede cruzar nodos intermedios.
 * Parametro: objeto JSON del campo input.
 * Retorno: orden de visita y distancia total.
 */
JsonValue resolverTsp(const JsonValue&) {
	throw std::runtime_error("modo tsp pendiente de implementar");
}

#endif
