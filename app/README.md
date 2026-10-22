# Aplicacion SENTINEL Digital Twin

Interfaz web sobre la red GEANT (SNDlib). **No calcula nada por su cuenta**: cada consulta ejecuta el programa en C++ (`./sentinel`) con el mismo JSON de entrada y salida que usa el autograder.

## Requisitos

- `g++` con C++17 y `python3` (solo biblioteca estandar, no hay que instalar nada).
- Un navegador.

## Pasos

Desde la raiz del repositorio:

```
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic src/main.cpp -o sentinel
python3 app/server.py   (en Windows: python app/server.py, y el programa se llama sentinel.exe)
```

Abrir http://localhost:8000.

## Datos (carpeta `app/data`)

| Archivo | Contenido | Fuente |
|---|---|---|
| `nodes.txt` | 22 nodos con longitud y latitud | SNDlib, red `geant`, seccion NODES (geant.txt, formato nativo) |
| `links.txt` | 36 enlaces: capacidad (40 000 Mbit/s) y costo | SNDlib, red `geant`, seccion LINKS |
| `demands.txt` | matriz de demandas (trafico entre pares de nodos) | SNDlib, Dynamic traffic de `geant` (o la seccion DEMANDS de geant.txt) |

`demands.txt` hay que descargarlo de https://sndlib.put.poznan.pl/geant.overview.action
(geant.txt en formato nativo, o el .tgz de Dynamic traffic: se extrae una matriz y se guarda con ese nombre).
El programa solo lee las lineas con el formato `id ( origen destino ) 1 valor ...`.

### Transformaciones aplicadas a los datos

1. Los nodos y enlaces se copiaron del archivo nativo sin cambiar valores; los nodos se numeran 0..21 en el orden del archivo.
2. Cada enlace se usa **sin direccion** para MST y TSP (con su costo), y como **dos arcos** (uno por sentido, con la capacidad de 40 000) para el flujo maximo.
3. Voronoi usa longitud y latitud como coordenadas planas (sin proyeccion), con un margen de 3 grados alrededor de los nodos.
4. Las "transmisiones" de la consola de ciberseguridad se **generan** a partir de la matriz de trafico: cada demanda se escribe como 8 digitos hexadecimales (origen, destino, valor) y se concatenan por nodo de origen. Son datos sinteticos derivados del trafico real, no capturas de red.

## Que hace cada pestana

| Pestana | Que muestra | Algoritmos usados |
|---|---|---|
| Network Explorer | mapa, nodos con mas trafico | (datos) |
| Algorithm Lab | ejecuta un algoritmo, dibuja el resultado y compara tecnicas | MST, TSP, flujo maximo, Voronoi |
| Cybersecurity Console | firmas, palindromo y subcadena comun sobre las transmisiones | KMP, Manacher, hashing |
| Scenario Manager | tira un enlace y compara conectividad, costo y flujo antes y despues | MST, flujo maximo (= corte minimo) |

## Pipeline sin interfaz

```
python3 app/geant.py ./sentinel            # imprime los resultados y guarda results/geant_resultados.json
```
