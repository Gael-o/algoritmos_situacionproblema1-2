/*
 * Descripcion: diagrama de Voronoi sobre coordenadas planas. Cada celda se obtiene recortando
 * el dominio rectangular con los semiplanos de cada otro sitio, y una consulta devuelve el
 * sitio mas cercano a una ubicacion.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 21/10/2026
 */

#ifndef VORONOI_H
#define VORONOI_H

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "json.h"
#include "lectura.h"

typedef std::vector<Punto> Poligono;

const double TOLERANCIA_VORONOI = 1.0e-12;
const double MARGEN_DOMINIO = 0.1;
const std::vector<std::string> CLAVES_SITIOS = {"sites", "points", "centers", "centrals", "stations", "seeds", "nodes", "centrales", "sitios"};
const std::vector<std::string> CLAVES_CONSULTAS = {"queries", "query", "locations", "targets", "consultas"};
const std::vector<std::string> CLAVES_DOMINIO = {"bounds", "domain", "bbox", "boundingBox", "bounding_box", "dominio"};
const std::vector<std::string> CLAVES_LIMITE_MINIMO_X = {"xmin", "minX", "min_x", "x_min", "left"};
const std::vector<std::string> CLAVES_LIMITE_MINIMO_Y = {"ymin", "minY", "min_y", "y_min", "bottom"};
const std::vector<std::string> CLAVES_LIMITE_MAXIMO_X = {"xmax", "maxX", "max_x", "x_max", "right"};
const std::vector<std::string> CLAVES_LIMITE_MAXIMO_Y = {"ymax", "maxY", "max_y", "y_max", "top"};

struct Dominio {
	double minimoX = 0.0;
	double minimoY = 0.0;
	double maximoX = 1.0;
	double maximoY = 1.0;
};

/*
 * Calcula el rectangulo que contiene a todos los sitios, con un margen alrededor.
 * Parametro sitios: coordenadas de los sitios.
 * Retorno: dominio rectangular por defecto.
 */
Dominio calcularDominioPorDefecto(const std::vector<Punto>& sitios) {
	Dominio dominio;
	dominio.minimoX = sitios[0].x;
	dominio.maximoX = sitios[0].x;
	dominio.minimoY = sitios[0].y;
	dominio.maximoY = sitios[0].y;
	for (size_t i = 1; i < sitios.size(); i++) {
		dominio.minimoX = std::min(dominio.minimoX, sitios[i].x);
		dominio.maximoX = std::max(dominio.maximoX, sitios[i].x);
		dominio.minimoY = std::min(dominio.minimoY, sitios[i].y);
		dominio.maximoY = std::max(dominio.maximoY, sitios[i].y);
	}
	double ancho = dominio.maximoX - dominio.minimoX;
	double alto = dominio.maximoY - dominio.minimoY;
	double margen = std::max(ancho, alto) * MARGEN_DOMINIO;
	if (margen <= 0.0) {
		margen = 1.0;
	}
	dominio.minimoX -= margen;
	dominio.maximoX += margen;
	dominio.minimoY -= margen;
	dominio.maximoY += margen;
	return dominio;
}

/*
 * Lee el dominio de la entrada, escrito como [xmin, ymin, xmax, ymax] o como objeto con limites.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro dominio: recibe el dominio leido.
 * Retorno: true si la entrada traia un dominio valido.
 */
bool leerDominio(const JsonValue& entrada, Dominio& dominio) {
	bool correcto = false;
	JsonValue campo;
	if (buscarAlguno(entrada, CLAVES_DOMINIO, campo)) {
		if (campo.tipo == JsonTipo::ARREGLO && campo.arreglo.size() >= 4) {
			correcto = extraerNumero(campo.arreglo[0], dominio.minimoX) && extraerNumero(campo.arreglo[1], dominio.minimoY);
			correcto = correcto && extraerNumero(campo.arreglo[2], dominio.maximoX) && extraerNumero(campo.arreglo[3], dominio.maximoY);
		} else if (campo.tipo == JsonTipo::OBJETO) {
			JsonValue minimoX;
			JsonValue minimoY;
			JsonValue maximoX;
			JsonValue maximoY;
			correcto = buscarAlguno(campo, CLAVES_LIMITE_MINIMO_X, minimoX) && buscarAlguno(campo, CLAVES_LIMITE_MINIMO_Y, minimoY);
			correcto = correcto && buscarAlguno(campo, CLAVES_LIMITE_MAXIMO_X, maximoX) && buscarAlguno(campo, CLAVES_LIMITE_MAXIMO_Y, maximoY);
			correcto = correcto && extraerNumero(minimoX, dominio.minimoX) && extraerNumero(minimoY, dominio.minimoY);
			correcto = correcto && extraerNumero(maximoX, dominio.maximoX) && extraerNumero(maximoY, dominio.maximoY);
		}
	}
	return correcto;
}

