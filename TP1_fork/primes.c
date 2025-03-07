#include <stdio.h>
#include <stdlib.h>
#include "herramientas_primes.h"


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("No ingreso el tope requerido \n");
		return EXIT_FAILURE;
	}
	if (atoi(argv[1]) < 2) {
		perror("Tope invalido \n");
		return EXIT_FAILURE;
	}
	int tope = atoi(argv[1]);

	reconocedor_de_numeros_primos(tope);

	return 0;
}
