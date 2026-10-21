# Pruebas

`bash tests/ejecutarPruebas.sh` compila y verifica los casos de esta carpeta.

## Casos conocidos de la pagina del reto (para quien haga cadenas)

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

## Caso MST

`mst_basico.json`: 5 nodos, 7 aristas. Arbol esperado: 0-2 (1), 1-2 (2), 3-4 (3), 1-3 (5). Costo total 11.