/*
 * Recorta un poligono convexo con el semiplano de los puntos mas cercanos al sitio que a otro sitio.
 * Parametro poligono: poligono actual de la celda.
 * Parametro sitio: sitio dueno de la celda.
 * Parametro otro: otro sitio que limita la celda.
 * Retorno: poligono recortado (puede quedar vacio).
 */
Poligono recortarConBisectriz(const Poligono& poligono, const Punto& sitio, const Punto& otro) {
	double normalX = otro.x - sitio.x;
	double normalY = otro.y - sitio.y;
	double limite = (otro.x * otro.x + otro.y * otro.y - sitio.x * sitio.x - sitio.y * sitio.y) / 2.0;
	Poligono resultado;
	size_t cantidad = poligono.size();
	for (size_t i = 0; i < cantidad; i++) {
		Punto actual = poligono[i];
		Punto siguiente = poligono[(i + 1) % cantidad];
		double valorActual = normalX * actual.x + normalY * actual.y - limite;
		double valorSiguiente = normalX * siguiente.x + normalY * siguiente.y - limite;
		bool actualDentro = valorActual <= TOLERANCIA_VORONOI;
		bool siguienteDentro = valorSiguiente <= TOLERANCIA_VORONOI;
		if (actualDentro) {
			resultado.push_back(actual);
		}
		if (actualDentro != siguienteDentro) {
			double fraccion = valorActual / (valorActual - valorSiguiente);
			Punto corte;
			corte.x = actual.x + fraccion * (siguiente.x - actual.x);
			corte.y = actual.y + fraccion * (siguiente.y - actual.y);
			resultado.push_back(corte);
		}
	}
	return resultado;
}

/*
 * Construye el poligono rectangular del dominio, punto de partida de cada celda.
 * Parametro dominio: rectangulo que limita las celdas.
 * Retorno: cuatro vertices en sentido antihorario.
 */
Poligono construirRectangulo(const Dominio& dominio) {
	Poligono rectangulo;
	Punto esquina;
	esquina.x = dominio.minimoX;
	esquina.y = dominio.minimoY;
	rectangulo.push_back(esquina);
	esquina.x = dominio.maximoX;
	rectangulo.push_back(esquina);
	esquina.y = dominio.maximoY;
	rectangulo.push_back(esquina);
	esquina.x = dominio.minimoX;
	rectangulo.push_back(esquina);
	return rectangulo;
}

/*
 * Calcula la celda de Voronoi de un sitio recortando con todos los demas sitios (tecnica directa).
 * Parametro sitios: coordenadas de todos los sitios.
 * Parametro indice: sitio cuya celda se calcula.
 * Parametro dominio: rectangulo que limita las celdas.
 * Retorno: vertices de la celda en sentido antihorario.
 */
Poligono calcularCelda(const std::vector<Punto>& sitios, size_t indice, const Dominio& dominio) {
	Poligono celda = construirRectangulo(dominio);
	for (size_t j = 0; j < sitios.size() && !celda.empty(); j++) {
		bool distinto = sitios[j].x != sitios[indice].x || sitios[j].y != sitios[indice].y;
		if (j != indice && distinto) {
			celda = recortarConBisectriz(celda, sitios[indice], sitios[j]);
		}
	}
	return celda;
}

struct VecinoSitio {
	size_t indice = 0;
	double distanciaCuadrada = 0.0;
};

/*
 * Compara dos vecinos por distancia (y por indice si empatan).
 * Parametro primero: vecino izquierdo.
 * Parametro segundo: vecino derecho.
 * Retorno: true si el primero debe ir antes.
 */
