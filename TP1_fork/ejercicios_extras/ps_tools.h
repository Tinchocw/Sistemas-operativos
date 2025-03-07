#ifndef LABS_PS_TOOLS_H
#define LABS_PS_TOOLS_H

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

void process_status(DIR *dir, struct dirent *entry);
void remove_end_of_line(char *process_name, size_t len);

#endif  // LABS_PS_TOOLS_H
