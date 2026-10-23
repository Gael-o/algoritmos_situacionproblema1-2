/*
 * Descripcion: propagacion de un mensaje por la red. Calcula los caminos minimos desde un nodo de
 * origen con Dijkstra (cola de prioridad, la idea de OSPF) o con Bellman-Ford (relajacion repetida,
 * la idea de los protocolos de vector distancia como RIP), sin pasar por los enlaces ni los nodos
 * marcados como caidos, y el tiempo en que el mensaje llega a cada nodo segun el retardo de cada
 * enlace mas un retardo fijo de procesamiento por salto. Incluye el adaptador JSON del modo
 * "propagation". Reutiliza de flujo.h la lectura del nodo de origen y del destino.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), Damian Alejandro Gomez Mena (A01385923), David Rangel Monsivais (A01385671)
 * Fecha: 22/10/2026
 */

#ifndef PROPAGACION_H
#define PROPAGACION_H

#include <algorithm>
#include <functional>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "flujo.h"
#include "grafo.h"
#include "json.h"
#include "lectura.h"

const double DISTANCIA_INFINITA = 1.0e18;
const double TOLERANCIA_DISTANCIA = 1.0e-9;
const std::vector<std::string> CLAVES_RETARDOS = {"delays", "latencies", "retardos"};
const std::vector<std::string> CLAVES_RETARDO_SALTO = {"hopDelay", "processing", "retardoSalto"};
const std::vector<std::string> CLAVES_ENLACES_CAIDOS = {"failedLinks", "failedEdges", "enlacesCaidos"};
const std::vector<std::string> CLAVES_NODOS_CAIDOS = {"failedNodes", "nodosCaidos"};
const std::vector<std::string> CLAVES_METRICA = {"metric", "metrica"};

struct ArcoRuta {
	int destino = 0;
	int arista = 0;
	double peso = 0.0;
};

struct ResultadoPropagacion {
	std::vector<double> distancia;
	std::vector<int> saltos;
	std::vector<int> padre;
	std::vector<int> aristaPadre;
	std::vector<double> tiempo;
};

struct MenorTiempo {
	const std::vector<double>* tiempos = nullptr;

	/*
	 * Comparador para ordenar nodos por su tiempo de llegada; ante empate, por indice.
	 * Parametro primero: indice de un nodo.
	 * Parametro segundo: indice de otro nodo.
	 * Retorno: true si el primero llega antes que el segundo.
	 */
	bool operator()(int primero, int segundo) const {
		double tiempoPrimero = (*tiempos)[static_cast<size_t>(primero)];
		double tiempoSegundo = (*tiempos)[static_cast<size_t>(segundo)];
		bool menor = primero < segundo;
		if (tiempoPrimero != tiempoSegundo) {
			menor = tiempoPrimero < tiempoSegundo;
		}
		return menor;
	}
};

/*
 * Indica si un nodo ya tiene una distancia conocida desde el origen.
 * Parametro resultado: estado actual de la busqueda.
 * Parametro nodo: indice del nodo.
 * Retorno: true si el nodo es alcanzable con lo calculado hasta ahora.
 */
bool esAlcanzado(const ResultadoPropagacion& resultado, int nodo) {
	return resultado.distancia[static_cast<size_t>(nodo)] < DISTANCIA_INFINITA / 2.0;
}

/*
 * Crea el estado inicial: solo el origen es alcanzable, con distancia 0 y sin saltos.
 * Parametro cantidad: numero de nodos.
 * Parametro origen: indice del nodo de origen.
 * Retorno: resultado inicial de la busqueda.
 */
ResultadoPropagacion iniciarResultado(size_t cantidad, int origen) {
	ResultadoPropagacion resultado;
	resultado.distancia.assign(cantidad, DISTANCIA_INFINITA);
	resultado.saltos.assign(cantidad, -1);
	resultado.padre.assign(cantidad, -1);
	resultado.aristaPadre.assign(cantidad, -1);
	resultado.tiempo.assign(cantidad, -1.0);
	resultado.distancia[static_cast<size_t>(origen)] = 0.0;
	resultado.saltos[static_cast<size_t>(origen)] = 0;
	return resultado;
}

