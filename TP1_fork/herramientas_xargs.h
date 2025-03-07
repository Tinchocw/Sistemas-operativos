#ifndef FORK_HERRAMIENTAS_XARGS_H
#define FORK_HERRAMIENTAS_XARGS_H

#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define NARGS 4
#define ARGS_TOTALES (NARGS + 2)

void xargs(char **argv);
void ejecutar_comando(char **argumentos);
void quitar_caracter_final_de_linea(char *linea, ssize_t read);
void limpiar_argumentos(char **argumentos,
                        int *cantidad_de_argumentos,
                        int *lineas_empaquetadas);
void agregar_argumento_recibido(const char *linea,
                                int *cant_argumentos,
                                char **argumentos);
void agregar_valor_nulo(int cant_argumentos, char **argumentos);


#endif  // FORK_HERRAMIENTAS_XARGS_H
