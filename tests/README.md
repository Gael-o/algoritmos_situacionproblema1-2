# Pruebas

- `bash tests/ejecutarPruebas.sh`: compila las dos versiones y verifica 33 casos conocidos: 21 con la version del autograder (los siete modos, los ejemplos oficiales de la pagina y dos pruebas de guardia que fallan si algo de la aplicacion se cuela en esa version) y 12 con la de la aplicacion (`-DSENTINEL_APP`: propagacion y detalle del flujo).
- `equivalencia.py` y `estres.py` reciben el ejecutable: con `./sentinel_app` prueban los ocho modos; con `./sentinel`, los siete del autograder, y avisan que omiten la propagacion.
- `python3 tests/equivalencia.py ./sentinel`: comprueba que las dos tecnicas de cada modo den el mismo resultado en cientos de casos aleatorios. En propagacion, Dijkstra y Bellman-Ford deben dar el mismo arbol completo (distancias, saltos, padres y tiempos), no solo la misma distancia.
- `python3 tests/estres.py ./sentinel`: compara contra fuerza bruta (palindromos, subcadena comun, TSP pequeno) y contra implementaciones de referencia en Python (flujo con Edmonds-Karp, propagacion con Floyd-Warshall con fallas al azar), revisa el detalle del flujo (capacidades, conservacion, corte minimo igual al flujo y que el corte separe fuente y sumidero) y que las celdas de Voronoi sumen el area del dominio.
- `python3 tests/benchmark.py ./sentinel results/`: genera los benchmarks.

## Ejemplos oficiales de la pagina del reto (cadenas)

Posiciones en base 1.

- Transmision 1: `AABCDCBA12FFABBA`
- Transmision 2: `00BCDCBAFFABBA77`
- Firmas: `ABBA`, `FF`, `DEAD`

| Resultado | Esperado |
|---|---|
| ABBA en T1 / T2 | posicion 13 / posicion 11 |
| FF en T1 / T2 | posicion 11 / posicion 9 |
| DEAD en T1 / T2 | no aparece / no aparece |
| Palindromo mas largo T1 | `ABCDCBA`, posiciones 2 a 8 |
| Palindromo mas largo T2 | `BAFFAB`, posiciones 7 a 12 |
| Subcadena comun mas larga | `BCDCBA`, posiciones 3 a 8 (en T1) |

## Otros casos

- `mst_basico.json`: 5 nodos, 7 aristas. Arbol esperado: 0-2 (1), 1-2 (2), 3-4 (3), 1-3 (5). Costo total 11.
- `tsp_basico.json`: 4 ciudades, costo optimo 80.
- `flujo_basico.json`: red clasica de 6 nodos, flujo maximo 23; corte minimo 1-3 (12), 4-3 (7) y 4-5 (4), que suma 23.
- `propagacion_basico.json`: 5 nodos y 6 enlaces con retardos. Ruta minima de 0 a 4: 0-2-1-3-4, distancia 11, llega en 15 ms con 1 ms de procesamiento por salto. Si cae 2-1 se reencamina por 0-1-3-4; si cae el nodo 3, el 4 queda aislado.
- `voronoi_basico.json`: 3 sitios en un dominio de 30 x 30; las celdas suman 900.