/*
 * Decide si un camino candidato hacia un nodo mejora el que ya se tenia. Primero cuenta la
 * distancia; ante distancias iguales gana el de menos saltos, luego el padre de indice menor y al
 * final la arista de indice menor. Ese desempate hace que Dijkstra y Bellman-Ford elijan
 * exactamente el mismo arbol, y que la ruta no cambie de una ejecucion a otra.
 * Parametro distancia: distancia del candidato.
 * Parametro saltos: numero de enlaces del candidato.
 * Parametro padre: nodo desde el que llegaria.
 * Parametro arista: arista de entrada por la que llegaria.
 * Parametro resultado: estado actual de la busqueda.
 * Parametro nodo: nodo al que se quiere llegar.
 * Retorno: true si el candidato es mejor que lo que ya se tenia.
 */
bool esMejorCamino(double distancia, int saltos, int padre, int arista, const ResultadoPropagacion& resultado, int nodo) {
	size_t posicion = static_cast<size_t>(nodo);
	bool mejor = false;
	if (distancia < resultado.distancia[posicion] - TOLERANCIA_DISTANCIA) {
		mejor = true;
	} else if (distancia <= resultado.distancia[posicion] + TOLERANCIA_DISTANCIA) {
		if (saltos != resultado.saltos[posicion]) {
			mejor = saltos < resultado.saltos[posicion];
		} else if (padre != resultado.padre[posicion]) {
			mejor = padre < resultado.padre[posicion];
		} else {
			mejor = arista < resultado.aristaPadre[posicion];
		}
	}
	return mejor;
}

/*
 * Guarda en el resultado el camino candidato hacia un nodo.
 * Parametro resultado: estado de la busqueda, se modifica.
 * Parametro nodo: nodo al que se llega.
 * Parametro distancia: nueva distancia.
 * Parametro saltos: nuevo numero de enlaces.
 * Parametro padre: nodo desde el que se llega.
 * Parametro arista: arista de entrada usada.
 */
void fijarCamino(ResultadoPropagacion& resultado, int nodo, double distancia, int saltos, int padre, int arista) {
	size_t posicion = static_cast<size_t>(nodo);
	resultado.distancia[posicion] = distancia;
	resultado.saltos[posicion] = saltos;
	resultado.padre[posicion] = padre;
	resultado.aristaPadre[posicion] = arista;
}

/*
 * Arma la lista de adyacencia dejando fuera los enlaces caidos y los que tocan un nodo caido.
 * Parametro grafo: grafo leido de la entrada.
 * Parametro dirigido: si es false, cada enlace se puede recorrer en los dos sentidos.
 * Parametro enlaceCaido: marca por arista de entrada.
 * Parametro nodoCaido: marca por nodo.
 * Retorno: arcos salientes de cada nodo.
 */
std::vector<std::vector<ArcoRuta>> construirAdyacenciaRuta(const Grafo& grafo, bool dirigido, const std::vector<bool>& enlaceCaido, const std::vector<bool>& nodoCaido) {
	std::vector<std::vector<ArcoRuta>> adyacencia(grafo.identificadores.size());
	for (size_t i = 0; i < grafo.aristas.size(); i++) {
		const Arista& actual = grafo.aristas[i];
		bool disponible = !enlaceCaido[i] && !nodoCaido[static_cast<size_t>(actual.origen)] && !nodoCaido[static_cast<size_t>(actual.destino)];
		if (disponible) {
			ArcoRuta ida;
			ida.destino = actual.destino;
			ida.arista = static_cast<int>(i);
			ida.peso = actual.costo;
			adyacencia[static_cast<size_t>(actual.origen)].push_back(ida);
			if (!dirigido) {
				ArcoRuta vuelta;
				vuelta.destino = actual.origen;
				vuelta.arista = static_cast<int>(i);
				vuelta.peso = actual.costo;
				adyacencia[static_cast<size_t>(actual.destino)].push_back(vuelta);
			}
		}
	}
	return adyacencia;
}

/*
 * Caminos minimos desde el origen con Dijkstra y una cola de prioridad: siempre se cierra el nodo
 * pendiente mas cercano, y su distancia ya no cambia. Es lo que hace cada router con OSPF.
 * Solo sirve con pesos no negativos (se valida antes de llamarla).
 * Parametro adyacencia: arcos salientes de cada nodo.
 * Parametro origen: nodo desde el que sale el mensaje.
 * Retorno: distancia, saltos, padre y arista de llegada de cada nodo.
 */
