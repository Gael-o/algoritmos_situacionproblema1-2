# results/

Evidencia experimental de las dos tecnicas de cada algoritmo en instancias chicas, medianas y grandes.

- `benchmarks.csv`: datos crudos (problema, tamano, instancia, tecnica, tiempo en ms, resultado).
- `benchmarks.md`: la misma tabla lista para leer.

## Como se generaron

- Equipo: contenedor en la nube con 2 CPU Intel Xeon a 2.10 GHz, Linux 6.18.
- Compilador: g++ 13.3 con `-std=c++17 -O2 -Wall -Wextra -Wpedantic`.
- Procedimiento: `python3 tests/benchmark.py ./sentinel results/` (semilla fija 2026, mediana de 3 corridas por tecnica).
- El tiempo es el total de una corrida del ejecutable, es decir, incluye leer y escribir el JSON. En los grafos grandes (hasta un millon de aristas) la lectura del JSON domina el tiempo, por eso Kruskal y Prim salen casi iguales.

Los tiempos cambian de una computadora a otra. Antes de la defensa conviene volver a correr el script en una laptop del equipo y comparar las tendencias (lo importante es cual tecnica gana y por cuanto), no los milisegundos exactos.

## Lectura rapida

| Problema | Que se compara | Resultado |
|---|---|---|
| Cadenas | KMP, Manacher y hashing contra busqueda ingenua, expansion por centros y tabla | Las lineales ganan entre 25 y 65 veces en el peor caso |
| TSP | Held-Karp contra vecino mas cercano y 2-opt | 2-opt iguala al optimo en 16 puntos y mejora 15 % al vecino mas cercano en 2 000 puntos |
| MST | Kruskal contra Prim | Parecidos; Prim 30 % mejor en 50 000 nodos |
| Flujo | Dinic contra Edmonds-Karp | Parecidos en grafos aleatorios dispersos |
| Voronoi | Recorte directo contra vecinos ordenados | La tecnica ordenada pierde con hasta 4 000 sitios |
