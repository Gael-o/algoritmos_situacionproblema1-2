"""Benchmarks de las dos tecnicas de cada algoritmo en instancias chicas, medianas y grandes.
Uso: python3 tests/benchmark.py ./sentinela results/
El tiempo es el total de una corrida del ejecutable (incluye leer y escribir el JSON); se reporta la mediana de 3."""
import sys, os, csv, json, random, subprocess, time, statistics

EJECUTABLE = sys.argv[1] if len(sys.argv) > 1 else "./sentinela"
SALIDA = sys.argv[2] if len(sys.argv) > 2 else "results"
REPETICIONES = 3
random.seed(2026)
filas = []

def medir(modo, entrada, tecnica, clave_resultado):
    entrada = dict(entrada)
    entrada["algorithm"] = tecnica
    cuerpo = json.dumps({"mode": modo, "input": entrada})
    tiempos = []
    respuesta = {}
    for _ in range(REPETICIONES):
        inicio = time.perf_counter()
        p = subprocess.run([EJECUTABLE], input=cuerpo, capture_output=True, text=True, timeout=300)
        tiempos.append((time.perf_counter() - inicio) * 1000.0)
        respuesta = json.loads(p.stdout)
    return statistics.median(tiempos), clave_resultado(respuesta)

def registrar(problema, tamano, descripcion, modo, entrada, tecnicas, clave_resultado):
    for tecnica in tecnicas:
        ms, resultado = medir(modo, entrada, tecnica, clave_resultado)
        filas.append([problema, tamano, descripcion, tecnica, round(ms, 1), resultado])
        print(problema, tamano, tecnica, round(ms, 1), "ms", resultado, flush=True)

def hexa(n):
    return "".join(random.choice("0123456789abcdef") for _ in range(n))

# MST
for tamano, nodos, aristas in [("chico", 1000, 5000), ("mediano", 50000, 250000), ("grande", 200000, 1000000)]:
    lista = [[i, random.randrange(i), random.randint(1, 1000)] for i in range(1, nodos)]
    lista += [[random.randrange(nodos), random.randrange(nodos), random.randint(1, 1000)] for _ in range(aristas - nodos)]
    registrar("MST", tamano, "%d nodos, %d aristas" % (nodos, aristas), "mst", {"edges": lista}, ["kruskal", "prim"], lambda r: "costo=%s" % r.get("totalCost"))

# TSP: exacto contra heuristicas en pocos nodos
for n in (8, 12, 16):
    pts = [[random.random() * 100, random.random() * 100] for _ in range(n)]
    registrar("TSP", "chico", "%d puntos" % n, "tsp", {"points": pts}, ["heldkarp", "2opt", "nn"], lambda r: "costo=%.2f" % r.get("cost", -1))
# TSP: heuristicas en muchos nodos (el exacto es inviable)
for tamano, n in [("mediano", 300), ("grande", 2000)]:
    pts = [[random.random() * 100, random.random() * 100] for _ in range(n)]
    registrar("TSP", tamano, "%d puntos" % n, "tsp", {"points": pts}, ["nn", "2opt"], lambda r: "costo=%.2f" % r.get("cost", -1))

# Flujo maximo
for tamano, nodos, aristas in [("chico", 200, 1000), ("mediano", 5000, 25000), ("grande", 30000, 150000)]:
    lista = [[i, i + 1, 50] for i in range(nodos - 1)]
    lista += [[random.randrange(nodos), random.randrange(nodos), random.randint(1, 100)] for _ in range(aristas)]
    lista = [e for e in lista if e[0] != e[1]]
    registrar("Flujo maximo", tamano, "%d nodos, %d aristas" % (nodos, len(lista)), "maxflow", {"n": nodos, "edges": lista, "source": 0, "sink": nodos - 1}, ["dinic", "edmondskarp"], lambda r: "flujo=%s" % r.get("maxFlow"))

# Voronoi
for tamano, n in [("chico", 100), ("mediano", 1000), ("grande", 4000)]:
    sitios = [[random.uniform(0, 1000), random.uniform(0, 1000)] for _ in range(n)]
    registrar("Voronoi", tamano, "%d sitios" % n, "voronoi", {"sites": sitios, "bounds": [0, 0, 1000, 1000]}, ["clipping", "sorted"], lambda r: "area=%.1f" % sum(c["area"] for c in r.get("regions", [])))

# Firmas: peor caso para la busqueda ingenua (texto de 'a' y firma casi igual)
for tamano, n, m in [("chico", 10000, 1000), ("mediano", 50000, 5000), ("grande", 100000, 10000)]:
    texto = "a" * n
    firmas = ["a" * (m - 1) + "b", hexa(8), "a" * 5]
    registrar("Firmas", tamano, "texto %d, firma %d (peor caso)" % (n, m), "matching", {"signatures": firmas, "transmissions": [texto]}, ["kmp", "naive"], lambda r: "posiciones=%s" % r.get("positions"))

# Palindromo: peor caso para expandir por centros (todo 'a')
for tamano, n in [("chico", 1000), ("mediano", 10000), ("grande", 30000)]:
    registrar("Palindromo", tamano, "texto de %d caracteres iguales" % n, "palindrome", {"transmissions": ["a" * n]}, ["manacher", "expand"], lambda r: "rangos=%s" % r.get("ranges"))

# Subcadena comun
for tamano, n in [("chico", 500), ("mediano", 5000), ("grande", 20000)]:
    registrar("Subcadena comun", tamano, "dos textos hex de %d" % n, "commonsubstr", {"transmissions": [hexa(n), hexa(n)]}, ["hash", "dp"], lambda r: "rango=%s" % r.get("range"))

os.makedirs(SALIDA, exist_ok=True)
with open(os.path.join(SALIDA, "benchmarks.csv"), "w", newline="", encoding="utf8") as archivo:
    escritor = csv.writer(archivo)
    escritor.writerow(["problema", "tamano", "instancia", "tecnica", "tiempo_ms", "resultado"])
    escritor.writerows(filas)

with open(os.path.join(SALIDA, "benchmarks.md"), "w", encoding="utf8") as archivo:
    archivo.write("# Benchmarks\n\nMediana de %d corridas del ejecutable (incluye leer y escribir el JSON).\n\n" % REPETICIONES)
    archivo.write("| Problema | Tamano | Instancia | Tecnica | Tiempo (ms) | Resultado |\n| --- | --- | --- | --- | ---: | --- |\n")
    for f in filas:
        archivo.write("| %s | %s | %s | %s | %s | %s |\n" % tuple(f))
print("Listo: benchmarks.csv y benchmarks.md en", SALIDA)
