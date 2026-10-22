# results/

Evidencia experimental de las dos tecnicas de cada algoritmo en instancias chicas, medianas y grandes.

- `benchmarks.csv`: datos crudos (problema, tamano, instancia, tecnica, tiempo en ms, memoria en MB, resultado).
- `benchmarks.md`: la misma tabla lista para leer.
- `entorno.txt`: maquina, sistema, compilador, opciones, semilla y procedimiento de la corrida.

## Como se generaron

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel
python3 tests/benchmark.py ./sentinel results/
```

El entorno exacto de la corrida que esta guardada aqui aparece en `entorno.txt`. En resumen: contenedor
Linux con 2 nucleos, g++ 13.3, semilla fija 2026, mediana de 3 corridas para el tiempo y maximo de 3 para
la memoria.

Dos aclaraciones sobre lo que mide el script:

- **Tiempo:** es el total de una corrida del ejecutable, es decir, incluye leer y escribir el JSON. En los
  grafos muy grandes (hasta un millon de aristas) esa lectura pesa, y por eso Kruskal y Prim salen parecidos:
  buena parte del tiempo no es del algoritmo.
- **Memoria:** es el pico de memoria residente del proceso. En Linux sale de `VmHWM` en `/proc/<pid>/status`,
  que es un valor exacto; en Windows y macOS hace falta `psutil` (`pip install psutil`).

Los tiempos cambian de una computadora a otra. Lo que se compara entre tecnicas es la tendencia (cual gana y
por cuanto), no los milisegundos exactos. Para repetir el experimento en otra maquina basta volver a correr
el script: se regenera tambien `entorno.txt`.

## Lectura rapida

| Problema | Que se compara | Resultado medido |
|---|---|---|
| Cadenas | KMP, Manacher y hashing contra busqueda ingenua, expansion por centros y tabla | Las lineales ganan entre 23 y 75 veces en el peor caso; la memoria es practicamente la misma |
| TSP | Held-Karp contra vecino mas cercano y 2-opt | 2-opt iguala al optimo en 8, 12 y 16 puntos, y mejora 15 % al vecino mas cercano en 2 000 puntos |
| MST | Kruskal contra Prim | Sin ganador claro: Prim gana en instancias chicas y medianas, Kruskal en la grande |
| Flujo | Dinic contra Edmonds-Karp | Dinic gana entre 7 % y 16 %, siempre con la misma memoria |
| Voronoi | Recorte directo contra vecinos ordenados | La tecnica ordenada pierde: 54 % mas lenta con 4 000 sitios |

## Los numeros que mas dicen

| Caso | Tecnica | Tiempo | Memoria |
|---|---|---:|---:|
| Firmas, texto 100 000 y firma 10 000 | KMP | 5.4 ms | 4.3 MB |
| Firmas, mismo caso | Busqueda ingenua | 310.9 ms | 4.4 MB |
| Palindromo, 30 000 caracteres iguales | Manacher | 4.0 ms | 3.9 MB |
| Palindromo, mismo caso | Expandir por centros | 298.2 ms | 4.2 MB |
| Subcadena comun, dos textos de 20 000 | Hashing | 32.9 ms | 5.4 MB |
| Subcadena comun, mismo caso | Tabla (DP) | 766.1 ms | 5.4 MB |
| TSP exacto, 8 puntos | Held-Karp | 11.4 ms | 3.2 MB |
| TSP exacto, 12 puntos | Held-Karp | 3.7 ms | 3.9 MB |
| TSP exacto, 16 puntos | Held-Karp | 26.0 ms | 9.3 MB |
| MST, 200 000 nodos y 1 000 000 de aristas | Kruskal | 7 179 ms | 1 509 MB |
| MST, mismo caso | Prim | 8 251 ms | 1 509 MB |
| Flujo, 30 000 nodos y 180 000 aristas | Dinic | 1 122 ms | 274 MB |
| Flujo, mismo caso | Edmonds-Karp | 1 207 ms | 274 MB |
| Voronoi, 4 000 sitios | Recorte directo | 2 024 ms | 66 MB |
| Voronoi, mismo caso | Vecinos ordenados | 3 120 ms | 66 MB |

## Que nos ensenaron estos numeros

1. **El limite de 16 nodos del TSP exacto es de memoria, no de tiempo.** Held-Karp pasa de 3.2 MB con 8 nodos a
   9.3 MB con 16: la tabla de 2^n subconjuntos crece como O(n 2^n), y un paso mas ya no cabe comodamente. El
   tiempo todavia era aceptable (26 ms) cuando la memoria ya era el problema.
2. **En las tres comparaciones de cadenas, la memoria no decide nada.** Ningun metodo pasa de 6 MB, asi que la
   eleccion es puramente de tiempo, y ahi las tecnicas lineales ganan por uno o dos ordenes de magnitud.
3. **En MST la memoria tampoco decide:** Kruskal y Prim ocupan exactamente lo mismo (1 509 MB), porque el costo
   lo domina guardar el millon de aristas, no la estrategia. Ninguna de las dos gana siempre en tiempo.
4. **Una tecnica mas elaborada puede perder.** El Voronoi "ordenado" deberia podar trabajo, pero ordenar los
   vecinos de cada sitio cuesta O(n log n) y las celdas de una nube aleatoria son pequenas, asi que el orden
   nunca se amortiza. Lo reportamos como resultado negativo en vez de esconderlo.
