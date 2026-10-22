# Sentinela del Norte - TC2038

Siete algoritmos en C++ (MST, TSP, flujo maximo, Voronoi, firmas con KMP, palindromo mas largo y subcomun mas larga) que se comunican por JSON, y una aplicacion que los ejecuta sobre datos reales de la red GEANT (SNDlib).

## Compilacion

Con las mismas opciones que usa el autograder (solo se compila `src/main.cpp`; los `.h` se incluyen):

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel
```

## Ejecucion

El ejecutable lee **un** objeto JSON por stdin y escribe **un** objeto JSON por stdout:

```
echo '{"mode":"mst","input":{"edges":[[0,1,4],[0,2,1],[1,2,2]]}}' | ./sentinel
```

Si algo falla responde `{"error": "..."}` (nunca usa excepciones: el compilador del navegador no las soporta). El campo opcional `"algorithm"` dentro de `input` elige la tecnica (ver tabla); sin el, se usa la tecnica por defecto.

## Modos y algoritmos

| Modo | Tecnica por defecto | Segunda tecnica (`algorithm`) | Complejidad por defecto |
|---|---|---|---|
| `mst` | Kruskal con Union-Find | `prim` (cola de prioridad) | O(E log E) |
| `tsp` | Held-Karp exacto hasta 16 nodos; si no, vecino mas cercano + 2-opt | `heldkarp`, `2opt`, `nn` | O(n^2 2^n) exacto; O(n^2) por pasada de 2-opt |
| `maxflow` | Dinic | `edmondskarp` | O(V^2 E) contra O(V E^2) |
| `voronoi` | Recorte del dominio con el semiplano de cada sitio | `sorted` (vecinos ordenados con corte anticipado) | O(n^2 k), k = vertices por celda |
| `matching` | KMP (primera posicion de cada firma) | `naive` | O(n + m) contra O(n m) |
| `palindrome` | Manacher | `expand` (expandir por centros) | O(n) contra O(n^2) |
| `commonsubstr` | Busqueda binaria sobre la longitud con hashing doble | `dp` (tabla, memoria O(m)) | O((n + m) log n) contra O(n m) |

Notas de diseno:

- Todas las posiciones de cadenas son **base 1** y los rangos incluyen los dos extremos. Matching devuelve seis resultados (tres firmas de la transmision 1 y luego las de la 2), cada uno `{"found":true,"position":p}` o `{"found":false}`.
- En palindromos y subcadena comun, ante empate se elige el que queda mas a la izquierda.
- TSP: si el grafo no es completo se calcula antes el camino minimo entre todos los pares (Floyd-Warshall) para visitar cada nodo una vez; la ruta devuelta tiene n + 1 nodos (regresa al inicio).
- Voronoi: cada celda es un poligono convexo; el area total de las celdas es igual al area del dominio, lo que sirve como verificacion.
- Los nombres de campos de entrada se aceptan con varios alias (`edges`/`links`, `capacity`/`weight`, etc.) mientras se confirma el esquema oficial.

## Arquitectura

| Archivo | Contenido | Responsable |
|---|---|---|
| `src/main.cpp` | Lee stdin, despacha por `mode`, escribe stdout | Gael |
| `src/json.h` | Lector y escritor JSON sin dependencias ni excepciones | Gael |
| `src/lectura.h` | Funciones para leer numeros, textos, puntos y listas desde el JSON | Gael |
| `src/grafo.h` | Grafo y lector tolerante de aristas | Gael |
| `src/mst.h` | Kruskal y Prim | Gael |
| `src/tsp.h` | Floyd-Warshall, Held-Karp, vecino mas cercano y 2-opt | Gael |
| `src/flujo.h` | Dinic y Edmonds-Karp | NOMBRE_COMPANERO_2 |
| `src/voronoi.h` | Celdas de Voronoi y consulta del sitio mas cercano | NOMBRE_COMPANERO_2 |
| `src/cadenas.h` | KMP, Manacher y subcadena comun mas larga | NOMBRE_COMPANERO_3 |
| `app/` | Interfaz web y pipeline GEANT | NOMBRE_COMPANERO_3 |

Reparto sugerido; ajustar al final con las contribuciones reales en `team.txt`.

## Pruebas

```
bash tests/ejecutarPruebas.sh          # casos conocidos, incluidos los ejemplos oficiales de la pagina
python3 tests/equivalencia.py ./sentinel   # las dos tecnicas de cada modo dan lo mismo (casos aleatorios)
python3 tests/estres.py ./sentinel         # compara contra fuerza bruta y contra una implementacion de referencia
```

## Benchmarks

```
python3 tests/benchmark.py ./sentinel results/
```

Genera `results/benchmarks.csv` y `results/benchmarks.md` con la mediana de 3 corridas por tecnica y tamano (chico, mediano, grande). El tiempo incluye leer y escribir el JSON.

Resultados principales (equipo donde se corrieron: ver `results/README.md`):

- **Cadenas:** las tecnicas lineales ganan con claridad en el peor caso: Manacher (3 ms) contra expandir por centros (178 ms) en 30 000 caracteres iguales; KMP (4 ms) contra busqueda ingenua (267 ms) con una firma de 10 000 caracteres; hashing (27 ms) contra tabla (661 ms) en dos textos de 20 000.
- **TSP:** en 16 puntos Held-Karp tarda 29 ms y 2-opt 4 ms, y 2-opt encontro el costo optimo en las tres instancias probadas (8, 12 y 16 puntos); el vecino mas cercano solo llega a 441 contra 360 del optimo en 16 puntos. En 2 000 puntos 2-opt baja el costo de 4 175 a 3 526 (15 %) por unos 60 ms extra. Held-Karp es inviable mas alla de 16 nodos.
- **MST y flujo:** Kruskal/Prim y Dinic/Edmonds-Karp dan tiempos parecidos en grafos aleatorios dispersos; Prim fue 30 % mas rapido en 50 000 nodos. Los peores casos teoricos de Edmonds-Karp no aparecen en este tipo de grafo.
- **Voronoi:** la tecnica ordenada fue mas lenta que el recorte directo (2.7 s contra 1.5 s con 4 000 sitios), porque ordenar los vecinos de cada sitio cuesta O(n log n) y las celdas aleatorias son pequenas. Se reporta como resultado negativo.

## Estado

- [x] MST, TSP, flujo maximo, Voronoi, firmas, palindromo y subcadena comun
- [x] Segunda tecnica y benchmarks por algoritmo
- [x] Integracion con la red GEANT (parte 2): `app/geant.py` y datos en `app/data` (falta bajar `demands.txt`)
- [x] Aplicacion web (`app/server.py`, ver `app/README.md`)
- [ ] PDF de reflexion

## Pendiente de confirmar con el profesor

- Nombres exactos de los campos JSON de entrada y salida de cada modo.
- Formato de los datos de SNDlib que se usaran y que se entrega de la parte 2.

## Estandar de codificacion

Se sigue el estandar del Departamento de Computacion del Tec: `camelCase` en variables y funciones, `MAYUSCULAS_CON_GUION_BAJO` en constantes, `PascalCase` singular en clases, sin acentos ni enie en identificadores, sangria con Tab, llave de apertura en la misma linea, una instruccion por linea, sin `break` ni `continue` en ciclos, variables con valor inicial, cabecera en cada archivo y comentario antes de cada funcion (proposito, parametros y retorno).
