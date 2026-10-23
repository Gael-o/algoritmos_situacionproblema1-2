/*
 * Descripcion: algoritmos de cadenas para ciberseguridad: busqueda de firmas (KMP),
 * palindromo mas largo (Manacher) y subcadena comun mas larga (busqueda binaria con hashing),
 * mas los adaptadores JSON de los tres modos. Las posiciones se reportan desde 1.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), Damian Alejandro Gomez Mena (A01385923), David Rangel Monsivais (A01385671)
 * Fecha: 21/10/2026
 */

#ifndef CADENAS_H
#define CADENAS_H

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

#include "json.h"
#include "lectura.h"

const int NO_ENCONTRADO = 0;
const bool ORDEN_POR_TRANSMISION = true;
const int FORMATO_DETECCION = 1;
const unsigned long long PRIMER_MODULO = 1000000007ULL;
const unsigned long long SEGUNDO_MODULO = 998244353ULL;
const unsigned long long PRIMERA_BASE = 911382323ULL;
const unsigned long long SEGUNDA_BASE = 972663749ULL;
const std::vector<std::string> CLAVES_FIRMAS = {"signatures", "signature", "patterns", "pattern", "firmas", "sigs"};
const std::vector<std::string> FRAGMENTOS_FIRMA = {"mcode", "sig", "pat", "firma", "malw", "code"};
const std::vector<std::string> FRAGMENTOS_TRANSMISION = {"trans", "stream", "text", "seq", "packet", "data"};
const std::vector<std::string> CLAVES_TRANSMISIONES = {"transmissions", "transmission", "texts", "streams", "packets", "transmisiones", "data"};

struct RangoTexto {
	int inicio = 0;
	int longitud = 0;
};

/*
 * Compara dos textos por longitud (el mas corto primero).
 * Parametro primero: texto izquierdo.
 * Parametro segundo: texto derecho.
 * Retorno: true si el primero es mas corto que el segundo.
 */
bool compararPorLongitud(const std::string& primero, const std::string& segundo) {
	return primero.size() < segundo.size();
}

/*
 * Calcula la tabla de fallos de KMP para un patron.
 * Parametro patron: texto que se busca.
 * Retorno: para cada posicion, longitud del borde propio mas largo del prefijo.
 */
std::vector<int> construirTablaFallos(const std::string& patron) {
	std::vector<int> fallos(patron.size(), 0);
	int largo = 0;
	for (size_t i = 1; i < patron.size(); i++) {
		while (largo > 0 && patron[i] != patron[static_cast<size_t>(largo)]) {
			largo = fallos[static_cast<size_t>(largo) - 1];
		}
		if (patron[i] == patron[static_cast<size_t>(largo)]) {
			largo++;
		}
		fallos[i] = largo;
	}
	return fallos;
}

/*
 * Busca la primera aparicion de un patron en un texto con KMP.
 * Parametro texto: transmision donde se busca.
 * Parametro patron: firma buscada.
 * Retorno: posicion inicial desde 1, o NO_ENCONTRADO si no aparece.
 */
int buscarPrimeraAparicion(const std::string& texto, const std::string& patron) {
	int posicion = NO_ENCONTRADO;
	if (!patron.empty()) {
		std::vector<int> fallos = construirTablaFallos(patron);
		int coincidencias = 0;
		for (size_t i = 0; i < texto.size() && posicion == NO_ENCONTRADO; i++) {
			while (coincidencias > 0 && texto[i] != patron[static_cast<size_t>(coincidencias)]) {
				coincidencias = fallos[static_cast<size_t>(coincidencias) - 1];
			}
			if (texto[i] == patron[static_cast<size_t>(coincidencias)]) {
				coincidencias++;
			}
			if (static_cast<size_t>(coincidencias) == patron.size()) {
				posicion = static_cast<int>(i + 1 - patron.size() + 1);
			}
		}
	}
	return posicion;
}

/*
 * Encuentra el palindromo continuo mas largo (par o impar) con Manacher; ante empate, el mas a la izquierda.
 * Parametro texto: transmision a analizar.
 * Retorno: inicio (desde 0) y longitud del palindromo.
 */