ResultadoPropagacion calcularDijkstra(const std::vector<std::vector<ArcoRuta>>& adyacencia, int origen) {
	ResultadoPropagacion resultado = iniciarResultado(adyacencia.size(), origen);
	std::vector<bool> cerrado(adyacencia.size(), false);
	std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, std::greater<std::pair<double, int>>> pendientes;
	pendientes.push(std::make_pair(0.0, origen));
	while (!pendientes.empty()) {
		int actual = pendientes.top().second;
		pendientes.pop();
		size_t posicion = static_cast<size_t>(actual);
		if (!cerrado[posicion]) {
			cerrado[posicion] = true;
			for (size_t i = 0; i < adyacencia[posicion].size(); i++) {
				const ArcoRuta& arco = adyacencia[posicion][i];
				double candidata = resultado.distancia[posicion] + arco.peso;
				int saltos = resultado.saltos[posicion] + 1;
				bool abierto = !cerrado[static_cast<size_t>(arco.destino)];
				if (abierto && esMejorCamino(candidata, saltos, actual, arco.arista, resultado, arco.destino)) {
					fijarCamino(resultado, arco.destino, candidata, saltos, actual, arco.arista);
					pendientes.push(std::make_pair(candidata, arco.destino));
				}
			}
		}
	}
	return resultado;
}

/*
 * Caminos minimos desde el origen con Bellman-Ford: recorre todos los arcos una y otra vez
 * mejorando distancias hasta que una ronda completa ya no cambia nada. Es la idea de los
 * protocolos de vector distancia (RIP). Acepta pesos negativos y detecta ciclos negativos.
 * Parametro adyacencia: arcos salientes de cada nodo.
 * Parametro origen: nodo desde el que sale el mensaje.
 * Parametro hayCicloNegativo: recibe true si hay un ciclo de peso negativo alcanzable.
 * Retorno: distancia, saltos, padre y arista de llegada de cada nodo.
 */
ResultadoPropagacion calcularBellmanFord(const std::vector<std::vector<ArcoRuta>>& adyacencia, int origen, bool& hayCicloNegativo) {
	ResultadoPropagacion resultado = iniciarResultado(adyacencia.size(), origen);
	size_t limiteRondas = adyacencia.size() + 1;
	size_t ronda = 0;
	bool huboCambio = true;
	while (huboCambio && ronda < limiteRondas) {
		huboCambio = false;
		for (size_t nodo = 0; nodo < adyacencia.size(); nodo++) {
			if (esAlcanzado(resultado, static_cast<int>(nodo))) {
				for (size_t i = 0; i < adyacencia[nodo].size(); i++) {
					const ArcoRuta& arco = adyacencia[nodo][i];
					double candidata = resultado.distancia[nodo] + arco.peso;
					int saltos = resultado.saltos[nodo] + 1;
					if (esMejorCamino(candidata, saltos, static_cast<int>(nodo), arco.arista, resultado, arco.destino)) {
						fijarCamino(resultado, arco.destino, candidata, saltos, static_cast<int>(nodo), arco.arista);
						huboCambio = true;
					}
				}
			}
		}
		ronda++;
	}
	hayCicloNegativo = huboCambio;
	return resultado;
}

/*
 * Calcula cuando llega el mensaje a cada nodo recorriendo el arbol de caminos desde el origen:
 * cada salto suma el retardo del enlace usado y el retardo fijo de procesamiento del router.
 * Se calcula al final, sobre el arbol ya elegido, para que las dos tecnicas den el mismo tiempo.
 * Parametro resultado: arbol de caminos ya calculado; se llena el campo tiempo.
 * Parametro retardos: retardo de cada arista de entrada.
 * Parametro retardoSalto: retardo de procesamiento por salto.
 * Parametro origen: nodo de origen.
 */
