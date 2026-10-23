#!/bin/bash
# Compila src/main.cpp de dos formas y verifica los casos de tests/:
#  - como el autograder (sin banderas extra): los siete modos del reto;
#  - como la aplicacion (-DSENTINEL_APP): ademas, la propagacion y el detalle del flujo.
# Las pruebas de guardia revisan que la version del autograder no traiga nada de la aplicacion.
# Uso: bash tests/ejecutarPruebas.sh   (desde la raiz del repositorio)

set -u
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
CARPETA="$(mktemp -d)"
EJECUTABLE="$CARPETA/sentinel"
EJECUTABLE_APP="$CARPETA/sentinel_app"
FALLAS=0

g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic "$RAIZ/src/main.cpp" -o "$EJECUTABLE" || exit 1
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -DSENTINEL_APP "$RAIZ/src/main.cpp" -o "$EJECUTABLE_APP" || exit 1

# Revisa que la respuesta de un ejecutable contenga el texto esperado.
# Parametros: ejecutable, descripcion, entrada JSON, texto esperado dentro de la respuesta.
verificarCon() {
	local programa="$1"
	local descripcion="$2"
	local entrada="$3"
	local esperado="$4"
	local respuesta
	respuesta="$(echo "$entrada" | "$programa")"
	if [[ "$respuesta" == *"$esperado"* ]]; then
		echo "OK    $descripcion"
	else
		echo "FALLA $descripcion"
		echo "      esperado: $esperado"
		echo "      obtenido: $respuesta"
		FALLAS=$((FALLAS + 1))
	fi
}

# Verifica con la version del autograder.
verificar() {
	verificarCon "$EJECUTABLE" "$@"
}

# Verifica con la version de la aplicacion.
verificarApp() {
	verificarCon "$EJECUTABLE_APP" "$@"
}

echo "== Version del autograder (los siete modos) =="
CASO_MST="$(cat "$RAIZ/tests/mst_basico.json")"
verificar "MST Kruskal: costo total 11" "$CASO_MST" '"totalCost":11'
verificar "MST Prim: costo total 11" "${CASO_MST/\"input\":\{/\"input\":\{\"algorithm\":\"prim\",}" '"totalCost":11'
verificar "MST con 4 enlaces" "$CASO_MST" '"connected":true'
verificar "MST desconectado se reporta" '{"mode":"mst","input":{"nodes":[0,1,2],"edges":[[0,1,5]]}}' '"connected":false'
verificar "MST con identificadores de texto" '{"mode":"mst","input":{"edges":[{"from":"Viena","to":"Berna","cost":3.5}]}}' '["Viena","Berna",3.5]'
verificar "TSP: costo optimo 80" "$(cat "$RAIZ/tests/tsp_basico.json")" '"cost":80'
verificar "TSP: la ruta regresa al inicio" "$(cat "$RAIZ/tests/tsp_basico.json")" '"route":[0,'
verificar "Flujo maximo: valor 23" "$(cat "$RAIZ/tests/flujo_basico.json")" '"maxFlow":23'
verificar "Voronoi: una celda por sitio" "$(cat "$RAIZ/tests/voronoi_basico.json")" '"numCells":3'
verificar "Voronoi: sitio mas cercano" "$(cat "$RAIZ/tests/voronoi_basico.json")" '"nearest":[0,1,2]'
verificar "Firmas: posiciones desde 1, ausente = found false" "$(cat "$RAIZ/tests/firmas_basico.json")" '"positions":[{"found":true,"position":3},{"found":false},{"found":true,"position":5},{"found":false},{"found":false},{"found":true,"position":1}]'
verificar "Palindromo: rangos desde 1" "$(cat "$RAIZ/tests/palindromo_basico.json")" '"ranges":[[1,3],[2,7]]'
verificar "Subcadena comun: rango en la primera" "$(cat "$RAIZ/tests/subcadena_basico.json")" '"range":[3,8]'
OFICIAL_T1="AABCDCBA12FFABBA"
OFICIAL_T2="00BCDCBAFFABBA77"
verificar "Ejemplo oficial: firmas (T1: 13, 11, no; T2: 11, 9, no)" "$(cat "$RAIZ/tests/ejemplo_oficial_cadenas.json")" '"positions":[{"found":true,"position":13},{"found":true,"position":11},{"found":false},{"found":true,"position":11},{"found":true,"position":9},{"found":false}]'
verificar "Ejemplo oficial: matches en el formato del autograder" "{\"mode\":\"matching\",\"input\":{\"patterns\":[\"ABBA\",\"FF\",\"DEAD\"],\"transmissions\":[\"$OFICIAL_T1\",\"$OFICIAL_T2\"]}}" '"matches":[[13,11,-1],[11,9,-1]]'
verificar "Ejemplo oficial: palindromos 2-8 y 7-12" "{\"mode\":\"palindrome\",\"input\":{\"transmissions\":[\"$OFICIAL_T1\",\"$OFICIAL_T2\"]}}" '"ranges":[[2,8],[7,12]]'
verificar "Ejemplo oficial: subcadena comun 3-8" "{\"mode\":\"commonsubstr\",\"input\":{\"transmissions\":[\"$OFICIAL_T1\",\"$OFICIAL_T2\"]}}" '"range":[3,8]'
verificar "Modo desconocido da error JSON" '{"mode":"xyz","input":{}}' '"error"'
verificar "JSON invalido da error JSON" '{"mode":' '"error"'
verificar "Guardia: el autograder no compila el modo de propagacion" '{"mode":"propagation","input":{"n":2,"edges":[[0,1,1]],"source":0}}' '"error":"modo desconocido: propagation"'
verificar "Guardia: el flujo del autograder no trae el detalle de la app" "$(cat "$RAIZ/tests/flujo_basico.json")" '"value":23}'