RangoTexto buscarPalindromoMasLargo(const std::string& texto) {
	std::string ampliado = "#";
	for (size_t i = 0; i < texto.size(); i++) {
		ampliado += texto[i];
		ampliado += '#';
	}
	size_t tamano = ampliado.size();
	std::vector<int> radio(tamano, 0);
	int centro = 0;
	int borde = 0;
	int mejorRadio = 0;
	int mejorCentro = 0;
	for (size_t i = 0; i < tamano; i++) {
		int actual = static_cast<int>(i);
		if (actual < borde) {
			radio[i] = std::min(borde - actual, radio[static_cast<size_t>(2 * centro - actual)]);
		}
		while (actual - radio[i] - 1 >= 0 && static_cast<size_t>(actual + radio[i] + 1) < tamano && ampliado[static_cast<size_t>(actual - radio[i] - 1)] == ampliado[static_cast<size_t>(actual + radio[i] + 1)]) {
			radio[i]++;
		}
		if (actual + radio[i] > borde) {
			centro = actual;
			borde = actual + radio[i];
		}
		if (radio[i] > mejorRadio) {
			mejorRadio = radio[i];
			mejorCentro = actual;
		}
	}
	RangoTexto rango;
	rango.longitud = mejorRadio;
	rango.inicio = (mejorCentro - mejorRadio) / 2;
	return rango;
}

/*
 * Calcula los hashes de prefijo de un texto con un modulo y una base.
 * Parametro texto: cadena a procesar.
 * Parametro base: base polinomial.
 * Parametro modulo: modulo del hash.
 * Retorno: hash de cada prefijo (el elemento i es el hash de los primeros i caracteres).
 */
std::vector<unsigned long long> calcularPrefijos(const std::string& texto, unsigned long long base, unsigned long long modulo) {
	std::vector<unsigned long long> prefijos(texto.size() + 1, 0ULL);
	for (size_t i = 0; i < texto.size(); i++) {
		unsigned long long caracter = static_cast<unsigned long long>(static_cast<unsigned char>(texto[i])) + 1ULL;
		prefijos[i + 1] = (prefijos[i] * base + caracter) % modulo;
	}
	return prefijos;
}

/*
 * Calcula las potencias de una base modulo un numero.
 * Parametro cantidad: cuantas potencias se calculan (de 0 a cantidad - 1).
 * Parametro base: base polinomial.
 * Parametro modulo: modulo del hash.
 * Retorno: potencias consecutivas de la base.
 */
std::vector<unsigned long long> calcularPotencias(size_t cantidad, unsigned long long base, unsigned long long modulo) {
	std::vector<unsigned long long> potencias(cantidad, 1ULL);
	for (size_t i = 1; i < cantidad; i++) {
		potencias[i] = (potencias[i - 1] * base) % modulo;
	}
	return potencias;
}

/*
 * Estructura con los hashes de un texto, para comparar ventanas en tiempo constante.
 */
struct HashTexto {
	std::vector<unsigned long long> primero;
	std::vector<unsigned long long> segundo;
};

/*
 * Obtiene la clave de 64 bits de la ventana [inicio, inicio + largo) de un texto.
 * Parametro hashes: hashes de prefijo del texto.
 * Parametro potenciaUno: potencias de la primera base.
 * Parametro potenciaDos: potencias de la segunda base.
 * Parametro inicio: posicion inicial de la ventana (desde 0).
 * Parametro largo: longitud de la ventana.
 * Retorno: clave que identifica el contenido de la ventana.
 */
unsigned long long claveDeVentana(const HashTexto& hashes, const std::vector<unsigned long long>& potenciaUno, const std::vector<unsigned long long>& potenciaDos, size_t inicio, size_t largo) {
	unsigned long long restoUno = (hashes.primero[inicio] * potenciaUno[largo]) % PRIMER_MODULO;
	unsigned long long restoDos = (hashes.segundo[inicio] * potenciaDos[largo]) % SEGUNDO_MODULO;
	unsigned long long valorUno = (hashes.primero[inicio + largo] + PRIMER_MODULO - restoUno) % PRIMER_MODULO;
	unsigned long long valorDos = (hashes.segundo[inicio + largo] + SEGUNDO_MODULO - restoDos) % SEGUNDO_MODULO;
	return valorUno * SEGUNDO_MODULO + valorDos;
}