void calcularTiempos(ResultadoPropagacion& resultado, const std::vector<double>& retardos, double retardoSalto, int origen) {
	size_t cantidad = resultado.padre.size();
	std::vector<std::vector<int>> hijos(cantidad);
	for (size_t i = 0; i < cantidad; i++) {
		if (resultado.padre[i] >= 0) {
			hijos[static_cast<size_t>(resultado.padre[i])].push_back(static_cast<int>(i));
		}
	}
	resultado.tiempo.assign(cantidad, -1.0);
	resultado.tiempo[static_cast<size_t>(origen)] = 0.0;
	std::queue<int> pendientes;
	pendientes.push(origen);
	while (!pendientes.empty()) {
		size_t actual = static_cast<size_t>(pendientes.front());
		pendientes.pop();
		for (size_t i = 0; i < hijos[actual].size(); i++) {
			size_t hijo = static_cast<size_t>(hijos[actual][i]);
			size_t arista = static_cast<size_t>(resultado.aristaPadre[hijo]);
			resultado.tiempo[hijo] = resultado.tiempo[actual] + retardos[arista] + retardoSalto;
			pendientes.push(static_cast<int>(hijo));
		}
	}
}

/*
 * Lee los enlaces caidos. Cada elemento puede ser el indice de la arista en la lista de entrada,
 * o el par [nodo, nodo] de sus extremos (en cualquier orden).
 * Parametro entrada: objeto JSON del campo input.
 * Parametro grafo: grafo ya leido.
 * Parametro enlaceCaido: recibe la marca de cada arista.
 */
void leerEnlacesCaidos(const JsonValue& entrada, const Grafo& grafo, std::vector<bool>& enlaceCaido) {
	JsonValue lista;
	if (buscarAlguno(entrada, CLAVES_ENLACES_CAIDOS, lista) && lista.tipo == JsonTipo::ARREGLO) {
		for (size_t i = 0; i < lista.arreglo.size(); i++) {
			const JsonValue& elemento = lista.arreglo[i];
			if (elemento.tipo == JsonTipo::NUMERO) {
				int indice = static_cast<int>(elemento.numero);
				if (indice >= 0 && static_cast<size_t>(indice) < grafo.aristas.size()) {
					enlaceCaido[static_cast<size_t>(indice)] = true;
				}
			} else if (elemento.tipo == JsonTipo::ARREGLO && elemento.arreglo.size() >= 2) {
				int extremoUno = -1;
				int extremoDos = -1;
				bool existen = buscarIndiceNodo(grafo, elemento.arreglo[0], extremoUno) && buscarIndiceNodo(grafo, elemento.arreglo[1], extremoDos);
				for (size_t j = 0; j < grafo.aristas.size() && existen; j++) {
					bool mismoSentido = grafo.aristas[j].origen == extremoUno && grafo.aristas[j].destino == extremoDos;
					bool sentidoContrario = grafo.aristas[j].origen == extremoDos && grafo.aristas[j].destino == extremoUno;
					if (mismoSentido || sentidoContrario) {
						enlaceCaido[j] = true;
					}
				}
			}
		}
	}
}

/*
 * Lee los nodos caidos: identificadores (numero o texto) de nodos que no reciben ni reenvian.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro grafo: grafo ya leido.
 * Parametro nodoCaido: recibe la marca de cada nodo.
 */
void leerNodosCaidos(const JsonValue& entrada, const Grafo& grafo, std::vector<bool>& nodoCaido) {
	JsonValue lista;
	if (buscarAlguno(entrada, CLAVES_NODOS_CAIDOS, lista) && lista.tipo == JsonTipo::ARREGLO) {
		for (size_t i = 0; i < lista.arreglo.size(); i++) {
			int indice = -1;
			if (buscarIndiceNodo(grafo, lista.arreglo[i], indice)) {
				nodoCaido[static_cast<size_t>(indice)] = true;
			}
		}
	}
}

/*
 * Lee el retardo de cada arista (campo delays, en el mismo orden que las aristas). Si no viene,
 * el retardo de cada arista es su peso, y entonces el tiempo coincide con la distancia.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro grafo: grafo ya leido.
 * Parametro retardos: recibe el retardo de cada arista.
 * Parametro mensaje: recibe la descripcion del problema si los retardos no son validos.
 * Retorno: true si hay un retardo no negativo por arista.
 */
