/*
 * ps (5pts): el comando process status muestra información básica de los
 * procesos que están corriendo en el sistema. Se pide como mínimo una
 * implementación que muestre el pid y comando (i.e. argv) de cada proceso (esto
 * es equivalente a hacer ps -eo pid,comm, se recomienda compararlo con tal
 * comando). Para más información, leer la sección ps0, de uno de los labs
 * anteriores. Ayuda: leer proc(5) para información sobre el directorio /proc.
 * */

/*
 * Syscalls recomendadas: opendir(3), readdir(3)
 * Ayudas: proc(5), isdigit(3), para corroborar que se esté accediendo al
 * directorio de un proceso y no a algún otro archivo de /proc.
 * */

#include "ps_tools.h"

#define EXIT_FAILURE 1

int
main()
{
	DIR *directory;  // referencia a un directorio abierto
	struct dirent *entry;

	// Abrir el directorio /proc
	directory = opendir("/proc");
	if (directory == NULL) {
		perror("opendir");
		return (EXIT_FAILURE);
	}
	entry = readdir(directory);

	process_status(directory, entry);

	closedir(directory);
	return 0;
}
