#!/usr/bin/env python3
"""
Descripcion: lee la red GEANT de SNDlib (nodos, enlaces y matriz de demandas), construye la entrada JSON
de cada uno de los siete modos del ejecutable en C++, lo ejecuta y devuelve los resultados.
Autores: Gael Alexander Zaragoza Torres (A01385956), Damian Alejandro Gomez Mena (A01385923), David Rangel Monsivais (A01385671)
Fecha: 21/10/2026

Uso:
    python3 app/geant.py ./sentinel                 # usa app/data/demands.txt si existe
    python3 app/geant.py ./sentinel ruta/demanda.txt

Archivos de datos (carpeta app/data):
    nodes.txt    lineas "nombre ( lon lat )"              (sacadas de geant.txt de SNDlib, seccion NODES)
    links.txt    lineas "origen destino capacidad costo"  (seccion LINKS: modulo de capacidad y su costo)
    demands.txt  seccion DEMANDS en formato nativo de SNDlib: "id ( origen destino ) 1 valor UNLIMITED"
                 (puede ser geant.txt completo o una matriz del Dynamic traffic: se leen las lineas de demanda)
"""

import json
import math
import random
import os
import re
import subprocess
import sys

RAIZ = os.path.dirname(os.path.abspath(__file__))
DATOS = os.path.join(RAIZ, "data")
RESULTADOS = os.path.join(os.path.dirname(RAIZ), "results")
SEMILLA_FIRMAS = ["DEADBEEF", "FF00FF", "C0FFEE"]
RADIO_TIERRA_KM = 6371.0
VELOCIDAD_FIBRA_KM_POR_MS = 200.0    # la luz en fibra optica viaja a ~2/3 de c: unos 200 000 km/s
RETARDO_PROCESAMIENTO_MS = 1.0       # retardo tipico de un router por salto, valor inicial de la simulacion


def distanciaKm(coordUno, coordDos):
    """Distancia en km sobre la superficie de la Tierra (formula de haversine) entre dos puntos [lon, lat].
    Es la distancia en linea recta: la fibra real suele ser mas larga, asi que es una cota inferior."""
    lonUno, latUno = math.radians(coordUno[0]), math.radians(coordUno[1])
    lonDos, latDos = math.radians(coordDos[0]), math.radians(coordDos[1])
    a = math.sin((latDos - latUno) / 2) ** 2 + math.cos(latUno) * math.cos(latDos) * math.sin((lonDos - lonUno) / 2) ** 2
    return 2 * RADIO_TIERRA_KM * math.asin(math.sqrt(a))


def retardoMs(coordUno, coordDos):
    """Retardo de propagacion de un enlace de fibra en milisegundos: su distancia entre la velocidad de la luz en la fibra."""
    return distanciaKm(coordUno, coordDos) / VELOCIDAD_FIBRA_KM_POR_MS


def retardosEnlaces(red):
    """Retardo en ms de cada enlace de la red, en el mismo orden que red["enlaces"]."""
    return [round(retardoMs(red["coords"][u], red["coords"][v]), 3) for (u, v, capacidad, costo) in red["enlaces"]]


def leerNodos(ruta):
    """Devuelve (nombres, coordenadas) con coordenadas [lon, lat] en el orden del archivo."""
    nombres = []
    coordenadas = []
    for linea in open(ruta):
        partes = re.findall(r"[^\s()]+", linea)
        if len(partes) == 3:
            nombres.append(partes[0])
            coordenadas.append([float(partes[1]), float(partes[2])])
    return nombres, coordenadas


def leerEnlaces(ruta, indice):
    """Devuelve lista de (u, v, capacidad, costo) con los nodos como indices."""
    enlaces = []
    for linea in open(ruta):
        partes = linea.split()
        if len(partes) == 4:
            enlaces.append((indice[partes[0]], indice[partes[1]], float(partes[2]), float(partes[3])))
    return enlaces


def leerDemandas(ruta, indice):
    """Lee lineas 'id ( origen destino ) 1 valor ...' de cualquier archivo nativo de SNDlib."""
    demandas = []
    if ruta is not None and os.path.exists(ruta):
        patron = re.compile(r"\(\s*(\S+)\s+(\S+)\s*\)\s+\d+\s+([0-9.eE+-]+)")
        for linea in open(ruta):
            if "NODES" in linea or "LINKS" in linea:
                continue
            encontrado = patron.search(linea)
            if encontrado and encontrado.group(1) in indice and encontrado.group(2) in indice:
                demandas.append((indice[encontrado.group(1)], indice[encontrado.group(2)], float(encontrado.group(3))))
    return demandas


