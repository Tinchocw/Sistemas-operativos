#include <stdio.h>
#include <stdlib.h>
#include "herramientas_xargs.h"

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("No ingreso los argumentos correctos\n");
		return EXIT_FAILURE;
	}

	xargs(argv);

	return 0;
}