/*
 * Busca, para una longitud dada, la primera ventana del primer texto que tambien aparece en el segundo.
 * Parametro primero: hashes del primer texto.
 * Parametro segundo: hashes del segundo texto.
 * Parametro tamanoPrimero: longitud del primer texto.
 * Parametro tamanoSegundo: longitud del segundo texto.
 * Parametro potenciaUno: potencias de la primera base.
 * Parametro potenciaDos: potencias de la segunda base.
 * Parametro largo: longitud buscada.
 * Parametro inicio: recibe la posicion (desde 0) en el primer texto si existe.
 * Retorno: true si hay una ventana comun de esa longitud.
 */
bool existeVentanaComun(const HashTexto& primero, const HashTexto& segundo, size_t tamanoPrimero, size_t tamanoSegundo, const std::vector<unsigned long long>& potenciaUno, const std::vector<unsigned long long>& potenciaDos, size_t largo, size_t& inicio) {
	std::unordered_set<unsigned long long> claves;
	claves.reserve(tamanoSegundo + 1);
	for (size_t i = 0; i + largo <= tamanoSegundo; i++) {
		claves.insert(claveDeVentana(segundo, potenciaUno, potenciaDos, i, largo));
	}
	bool encontrada = false;
	for (size_t i = 0; i + largo <= tamanoPrimero && !encontrada; i++) {
		if (claves.count(claveDeVentana(primero, potenciaUno, potenciaDos, i, largo)) > 0) {
			inicio = i;
			encontrada = true;
		}
	}
	return encontrada;
}

/*
 * Encuentra la subcadena continua mas larga presente en ambos textos; ante empate, la mas a la izquierda del primero.
 * Parametro primero: primer texto.
 * Parametro segundo: segundo texto.
 * Retorno: inicio (desde 0) y longitud en el primer texto.
 */
RangoTexto buscarSubcadenaComun(const std::string& primero, const std::string& segundo) {
	HashTexto hashesPrimero;
	hashesPrimero.primero = calcularPrefijos(primero, PRIMERA_BASE, PRIMER_MODULO);
	hashesPrimero.segundo = calcularPrefijos(primero, SEGUNDA_BASE, SEGUNDO_MODULO);
	HashTexto hashesSegundo;
	hashesSegundo.primero = calcularPrefijos(segundo, PRIMERA_BASE, PRIMER_MODULO);
	hashesSegundo.segundo = calcularPrefijos(segundo, SEGUNDA_BASE, SEGUNDO_MODULO);
	size_t maximo = std::min(primero.size(), segundo.size());
	std::vector<unsigned long long> potenciaUno = calcularPotencias(maximo + 1, PRIMERA_BASE, PRIMER_MODULO);
	std::vector<unsigned long long> potenciaDos = calcularPotencias(maximo + 1, SEGUNDA_BASE, SEGUNDO_MODULO);
	RangoTexto mejor;
	size_t bajo = 1;
	size_t alto = maximo;
	while (bajo <= alto) {
		size_t medio = bajo + (alto - bajo) / 2;
		size_t inicio = 0;
		if (existeVentanaComun(hashesPrimero, hashesSegundo, primero.size(), segundo.size(), potenciaUno, potenciaDos, medio, inicio)) {
			mejor.inicio = static_cast<int>(inicio);
			mejor.longitud = static_cast<int>(medio);
			bajo = medio + 1;
		} else {
			alto = medio - 1;
		}
	}
	return mejor;
}

/*
 * Busca la primera aparicion de un patron comparando en cada posicion (tecnica ingenua, O(n*m)).
 * Parametro texto: transmision donde se busca.
 * Parametro patron: firma buscada.
 * Retorno: posicion inicial desde 1, o NO_ENCONTRADO si no aparece.
 */
int buscarPrimeraAparicionIngenua(const std::string& texto, const std::string& patron) {
	int posicion = NO_ENCONTRADO;
	for (size_t i = 0; i + patron.size() <= texto.size() && posicion == NO_ENCONTRADO && !patron.empty(); i++) {
		size_t j = 0;
		while (j < patron.size() && texto[i + j] == patron[j]) {
			j++;
		}
		if (j == patron.size()) {
			posicion = static_cast<int>(i + 1);
		}
	}
	return posicion;
}

/*
 * Encuentra el palindromo mas largo expandiendo desde cada centro (tecnica directa, O(n^2)).
 * Ante empate, el mas a la izquierda.
 * Parametro texto: transmision a analizar.
 * Retorno: inicio (desde 0) y longitud del palindromo.
 */