def ejecutar(ejecutable, modo, entrada):
    """Ejecuta el programa en C++ con un JSON y devuelve la respuesta ya decodificada."""
    proceso = subprocess.run([ejecutable], input=json.dumps({"mode": modo, "input": entrada}),
                             capture_output=True, text=True, timeout=120)
    return json.loads(proceso.stdout)


def cargarRed(rutaDemandas=None, rutaNodos=None, rutaEnlaces=None):
    """Carga nodos, enlaces y demandas de GEANT y devuelve un diccionario con todo."""
    nombres, coordenadas = leerNodos(rutaNodos or os.path.join(DATOS, "nodes.txt"))
    indice = {nombre: i for i, nombre in enumerate(nombres)}
    enlaces = leerEnlaces(rutaEnlaces or os.path.join(DATOS, "links.txt"), indice)
    if rutaDemandas is None:
        rutaDemandas = os.path.join(DATOS, "demands.txt")
    demandas = leerDemandas(rutaDemandas, indice)
    return {"nombres": nombres, "coords": coordenadas, "indice": indice, "enlaces": enlaces, "demandas": demandas}


def listarEscenarios():
    """Devuelve la lista de casos de prueba disponibles en app/data/scenarios, ordenados por nombre de archivo."""
    carpeta = os.path.join(DATOS, "scenarios")
    escenarios = []
    if os.path.isdir(carpeta):
        for nombre in sorted(os.listdir(carpeta)):
            if nombre.endswith(".json"):
                with open(os.path.join(carpeta, nombre), encoding="utf-8") as archivo:
                    definicion = json.load(archivo)
                escenarios.append({"id": nombre[:-5], "nombre": definicion.get("nombre", nombre[:-5]),
                                   "descripcion": definicion.get("descripcion", ""),
                                   "fuente": definicion.get("fuente", "")})
    return escenarios


def generarRed(parametros):
    """Crea una red sintetica conexa con el numero de nodos y enlaces pedido, para probar escalabilidad."""
    aleatorio = random.Random(parametros.get("semilla", 2026))
    total = parametros.get("nodos", 100)
    nombres = ["n%03d" % i for i in range(total)]
    coordenadas = [[aleatorio.uniform(-10.0, 35.0), aleatorio.uniform(35.0, 60.0)] for _ in range(total)]
    enlaces = [(i, aleatorio.randrange(i), 40000.0, float(aleatorio.randint(100, 3000))) for i in range(1, total)]
    for _ in range(max(0, parametros.get("enlaces", total * 2) - (total - 1))):
        origen = aleatorio.randrange(total)
        destino = aleatorio.randrange(total)
        if origen != destino:
            enlaces.append((origen, destino, 40000.0, float(aleatorio.randint(100, 3000))))
    demandas = []
    for _ in range(parametros.get("demandas", total * 4)):
        origen = aleatorio.randrange(total)
        destino = aleatorio.randrange(total)
        if origen != destino:
            demandas.append((origen, destino, float(aleatorio.randint(1, 50000))))
    return {"nombres": nombres, "coords": coordenadas,
            "indice": {n: i for i, n in enumerate(nombres)}, "enlaces": enlaces, "demandas": demandas}


def filtrarNodos(red, excluidos):
    """Devuelve la red sin los nodos nombrados, renumerando los que quedan."""
    conservados = [i for i, nombre in enumerate(red["nombres"]) if nombre not in excluidos]
    nuevo = {viejo: i for i, viejo in enumerate(conservados)}
    return {
        "nombres": [red["nombres"][i] for i in conservados],
        "coords": [red["coords"][i] for i in conservados],
        "indice": {red["nombres"][i]: nuevo[i] for i in conservados},
        "enlaces": [(nuevo[u], nuevo[v], c, k) for (u, v, c, k) in red["enlaces"] if u in nuevo and v in nuevo],
        "demandas": [(nuevo[o], nuevo[d], val) for (o, d, val) in red["demandas"] if o in nuevo and d in nuevo],
    }


def cargarEscenario(idEscenario):
    """Carga un caso de prueba de app/data/scenarios y devuelve la red mas sus parametros de prueba."""
    ruta = os.path.join(DATOS, "scenarios", idEscenario + ".json")
    with open(ruta, encoding="utf-8") as archivo:
        definicion = json.load(archivo)
    if "generar" in definicion:
        red = generarRed(definicion["generar"])
    else:
        red = cargarRed(os.path.join(DATOS, definicion.get("demandas", "demands.txt")),
                        os.path.join(DATOS, definicion.get("nodos", "nodes.txt")),
                        os.path.join(DATOS, definicion.get("enlaces", "links.txt")))
    if definicion.get("excluir"):
        red = filtrarNodos(red, set(definicion["excluir"]))
    red["escenario"] = {"id": idEscenario, "nombre": definicion.get("nombre", idEscenario),
                        "descripcion": definicion.get("descripcion", ""), "fuente": definicion.get("fuente", "")}
    red["flujo"] = definicion.get("flujo", {})
    red["transmisiones"] = definicion.get("transmisiones", [])
    red["firmas"] = definicion.get("firmas", [])
    return red


