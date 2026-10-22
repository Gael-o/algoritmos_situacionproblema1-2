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

Abrir http://localhost:8000 en cualquier navegador y **dejar esa terminal abierta**; para apagarlo, Ctrl+C.

Si el puerto 8000 esta ocupado (por ejemplo, porque quedo corriendo otra copia del servidor), se puede usar otro:

```
python3 app/server.py 8080
```

y abrir http://localhost:8080.

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

## Casos de prueba (`app/data/scenarios`)

Cada archivo `.json` de esa carpeta es un caso de prueba que la aplicacion puede cargar desde la pestana
Scenario Manager. Al cargar uno, el mapa, el laboratorio y la consola pasan a trabajar con esos datos.

| Caso | Que trae | Para que sirve |
|---|---|---|
| `01_geant_completo` | 22 nodos, 36 enlaces, 462 demandas | El caso base: la red real de SNDlib |
| `02_geant_europa` | La misma red sin `ny1.ny` ni `il1.il` | Ver como cambian el MST y el TSP sin los saltos transoceanicos |
| `03_ejemplo_oficial` | Las dos transmisiones y las tres firmas de la pagina | Comprobar a mano que la consola da 13 y 11, 11 y 9, DEAD ausente, palindromos 2-8 y 7-12, subcadena comun 3-8 |
| `04_estres_300` | Red generada con semilla fija: 300 nodos, 1 200 enlaces | Ver que los algoritmos aguantan instancias grandes y que el TSP pasa a la ruta aproximada |

Para agregar un caso basta con poner otro `.json` en esa carpeta. Los campos son: `nombre`, `descripcion`,
`fuente`, los archivos de datos (`nodos`, `enlaces`, `demandas`), y opcionalmente `excluir` (lista de nodos que
se quitan), `flujo` (origen y destino), `transmisiones`, `firmas` o `generar` (red sintetica con semilla).

## Que hace cada pestana

| Pestana | Que muestra | Algoritmos usados |
|---|---|---|
| Network Explorer | mapa, nodos con mas trafico | (datos) |
| Algorithm Lab | ejecuta un algoritmo, dibuja el resultado y compara tecnicas | MST, TSP, flujo maximo, Voronoi |
| Cybersecurity Console | firmas, palindromo y subcadena comun sobre las transmisiones | KMP, Manacher, hashing |
| Scenario Manager | carga casos de prueba y, ademas, tira un enlace y compara conectividad, costo y flujo antes y despues | MST, flujo maximo (= corte minimo) |

## Pipeline sin interfaz

```
python3 app/geant.py ./sentinel            # imprime los resultados y guarda results/geant_resultados.json
```
