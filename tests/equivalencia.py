"""Verifica que las dos tecnicas de cada algoritmo den el mismo resultado (o uno comparable).
Uso: python3 tests/equivalencia.py ./sentinela   (el ejecutable compilado de src/main.cpp)"""
import sys, json, random, subprocess, math

EJECUTABLE = sys.argv[1] if len(sys.argv) > 1 else "./sentinela"
random.seed(11)

def correr(modo, entrada, tecnica):
    entrada = dict(entrada)
    entrada["algorithm"] = tecnica
    p = subprocess.run([EJECUTABLE], input=json.dumps({"mode": modo, "input": entrada}), capture_output=True, text=True)
    return json.loads(p.stdout)

def texto(alfabeto, n):
    return "".join(random.choice(alfabeto) for _ in range(n))

fallas = 0
def comprobar(nombre, condicion, detalle=""):
    global fallas
    if not condicion:
        fallas += 1
        print("FALLA", nombre, detalle)

for _ in range(150):
    n = random.randint(2, 30)
    aristas = [[i, random.randrange(i), random.randint(1, 50)] for i in range(1, n)]
    aristas += [[random.randrange(n), random.randrange(n), random.randint(1, 50)] for _ in range(random.randint(0, 3 * n))]
    aristas = [a for a in aristas if a[0] != a[1]]
    a = correr("mst", {"edges": aristas}, "kruskal")
    b = correr("mst", {"edges": aristas}, "prim")
    comprobar("mst", a["totalCost"] == b["totalCost"] and len(a["edges"]) == len(b["edges"]), (a, b))

for _ in range(150):
    n = random.randint(2, 14)
    aristas = [[random.randrange(n), random.randrange(n), random.randint(1, 20)] for _ in range(random.randint(1, 40))]
    aristas = [e for e in aristas if e[0] != e[1]]
    if aristas:
        e = {"n": n, "edges": aristas, "source": 0, "sink": n - 1}
        comprobar("flujo", correr("maxflow", e, "dinic")["maxFlow"] == correr("maxflow", e, "edmondskarp")["maxFlow"], e)

for _ in range(150):
    n = random.randint(1, 25)
    aristas = [[random.randrange(n), random.randrange(n), random.randint(1, 6)] for _ in range(random.randint(0, 3 * n))]
    aristas = [a for a in aristas if a[0] != a[1]]
    e = {"n": n, "edges": aristas, "delays": [random.randint(1, 40) for _ in aristas], "hopDelay": 1,
         "source": random.randrange(n), "failedLinks": [i for i in range(len(aristas)) if random.random() < 0.2]}
    a = correr("propagation", e, "dijkstra")
    b = correr("propagation", e, "bellmanford")
    comprobar("propagacion", a["nodes"] == b["nodes"] and a["events"] == b["events"], e)

razones = []
for _ in range(40):
    n = random.randint(5, 14)
    pts = [[random.random() * 100, random.random() * 100] for _ in range(n)]
    exacto = correr("tsp", {"points": pts}, "heldkarp")["cost"]
    heur = correr("tsp", {"points": pts}, "2opt")["cost"]
    vecino = correr("tsp", {"points": pts}, "nn")["cost"]
    comprobar("tsp", heur >= exacto - 1e-6 and vecino >= exacto - 1e-6 and heur <= vecino + 1e-6, (exacto, heur, vecino))
    razones.append(heur / exacto)
print("TSP: 2-opt / exacto, promedio %.3f, peor %.3f" % (sum(razones) / len(razones), max(razones)))

for _ in range(30):
    n = random.randint(1, 40)
    sitios = [[random.uniform(0, 100), random.uniform(0, 100)] for _ in range(n)]
    a = correr("voronoi", {"sites": sitios, "bounds": [0, 0, 100, 100]}, "clipping")["regions"]
    b = correr("voronoi", {"sites": sitios, "bounds": [0, 0, 100, 100]}, "sorted")["regions"]
    comprobar("voronoi", all(abs(x["area"] - y["area"]) < 1e-6 for x, y in zip(a, b)) and abs(sum(x["area"] for x in a) - 10000) < 1e-6)

for _ in range(200):
    a = texto("0123456789abcdef"[:random.randint(2, 6)], random.randint(1, 40))
    b = texto("0123456789abcdef"[:random.randint(2, 6)], random.randint(1, 40))
    firmas = [texto("ab", random.randint(1, 4)) for _ in range(3)]
    comprobar("firmas", correr("matching", {"signatures": firmas, "transmissions": [a, b]}, "kmp")["positions"] == correr("matching", {"signatures": firmas, "transmissions": [a, b]}, "naive")["positions"])
    comprobar("palindromo", correr("palindrome", {"transmissions": [a, b]}, "manacher")["ranges"] == correr("palindrome", {"transmissions": [a, b]}, "expand")["ranges"])
    comprobar("subcadena", correr("commonsubstr", {"transmissions": [a, b]}, "hash")["range"] == correr("commonsubstr", {"transmissions": [a, b]}, "dp")["range"], (a, b))

print("Equivalencia terminada, fallas:", fallas)
sys.exit(1 if fallas else 0)