bool compararVecinos(const VecinoSitio& primero, const VecinoSitio& segundo) {
	bool menor = false;
	if (primero.distanciaCuadrada != segundo.distanciaCuadrada) {
		menor = primero.distanciaCuadrada < segundo.distanciaCuadrada;
	} else {
		menor = primero.indice < segundo.indice;
	}
	return menor;
}

/*
 * Calcula el cuadrado de la distancia del sitio a su vertice mas lejano de la celda.
 * Parametro celda: vertices de la celda actual.
 * Parametro sitio: sitio dueno de la celda.
 * Retorno: radio al cuadrado que cubre la celda.
 */
double calcularRadioCuadrado(const Poligono& celda, const Punto& sitio) {
	double radio = 0.0;
	for (size_t i = 0; i < celda.size(); i++) {
		double dx = celda[i].x - sitio.x;
		double dy = celda[i].y - sitio.y;
		radio = std::max(radio, dx * dx + dy * dy);
	}
	return radio;
}

/*
 * Calcula la celda de Voronoi recortando primero con los sitios mas cercanos y deteniendose
 * cuando los demas ya no pueden afectar la celda (tecnica ordenada).
 * Parametro sitios: coordenadas de todos los sitios.
 * Parametro indice: sitio cuya celda se calcula.
 * Parametro dominio: rectangulo que limita las celdas.
 * Retorno: vertices de la celda en sentido antihorario.
 */
Poligono calcularCeldaOrdenada(const std::vector<Punto>& sitios, size_t indice, const Dominio& dominio) {
	std::vector<VecinoSitio> vecinos;
	for (size_t j = 0; j < sitios.size(); j++) {
		if (j != indice) {
			VecinoSitio vecino;
			vecino.indice = j;
			double dx = sitios[j].x - sitios[indice].x;
			double dy = sitios[j].y - sitios[indice].y;
			vecino.distanciaCuadrada = dx * dx + dy * dy;
			vecinos.push_back(vecino);
		}
	}
	std::sort(vecinos.begin(), vecinos.end(), compararVecinos);
	Poligono celda = construirRectangulo(dominio);
	size_t posicion = 0;
	bool continuar = true;
	while (posicion < vecinos.size() && !celda.empty() && continuar) {
		double radio = calcularRadioCuadrado(celda, sitios[indice]);
		if (vecinos[posicion].distanciaCuadrada > 4.0 * radio) {
			continuar = false;
		} else {
			celda = recortarConBisectriz(celda, sitios[indice], sitios[vecinos[posicion].indice]);
			posicion++;
		}
	}
	return celda;
}

/*
 * Calcula el area de un poligono con la formula del cordon.
 * Parametro poligono: vertices del poligono.
 * Retorno: area (0 si tiene menos de tres vertices).
 */
double calcularArea(const Poligono& poligono) {
	double suma = 0.0;
	size_t cantidad = poligono.size();
	for (size_t i = 0; i < cantidad && cantidad >= 3; i++) {
		Punto actual = poligono[i];
		Punto siguiente = poligono[(i + 1) % cantidad];
		suma += actual.x * siguiente.y - siguiente.x * actual.y;
	}
	return std::fabs(suma) / 2.0;
}

/*
 * Encuentra el sitio mas cercano a una ubicacion (empate: el de menor indice).
 * Parametro sitios: coordenadas de los sitios.
 * Parametro ubicacion: punto consultado.
 * Retorno: indice del sitio mas cercano.
 */
size_t buscarSitioCercano(const std::vector<Punto>& sitios, const Punto& ubicacion) {
	size_t mejor = 0;
	double mejorDistancia = -1.0;
	for (size_t i = 0; i < sitios.size(); i++) {
		double dx = sitios[i].x - ubicacion.x;
		double dy = sitios[i].y - ubicacion.y;
		double distancia = dx * dx + dy * dy;
		if (mejorDistancia < 0.0 || distancia < mejorDistancia) {
			mejorDistancia = distancia;
			mejor = i;
		}
	}
	return mejor;
}

/*
 * Convierte un poligono a un arreglo JSON de pares [x, y].
 * Parametro poligono: vertices del poligono.
 * Retorno: arreglo JSON de vertices.
 */
