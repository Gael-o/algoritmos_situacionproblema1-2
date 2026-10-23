#!/usr/bin/env python3
"""
Descripcion: servidor local de la aplicacion SENTINEL. Entrega la pagina web y ejecuta el programa en C++
para cada consulta (la interfaz no calcula nada de los algoritmos: todo pasa por el ejecutable).
La aplicacion usa sentinel_app: el mismo src/main.cpp compilado con -DSENTINEL_APP, que ademas de los
siete modos trae la propagacion y el detalle del flujo. El servidor lo compila solo si falta o si algun
archivo de src/ cambio despues. El autograder compila src/main.cpp sin esa bandera.
Autores: Gael Alexander Zaragoza Torres (A01385956), Damian Alejandro Gomez Mena (A01385923), David Rangel Monsivais (A01385671)
Fecha: 22/10/2026

Uso (desde la raiz del repositorio):
    python3 app/server.py            # abre http://localhost:8000 (en Windows: python app/server.py)
    python3 app/server.py 8080       # otro puerto, si el 8000 esta ocupado
"""

import json
import os
import subprocess
import urllib.parse
import sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import geant

RAIZ = os.path.dirname(os.path.abspath(__file__))
REPOSITORIO = os.path.dirname(RAIZ)
FUENTES = os.path.join(REPOSITORIO, "src")
EXTENSION = ".exe" if os.name == "nt" else ""
EJECUTABLE_APP = os.path.join(REPOSITORIO, "sentinel_app" + EXTENSION)
EJECUTABLE_AUTOGRADER = os.path.join(REPOSITORIO, "sentinel" + EXTENSION)
COMANDO_APP = ["g++", "-std=c++17", "-O2", "-DSENTINEL_APP", os.path.join(FUENTES, "main.cpp"), "-o", EJECUTABLE_APP]
ESCENARIO_POR_DEFECTO = "01_geant_completo"
EJECUTABLE = EJECUTABLE_APP


def ejecutableAlDia():
    """True si sentinel_app existe y es mas nuevo que todos los archivos de src/."""
    alDia = os.path.exists(EJECUTABLE_APP)
    if alDia:
        marca = os.path.getmtime(EJECUTABLE_APP)
        alDia = all(os.path.getmtime(os.path.join(FUENTES, nombre)) <= marca for nombre in os.listdir(FUENTES))
    return alDia


def prepararEjecutable():
    """Compila sentinel_app si hace falta. Devuelve la ruta del ejecutable que usara la aplicacion,
    o None si no hay ninguno. Si no se puede compilar, usa ./sentinel (sin propagacion ni flujo en el mapa)."""
    elegido = EJECUTABLE_APP
    if not ejecutableAlDia():
        print("Compilando la version de la aplicacion (src/main.cpp con -DSENTINEL_APP)...")
        try:
            proceso = subprocess.run(COMANDO_APP, capture_output=True, text=True)
            compilo = proceso.returncode == 0
            if not compilo:
                print(proceso.stderr)
        except FileNotFoundError:
            compilo = False
            print("No se encontro g++ en esta computadora.")
        if not compilo:
            print("No se pudo compilar. Comando manual:")
            print("    g++ -std=c++17 -O2 -DSENTINEL_APP src/main.cpp -o sentinel_app")
            elegido = EJECUTABLE_AUTOGRADER if os.path.exists(EJECUTABLE_AUTOGRADER) else None
            if elegido:
                print("Por ahora se usa ./sentinel: la simulacion de propagacion y el flujo en el mapa no van a funcionar.")
    return elegido


def descripcionRed(red):
    """Red en formato JSON para la interfaz, con el caso de prueba del que salio."""
    carga = geant.cargaPorNodo(red)
    transmisiones = red.get("transmisiones") or geant.cargaComoHexadecimal(red)[:2]
    firmas = red.get("firmas") or geant.firmasDeEjemplo(transmisiones)
    flujo = red.get("flujo") or {}
    retardos = geant.retardosEnlaces(red)
    return {
        "scenario": red.get("escenario", {"id": "", "nombre": "", "descripcion": "", "fuente": ""}),
        "nodes": [{"id": i, "name": red["nombres"][i], "lon": red["coords"][i][0], "lat": red["coords"][i][1], "load": carga[i]}
                  for i in range(len(red["nombres"]))],
        "links": [{"u": u, "v": v, "capacity": c, "cost": k, "km": round(retardos[i] * geant.VELOCIDAD_FIBRA_KM_POR_MS, 1), "delay": retardos[i]}
                  for i, (u, v, c, k) in enumerate(red["enlaces"])],
        "hopDelay": geant.RETARDO_PROCESAMIENTO_MS,
        "demands": len(red["demandas"]),
        "payloads": transmisiones,
        "signatures": firmas,
        "source": red["indice"].get(flujo.get("origen"), 0),
        "sink": red["indice"].get(flujo.get("destino"), len(red["nombres"]) - 1),
    }


class Manejador(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def responder(self, codigo, tipo, cuerpo):
        datos = cuerpo.encode("utf-8")
        self.send_response(codigo)
        self.send_header("Content-Type", tipo)
        self.send_header("Content-Length", str(len(datos)))
        self.end_headers()
        self.wfile.write(datos)

    def do_GET(self):
        if self.path.startswith("/api/network"):
            pedido = urllib.parse.urlparse(self.path)
            parametros = urllib.parse.parse_qs(pedido.query)
            idEscenario = parametros.get("scenario", [ESCENARIO_POR_DEFECTO])[0]
            self.responder(200, "application/json", json.dumps(descripcionRed(geant.cargarEscenario(idEscenario))))
        elif self.path == "/api/scenarios":
            self.responder(200, "application/json", json.dumps(geant.listarEscenarios()))
        elif self.path == "/favicon.ico":
            self.responder(404, "text/plain", "sin icono")
        elif self.path in ("/", "/index.html"):
            self.responder(200, "text/html; charset=utf-8", open(os.path.join(RAIZ, "static", "index.html"), encoding="utf-8").read())
        else:
            self.responder(404, "text/plain", "no existe")

    def do_POST(self):
        if self.path == "/api/run":
            largo = int(self.headers.get("Content-Length", "0"))
            pedido = json.loads(self.rfile.read(largo))
            proceso = subprocess.run([EJECUTABLE], input=json.dumps(pedido), capture_output=True, text=True, timeout=120)
            self.responder(200, "application/json", proceso.stdout if proceso.stdout else json.dumps({"error": proceso.stderr}))
        else:
            self.responder(404, "text/plain", "no existe")

    def log_message(self, formato, *args):
        pass


if __name__ == "__main__":
    EJECUTABLE = prepararEjecutable()
    if EJECUTABLE is None:
        print("No hay ejecutable. Compila desde la raiz del repositorio:")
        print("    g++ -std=c++17 -O2 -DSENTINEL_APP src/main.cpp -o sentinel_app")
        sys.exit(1)
    puerto = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    print("SENTINEL listo en http://localhost:%d  (Ctrl+C para salir)" % puerto)
    print("Si el puerto esta ocupado, corre: python3 app/server.py 8080")
    servidor = ThreadingHTTPServer(("127.0.0.1", puerto), Manejador)
    servidor.daemon_threads = True
    servidor.serve_forever()
