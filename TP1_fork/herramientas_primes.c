#include "herramientas_primes.h"
#include <stdio.h>
#include <stdlib.h>


void
reconocedor_de_numeros_primos(int tope)
{
	int fds_padre_hijo[2];
	pid_t pid = nuevo_proceso(fds_padre_hijo);

	if (pid == 0) {
		close(fds_padre_hijo[WRITE]);
		crear_proximo_filtro(fds_padre_hijo);

	} else {
		int estado;

		close(fds_padre_hijo[READ]);
		generador_de_numeros(fds_padre_hijo, tope);
		close(fds_padre_hijo[WRITE]);

		wait(&estado);
	}
}

void
generador_de_numeros(const int *fds_padre_hijo, int tope)
{
	for (int i = 1; i < tope; ++i) {
		int numero_a_enviar = i + 1;
		write(fds_padre_hijo[WRITE],
		      &numero_a_enviar,
		      sizeof(numero_a_enviar));
	}
}
void
crear_proximo_filtro(int *fds_padre_hijo)
{
	int estado;
	int numero_primo;

	if (read(fds_padre_hijo[READ], &numero_primo, sizeof(numero_primo)) <= 0) {
		close(fds_padre_hijo[READ]);
		exit(EXIT_SUCCESS);
	}

	printf("primo %i\n", numero_primo);
	fflush(stdout);

	int fds_padre_hijo_rec[2];
	pid_t pid_rec = nuevo_proceso(fds_padre_hijo_rec);

	if (pid_rec == 0) {
		close(fds_padre_hijo[READ]);
		close(fds_padre_hijo_rec[WRITE]);
		crear_proximo_filtro(fds_padre_hijo_rec);
	} else {
		close(fds_padre_hijo_rec[READ]);
		int numero_recibido;

		while (read(fds_padre_hijo[READ],
		            &numero_recibido,
		            sizeof(numero_recibido)) > 0) {
			if (numero_recibido % numero_primo != 0) {
				write(fds_padre_hijo_rec[WRITE],
				      &numero_recibido,
				      sizeof(numero_recibido));
			}
		}

		close(fds_padre_hijo[WRITE]);
		close(fds_padre_hijo[READ]);
		close(fds_padre_hijo_rec[WRITE]);
		wait(&estado);
	}
}

pid_t
nuevo_proceso(int *fds_padre_hijo)
{
	if (pipe(fds_padre_hijo) < 0) {
		perror("error in pipe\n");
		exit(EXIT_FAILURE);
	}
	pid_t pid = fork();
	return pid;
}
