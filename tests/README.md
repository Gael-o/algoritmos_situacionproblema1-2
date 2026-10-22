# Pruebas

- `bash tests/ejecutarPruebas.sh`: compila y verifica casos conocidos de los 7 modos (incluye los ejemplos oficiales de la pagina).
- `python3 tests/equivalencia.py ./sentinel`: comprueba que las dos tecnicas de cada modo den el mismo resultado en cientos de casos aleatorios.
- `python3 tests/estres.py ./sentinel`: compara contra fuerza bruta (palindromos, subcadena comun, TSP pequeno) y contra una implementacion de referencia (flujo), y revisa que las celdas de Voronoi sumen el area del dominio.
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
- `flujo_basico.json`: red clasica de 6 nodos, flujo maximo 23.
- `voronoi_basico.json`: 3 sitios en un dominio de 30 x 30; las celdas suman 900.
