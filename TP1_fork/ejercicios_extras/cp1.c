#include <stdio.h>
#include "cp1_tools.h"

#define ARGS 3

int
main(int argc, char *argv[])
{
	if (argc != ARGS) {
		perror("Cantidad de argumentos incorrecto");
		return EXIT_FAILURE;
	}

	char *source_path = argv[1];
	char *destination_path = argv[2];

	copy(source_path, destination_path);
	return 0;
}