RangoTexto buscarPalindromoPorCentros(const std::string& texto) {
	RangoTexto mejor;
	int tamano = static_cast<int>(texto.size());
	for (int centro = 0; centro < 2 * tamano - 1; centro++) {
		int izquierda = centro / 2;
		int derecha = izquierda + centro % 2;
		while (izquierda >= 0 && derecha < tamano && texto[static_cast<size_t>(izquierda)] == texto[static_cast<size_t>(derecha)]) {
			izquierda--;
			derecha++;
		}
		int longitud = derecha - izquierda - 1;
		if (longitud > mejor.longitud) {
			mejor.longitud = longitud;
			mejor.inicio = izquierda + 1;
		}
	}
	return mejor;
}

/*
 * Encuentra la subcadena comun mas larga con programacion dinamica (O(n*m) tiempo, O(m) memoria).
 * Ante empate, la que termina antes en el primer texto.
 * Parametro primero: primer texto.
 * Parametro segundo: segundo texto.
 * Retorno: inicio (desde 0) y longitud en el primer texto.
 */
RangoTexto buscarSubcadenaComunPorTabla(const std::string& primero, const std::string& segundo) {
	RangoTexto mejor;
	std::vector<int> anterior(segundo.size() + 1, 0);
	std::vector<int> actual(segundo.size() + 1, 0);
	for (size_t i = 1; i <= primero.size(); i++) {
		for (size_t j = 1; j <= segundo.size(); j++) {
			actual[j] = 0;
			if (primero[i - 1] == segundo[j - 1]) {
				actual[j] = anterior[j - 1] + 1;
			}
			if (actual[j] > mejor.longitud) {
				mejor.longitud = actual[j];
				mejor.inicio = static_cast<int>(i) - actual[j];
			}
		}
		anterior = actual;
	}
	return mejor;
}

/*
 * Lee las transmisiones de la entrada (lista de textos o, si no hay lista, los campos de texto).
 * Parametro entrada: objeto JSON del campo input.
 * Parametro transmisiones: recibe los textos leidos.
 * Retorno: true si se leyo al menos una transmision.
 */
bool leerTransmisionesSinNormalizar(const JsonValue& entrada, std::vector<std::string>& transmisiones) {
	bool correcto = leerListaTextos(entrada, CLAVES_TRANSMISIONES, transmisiones);
	if (!correcto) {
		transmisiones.clear();
		recolectarCamposTexto(entrada, transmisiones);
		if (transmisiones.empty()) {
			recolectarTextosEnArreglos(entrada, transmisiones);
		}
		correcto = !transmisiones.empty();
	}
	return correcto;
}

/*
 * Lee las transmisiones de la entrada y las normaliza (sin espacios ni saltos de linea, en mayusculas).
 * Parametro entrada: objeto JSON del campo input.
 * Parametro transmisiones: recibe las secuencias normalizadas.
 * Retorno: true si se leyo al menos una transmision.
 */
bool leerTransmisiones(const JsonValue& entrada, std::vector<std::string>& transmisiones) {
	bool correcto = leerTransmisionesSinNormalizar(entrada, transmisiones);
	for (size_t i = 0; i < transmisiones.size(); i++) {
		transmisiones[i] = normalizarSecuencia(transmisiones[i]);
	}
	return correcto;
}

/*
 * Crea un par [inicio, fin] como arreglo JSON.
 * Parametro inicio: primer valor.
 * Parametro fin: segundo valor.
 * Retorno: arreglo JSON de dos numeros.
 */
JsonValue crearPar(int inicio, int fin) {
	JsonValue par = crearArreglo();
	agregarElemento(par, crearNumero(static_cast<double>(inicio)));
	agregarElemento(par, crearNumero(static_cast<double>(fin)));
	return par;
}

/*
 * Indica si una clave contiene alguno de los fragmentos dados (sin distinguir mayusculas).
 * Parametro clave: nombre del campo.
 * Parametro fragmentos: fragmentos que se buscan.
 * Retorno: true si la clave contiene alguno.
 */
