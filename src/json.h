/*
 * Descripcion: lector y escritor minimo de JSON, sin librerias externas y sin excepciones
 * (el compilador del autograder corre en WebAssembly), para comunicar el ejecutable de
 * Sentinela del Norte por stdin y stdout. Los errores se reportan con valores de retorno.
 * Autores: Gael Alexander Zaragoza Torres (A01385956), NOMBRE_COMPANERO_2 (MATRICULA_2), NOMBRE_COMPANERO_3 (MATRICULA_3)
 * Fecha: 21/10/2026
 */

#ifndef JSON_H
#define JSON_H

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

enum class JsonTipo {
	NULO,
	BOOLEANO,
	NUMERO,
	TEXTO,
	ARREGLO,
	OBJETO
};

struct JsonValue {
	JsonTipo tipo = JsonTipo::NULO;
	bool booleano = false;
	double numero = 0.0;
	std::string texto = "";
	std::vector<JsonValue> arreglo;
	std::vector<std::string> claves;
	std::vector<JsonValue> valores;
};

/*
 * Crea un valor JSON numerico.
 * Parametro numero: valor que se guarda.
 * Retorno: JsonValue de tipo NUMERO.
 */
JsonValue crearNumero(double numero) {
	JsonValue valor;
	valor.tipo = JsonTipo::NUMERO;
	valor.numero = numero;
	return valor;
}

/*
 * Crea un valor JSON de texto.
 * Parametro texto: cadena que se guarda.
 * Retorno: JsonValue de tipo TEXTO.
 */
JsonValue crearTexto(const std::string& texto) {
	JsonValue valor;
	valor.tipo = JsonTipo::TEXTO;
	valor.texto = texto;
	return valor;
}

/*
 * Crea un valor JSON booleano.
 * Parametro booleano: valor que se guarda.
 * Retorno: JsonValue de tipo BOOLEANO.
 */
JsonValue crearBooleano(bool booleano) {
	JsonValue valor;
	valor.tipo = JsonTipo::BOOLEANO;
	valor.booleano = booleano;
	return valor;
}

/*
 * Crea un arreglo JSON vacio.
 * Retorno: JsonValue de tipo ARREGLO sin elementos.
 */
JsonValue crearArreglo() {
	JsonValue valor;
	valor.tipo = JsonTipo::ARREGLO;
	return valor;
}

/*
 * Crea un objeto JSON vacio.
 * Retorno: JsonValue de tipo OBJETO sin campos.
 */
JsonValue crearObjeto() {
	JsonValue valor;
	valor.tipo = JsonTipo::OBJETO;
	return valor;
}

/*
 * Agrega un elemento al final de un arreglo JSON.
 * Parametro arreglo: arreglo que se modifica.
 * Parametro elemento: valor que se agrega.
 */
void agregarElemento(JsonValue& arreglo, const JsonValue& elemento) {
	arreglo.arreglo.push_back(elemento);
}

/*
 * Agrega un campo a un objeto JSON, conservando el orden de insercion.
 * Parametro objeto: objeto que se modifica.
 * Parametro clave: nombre del campo.
 * Parametro valor: valor del campo.
 */
void agregarCampo(JsonValue& objeto, const std::string& clave, const JsonValue& valor) {
	objeto.claves.push_back(clave);
	objeto.valores.push_back(valor);
}

/*
 * Busca un campo por nombre dentro de un objeto JSON y copia su valor.
 * Parametro objeto: valor donde se busca.
 * Parametro clave: nombre del campo.
 * Parametro resultado: recibe la copia del valor si se encuentra (si no, no se modifica).
 * Retorno: true si el campo existe.
 */
bool buscarCampo(const JsonValue& objeto, const std::string& clave, JsonValue& resultado) {
	bool encontrado = false;
	if (objeto.tipo == JsonTipo::OBJETO) {
		for (size_t i = 0; i < objeto.claves.size(); i++) {
			if (!encontrado && objeto.claves[i] == clave) {
				resultado = objeto.valores[i];
				encontrado = true;
			}
		}
	}
	return encontrado;
}

