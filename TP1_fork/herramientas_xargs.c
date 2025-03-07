#include "herramientas_xargs.h"

void
xargs(char **argv)
{
	char *linea = NULL;  // Puntero para almacenar la línea leída
	size_t len = 0;      // Variable para almacenar el tamaño de la línea
	ssize_t read;  // Variable para almacenar el número de caracteres leídos

	int lineas_empaquetadas = 0;
	int cant_argumentos = 0;

	char *argumentos[ARGS_TOTALES];

	agregar_argumento_recibido(argv[1], &cant_argumentos, argumentos);

	while ((read = getline(&linea, &len, stdin)) != -1) {
		if (read > 0) {
			quitar_caracter_final_de_linea(linea, read);

			if (lineas_empaquetadas < NARGS) {
				agregar_argumento_recibido(linea,
				                           &cant_argumentos,
				                           argumentos);
				lineas_empaquetadas++;
			}

			if (lineas_empaquetadas == NARGS) {
				agregar_valor_nulo(cant_argumentos, argumentos);
				ejecutar_comando(argumentos);
				limpiar_argumentos(argumentos,
				                   &cant_argumentos,
				                   &lineas_empaquetadas);
			}
		}
	}
	agregar_valor_nulo(cant_argumentos, argumentos);
	ejecutar_comando(argumentos);

	free(linea);
}


void
agregar_valor_nulo(int cant_argumentos, char **argumentos)
{
	argumentos[cant_argumentos] = NULL;
	cant_argumentos++;
}
void
agregar_argumento_recibido(const char *linea, int *cant_argumentos, char **argumentos)
{
	argumentos[*cant_argumentos] = strdup(linea);
	*cant_argumentos += 1;
}
void
limpiar_argumentos(char **argumentos,
                   int *cantidad_de_argumentos,
                   int *lineas_empaquetadas)
{
	for (int i = 1; i < *lineas_empaquetadas; ++i) {
		argumentos[i] = NULL;
	}
	*cantidad_de_argumentos = 1;
	*lineas_empaquetadas = 0;
}
void
quitar_caracter_final_de_linea(char *linea, ssize_t read)
{
	if (linea[read - 1] == '\n') {
		linea[read - 1] = '\0';
	}
}
void
ejecutar_comando(char **argumentos)
{
	int pid = fork();
	if (pid == 0) {
		execvp(argumentos[0], argumentos);

	} else {
		wait((int *) 0);
	}
}
