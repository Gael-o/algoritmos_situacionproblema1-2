#!/usr/bin/env python3
"""
Descripcion: servidor local de la aplicacion SENTINEL. Entrega la pagina web y ejecuta el programa en C++
para cada consulta (la interfaz no calcula nada de los algoritmos: todo pasa por el ejecutable).
Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
Fecha: 21/10/2026

Uso (desde la raiz del repositorio, despues de compilar ./sentinel):
    python3 app/server.py            # abre http://localhost:8000 (en Windows: python app/server.py)
"""

import json
import os
import subprocess
import sys
from http.server import BaseHTTPRequestHandler, HTTPServer

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import geant

RAIZ = os.path.dirname(os.path.abspath(__file__))
EJECUTABLE = os.path.join(os.path.dirname(RAIZ), "sentinel")
if not os.path.exists(EJECUTABLE) and os.path.exists(EJECUTABLE + ".exe"):
    EJECUTABLE = EJECUTABLE + ".exe"
RED = geant.cargarRed()


def descripcionRed():
    """Red en formato JSON para la interfaz."""
    carga = geant.cargaPorNodo(RED)
    return {
        "nodes": [{"id": i, "name": RED["nombres"][i], "lon": RED["coords"][i][0], "lat": RED["coords"][i][1], "load": carga[i]}
                  for i in range(len(RED["nombres"]))],
        "links": [{"u": u, "v": v, "capacity": c, "cost": k} for (u, v, c, k) in RED["enlaces"]],
        "demands": len(RED["demandas"]),
        "payloads": geant.cargaComoHexadecimal(RED)[:2],
    }


class Manejador(BaseHTTPRequestHandler):
    def responder(self, codigo, tipo, cuerpo):
        datos = cuerpo.encode("utf-8")
        self.send_response(codigo)
        self.send_header("Content-Type", tipo)
        self.send_header("Content-Length", str(len(datos)))
        self.end_headers()
        self.wfile.write(datos)

    def do_GET(self):
        if self.path == "/api/network":
            self.responder(200, "application/json", json.dumps(descripcionRed()))
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
    if not os.path.exists(EJECUTABLE):
        print("Primero compila: g++ -std=c++17 -O2 src/main.cpp -o sentinel")
        sys.exit(1)
    print("SENTINEL listo en http://localhost:8000  (Ctrl+C para salir)")
    HTTPServer(("localhost", 8000), Manejador).serve_forever()
