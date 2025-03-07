#ifndef FORK_HERRAMIENTAS_PRIMES_H
#define FORK_HERRAMIENTAS_PRIMES_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ 0   // file descriptor de lectura
#define WRITE 1  // file descriptor de escritura

void reconocedor_de_numeros_primos(int tope);
void generador_de_numeros(const int *fds_padre_hijo, int tope);
void crear_proximo_filtro(int *fds_padre_hijo);
pid_t nuevo_proceso(int *fds_padre_hijo);

#endif  // FORK_HERRAMIENTAS_PRIMES_H