bool leerRetardos(const JsonValue& entrada, const Grafo& grafo, std::vector<double>& retardos, std::string& mensaje) {
	bool correcto = true;
	retardos.assign(grafo.aristas.size(), 0.0);
	for (size_t i = 0; i < grafo.aristas.size(); i++) {
		retardos[i] = grafo.aristas[i].costo;
	}
	JsonValue lista;
	if (buscarAlguno(entrada, CLAVES_RETARDOS, lista)) {
		correcto = lista.tipo == JsonTipo::ARREGLO && lista.arreglo.size() == grafo.aristas.size();
		for (size_t i = 0; i < grafo.aristas.size() && correcto; i++) {
			double valor = 0.0;
			correcto = extraerNumero(lista.arreglo[i], valor) && valor >= 0.0;
			retardos[i] = valor;
		}
		if (!correcto) {
			mensaje = "propagacion: delays debe traer un retardo no negativo por cada arista, en el mismo orden";
		}
	}
	return correcto;
}

/*
 * Decide con que peso se comparan los caminos. "weight" (por omision) usa el peso de cada arista
 * tal como viene; "hops" cuenta saltos (todas valen 1, como RIP); "time" usa el retardo del
 * enlace mas el retardo de procesamiento, de modo que el camino elegido es el que llega antes.
 * Parametro entrada: objeto JSON del campo input.
 * Parametro grafo: grafo ya leido; el costo de cada arista se reemplaza segun la metrica.
 * Parametro retardos: retardo de cada arista.
 * Parametro retardoSalto: retardo de procesamiento por salto.
 * Retorno: nombre de la metrica aplicada.
 */
std::string aplicarMetrica(const JsonValue& entrada, Grafo& grafo, const std::vector<double>& retardos, double retardoSalto) {
	std::string metrica = "weight";
	JsonValue campo;
	if (buscarAlguno(entrada, CLAVES_METRICA, campo) && campo.tipo == JsonTipo::TEXTO) {
		std::string pedida = aMinusculas(campo.texto);
		if (pedida == "hops" || pedida == "saltos") {
			metrica = "hops";
		} else if (pedida == "time" || pedida == "tiempo" || pedida == "latency") {
			metrica = "time";
		}
	}
	for (size_t i = 0; i < grafo.aristas.size(); i++) {
		if (metrica == "hops") {
			grafo.aristas[i].costo = 1.0;
		} else if (metrica == "time") {
			grafo.aristas[i].costo = retardos[i] + retardoSalto;
		}
	}
	return metrica;
}

/*
 * Convierte el resultado a la respuesta JSON del modo "propagation".
 * Parametro grafo: grafo con los identificadores originales.
 * Parametro resultado: arbol de caminos con tiempos.
 * Parametro nodoCaido: marca de cada nodo caido.
 * Parametro origen: indice del nodo de origen.
 * Parametro destino: indice del destino (solo si hayDestino).
 * Parametro hayDestino: true si se pidio la ruta a un destino.
 * Parametro algoritmo: nombre de la tecnica usada.
 * Parametro metrica: nombre de la metrica con la que se compararon los caminos.
 * Retorno: objeto con nodes (estado de cada nodo), events (llegadas en orden de tiempo),
 * unreachable, reached, maxTime y, si hay destino, path, pathEdges, pathTime y pathDistance.
 */
