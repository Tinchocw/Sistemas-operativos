#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define ARGS 3
#define EXIT_FAILURE 1
#define ERROR (-1)
#define BUFFER_SIZE                                                            \
	4096  // Podría hacerlo de forma dinámica y evitar esta constante.


int
main(int argc, char *argv[])
{
	if (argc != ARGS) {
		perror("Cantidad de argumentos incorrecto");
		return EXIT_FAILURE;
	}

	// En el caso de que uno de los archivos no exista voy a tener que crear
	// uno con el nombre recibido por parámetro

	char *source_path = argv[1];
	char *destination_path = argv[2];

	int source_fd = open(source_path, O_RDONLY);
	if (source_fd == ERROR) {
		perror("open");
		return EXIT_FAILURE;
	}

	// Crear el archivo de destino para escritura, pero fallar si ya existe
	int dest_fd = open(destination_path, O_WRONLY | O_CREAT | O_EXCL, 0666);
	if (dest_fd == ERROR) {
		perror("open");
		close(source_fd);
		return EXIT_FAILURE;
	}

	char buffer[BUFFER_SIZE];
	ssize_t bytes_read, bytes_written;
	while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0) {
		bytes_written = write(dest_fd, buffer, bytes_read);
		if (bytes_written == ERROR) {
			perror("write");
			close(source_fd);
			close(dest_fd);
			return EXIT_FAILURE;
		}
	}

	if (bytes_read == ERROR) {
		perror("read");
		close(source_fd);
		close(dest_fd);
		return EXIT_FAILURE;
	}

	// Cerrar archivos
	if (close(source_fd) == ERROR || close(dest_fd) == ERROR) {
		perror("close");
		return EXIT_FAILURE;
	}

	return 0;
}
