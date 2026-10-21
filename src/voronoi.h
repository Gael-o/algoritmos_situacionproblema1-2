/*
 * Descripcion: diagrama de Voronoi sobre coordenadas planas, recortado al dominio
 * del paquete oficial, con consulta de la central mas cercana a una ubicacion.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 20/10/2026
 */

#ifndef VORONOI_H
#define VORONOI_H

#include <stdexcept>

#include "json.h"

/*
 * Resuelve el modo "voronoi". Pendiente (persona B): construir las regiones, recortarlas
 * al dominio de scenarios.json y responder consultas de proximidad.
 * Parametro: objeto JSON del campo input.
 * Retorno: regiones y resultados de las consultas.
 */
JsonValue resolverVoronoi(const JsonValue&) {
	throw std::runtime_error("modo voronoi pendiente de implementar");
}

#endif