JsonValue construirRespuestaPropagacion(const Grafo& grafo, const ResultadoPropagacion& resultado, const std::vector<bool>& nodoCaido, int origen, int destino, bool hayDestino, const std::string& algoritmo, const std::string& metrica) {
	JsonValue nulo;
	JsonValue nodos = crearArreglo();
	JsonValue inalcanzables = crearArreglo();
	std::vector<int> alcanzados;
	double tiempoMaximo = 0.0;
	for (size_t i = 0; i < grafo.identificadores.size(); i++) {
		bool alcanzado = esAlcanzado(resultado, static_cast<int>(i));
		JsonValue nodo = crearObjeto();
		agregarCampo(nodo, "id", grafo.identificadores[i]);
		agregarCampo(nodo, "reachable", crearBooleano(alcanzado));
		agregarCampo(nodo, "failed", crearBooleano(nodoCaido[i]));
		agregarCampo(nodo, "distance", crearNumero(alcanzado ? resultado.distancia[i] : -1.0));
		agregarCampo(nodo, "time", crearNumero(alcanzado ? resultado.tiempo[i] : -1.0));
		agregarCampo(nodo, "hops", crearNumero(static_cast<double>(alcanzado ? resultado.saltos[i] : -1)));
		agregarCampo(nodo, "parent", resultado.padre[i] >= 0 ? grafo.identificadores[static_cast<size_t>(resultado.padre[i])] : nulo);
		agregarCampo(nodo, "via", crearNumero(static_cast<double>(resultado.aristaPadre[i])));
		agregarElemento(nodos, nodo);
		if (alcanzado) {
			alcanzados.push_back(static_cast<int>(i));
			tiempoMaximo = std::max(tiempoMaximo, resultado.tiempo[i]);
		} else if (!nodoCaido[i]) {
			agregarElemento(inalcanzables, grafo.identificadores[i]);
		}
	}
	MenorTiempo comparador;
	comparador.tiempos = &resultado.tiempo;
	std::sort(alcanzados.begin(), alcanzados.end(), comparador);
	JsonValue eventos = crearArreglo();
	for (size_t i = 0; i < alcanzados.size(); i++) {
		size_t nodo = static_cast<size_t>(alcanzados[i]);
		JsonValue evento = crearObjeto();
		agregarCampo(evento, "time", crearNumero(resultado.tiempo[nodo]));
		agregarCampo(evento, "node", grafo.identificadores[nodo]);
		agregarCampo(evento, "from", resultado.padre[nodo] >= 0 ? grafo.identificadores[static_cast<size_t>(resultado.padre[nodo])] : nulo);
		agregarCampo(evento, "edge", crearNumero(static_cast<double>(resultado.aristaPadre[nodo])));
		agregarCampo(evento, "hops", crearNumero(static_cast<double>(resultado.saltos[nodo])));
		agregarCampo(evento, "distance", crearNumero(resultado.distancia[nodo]));
		agregarElemento(eventos, evento);
	}
	JsonValue respuesta = crearObjeto();
	agregarCampo(respuesta, "algorithm", crearTexto(algoritmo));
	agregarCampo(respuesta, "metric", crearTexto(metrica));
	agregarCampo(respuesta, "source", grafo.identificadores[static_cast<size_t>(origen)]);
	agregarCampo(respuesta, "reached", crearNumero(static_cast<double>(alcanzados.size())));
	agregarCampo(respuesta, "unreachable", inalcanzables);
	agregarCampo(respuesta, "maxTime", crearNumero(tiempoMaximo));
	agregarCampo(respuesta, "events", eventos);
	agregarCampo(respuesta, "nodes", nodos);
	if (hayDestino) {
		bool llega = esAlcanzado(resultado, destino);
		std::vector<int> camino;
		int actual = llega ? destino : -1;
		while (actual >= 0) {
			camino.push_back(actual);
			actual = resultado.padre[static_cast<size_t>(actual)];
		}
		std::reverse(camino.begin(), camino.end());
		JsonValue ruta = crearArreglo();
		JsonValue aristasRuta = crearArreglo();
		for (size_t i = 0; i < camino.size(); i++) {
			agregarElemento(ruta, grafo.identificadores[static_cast<size_t>(camino[i])]);
			if (i > 0) {
				agregarElemento(aristasRuta, crearNumero(static_cast<double>(resultado.aristaPadre[static_cast<size_t>(camino[i])])));
			}
		}
		agregarCampo(respuesta, "target", grafo.identificadores[static_cast<size_t>(destino)]);
		agregarCampo(respuesta, "targetReachable", crearBooleano(llega));
		agregarCampo(respuesta, "path", ruta);
		agregarCampo(respuesta, "pathEdges", aristasRuta);
		agregarCampo(respuesta, "pathTime", crearNumero(llega ? resultado.tiempo[static_cast<size_t>(destino)] : -1.0));
		agregarCampo(respuesta, "pathDistance", crearNumero(llega ? resultado.distancia[static_cast<size_t>(destino)] : -1.0));
	}
	return respuesta;
}

