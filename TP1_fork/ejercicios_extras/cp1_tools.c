#include "cp1_tools.h"

void
copy(char *source_path, char *destination_path)
{
	int source_fd = open(source_path, O_RDONLY);
	if (source_fd == ERROR) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	struct stat source_st = get_stat(source_fd);

	int dest_fd = open(destination_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (dest_fd == -1) {
		perror("open");
		close(source_fd);
		exit(EXIT_FAILURE);
	}

	truncate_dest_fd(source_fd, source_st, dest_fd);

	void *src_ptr;
	void *dest_ptr;
	map_fds_to_memory(source_fd, source_st, dest_fd, &src_ptr, &dest_ptr);

	memcpy(dest_ptr, src_ptr, source_st.st_size);
	end_program(source_fd, source_st, dest_fd, src_ptr, dest_ptr);
}

void
truncate_dest_fd(int source_fd, struct stat source_st, int dest_fd)
{
	if (ftruncate(dest_fd, source_st.st_size) == ERROR) {
		perror("ftruncate");
		close(source_fd);
		close(dest_fd);
		exit(EXIT_FAILURE);
	}
}

void
map_fds_to_memory(int source_fd,
                  struct stat source_st,
                  int dest_fd,
                  void **src_ptr,
                  void **dest_ptr)
{
	(*src_ptr) =
	        mmap(NULL, source_st.st_size, PROT_READ, MAP_PRIVATE, source_fd, 0);
	if ((*src_ptr) == MAP_FAILED) {
		execute_mmap_error(source_fd, dest_fd);
	}

	(*dest_ptr) =
	        mmap(NULL, source_st.st_size, PROT_WRITE, MAP_SHARED, dest_fd, 0);
	if ((*dest_ptr) == MAP_FAILED) {
		munmap((*src_ptr), (source_st).st_size);
		execute_mmap_error(source_fd, dest_fd);
	}
}

struct stat
get_stat(int fd)
{
	struct stat st;
	if (fstat(fd, &st) == ERROR) {
		perror("fstat");
		close(fd);
		exit(EXIT_FAILURE);
	}
	return st;
}

void
execute_mmap_error(int source_fd, int dest_fd)
{
	perror("mmap");
	close(source_fd);
	close(dest_fd);
	exit(EXIT_FAILURE);
}

void
end_program(int source_fd, struct stat st, int dest_fd, void *src_ptr, void *dest_ptr)
{
	munmap(src_ptr, st.st_size);
	munmap(dest_ptr, st.st_size);

	close(source_fd);
	close(dest_fd);
}