#ifndef LABS_CP_TOOLS_H
#define LABS_CP_TOOLS_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

#define EXIT_FAILURE 1
#define ERROR (-1)

// Obtener el tamaño del archivo de origen
struct stat get_stat(int source_fd);

void copy(char *source_path, char *destination_path);

void execute_mmap_error(int source_fd, int dest_fd);

void map_fds_to_memory(int source_fd,
                       struct stat source_st,
                       int dest_fd,
                       void **src_ptr,
                       void **dest_ptr);

// Ajustar el tamaño del archivo de destino para que sea igual al del archivo de origen
void truncate_dest_fd(int source_fd, struct stat source_st, int dest_fd);

void end_program(int source_fd,
                 struct stat st,
                 int dest_fd,
                 void *src_ptr,
                 void *dest_ptr);


#endif  // LABS_CP_TOOLS_H