echo "== Version de la aplicacion (-DSENTINEL_APP) =="
verificarApp "Flujo maximo: mismo valor 23" "$(cat "$RAIZ/tests/flujo_basico.json")" '"maxFlow":23'
verificarApp "Flujo maximo: el corte minimo suma 23" "$(cat "$RAIZ/tests/flujo_basico.json")" '"minCutCapacity":23'
verificarApp "Flujo maximo: aristas del corte minimo" "$(cat "$RAIZ/tests/flujo_basico.json")" '"minCut":[[1,3,12,4],[4,3,7,7],[4,5,4,9]]'
CASO_PROPAGACION="$(cat "$RAIZ/tests/propagacion_basico.json")"
verificarApp "Propagacion: ruta minima 0-2-1-3-4" "$CASO_PROPAGACION" '"path":[0,2,1,3,4]'
verificarApp "Propagacion: distancia 11 y llegada en 15 ms" "$CASO_PROPAGACION" '"pathTime":15,"pathDistance":11'
verificarApp "Propagacion Bellman-Ford: misma ruta" "${CASO_PROPAGACION/\"input\":\{/\"input\":\{\"algorithm\":\"bellmanford\",}" '"path":[0,2,1,3,4]'
verificarApp "Propagacion: si cae 2-1 se reencamina por 0-1" "${CASO_PROPAGACION/\"input\":\{/\"input\":\{\"failedLinks\":[[2,1]],}" '"path":[0,1,3,4]'
verificarApp "Propagacion: si cae el nodo 3, el 4 queda aislado" "${CASO_PROPAGACION/\"input\":\{/\"input\":\{\"failedNodes\":[3],}" '"unreachable":[4]'
verificarApp "Propagacion: origen caido da error JSON" "${CASO_PROPAGACION/\"input\":\{/\"input\":\{\"failedNodes\":[0],}" '"error"'
verificarApp "Propagacion por saltos: 3 saltos, gana el padre de indice menor" "${CASO_PROPAGACION/\"input\":\{/\"input\":\{\"metric\":\"hops\",}" '"path":[0,1,3,4],"pathEdges":[0,3,5]'
verificarApp "Propagacion por tiempo: ruta 0-1-3-4, llega en 15 ms" "${CASO_PROPAGACION/\"input\":\{/\"input\":\{\"metric\":\"time\",}" '"path":[0,1,3,4],"pathEdges":[0,3,5],"pathTime":15,"pathDistance":15'
verificarApp "La app conserva los siete modos (MST)" "$CASO_MST" '"totalCost":11'

if [ "$FALLAS" -eq 0 ]; then
	echo "Todas las pruebas pasaron."
else
	echo "$FALLAS prueba(s) fallaron."
	exit 1
fi