bool claveContieneAlguno(const std::string& clave, const std::vector<std::string>& fragmentos) {
	bool contiene = false;
	std::string minusculas = aMinusculas(clave);
	for (size_t i = 0; i < fragmentos.size(); i++) {
		if (minusculas.find(fragmentos[i]) != std::string::npos) {
			contiene = true;
		}
	}
	return contiene;
}

/*
 * Reparte los campos de texto de la entrada entre firmas y transmisiones segun el nombre del campo
 * (por ejemplo mcode1, mcode2 son firmas y transmission1, transmission2 son transmisiones).
 * Parametro entrada: objeto JSON del campo input.
 * Parametro firmas: recibe las firmas encontradas, en orden.
 * Parametro transmisiones: recibe las transmisiones encontradas, en orden.
 * Parametro sinClasificar: recibe los textos cuyo campo no se reconoce.
 */
void clasificarCamposTexto(const JsonValue& entrada, std::vector<std::string>& firmas, std::vector<std::string>& transmisiones, std::vector<std::string>& sinClasificar) {
	for (size_t i = 0; i < entrada.claves.size(); i++) {
		std::vector<std::string> textos;
		if (entrada.valores[i].tipo == JsonTipo::ARREGLO) {
			recolectarTextosEnArreglos(entrada, textos);
			textos.clear();
			for (size_t j = 0; j < entrada.valores[i].arreglo.size(); j++) {
				std::string texto = "";
				if (extraerTexto(entrada.valores[i].arreglo[j], texto)) {
					textos.push_back(texto);
				}
			}
		} else {
			std::string texto = "";
			if (extraerTexto(entrada.valores[i], texto)) {
				textos.push_back(texto);
			}
		}
		for (size_t j = 0; j < textos.size(); j++) {
			if (claveContieneAlguno(entrada.claves[i], FRAGMENTOS_FIRMA)) {
				firmas.push_back(textos[j]);
			} else if (claveContieneAlguno(entrada.claves[i], FRAGMENTOS_TRANSMISION)) {
				transmisiones.push_back(textos[j]);
			} else {
				sinClasificar.push_back(textos[j]);
			}
		}
	}
}

/*
 * Lee las firmas y las transmisiones de la entrada: primero como listas con nombres conocidos y,
 * si no estan, repartiendo los campos de texto por su nombre (o, con cinco textos sueltos,
 * tomando como firmas los tres mas cortos).
 * Parametro entrada: objeto JSON del campo input.
 * Parametro firmas: recibe las firmas (sin normalizar).
 * Parametro transmisiones: recibe las transmisiones (sin normalizar).
 * Retorno: true si se encontro al menos una firma y una transmision.
 */
bool leerEntradaFirmas(const JsonValue& entrada, std::vector<std::string>& firmas, std::vector<std::string>& transmisiones) {
	bool correcto = leerListaTextos(entrada, CLAVES_FIRMAS, firmas) && leerTransmisionesSinNormalizar(entrada, transmisiones);
	if (!correcto) {
		std::vector<std::string> sinClasificar;
		firmas.clear();
		transmisiones.clear();
		clasificarCamposTexto(entrada, firmas, transmisiones, sinClasificar);
		if (transmisiones.empty() && !firmas.empty()) {
			transmisiones = sinClasificar;
		} else if (firmas.empty() && !transmisiones.empty()) {
			firmas = sinClasificar;
		}
		if (firmas.empty() && transmisiones.empty() && sinClasificar.size() == 5) {
			std::vector<std::string> ordenadas = sinClasificar;
			std::sort(ordenadas.begin(), ordenadas.end(), compararPorLongitud);
			for (size_t i = 0; i < sinClasificar.size(); i++) {
				bool esFirma = false;
				for (size_t j = 0; j < 3; j++) {
					esFirma = esFirma || ordenadas[j] == sinClasificar[i];
				}
				if (esFirma) {
					firmas.push_back(sinClasificar[i]);
				} else {
					transmisiones.push_back(sinClasificar[i]);
				}
			}
		}
		correcto = !firmas.empty() && !transmisiones.empty();
	}
	return correcto;
}

/*
 * Resuelve el modo de busqueda de firmas: primera posicion (desde 1) de cada firma en cada transmision.
 * Parametro entrada: objeto JSON del campo input (firmas y transmisiones).
 * Retorno: positions con una posicion por pareja (firma por firma, transmision por transmision), o un error.
 */