JsonValue construirPoligonoJson(const Poligono& poligono) {
	JsonValue vertices = crearArreglo();
	for (size_t i = 0; i < poligono.size(); i++) {
		JsonValue par = crearArreglo();
		agregarElemento(par, crearNumero(poligono[i].x));
		agregarElemento(par, crearNumero(poligono[i].y));
		agregarElemento(vertices, par);
	}
	return vertices;
}

/*
 * Construye la respuesta JSON del modo "voronoi" con una celda por sitio y las consultas resueltas.
 * Parametro sitios: coordenadas de los sitios.
 * Parametro dominio: rectangulo que limita las celdas.
 * Parametro consultas: ubicaciones consultadas (pueden ser ninguna).
 * Parametro ordenado: true para usar la tecnica ordenada en lugar de la directa.
 * Retorno: objeto JSON con cells (un arreglo de vertices [x, y] por sitio), regions (con detalle) y nearest si hubo consultas.
 */
JsonValue construirRespuestaVoronoi(const std::vector<Punto>& sitios, const Dominio& dominio, const std::vector<Punto>& consultas, bool ordenado) {
	JsonValue celdas = crearArreglo();
	JsonValue regiones = crearArreglo();
	for (size_t i = 0; i < sitios.size(); i++) {
		Poligono poligono = calcularCelda(sitios, i, dominio);
		if (ordenado) {
			poligono = calcularCeldaOrdenada(sitios, i, dominio);
		}
		agregarElemento(celdas, construirPoligonoJson(poligono));
		JsonValue celda = crearObjeto();
		agregarCampo(celda, "site", crearNumero(static_cast<double>(i)));
		agregarCampo(celda, "index", crearNumero(static_cast<double>(i)));
		agregarCampo(celda, "polygon", construirPoligonoJson(poligono));
		agregarCampo(celda, "vertices", construirPoligonoJson(poligono));
		agregarCampo(celda, "area", crearNumero(calcularArea(poligono)));
		agregarElemento(regiones, celda);
	}
	JsonValue respuesta = crearObjeto();
	agregarCampo(respuesta, "algorithm", crearTexto(ordenado ? "half-plane-sorted" : "half-plane-clipping"));
	agregarCampo(respuesta, "cells", celdas);
	agregarCampo(respuesta, "regions", regiones);
	agregarCampo(respuesta, "numCells", crearNumero(static_cast<double>(sitios.size())));
	if (!consultas.empty()) {
		JsonValue cercanos = crearArreglo();
		for (size_t i = 0; i < consultas.size(); i++) {
			agregarElemento(cercanos, crearNumero(static_cast<double>(buscarSitioCercano(sitios, consultas[i]))));
		}
		agregarCampo(respuesta, "nearest", cercanos);
		agregarCampo(respuesta, "assignments", cercanos);
	}
	return respuesta;
}

/*
 * Resuelve el modo "voronoi": regiones de cercania de cada sitio recortadas al dominio.
 * Parametro entrada: objeto JSON del campo input (sitios, dominio opcional y consultas opcionales).
 * Retorno: respuesta JSON con una celda por sitio, o un error si la entrada no se reconoce.
 */
JsonValue resolverVoronoi(const JsonValue& entrada) {
	std::vector<Punto> sitios;
	std::vector<Punto> consultas;
	JsonValue respuesta;
	if (!leerListaPuntos(entrada, CLAVES_SITIOS, sitios) || sitios.empty()) {
		respuesta = construirError("voronoi: no se reconocen los sitios, campos recibidos: " + listarClaves(entrada));
	} else {
		Dominio dominio = calcularDominioPorDefecto(sitios);
		Dominio leido = dominio;
		if (leerDominio(entrada, leido)) {
			dominio = leido;
		}
		leerListaPuntos(entrada, CLAVES_CONSULTAS, consultas);
		std::string tecnica = leerAlgoritmo(entrada);
		bool ordenado = tecnica == "sorted" || tecnica == "ordenado" || tecnica == "halfplanesorted";
		respuesta = construirRespuestaVoronoi(sitios, dominio, consultas, ordenado);
	}
	return respuesta;
}

#endif