def cargaPorNodo(red):
    """Trafico total que sale y entra de cada nodo (suma de la matriz de demandas)."""
    carga = [0.0] * len(red["nombres"])
    for origen, destino, valor in red["demandas"]:
        carga[origen] += valor
        carga[destino] += valor
    return carga


def cargaComoHexadecimal(red):
    """Transmisiones de ejemplo: la demanda de cada par se escribe en hexadecimal, un paquete por nodo origen."""
    paquetes = [""] * len(red["nombres"])
    for origen, destino, valor in red["demandas"]:
        paquetes[origen] += "%02X%02X%04X" % (origen, destino, int(round(valor)) % 65536)
    return [p for p in paquetes if p]


def firmasDeEjemplo(transmisiones):
    """Tres firmas de ejemplo tomadas de las propias transmisiones, para que la consola muestre coincidencias."""
    firmas = []
    if transmisiones:
        firmas.append(transmisiones[0][16:24] or "DEADBEEF")
    if len(transmisiones) > 1:
        firmas.append(transmisiones[1][32:40] or "FF00FF")
    firmas.append("DEADBEEF")
    return firmas[:3]


def construirEntradas(red, origen="uk1.uk", destino="gr1.gr"):
    """Arma la entrada de cada modo a partir de la red GEANT."""
    aristas = [[u, v, costo] for (u, v, capacidad, costo) in red["enlaces"]]
    flujo = []
    for (u, v, capacidad, costo) in red["enlaces"]:
        flujo.append([u, v, capacidad])
        flujo.append([v, u, capacidad])
    longitudes = [c[0] for c in red["coords"]]
    latitudes = [c[1] for c in red["coords"]]
    margen = 3.0
    transmisiones = cargaComoHexadecimal(red)
    entradas = {
        "mst": {"edges": aristas},
        "tsp": {"n": len(red["nombres"]), "edges": aristas},
        "maxflow": {"n": len(red["nombres"]), "edges": flujo,
                    "source": red["indice"][origen], "sink": red["indice"][destino]},
        "voronoi": {"sites": red["coords"],
                    "bounds": [min(longitudes) - margen, min(latitudes) - margen, max(longitudes) + margen, max(latitudes) + margen],
                    "queries": red["coords"][:3]},
        "propagation": {"n": len(red["nombres"]), "edges": aristas, "delays": retardosEnlaces(red),
                        "hopDelay": RETARDO_PROCESAMIENTO_MS, "metric": "time",
                        "source": red["indice"][origen], "target": red["indice"][destino]},
    }
    if len(transmisiones) >= 2:
        firmas = [transmisiones[0][16:24], transmisiones[1][32:40], SEMILLA_FIRMAS[0]]
        entradas["matching"] = {"signatures": firmas, "transmissions": transmisiones[:2]}
        entradas["palindrome"] = {"transmissions": transmisiones[:2]}
        entradas["commonsubstr"] = {"transmissions": transmisiones[:2]}
    return entradas


def correrTodo(ejecutable, red):
    """Ejecuta los modos disponibles y devuelve entradas y salidas."""
    entradas = construirEntradas(red)
    salidas = {}
    for modo, entrada in entradas.items():
        salidas[modo] = ejecutar(ejecutable, modo, entrada)
    return entradas, salidas


def main():
    ejecutable = sys.argv[1] if len(sys.argv) > 1 else "./sentinel"
    rutaDemandas = sys.argv[2] if len(sys.argv) > 2 else None
    red = cargarRed(rutaDemandas)
    entradas, salidas = correrTodo(ejecutable, red)
    print("Nodos: %d  Enlaces: %d  Demandas: %d" % (len(red["nombres"]), len(red["enlaces"]), len(red["demandas"])))
    for modo in salidas:
        texto = json.dumps(salidas[modo])
        print("%-12s %s" % (modo, texto[:110] + ("..." if len(texto) > 110 else "")))
    os.makedirs(RESULTADOS, exist_ok=True)
    with open(os.path.join(RESULTADOS, "geant_resultados.json"), "w") as archivo:
        json.dump({"entradas": entradas, "salidas": salidas}, archivo)


if __name__ == "__main__":
    main()
