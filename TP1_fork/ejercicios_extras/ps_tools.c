#include "ps_tools.h"

void
process_status(DIR *dir, struct dirent *entry)
{
	printf("%6s %s\n", "PID", "COMMAND");
	while (entry != NULL) {
		// Filtrar las entradas que son directorios y tienen nombres numéricos
		if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
			char comm_path[512];
			snprintf(comm_path,
			         sizeof(comm_path),
			         "/proc/%s/comm",
			         entry->d_name);

			FILE *comm_file = fopen(comm_path, "r");
			if (comm_file != NULL) {
				char process_name[32];
				size_t len = fread(process_name,
				                   1,
				                   sizeof(process_name),
				                   comm_file);

				remove_end_of_line(process_name, len);
				printf("%6s %s\n", entry->d_name, process_name);
			}
			fclose(comm_file);
		}
		entry = readdir(dir);
	}
}

void
remove_end_of_line(char *process_name, size_t len)
{
	if (process_name[len - 1] == '\n') {
		process_name[len - 1] = '\0';
	}
}