/*
 * Resuelve el modo "propagation": lee la red, los enlaces y nodos caidos y los retardos, calcula
 * los caminos minimos desde el origen con la tecnica pedida y el tiempo de llegada a cada nodo.
 * Parametro entrada: objeto JSON del campo input. Campos: edges, source, y opcionales n o nodes,
 * target, delays, hopDelay, metric ("weight", "hops" o "time"), failedLinks, failedNodes,
 * directed (por omision false) y algorithm ("dijkstra" por omision, o "bellmanford").
 * Retorno: respuesta JSON del modo, o un error si la entrada no se reconoce.
 */
JsonValue resolverPropagacion(const JsonValue& entrada) {
	Grafo grafo;
	std::string mensaje = "";
	JsonValue respuesta;
	std::vector<double> retardos;
	bool correcto = leerGrafo(entrada, grafo, mensaje) && leerRetardos(entrada, grafo, retardos, mensaje);
	if (correcto && grafo.identificadores.empty()) {
		correcto = false;
		mensaje = "propagacion: la red no tiene nodos";
	}
	int origen = 0;
	int destino = -1;
	bool hayDestino = false;
	JsonValue campo;
	double retardoSalto = 0.0;
	if (buscarAlguno(entrada, CLAVES_RETARDO_SALTO, campo)) {
		extraerNumero(campo, retardoSalto);
	}
	retardoSalto = std::max(0.0, retardoSalto);
	std::string metrica = "weight";
	if (correcto) {
		metrica = aplicarMetrica(entrada, grafo, retardos, retardoSalto);
	}
	if (correcto && buscarAlguno(entrada, CLAVES_FUENTE, campo) && !buscarIndiceNodo(grafo, campo, origen)) {
		correcto = false;
		mensaje = "propagacion: el nodo de origen no existe en la red";
	}
	if (correcto && buscarAlguno(entrada, CLAVES_SUMIDERO, campo) && campo.tipo != JsonTipo::NULO) {
		hayDestino = buscarIndiceNodo(grafo, campo, destino);
		if (!hayDestino) {
			correcto = false;
			mensaje = "propagacion: el nodo de destino no existe en la red";
		}
	}
	std::string tecnica = leerAlgoritmo(entrada);
	bool usarBellmanFord = tecnica == "bellmanford" || tecnica == "bellman" || tecnica == "bf";
	for (size_t i = 0; i < grafo.aristas.size() && correcto && !usarBellmanFord; i++) {
		if (grafo.aristas[i].costo < 0.0) {
			correcto = false;
			mensaje = "propagacion: Dijkstra no admite pesos negativos (usa algorithm: bellmanford)";
		}
	}
	if (!correcto) {
		respuesta = construirError(mensaje);
	} else {
		std::vector<bool> enlaceCaido(grafo.aristas.size(), false);
		std::vector<bool> nodoCaido(grafo.identificadores.size(), false);
		leerEnlacesCaidos(entrada, grafo, enlaceCaido);
		leerNodosCaidos(entrada, grafo, nodoCaido);
		bool dirigido = false;
		if (buscarAlguno(entrada, CLAVES_DIRIGIDO, campo) && campo.tipo == JsonTipo::BOOLEANO) {
			dirigido = campo.booleano;
		}
		if (nodoCaido[static_cast<size_t>(origen)]) {
			respuesta = construirError("propagacion: el nodo de origen esta caido, el mensaje no puede salir");
		} else {
			std::vector<std::vector<ArcoRuta>> adyacencia = construirAdyacenciaRuta(grafo, dirigido, enlaceCaido, nodoCaido);
			bool hayCicloNegativo = false;
			ResultadoPropagacion resultado;
			std::string nombre = "dijkstra";
			if (usarBellmanFord) {
				nombre = "bellman-ford";
				resultado = calcularBellmanFord(adyacencia, origen, hayCicloNegativo);
			} else {
				resultado = calcularDijkstra(adyacencia, origen);
			}
			if (hayCicloNegativo) {
				respuesta = construirError("propagacion: hay un ciclo de peso negativo, no existe camino minimo");
			} else {
				calcularTiempos(resultado, retardos, retardoSalto, origen);
				respuesta = construirRespuestaPropagacion(grafo, resultado, nodoCaido, origen, destino, hayDestino, nombre, metrica);
			}
		}
	}
	return respuesta;
}

#endif