/*
 * Lista los nombres de campo de un objeto, para mensajes de diagnostico.
 * Parametro valor: valor a inspeccionar.
 * Retorno: nombres separados por coma, o un aviso si el valor no es un objeto.
 */
std::string listarClaves(const JsonValue& valor) {
	std::string resultado = "(no es un objeto)";
	if (valor.tipo == JsonTipo::OBJETO) {
		resultado = "";
		for (size_t i = 0; i < valor.claves.size(); i++) {
			resultado += i > 0 ? ", " : "";
			resultado += valor.claves[i];
		}
	}
	return resultado;
}

/*
 * Construye una respuesta de error con el mensaje dado.
 * Parametro mensaje: descripcion del error.
 * Retorno: objeto JSON con el campo error.
 */
JsonValue construirError(const std::string& mensaje) {
	JsonValue respuesta = crearObjeto();
	agregarCampo(respuesta, "error", crearTexto(mensaje));
	return respuesta;
}

/*
 * Convierte un texto a minusculas ASCII.
 * Parametro texto: cadena original.
 * Retorno: copia en minusculas.
 */
std::string aMinusculas(const std::string& texto) {
	std::string resultado = texto;
	for (size_t i = 0; i < resultado.size(); i++) {
		if (resultado[i] >= 'A' && resultado[i] <= 'Z') {
			resultado[i] = static_cast<char>(resultado[i] - 'A' + 'a');
		}
	}
	return resultado;
}

/*
 * Da formato a un numero para JSON: enteros sin decimales y 15 cifras significativas.
 * Parametro numero: valor a formatear.
 * Retorno: texto del numero.
 */
std::string formatearNumero(double numero) {
	std::ostringstream salida;
	salida << std::setprecision(15) << numero;
	return salida.str();
}

/*
 * Escapa los caracteres especiales de un texto para incluirlo entre comillas.
 * Parametro texto: cadena original.
 * Retorno: cadena escapada, sin las comillas externas.
 */
std::string escaparTexto(const std::string& texto) {
	std::string resultado = "";
	for (size_t i = 0; i < texto.size(); i++) {
		char actual = texto[i];
		if (actual == '"') {
			resultado += "\\\"";
		} else if (actual == '\\') {
			resultado += "\\\\";
		} else if (actual == '\n') {
			resultado += "\\n";
		} else if (actual == '\r') {
			resultado += "\\r";
		} else if (actual == '\t') {
			resultado += "\\t";
		} else if (static_cast<unsigned char>(actual) < 0x20) {
			std::ostringstream formato;
			formato << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(actual);
			resultado += formato.str();
		} else {
			resultado += actual;
		}
	}
	return resultado;
}

/*
 * Serializa un valor JSON en forma compacta (sin espacios).
 * Parametro valor: valor a escribir.
 * Retorno: texto JSON.
 */
std::string serializar(const JsonValue& valor) {
	std::string resultado = "null";
	if (valor.tipo == JsonTipo::BOOLEANO) {
		resultado = valor.booleano ? "true" : "false";
	} else if (valor.tipo == JsonTipo::NUMERO) {
		if (std::isfinite(valor.numero)) {
			resultado = formatearNumero(valor.numero);
		}
	} else if (valor.tipo == JsonTipo::TEXTO) {
		resultado = "\"" + escaparTexto(valor.texto) + "\"";
	} else if (valor.tipo == JsonTipo::ARREGLO) {
		resultado = "[";
		for (size_t i = 0; i < valor.arreglo.size(); i++) {
			resultado += i > 0 ? "," : "";
			resultado += serializar(valor.arreglo[i]);
		}
		resultado += "]";
	} else if (valor.tipo == JsonTipo::OBJETO) {
		resultado = "{";
		for (size_t i = 0; i < valor.claves.size(); i++) {
			resultado += i > 0 ? "," : "";
			resultado += "\"" + escaparTexto(valor.claves[i]) + "\":" + serializar(valor.valores[i]);
		}
		resultado += "}";
	}
	return resultado;
}

class JsonParser {
private:
	std::string contenido;
	size_t posicion = 0;
	bool hayError = false;
	std::string mensajeError = "";