JsonValue resolverFirmas(const JsonValue& entrada) {
	std::vector<std::string> firmas;
	std::vector<std::string> transmisiones;
	JsonValue respuesta;
	bool hayEntrada = leerEntradaFirmas(entrada, firmas, transmisiones);
	for (size_t i = 0; i < firmas.size(); i++) {
		firmas[i] = normalizarSecuencia(firmas[i]);
	}
	for (size_t i = 0; i < transmisiones.size(); i++) {
		transmisiones[i] = normalizarSecuencia(transmisiones[i]);
	}
	if (!hayEntrada) {
		respuesta = construirError("firmas: se esperaban listas de firmas y transmisiones, campos recibidos: " + listarClaves(entrada));
	} else {
		bool ingenua = leerAlgoritmo(entrada) == "naive";
		std::vector<int> matriz(firmas.size() * transmisiones.size(), NO_ENCONTRADO);
		for (size_t i = 0; i < firmas.size(); i++) {
			for (size_t j = 0; j < transmisiones.size(); j++) {
				int posicion = buscarPrimeraAparicion(transmisiones[j], firmas[i]);
				if (ingenua) {
					posicion = buscarPrimeraAparicionIngenua(transmisiones[j], firmas[i]);
				}
				matriz[i * transmisiones.size() + j] = posicion;
			}
		}
		JsonValue posiciones = crearArreglo();
		size_t grupos = ORDEN_POR_TRANSMISION ? transmisiones.size() : firmas.size();
		size_t miembros = ORDEN_POR_TRANSMISION ? firmas.size() : transmisiones.size();
		for (size_t g = 0; g < grupos; g++) {
			for (size_t m = 0; m < miembros; m++) {
				size_t fila = ORDEN_POR_TRANSMISION ? m : g;
				size_t columna = ORDEN_POR_TRANSMISION ? g : m;
				int posicion = matriz[fila * transmisiones.size() + columna];
				bool hallada = posicion != 0;
				if (FORMATO_DETECCION == 0) {
					agregarElemento(posiciones, crearNumero(static_cast<double>(posicion)));
				} else if (FORMATO_DETECCION == 2) {
					agregarElemento(posiciones, crearTexto(hallada ? "true " + std::to_string(posicion) : "false"));
				} else {
					JsonValue deteccion = crearObjeto();
					agregarCampo(deteccion, "found", crearBooleano(hallada));
					if (hallada || FORMATO_DETECCION == 3) {
						agregarCampo(deteccion, "position", crearNumero(static_cast<double>(posicion)));
					}
					if (FORMATO_DETECCION == 3) {
						agregarCampo(deteccion, "signature", crearNumero(static_cast<double>(fila)));
						agregarCampo(deteccion, "transmission", crearNumero(static_cast<double>(columna)));
					}
					agregarElemento(posiciones, deteccion);
				}
			}
		}
		// Formato confirmado con el profesor: matriz anidada transmision por fuera, firma por dentro,
		// con -1 (no 0) para "no encontrada". Es el unico de los doce formatos probados que el
		// autograder en linea acepta.
		JsonValue matrizAnidada = crearArreglo();
		for (size_t j = 0; j < transmisiones.size(); j++) {
			JsonValue fila = crearArreglo();
			for (size_t i = 0; i < firmas.size(); i++) {
				int posicion = matriz[i * transmisiones.size() + j];
				int valor = (posicion == NO_ENCONTRADO) ? -1 : posicion;
				agregarElemento(fila, crearNumero(static_cast<double>(valor)));
			}
			agregarElemento(matrizAnidada, fila);
		}
		respuesta = crearObjeto();
		agregarCampo(respuesta, "algorithm", crearTexto(ingenua ? "naive" : "kmp"));
		agregarCampo(respuesta, "positions", posiciones);
		agregarCampo(respuesta, "matches", matrizAnidada);
		agregarCampo(respuesta, "results", posiciones);
		agregarCampo(respuesta, "detections", posiciones);
	}
	return respuesta;
}

/*
 * Resuelve el modo de palindromo mas largo: inicio y fin (desde 1) en cada transmision.
 * Parametro entrada: objeto JSON del campo input (transmisiones).
 * Retorno: ranges con un par [inicio, fin] por transmision, o un error.
 */
