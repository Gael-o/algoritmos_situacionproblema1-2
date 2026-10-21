#!/bin/bash
# Compila src/main.cpp con las opciones del autograder y verifica los casos de tests/.
# Uso: bash tests/ejecutarPruebas.sh   (desde la raiz del repositorio)

set -u
RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
EJECUTABLE="$(mktemp -d)/sentinel"
FALLAS=0

g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic "$RAIZ/src/main.cpp" -o "$EJECUTABLE" || exit 1

# Revisa que la respuesta del ejecutable contenga el texto esperado.
# Parametros: descripcion, entrada JSON, texto esperado dentro de la respuesta.
verificar() {
	local descripcion="$1"
	local entrada="$2"
	local esperado="$3"
	local respuesta
	respuesta="$(echo "$entrada" | "$EJECUTABLE")"
	if [[ "$respuesta" == *"$esperado"* ]]; then
		echo "OK    $descripcion"
	else
		echo "FALLA $descripcion"
		echo "      esperado: $esperado"
		echo "      obtenido: $respuesta"
		FALLAS=$((FALLAS + 1))
	fi
}

CASO_MST="$(cat "$RAIZ/tests/mst_basico.json")"
verificar "MST Kruskal: costo total 11" "$CASO_MST" '"totalCost":11'
verificar "MST Prim: costo total 11" "${CASO_MST/\"input\":\{/\"input\":\{\"algorithm\":\"prim\",}" '"totalCost":11'
verificar "MST con 4 enlaces" "$CASO_MST" '"connected":true'
verificar "MST desconectado se reporta" '{"mode":"mst","input":{"nodes":[0,1,2],"edges":[[0,1,5]]}}' '"connected":false'
verificar "MST con identificadores de texto" '{"mode":"mst","input":{"edges":[{"from":"Viena","to":"Berna","cost":3.5}]}}' '"u":"Viena"'
verificar "Modo desconocido da error JSON" '{"mode":"xyz","input":{}}' '"error"'
verificar "JSON invalido da error JSON" '{"mode":' '"error"'

if [ "$FALLAS" -eq 0 ]; then
	echo "Todas las pruebas pasaron."
else
	echo "$FALLAS prueba(s) fallaron."
	exit 1
fi
