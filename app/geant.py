#!/usr/bin/env python3
"""
Descripcion: lee la red GEANT de SNDlib (nodos, enlaces y matriz de demandas), construye la entrada JSON
de cada uno de los siete modos del ejecutable en C++, lo ejecuta y devuelve los resultados.
Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
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
import os
import re
import subprocess
import sys

RAIZ = os.path.dirname(os.path.abspath(__file__))
DATOS = os.path.join(RAIZ, "data")
RESULTADOS = os.path.join(os.path.dirname(RAIZ), "results")
SEMILLA_FIRMAS = ["DEADBEEF", "FF00FF", "C0FFEE"]


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


def cargarRed(rutaDemandas=None):
    """Carga nodos, enlaces y demandas de GEANT y devuelve un diccionario con todo."""
    nombres, coordenadas = leerNodos(os.path.join(DATOS, "nodes.txt"))
    indice = {nombre: i for i, nombre in enumerate(nombres)}
    enlaces = leerEnlaces(os.path.join(DATOS, "links.txt"), indice)
    if rutaDemandas is None:
        rutaDemandas = os.path.join(DATOS, "demands.txt")
    demandas = leerDemandas(rutaDemandas, indice)
    return {"nombres": nombres, "coords": coordenadas, "indice": indice, "enlaces": enlaces, "demandas": demandas}


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