	/*
	 * Registra el primer error encontrado; los siguientes se ignoran.
	 * Parametro mensaje: descripcion del error.
	 */
	void registrarError(const std::string& mensaje) {
		if (!hayError) {
			hayError = true;
			mensajeError = mensaje;
		}
	}

	/*
	 * Indica si un caracter es espacio en blanco JSON.
	 * Parametro caracter: caracter a revisar.
	 * Retorno: true si es espacio, tabulador o salto de linea.
	 */
	bool esEspacio(char caracter) {
		return caracter == ' ' || caracter == '\t' || caracter == '\n' || caracter == '\r';
	}

	/*
	 * Avanza la posicion hasta el siguiente caracter que no sea espacio.
	 */
	void saltarEspacios() {
		while (posicion < contenido.size() && esEspacio(contenido[posicion])) {
			posicion++;
		}
	}

	/*
	 * Consume el caracter actual solo si coincide con el esperado.
	 * Parametro esperado: caracter que se desea consumir.
	 * Retorno: true si se consumio.
	 */
	bool consumirSiEs(char esperado) {
		bool coincide = posicion < contenido.size() && contenido[posicion] == esperado;
		if (coincide) {
			posicion++;
		}
		return coincide;
	}

	/*
	 * Exige que el caracter actual sea el esperado y lo consume; si no, registra el error.
	 * Parametro esperado: caracter obligatorio.
	 */
	void esperar(char esperado) {
		if (!consumirSiEs(esperado)) {
			registrarError(std::string("JSON invalido: se esperaba '") + esperado + "'");
		}
	}

	/*
	 * Consume un literal (true, false, null) si aparece en la posicion actual.
	 * Parametro literal: texto que se busca.
	 * Retorno: true si estaba y se consumio.
	 */
	bool coincideLiteral(const std::string& literal) {
		bool coincide = contenido.compare(posicion, literal.size(), literal) == 0;
		if (coincide) {
			posicion += literal.size();
		}
		return coincide;
	}

	/*
	 * Lee cuatro digitos hexadecimales de una secuencia \u y los codifica en UTF-8.
	 * Retorno: texto UTF-8 del caracter (solo plano basico).
	 */
	std::string leerUnicode() {
		std::string resultado = "";
		if (posicion + 4 > contenido.size()) {
			registrarError("JSON invalido: secuencia unicode incompleta");
		} else {
			unsigned long codigo = std::strtoul(contenido.substr(posicion, 4).c_str(), nullptr, 16);
			posicion += 4;
			unsigned long parteAlta = codigo >> 12;
			unsigned long parteMedia = codigo >> 6;
			unsigned long parteBaja = codigo & 0x3F;
			if (codigo < 0x80) {
				resultado += static_cast<char>(codigo);
			} else if (codigo < 0x800) {
				resultado += static_cast<char>(0xC0 | parteMedia);
				resultado += static_cast<char>(0x80 | parteBaja);
			} else {
				resultado += static_cast<char>(0xE0 | parteAlta);
				resultado += static_cast<char>(0x80 | (parteMedia & 0x3F));
				resultado += static_cast<char>(0x80 | parteBaja);
			}
		}
		return resultado;
	}

	/*
	 * Interpreta la secuencia que sigue a una diagonal invertida dentro de un texto.
	 * Retorno: texto que representa el escape.
	 */
	std::string leerEscape() {
		std::string resultado = "";
		if (posicion >= contenido.size()) {
			registrarError("JSON invalido: escape incompleto");
		} else {
			char marca = contenido[posicion];
			posicion++;
			resultado = std::string(1, marca);
			if (marca == 'n') {
				resultado = "\n";
			} else if (marca == 't') {
				resultado = "\t";
			} else if (marca == 'r') {
				resultado = "\r";
			} else if (marca == 'b') {
				resultado = "\b";
			} else if (marca == 'f') {
				resultado = "\f";
			} else if (marca == 'u') {
				resultado = leerUnicode();
			}
		}
		return resultado;
	}

