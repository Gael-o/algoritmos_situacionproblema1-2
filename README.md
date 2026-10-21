# Sentinela del Norte - TC2038

Siete algoritmos en C++ (MST, TSP, flujo maximo, Voronoi, firmas con KMP, palindromo mas largo y subcadena comun mas larga) y una aplicacion interactiva (SENTINEL Digital Twin) que los ejecuta.

## Compilacion

Con las mismas opciones que usa el autograder:

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel
```

## Ejecucion

El ejecutable lee **un** objeto JSON por stdin y escribe **un** objeto JSON por stdout:

```
echo '{"mode":"mst","input":{"edges":[[0,1,4],[0,2,1],[1,2,2]]}}' | ./sentinel
```

Si algo falla responde `{"error": "..."}` y termina con codigo 1.

## Pruebas

```
bash tests/ejecutarPruebas.sh
```

## Arquitectura

| Archivo | Contenido | Responsable |
|---|---|---|
| `src/main.cpp` | Lee stdin, despacha por `mode`, escribe stdout | Gael |
| `src/json.h` | Lector y escritor JSON sin dependencias | Gael |
| `src/grafo.h` | Grafo y lector tolerante de aristas desde JSON | Gael |
| `src/mst.h` | Kruskal (Union-Find) y Prim (cola de prioridad) | Gael |
| `src/tsp.h` | Held-Karp exacto y heuristica para GEANT | Gael |
| `src/flujo.h` | Edmonds-Karp y Dinic | NOMBRE_COMPANERO_2 |
| `src/voronoi.h` | Regiones de Voronoi y consulta de proximidad | NOMBRE_COMPANERO_2 |
| `src/cadenas.h` | KMP, Manacher y subcadena comun mas larga | NOMBRE_COMPANERO_3 |
| `app/` | Interfaz (Network Explorer, Algorithm Laboratory, Cybersecurity Console, Scenario Manager) | NOMBRE_COMPANERO_3 |

Reparto sugerido; ajustar al final con las contribuciones reales en `team.txt`.

## Estado

- [x] MST (Prim y Kruskal)
- [ ] TSP
- [ ] Flujo maximo
- [ ] Voronoi
- [ ] Firmas (KMP)
- [ ] Palindromo (Manacher)
- [ ] Subcadena comun mas larga
- [ ] Aplicacion
- [ ] Benchmarks en `results/`
- [ ] PDF de reflexion

## Pendiente de confirmar

- Nombres exactos de los campos JSON de entrada y salida de cada modo (hoy `grafo.h` acepta varios nombres; ajustar cuando se conozca el esquema oficial).
- Nombres oficiales de los modos (`main.cpp` acepta varios alias).
- Formato de `nodes.csv`, `edges.csv`, `capacities.csv`, `demands.csv` y `scenarios.json`.

## Estandar de codificacion

Se sigue el estandar del Departamento de Computacion del Tec: `camelCase` en variables y funciones, `MAYUSCULAS_CON_GUION_BAJO` en constantes, `PascalCase` singular en clases, sin acentos ni enie en identificadores, sangria con Tab, llave de apertura en la misma linea, una instruccion por linea, sin `break` ni `continue` en ciclos, variables con valor inicial, cabecera en cada archivo y comentario antes de cada funcion (proposito, parametros y retorno).
