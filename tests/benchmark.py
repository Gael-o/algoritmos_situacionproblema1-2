"""Benchmarks de las dos tecnicas de cada algoritmo en instancias chicas, medianas y grandes.
Uso: python3 tests/benchmark.py ./sentinel results/
Mide dos cosas por corrida:
  - tiempo: el total de una corrida del ejecutable (incluye leer y escribir el JSON), mediana de 3.
  - memoria: el pico de memoria residente del proceso hijo, en MB, el maximo de las 3 corridas.
La memoria se lee del sistema operativo mientras el proceso corre:
  - En Linux, de VmHWM en /proc/<pid>/status, que es la marca de agua del proceso (valor exacto).
  - En Windows y macOS, de psutil si esta instalado (pip install psutil); en Windows usa peak_wset,
    que tambien es un pico exacto.
  - Si no hay ninguno de los dos, la columna queda vacia y se reporta asi, sin inventar el dato.
Tambien guarda en results/entorno.txt la maquina, el sistema, el compilador y las opciones usadas,
para que la corrida sea reproducible."""
import sys, os, csv, json, platform, random, subprocess, time, statistics, threading

try:
    import psutil
except ImportError:
    psutil = None

EJECUTABLE = sys.argv[1] if len(sys.argv) > 1 else "./sentinel"
SALIDA = sys.argv[2] if len(sys.argv) > 2 else "results"
REPETICIONES = 3
OPCIONES_COMPILACION = "-std=c++17 -O2 -Wall -Wextra -Wpedantic"
random.seed(2026)
filas = []

def picoEnLinux(pid):
    """Lee VmHWM (pico de memoria residente, en kB) de /proc/<pid>/status; None si no se puede."""
    pico = None
    try:
        with open("/proc/%d/status" % pid, encoding="utf8") as archivo:
            for linea in archivo:
                if linea.startswith("VmHWM:"):
                    pico = int(linea.split()[1])
    except (OSError, ValueError, IndexError):
        pico = None
    return pico

def picoConPsutil(proceso):
    """Pico de memoria (en kB) via psutil: peak_wset en Windows, rss en el resto; None si falla."""
    pico = None
    try:
        info = proceso.memory_info()
        pico = int(getattr(info, "peak_wset", info.rss) / 1024)
    except Exception:
        pico = None
    return pico

def vigilarMemoria(popen, estado):
    """Hilo que consulta el pico de memoria del hijo hasta que termina y lo guarda en estado."""
    observado = psutil.Process(popen.pid) if psutil is not None else None
    while popen.poll() is None:
        actual = picoEnLinux(popen.pid)
        if actual is None and observado is not None:
            actual = picoConPsutil(observado)
        if actual is not None and actual > estado["kb"]:
            estado["kb"] = actual
        time.sleep(0.0005)

def correrUnaVez(cuerpo):
    """Ejecuta el programa una vez y devuelve (milisegundos, pico en kB o None, respuesta)."""
    estado = {"kb": 0}
    inicio = time.perf_counter()
    popen = subprocess.Popen([EJECUTABLE], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE, text=True)
    vigilante = threading.Thread(target=vigilarMemoria, args=(popen, estado), daemon=True)
    vigilante.start()
    salida, _ = popen.communicate(cuerpo, timeout=300)
    vigilante.join(timeout=1.0)
    milisegundos = (time.perf_counter() - inicio) * 1000.0
    return milisegundos, (estado["kb"] if estado["kb"] > 0 else None), json.loads(salida)

def medir(modo, entrada, tecnica, clave_resultado):
    entrada = dict(entrada)
    entrada["algorithm"] = tecnica
    cuerpo = json.dumps({"mode": modo, "input": entrada})
    tiempos = []
    picos = []
    respuesta = {}
    for _ in range(REPETICIONES):
        milisegundos, kb, respuesta = correrUnaVez(cuerpo)
        tiempos.append(milisegundos)
        if kb is not None:
            picos.append(kb)
    memoria = round(max(picos) / 1024.0, 1) if picos else ""
    return statistics.median(tiempos), memoria, clave_resultado(respuesta)

