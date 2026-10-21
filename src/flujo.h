/*
 * Descripcion: flujo maximo entre dos nodos (Edmonds-Karp y Dinic).
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 20/10/2026
 */

#ifndef FLUJO_H
#define FLUJO_H

#include <stdexcept>

#include "json.h"

/*
 * Resuelve el modo de flujo maximo. Pendiente (persona B): distinguir capacidades
 * (capacities.csv) de demandas (demands.csv) y devolver el valor del flujo maximo.
 * Parametro: objeto JSON del campo input.
 * Retorno: valor del flujo maximo y, si se desea, el flujo por enlace.
 */
JsonValue resolverFlujoMaximo(const JsonValue&) {
	throw std::runtime_error("modo de flujo maximo pendiente de implementar");
}

#endif
