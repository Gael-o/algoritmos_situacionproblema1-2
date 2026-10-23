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

## Como esta organizada la pantalla

- **Barra de arriba con menus desplegables**: Red, Algoritmos, Simulacion, Ciberseguridad y Escenarios. En
  pantallas chicas se vuelve un menu vertical (boton "Menu").
- **Mapa, siempre visible**, con zoom (rueda del mouse, pellizco, botones + y -, doble clic o teclas + y -) y
  arrastre para moverse. "Europa" ajusta la vista al continente y "Toda la red" incluye Nueva York. Al pasar
  el cursor sobre un nodo o enlace se ven sus datos (trafico, capacidad, km, retardo y, si aplica, su flujo).
- **Que hace un clic en el mapa**: se elige arriba del mapa entre marcar el **origen**, marcar el **destino**
  o **provocar una falla** (un clic tira un enlace o un nodo; otro clic lo repara).
- **Tablero a la derecha**: los parametros y resultados de la herramienta activa.

Las fallas, el origen y el destino son **globales**: todas las herramientas los toman en cuenta y se
recalculan solas al cambiarlos. Asi la aplicacion responde la pregunta "que pasa si falla esto".

## Herramientas

| Menu | Herramienta | Que muestra | Modo del C++ |
|---|---|---|---|
| Red | Explorar la red | Mapa con el tamano de cada nodo segun su trafico, datos de la red y nodos con mas trafico | (datos) |
| Algoritmos | Arbol de expansion minima | Arbol en el mapa, costo y si la red sigue conectada con las fallas | `mst` |
| Algoritmos | Ruta de mantenimiento | Ruta del TSP en el mapa, siempre por enlaces que existen | `tsp` (y `propagation` para los tramos sin enlace directo) |
| Algoritmos | Flujo maximo | Flujo pintado en el mapa: color y grosor segun el flujo, flechas con el sentido, corte minimo en morado | `maxflow` |
| Algoritmos | Zonas de cobertura | Celdas de Voronoi; si cae un nodo, sus vecinos se reparten su zona | `voronoi` |
| Simulacion | Propagacion de un mensaje | El mensaje viaja salto por salto con reloj en ms, lista de eventos como Packet Tracer y comparacion contra la red sin fallas | `propagation` |
| Simulacion | Que pasa si falla... | Indicadores antes y despues de las fallas, y busqueda de los enlaces mas criticos | `mst`, `maxflow`, `propagation` |
| Ciberseguridad | Consola de firmas | Firmas, palindromo y subcadena comun sobre las transmisiones | `matching`, `palindrome`, `commonsubstr` |
| Escenarios | (cada caso) | Carga un caso de prueba; toda la aplicacion pasa a usar esos datos | (datos) |

Cada herramienta de algoritmos tiene un boton **Comparar tecnicas**, que corre las dos tecnicas del C++
sobre la misma red y muestra tiempo y resultado de cada una.

### Simulacion de propagacion (estilo Packet Tracer)

- **Tipo**: a toda la red (difusion) o a un destino.
- **Como eligen la ruta los routers**: menor tiempo de llegada, menos saltos (como RIP) o costo de SNDlib
  (como OSPF).
- **Algoritmo**: Dijkstra (estado de enlace, OSPF) o Bellman-Ford (vector distancia, RIP).
- **Procesamiento en cada router**: retardo fijo por salto, de 0 a 20 ms.
- **Reloj**: tiempo simulado en ms y s, con reproducir, pausa, siguiente salto y reiniciar.
- **Ejemplo sugerido**: de Eslovenia a Suecia, con 0 ms de procesamiento la ruta mas rapida tiene 6 saltos por
  Europa del Este (9.8 ms); con 3 ms por salto cambia a 3 saltos por Austria y Alemania.

## Datos (carpeta `app/data`)

| Archivo | Contenido | Fuente |
|---|---|---|
| `nodes.txt` | 22 nodos con longitud y latitud | SNDlib, red `geant`, seccion NODES (geant.txt, formato nativo) |
| `links.txt` | 36 enlaces: capacidad (40 000 Mbit/s) y costo | SNDlib, red `geant`, seccion LINKS |
| `demands.txt` | 462 demandas (trafico entre pares de nodos) | SNDlib, red `geant`, seccion DEMANDS |

Fuente: https://sndlib.put.poznan.pl, red `geant`, archivo `geant.txt` en formato nativo. El programa solo lee
las lineas de demanda con el formato `id ( origen destino ) 1 valor ...`.

En GEANT, el costo de cada enlace en SNDlib es su distancia en kilometros (la razon costo/km da 1.00 en los 36
enlaces), por eso el costo se muestra en km.

### Transformaciones aplicadas a los datos

1. Los nodos y enlaces se copiaron del archivo nativo sin cambiar valores; los nodos se numeran 0..21 en el orden del archivo.
2. Cada enlace se usa **sin direccion** para MST, TSP y propagacion (con su costo). Para el flujo maximo, la aplicacion lo manda como arista no dirigida (`"directed": false`) y el pipeline sin interfaz como **dos arcos**, uno por sentido; las dos formas dan el mismo flujo maximo en los 462 pares de nodos.
3. Voronoi usa longitud y latitud como coordenadas planas (sin proyeccion), con un margen de 3 grados alrededor de los nodos.
4. Las "transmisiones" de la consola de ciberseguridad se **generan** a partir de la matriz de trafico: cada demanda se escribe como 8 digitos hexadecimales (origen, destino, valor) y se concatenan por nodo de origen. Son datos sinteticos derivados del trafico real, no capturas de red.
5. El **retardo de cada enlace** se calcula con la distancia real entre sus dos ciudades (formula de haversine) entre la velocidad de la luz en la fibra, unos 200 km por milisegundo. Es la distancia en linea recta, asi que es una cota inferior del retardo real. Lo hace `geant.py` (`retardosEnlaces`).

## Casos de prueba (`app/data/scenarios`)

Cada archivo `.json` de esa carpeta es un caso de prueba que la aplicacion puede cargar desde el menu Escenarios.
Al cargar uno, el mapa y todas las herramientas pasan a trabajar con esos datos y se quitan las fallas.

| Caso | Que trae | Para que sirve |
|---|---|---|
| `01_geant_completo` | 22 nodos, 36 enlaces, 462 demandas | El caso base: la red real de SNDlib |
| `02_geant_europa` | La misma red sin `ny1.ny` ni `il1.il` | Ver como cambian el MST y el TSP sin los saltos transoceanicos |
| `03_ejemplo_oficial` | Las dos transmisiones y las tres firmas de la pagina | Comprobar a mano que la consola da 13 y 11, 11 y 9, DEAD ausente, palindromos 2-8 y 7-12, subcadena comun 3-8 |
| `04_estres_300` | Red generada con semilla fija: 300 nodos, 1 200 enlaces | Ver que los algoritmos aguantan instancias grandes y que el TSP pasa a la ruta aproximada |

Para agregar un caso basta con poner otro `.json` en esa carpeta. Los campos son: `nombre`, `descripcion`,
`fuente`, los archivos de datos (`nodos`, `enlaces`, `demandas`), y opcionalmente `excluir` (lista de nodos que
se quitan), `flujo` (origen y destino), `transmisiones`, `firmas` o `generar` (red sintetica con semilla).

## Pipeline sin interfaz

```
python3 app/geant.py ./sentinel            # imprime los resultados y guarda results/geant_resultados.json
```

Corre los ocho modos sobre la red real, incluida la propagacion desde el origen del caso con los retardos reales.