def registrar(problema, tamano, descripcion, modo, entrada, tecnicas, clave_resultado):
    for tecnica in tecnicas:
        ms, memoria, resultado = medir(modo, entrada, tecnica, clave_resultado)
        filas.append([problema, tamano, descripcion, tecnica, round(ms, 1), memoria, resultado])
        print(problema, tamano, tecnica, round(ms, 1), "ms", memoria, "MB", resultado, flush=True)

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

def versionDelCompilador():
    """Primera linea de 'g++ --version'; texto vacio si no se puede consultar."""
    version = ""
    try:
        salida = subprocess.run(["g++", "--version"], capture_output=True, text=True, timeout=30)
        version = salida.stdout.splitlines()[0]
    except (OSError, IndexError, subprocess.SubprocessError):
        version = ""
    return version

def comoSeMidioLaMemoria():
    """Describe de donde salio el dato de memoria, para que la corrida sea reproducible."""
    if os.path.exists("/proc/self/status"):
        origen = "VmHWM de /proc/<pid>/status (pico exacto del proceso hijo)"
    elif psutil is not None and platform.system() == "Windows":
        origen = "psutil, peak_wset del proceso hijo (pico exacto)"
    elif psutil is not None:
        origen = "psutil, muestreo del RSS cada 0.5 ms (aproximado en corridas muy cortas)"
    else:
        origen = "no disponible: instala psutil (pip install psutil) y vuelve a correr"
    return origen

os.makedirs(SALIDA, exist_ok=True)
with open(os.path.join(SALIDA, "entorno.txt"), "w", encoding="utf8") as archivo:
    archivo.write("Entorno de la corrida de benchmarks\n\n")
    archivo.write("Fecha: %s\n" % time.strftime("%d/%m/%Y %H:%M"))
    archivo.write("Sistema: %s %s (%s)\n" % (platform.system(), platform.release(), platform.machine()))
    archivo.write("Procesador: %s\n" % (platform.processor() or "no reportado"))
    archivo.write("Nucleos logicos: %s\n" % (os.cpu_count() or "no reportado"))
    archivo.write("Python: %s\n" % platform.python_version())
    archivo.write("Compilador: %s\n" % (versionDelCompilador() or "no se pudo consultar g++"))
    archivo.write("Opciones de compilacion: %s\n" % OPCIONES_COMPILACION)
    archivo.write("Repeticiones por medicion: %d (mediana para el tiempo, maximo para la memoria)\n" % REPETICIONES)
    archivo.write("Semilla de generacion de instancias: 2026\n")
    archivo.write("Medicion de memoria: %s\n" % comoSeMidioLaMemoria())
    archivo.write("\nProcedimiento:\n")
    archivo.write("  g++ %s src/main.cpp -o sentinel\n" % OPCIONES_COMPILACION)
    archivo.write("  python3 tests/benchmark.py ./sentinel results/\n")

with open(os.path.join(SALIDA, "benchmarks.csv"), "w", newline="", encoding="utf8") as archivo:
    escritor = csv.writer(archivo)
    escritor.writerow(["problema", "tamano", "instancia", "tecnica", "tiempo_ms", "memoria_mb", "resultado"])
    escritor.writerows(filas)

with open(os.path.join(SALIDA, "benchmarks.md"), "w", encoding="utf8") as archivo:
    archivo.write("# Benchmarks\n\n")
    archivo.write("Tiempo: mediana de %d corridas del ejecutable (incluye leer y escribir el JSON).\n" % REPETICIONES)
    archivo.write("Memoria: pico de memoria residente del proceso, el maximo de las %d corridas.\n" % REPETICIONES)
    archivo.write("Medicion de memoria: %s.\n" % comoSeMidioLaMemoria())
    archivo.write("El entorno exacto de esta corrida esta en `results/entorno.txt`.\n\n")
    archivo.write("| Problema | Tamano | Instancia | Tecnica | Tiempo (ms) | Memoria (MB) | Resultado |\n")
    archivo.write("| --- | --- | --- | --- | ---: | ---: | --- |\n")
    for f in filas:
        archivo.write("| %s | %s | %s | %s | %s | %s | %s |\n" % tuple(f))
print("Listo: benchmarks.csv, benchmarks.md y entorno.txt en", SALIDA)
