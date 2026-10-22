# Benchmarks

Mediana de 3 corridas del ejecutable (incluye leer y escribir el JSON).

| Problema | Tamano | Instancia | Tecnica | Tiempo (ms) | Resultado |
| --- | --- | --- | --- | ---: | --- |
| MST | chico | 1000 nodos, 5000 aristas | kruskal | 20.0 | costo=118296 |
| MST | chico | 1000 nodos, 5000 aristas | prim | 15.6 | costo=118296 |
| MST | mediano | 50000 nodos, 250000 aristas | kruskal | 1859.1 | costo=5977059 |
| MST | mediano | 50000 nodos, 250000 aristas | prim | 1294.1 | costo=5977059 |
| MST | grande | 200000 nodos, 1000000 aristas | kruskal | 7396.1 | costo=24062773 |
| MST | grande | 200000 nodos, 1000000 aristas | prim | 7216.2 | costo=24062773 |
| TSP | chico | 8 puntos | heldkarp | 2.1 | costo=275.20 |
| TSP | chico | 8 puntos | 2opt | 1.9 | costo=275.20 |
| TSP | chico | 8 puntos | nn | 1.8 | costo=275.20 |
| TSP | chico | 12 puntos | heldkarp | 2.4 | costo=259.41 |
| TSP | chico | 12 puntos | 2opt | 1.4 | costo=259.41 |
| TSP | chico | 12 puntos | nn | 1.4 | costo=259.41 |
| TSP | chico | 16 puntos | heldkarp | 28.6 | costo=359.95 |
| TSP | chico | 16 puntos | 2opt | 3.8 | costo=359.95 |
| TSP | chico | 16 puntos | nn | 3.2 | costo=440.96 |
| TSP | mediano | 300 puntos | nn | 4.8 | costo=1676.44 |
| TSP | mediano | 300 puntos | 2opt | 4.0 | costo=1405.12 |
| TSP | grande | 2000 puntos | nn | 42.5 | costo=4174.59 |
| TSP | grande | 2000 puntos | 2opt | 105.3 | costo=3526.20 |
| Flujo maximo | chico | 200 nodos, 1194 aristas | dinic | 5.0 | flujo=330 |
| Flujo maximo | chico | 200 nodos, 1194 aristas | edmondskarp | 5.0 | flujo=330 |
| Flujo maximo | mediano | 5000 nodos, 29996 aristas | dinic | 104.0 | flujo=251 |
| Flujo maximo | mediano | 5000 nodos, 29996 aristas | edmondskarp | 117.1 | flujo=251 |
| Flujo maximo | grande | 30000 nodos, 179994 aristas | dinic | 815.1 | flujo=102 |
| Flujo maximo | grande | 30000 nodos, 179994 aristas | edmondskarp | 862.6 | flujo=102 |
| Voronoi | chico | 100 sitios | clipping | 8.3 | area=1000000.0 |
| Voronoi | chico | 100 sitios | sorted | 9.0 | area=1000000.0 |
| Voronoi | mediano | 1000 sitios | clipping | 131.9 | area=1000000.0 |
| Voronoi | mediano | 1000 sitios | sorted | 207.3 | area=1000000.0 |
| Voronoi | grande | 4000 sitios | clipping | 1477.7 | area=1000000.0 |
| Voronoi | grande | 4000 sitios | sorted | 2723.0 | area=1000000.0 |
| Firmas | chico | texto 10000, firma 1000 (peor caso) | kmp | 2.3 | posiciones=[-1, -1, 1] |
| Firmas | chico | texto 10000, firma 1000 (peor caso) | naive | 6.9 | posiciones=[-1, -1, 1] |
| Firmas | mediano | texto 50000, firma 5000 (peor caso) | kmp | 4.5 | posiciones=[-1, -1, 1] |
| Firmas | mediano | texto 50000, firma 5000 (peor caso) | naive | 76.1 | posiciones=[-1, -1, 1] |
| Firmas | grande | texto 100000, firma 10000 (peor caso) | kmp | 4.2 | posiciones=[-1, -1, 1] |
| Firmas | grande | texto 100000, firma 10000 (peor caso) | naive | 266.9 | posiciones=[-1, -1, 1] |
| Palindromo | chico | texto de 1000 caracteres iguales | manacher | 2.2 | rangos=[[1, 1000]] |
| Palindromo | chico | texto de 1000 caracteres iguales | expand | 2.7 | rangos=[[1, 1000]] |
| Palindromo | mediano | texto de 10000 caracteres iguales | manacher | 2.2 | rangos=[[1, 10000]] |
| Palindromo | mediano | texto de 10000 caracteres iguales | expand | 28.3 | rangos=[[1, 10000]] |
| Palindromo | grande | texto de 30000 caracteres iguales | manacher | 2.9 | rangos=[[1, 30000]] |
| Palindromo | grande | texto de 30000 caracteres iguales | expand | 178.4 | rangos=[[1, 30000]] |
| Subcadena comun | chico | dos textos hex de 500 | hash | 2.1 | rango=[300, 303] |
| Subcadena comun | chico | dos textos hex de 500 | dp | 2.2 | rango=[300, 303] |
| Subcadena comun | mediano | dos textos hex de 5000 | hash | 7.3 | rango=[453, 458] |
| Subcadena comun | mediano | dos textos hex de 5000 | dp | 45.8 | rango=[453, 458] |
| Subcadena comun | grande | dos textos hex de 20000 | hash | 27.4 | rango=[235, 240] |
| Subcadena comun | grande | dos textos hex de 20000 | dp | 661.3 | rango=[235, 240] |
