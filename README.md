# Sentinela del Norte - TC2038

Siete algoritmos en C++ (MST, TSP, flujo maximo, Voronoi, firmas con KMP, palindromo mas largo y subcomun mas larga) que se comunican por JSON, y una aplicacion que los ejecuta sobre datos reales de la red GEANT (SNDlib).

## Instalacion y dependencias

El nucleo algoritmico **no tiene dependencias**: es C++17 estandar, sin bibliotecas externas y sin
excepciones (el compilador WebAssembly del autograder no las soporta). Basta `g++` o `clang++` con C++17.

| Para | Hace falta | Nota |
|---|---|---|
| Compilar y correr los siete algoritmos | `g++` con C++17 | Nada mas |
| Correr las pruebas y los benchmarks | `python3` 3.8 o mas | Solo biblioteca estandar |
| Medir memoria en Windows o macOS | `psutil` (`pip install psutil`) | En Linux no hace falta |
| Levantar la aplicacion | `python3` y un navegador | Sin frameworks ni `npm` |

No hay nada que instalar ni configurar: se clona el repositorio, se compila y se ejecuta.

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

| Modo | Tecnica por defecto | Segunda tecnica (`algorithm`) | Tiempo | Memoria |
|---|---|---|---|---|
| `mst` | Kruskal con Union-Find | `prim` (cola de prioridad) | O(E log E) | O(V + E) |
| `tsp` | Held-Karp exacto hasta 16 nodos; si no, vecino mas cercano + 2-opt | `heldkarp`, `2opt`, `nn` | O(n^2 2^n) exacto; O(n^2) por pasada de 2-opt | O(n 2^n) exacto; O(n^2) aproximado |
| `maxflow` | Dinic | `edmondskarp` | O(V^2 E) contra O(V E^2) | O(V + E) |
| `voronoi` | Recorte del dominio con el semiplano de cada sitio | `sorted` (vecinos ordenados con corte anticipado) | O(n^2 k), k = vertices por celda | O(n k) la salida, O(k) de trabajo por celda |
| `matching` | KMP (primera posicion de cada firma) | `naive` | O(n + m) contra O(n m) | O(m) la tabla de fallos; O(1) la ingenua |
| `palindrome` | Manacher | `expand` (expandir por centros) | O(n) contra O(n^2) | O(n) contra O(1) |
| `commonsubstr` | Busqueda binaria sobre la longitud con hashing doble | `dp` (tabla con fila rodante) | O((n + m) log n) contra O(n m) | O(n + m) los prefijos de hash; O(min(n, m)) la tabla |

La columna de memoria es **espacio auxiliar**: lo que el algoritmo reserva ademas de la entrada. Las dos
complejidades se contrastan con los tiempos y los picos de memoria medidos en `results/benchmarks.md`.

Detalle de donde sale cada cota de memoria:

- **MST:** Kruskal guarda las E aristas para ordenarlas y dos arreglos de tamano V para Union-Find. Prim guarda la lista de adyacencia, O(V + E), y una cola de prioridad que llega a O(E).
- **TSP:** Held-Karp llena una tabla de 2^n subconjuntos por n nodos, de ahi O(n 2^n); esa memoria, y no el tiempo, es la razon del limite de 16 nodos (con 16 ya son ~1 millon de celdas). El camino aproximado solo necesita la matriz de distancias, O(n^2), que es lo mismo que cuesta Floyd-Warshall cuando el grafo no es completo.
- **Flujo maximo:** los dos guardan la red residual como lista de arcos con su arco inverso, O(V + E); Dinic agrega los niveles del BFS y los iteradores del DFS, ambos O(V).
- **Voronoi:** cada celda es un poligono que se recorta semiplano por semiplano, asi que en cualquier momento solo viven los k vertices de una celda; la salida completa ocupa O(n k).
- **Firmas:** KMP solo necesita la tabla de fallos de la firma, O(m); no copia el texto. La busqueda ingenua no reserva nada.
- **Palindromo:** Manacher guarda un radio por cada centro, O(n); expandir por centros no guarda nada, y ese es su unico punto a favor.
- **Subcadena comun:** el hashing precalcula los prefijos de las dos cadenas con dos modulos, O(n + m). La tabla de programacion dinamica se reduce a dos filas, O(min(n, m)), en vez de la matriz completa O(n m).

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
| `src/flujo.h` | Dinic y Edmonds-Karp | Damian Gomez |
| `src/voronoi.h` | Celdas de Voronoi y consulta del sitio mas cercano | Damian Gomez |
| `src/cadenas.h` | KMP, Manacher y subcadena comun mas larga | David Rangel |
| `app/` | Interfaz web y pipeline GEANT | David Rangel |

Las pruebas (`tests/`), los benchmarks (`results/`) y la revision del estandar de codificacion se hicieron entre los tres integrantes. El detalle de las contribuciones esta en `team.txt`.

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

