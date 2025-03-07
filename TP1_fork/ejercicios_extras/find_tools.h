#ifndef EJERCICIOS_EXTRAS_FIND_TOOLS_H
#define EJERCICIOS_EXTRAS_FIND_TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>

void print_file(const char *dir_path, const char *file_name);

char *lowercase_string(char *lower_filename);

void print_lowercase(const char *dir_path,
                     const char *target,
                     bool case_sensitive,
                     const char *file_name);

void search_files(DIR *dir,
                  const char *dir_path,
                  const char *target,
                  bool case_sensitive);


#endif  // EJERCICIOS_EXTRAS_FIND_TOOLS_H
