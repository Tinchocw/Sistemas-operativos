#include "find_tools.h"

void
print_file(const char *dir_path, const char *file_name)
{
	char full_path[PATH_MAX];
	snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, file_name);
	printf("%s\n", full_path);
}


char *
lowercase_string(char *lower_filename)
{
	for (int i = 0; lower_filename[i]; i++) {
		lower_filename[i] = tolower(lower_filename[i]);
	}
	return lower_filename;
}

void
print_lowercase(const char *dir_path,
                const char *target,
                bool case_sensitive,
                const char *file_name)
{
	if (case_sensitive) {
		if (strstr(file_name, target) != NULL) {
			print_file(dir_path, file_name);
		}
	} else {
		char *lower_filename = lowercase_string(strdup(file_name));
		char *lower_target = lowercase_string(strdup(target));

		if (strstr(lower_filename, lower_target) != NULL) {
			print_file(dir_path, file_name);
		}
		free(lower_filename);
		free(lower_target);
	}
}

void
search_files(DIR *dir, const char *dir_path, const char *target, bool case_sensitive)
{
	struct dirent *entry;

	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 ||
		    strcmp(entry->d_name, "..") == 0)
			continue;

		// Si es un directorio, lo abrimos con openat
		if (entry->d_type == DT_DIR) {
			int sub_dir_fd = openat(dirfd(dir),
			                        entry->d_name,
			                        O_RDONLY | O_DIRECTORY);
			if (sub_dir_fd == -1) {
				perror("openat");
				exit(EXIT_FAILURE);
			}
			DIR *sub_dir = fdopendir(sub_dir_fd);
			if (sub_dir == NULL) {
				perror("fdopendir");
				exit(EXIT_FAILURE);
			}

			char sub_dir_path[PATH_MAX];
			snprintf(sub_dir_path,
			         sizeof(sub_dir_path),
			         "%s/%s",
			         dir_path,
			         entry->d_name);
			search_files(sub_dir, sub_dir_path, target, case_sensitive);

			if (closedir(sub_dir) == -1) {
				perror("closedir");
				exit(EXIT_FAILURE);
			}
		}
		print_lowercase(dir_path, target, case_sensitive, entry->d_name);
	}
}