Genera `results/benchmarks.csv`, `results/benchmarks.md` y `results/entorno.txt`. Por cada tecnica y tamano (chico, mediano, grande) mide el **tiempo** (mediana de 3 corridas, incluye leer y escribir el JSON) y el **pico de memoria** del proceso (maximo de 3 corridas). En Linux la memoria sale de `VmHWM` en `/proc/<pid>/status`, que es exacta; en Windows y macOS hace falta `psutil` (`pip install psutil`). `entorno.txt` guarda maquina, sistema, compilador, opciones y semilla, para que la corrida se pueda repetir.

Resultados principales (entorno exacto en `results/entorno.txt`):

- **Cadenas:** las tecnicas lineales ganan con claridad en el peor caso, y la memoria es practicamente la misma: Manacher 4 ms contra 298 ms de expandir por centros en 30 000 caracteres iguales; KMP 5 ms contra 311 ms de la busqueda ingenua con una firma de 10 000; hashing 33 ms contra 766 ms de la tabla en dos textos de 20 000. Con cadenas de este tamano ningun metodo pasa de 6 MB, asi que aqui la decision es solo de tiempo.
- **TSP:** la memoria es la que manda. Held-Karp sube de 3.2 MB con 8 nodos a 3.9 MB con 12 y a 9.3 MB con 16, que es el crecimiento O(n 2^n) esperado; un paso mas y no cabe, y por eso el limite de 16 esta en el codigo. El camino aproximado usa 37 MB con 2 000 puntos, lo que corresponde a la matriz de distancias O(n^2). En calidad, 2-opt encontro el optimo en 8, 12 y 16 puntos, con un promedio de 1.008 y un peor caso de 1.073 contra el exacto.
- **MST y flujo:** en el caso grande (200 000 nodos y 1 000 000 de aristas) Kruskal tarda 7.2 s y Prim 8.3 s, y los dos ocupan 1 509 MB, que es la lista de aristas: la diferencia entre las dos tecnicas esta en el tiempo, no en el espacio. En flujo, Dinic 1.12 s contra 1.21 s de Edmonds-Karp con 30 000 nodos, ambos con 274 MB. Los peores casos teoricos de Edmonds-Karp no aparecen en grafos aleatorios dispersos.
- **Voronoi:** la tecnica ordenada resulto mas lenta que el recorte directo (3.1 s contra 2.0 s con 4 000 sitios) usando la misma memoria, 66 MB, porque ordenar los vecinos de cada sitio cuesta O(n log n) y las celdas aleatorias son pequenas. Se reporta como resultado negativo.

Las dos ultimas observaciones son las que mejor contrastan teoria y practica: el limite de Held-Karp es de
memoria y se ve en los numeros, y una tecnica mas elaborada (Voronoi ordenado) puede perder contra una simple.

## Estado

- [x] MST, TSP, flujo maximo, Voronoi, firmas, palindromo y subcadena comun
- [x] Segunda tecnica y benchmarks por algoritmo
- [x] Integracion con la red GEANT (parte 2): `app/geant.py` y los datos reales en `app/data` (nodes, links y demands de SNDlib)
- [x] Aplicacion web (`app/server.py`, ver `app/README.md`)
- [x] Benchmarks de tiempo y memoria, con el entorno documentado en `results/entorno.txt`
- [x] Casos de prueba cargables desde la aplicacion (`app/data/scenarios`)
- [ ] PDF de reflexion

## Limitacion conocida

El modo `matching` no pasa los casos del autograder en linea, aunque el algoritmo es correcto: reproduce el
ejemplo oficial de la pagina (ABBA en 13 y 11, FF en 11 y 9, DEAD ausente en las dos transmisiones) y coincide
con fuerza bruta en miles de casos aleatorios, como comprueba `tests/estres.py`. Lo que no logramos acertar es
el **esquema de salida** que espera el evaluador: probamos once combinaciones de orden de los seis resultados
y de como marcar la firma ausente. La version entregada usa el formato que describe la pagina en "Formatos de
salida": seis resultados, las tres firmas sobre la transmision 1 y luego sobre la 2, cada uno como
`{"found": true, "position": p}` o `{"found": false}`. Queda como pregunta abierta para el profesor.

## Estandar de codificacion

Se sigue el estandar del Departamento de Computacion del Tec: `camelCase` en variables y funciones, `MAYUSCULAS_CON_GUION_BAJO` en constantes, `PascalCase` singular en clases, sin acentos ni enie en identificadores, sangria con Tab, llave de apertura en la misma linea, una instruccion por linea, sin `break` ni `continue` en ciclos, variables con valor inicial, cabecera en cada archivo y comentario antes de cada funcion (proposito, parametros y retorno).