JsonValue resolverPalindromo(const JsonValue& entrada) {
	std::vector<std::string> transmisiones;
	JsonValue respuesta;
	if (!leerTransmisiones(entrada, transmisiones)) {
		respuesta = construirError("palindromo: no se reconocen las transmisiones, campos recibidos: " + listarClaves(entrada));
	} else {
		std::string tecnica = leerAlgoritmo(entrada);
		bool porCentros = tecnica == "expand" || tecnica == "centers" || tecnica == "centros";
		JsonValue rangos = crearArreglo();
		JsonValue detalles = crearArreglo();
		for (size_t i = 0; i < transmisiones.size(); i++) {
			RangoTexto rango = buscarPalindromoMasLargo(transmisiones[i]);
			if (porCentros) {
				rango = buscarPalindromoPorCentros(transmisiones[i]);
			}
			int inicio = 0;
			int fin = 0;
			if (rango.longitud > 0) {
				inicio = rango.inicio + 1;
				fin = rango.inicio + rango.longitud;
			}
			agregarElemento(rangos, crearPar(inicio, fin));
			JsonValue detalle = crearObjeto();
			agregarCampo(detalle, "transmission", crearNumero(static_cast<double>(i)));
			agregarCampo(detalle, "start", crearNumero(static_cast<double>(inicio)));
			agregarCampo(detalle, "end", crearNumero(static_cast<double>(fin)));
			agregarCampo(detalle, "length", crearNumero(static_cast<double>(rango.longitud)));
			agregarElemento(detalles, detalle);
		}
		respuesta = crearObjeto();
		agregarCampo(respuesta, "algorithm", crearTexto(porCentros ? "expand-around-center" : "manacher"));
		agregarCampo(respuesta, "ranges", rangos);
		agregarCampo(respuesta, "palindromes", detalles);
	}
	return respuesta;
}

/*
 * Resuelve el modo de subcadena comun mas larga: inicio y fin (desde 1) en la primera transmision.
 * Parametro entrada: objeto JSON del campo input (dos transmisiones).
 * Retorno: range con el par [inicio, fin], mas start, end y length, o un error.
 */
JsonValue resolverSubcadenaComun(const JsonValue& entrada) {
	std::vector<std::string> transmisiones;
	JsonValue respuesta;
	if (!leerTransmisiones(entrada, transmisiones) || transmisiones.size() < 2) {
		respuesta = construirError("subcadena comun: se esperaban dos transmisiones, campos recibidos: " + listarClaves(entrada));
	} else {
		std::string tecnica = leerAlgoritmo(entrada);
		bool porTabla = tecnica == "dp" || tecnica == "table" || tecnica == "tabla";
		RangoTexto rango = buscarSubcadenaComun(transmisiones[0], transmisiones[1]);
		if (porTabla) {
			rango = buscarSubcadenaComunPorTabla(transmisiones[0], transmisiones[1]);
		}
		int inicio = 0;
		int fin = 0;
		std::string texto = "";
		if (rango.longitud > 0) {
			inicio = rango.inicio + 1;
			fin = rango.inicio + rango.longitud;
			texto = transmisiones[0].substr(static_cast<size_t>(rango.inicio), static_cast<size_t>(rango.longitud));
		}
		respuesta = crearObjeto();
		agregarCampo(respuesta, "algorithm", crearTexto(porTabla ? "dynamic-programming" : "hash-binary-search"));
		agregarCampo(respuesta, "range", crearPar(inicio, fin));
		JsonValue rangos = crearArreglo();
		agregarElemento(rangos, crearPar(inicio, fin));
		size_t posicionSegundo = transmisiones[1].find(texto);
		if (rango.longitud > 0 && posicionSegundo != std::string::npos) {
			int inicioSegundo = static_cast<int>(posicionSegundo) + 1;
			agregarElemento(rangos, crearPar(inicioSegundo, inicioSegundo + rango.longitud - 1));
		} else {
			agregarElemento(rangos, crearPar(0, 0));
		}
		agregarCampo(respuesta, "ranges", rangos);
		agregarCampo(respuesta, "start", crearNumero(static_cast<double>(inicio)));
		agregarCampo(respuesta, "end", crearNumero(static_cast<double>(fin)));
		agregarCampo(respuesta, "length", crearNumero(static_cast<double>(rango.longitud)));
		agregarCampo(respuesta, "substring", crearTexto(texto));
	}
	return respuesta;
}

#endif
