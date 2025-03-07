#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include "find_tools.h"


int
main(int argc, char *argv[])
{
	if (argc < 2 || argc > 3) {
		perror("Argumentos incorrectos");
	}
	int case_sensitive = true;

	if (argc == 3 && strcmp(argv[1], "-i") == 0) {
		case_sensitive = false;
	} else if (argc == 3) {
		printf("Opción no reconocida: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	DIR *directory;  // referencia a un directorio abierto
	directory = opendir(".");
	if (directory == NULL) {
		perror("opendir, No se pudo abrir el directorio");
		return EXIT_FAILURE;
	}

	const char *search_str = argv[argc - 1];
	char path[PATH_MAX];
	char *filename = ".";
	snprintf(path, sizeof(path), "%s", filename);

	// Llamamos a la función para buscar archivos
	search_files(directory, filename, search_str, case_sensitive);


	if (closedir(directory) == -1) {
		perror("closedir");
		exit(EXIT_FAILURE);
	}

	return 0;
}