	/*
	 * Lee un texto entre comillas a partir de la posicion actual.
	 * Retorno: contenido del texto, ya sin escapes.
	 */
	std::string parsearTexto() {
		esperar('"');
		std::string resultado = "";
		bool cerrado = false;
		while (!cerrado && !hayError) {
			if (posicion >= contenido.size()) {
				registrarError("JSON invalido: texto sin cerrar");
			} else {
				char actual = contenido[posicion];
				posicion++;
				if (actual == '"') {
					cerrado = true;
				} else if (actual == '\\') {
					resultado += leerEscape();
				} else {
					resultado += actual;
				}
			}
		}
		return resultado;
	}

	/*
	 * Lee un numero a partir de la posicion actual.
	 * Retorno: valor numerico leido.
	 */
	double parsearNumero() {
		char* inicio = &contenido[posicion];
		char* fin = nullptr;
		double valor = std::strtod(inicio, &fin);
		if (fin == inicio) {
			registrarError("JSON invalido: valor no reconocido");
		} else {
			posicion += static_cast<size_t>(fin - inicio);
		}
		return valor;
	}

	/*
	 * Lee un arreglo JSON a partir del corchete de apertura.
	 * Retorno: arreglo con sus elementos.
	 */
	JsonValue parsearArreglo() {
		JsonValue resultado = crearArreglo();
		esperar('[');
		saltarEspacios();
		if (!hayError && !consumirSiEs(']')) {
			bool continuar = true;
			while (continuar && !hayError) {
				agregarElemento(resultado, parsearValor());
				saltarEspacios();
				continuar = consumirSiEs(',');
			}
			esperar(']');
		}
		return resultado;
	}

	/*
	 * Lee un objeto JSON a partir de la llave de apertura.
	 * Retorno: objeto con sus campos.
	 */
	JsonValue parsearObjeto() {
		JsonValue resultado = crearObjeto();
		esperar('{');
		saltarEspacios();
		if (!hayError && !consumirSiEs('}')) {
			bool continuar = true;
			while (continuar && !hayError) {
				saltarEspacios();
				std::string clave = parsearTexto();
				saltarEspacios();
				esperar(':');
				agregarCampo(resultado, clave, parsearValor());
				saltarEspacios();
				continuar = consumirSiEs(',');
			}
			esperar('}');
		}
		return resultado;
	}

	/*
	 * Lee cualquier valor JSON a partir de la posicion actual.
	 * Retorno: valor leido (NULO si hubo un error).
	 */
	JsonValue parsearValor() {
		JsonValue resultado;
		saltarEspacios();
		if (posicion >= contenido.size()) {
			registrarError("JSON invalido: fin de entrada inesperado");
		} else {
			char actual = contenido[posicion];
			if (actual == '{') {
				resultado = parsearObjeto();
			} else if (actual == '[') {
				resultado = parsearArreglo();
			} else if (actual == '"') {
				resultado = crearTexto(parsearTexto());
			} else if (coincideLiteral("true")) {
				resultado = crearBooleano(true);
			} else if (coincideLiteral("false")) {
				resultado = crearBooleano(false);
			} else if (coincideLiteral("null")) {
				resultado = JsonValue();
			} else {
				resultado = crearNumero(parsearNumero());
			}
		}
		return resultado;
	}

public:
	/*
	 * Crea un lector sobre un texto JSON.
	 * Parametro texto: documento JSON completo.
	 */
	explicit JsonParser(const std::string& texto) : contenido(texto) {
	}

	/*
	 * Lee el documento completo y verifica que no sobre contenido.
	 * Parametro resultado: recibe el valor raiz del documento.
	 * Parametro mensaje: recibe la descripcion del error si el documento es invalido.
	 * Retorno: true si el documento es JSON valido.
	 */
	bool parsear(JsonValue& resultado, std::string& mensaje) {
		resultado = parsearValor();
		saltarEspacios();
		if (!hayError && posicion != contenido.size()) {
			registrarError("JSON invalido: contenido sobrante despues del valor");
		}
		mensaje = mensajeError;
		return !hayError;
	}
};

#endif
