#include "inode.h"
#include "structures.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>

struct inode *
inode_find_by_index(size_t idx)
{
	printf("[debug] inode_find_by_index - index: %lu\n", idx);

	if (idx >= MAX_INODES_PER_BLOCK * INODE_BLOCKS) {
		return NULL;
	}

	return fs.inodes[idx / MAX_INODES_PER_BLOCK] +
	       (idx % MAX_INODES_PER_BLOCK);
}

/*
    ret idx
    if not found, idx == #inodes
*/
size_t
inode_find_index(const struct inode *inode)
{
	if (inode == NULL) {
		return MAX_INODES_PER_BLOCK * INODE_BLOCKS;
	}

	for (size_t i = 0; i < INODE_BLOCKS; i++) {
		for (size_t j = 0; j < MAX_INODES_PER_BLOCK; j++) {
			if (inode == (fs.inodes[i] + j)) {
				return (i * MAX_INODES_PER_BLOCK) + j;
			}
		}
	}

	return INODE_BLOCKS * MAX_INODES_PER_BLOCK;
}

char *
inode_get_dir_name(struct inode *inode)
{
	char *name = inode_get_dir_path(inode);

	if (name[0] == '\0') {
		free(name);
		return NULL;
	}

	name = strrchr(name, '/') + 1;

	return name;
}

struct inode *
inode_find_by_path(const char *path)
{
	printf("[debug] inode_find_by_path - path: %s\n", path);

	if (path == NULL) {
		return NULL;
	}

	for (size_t i = 0; i < INODE_BLOCKS; i++) {
		for (size_t j = 0; j < MAX_INODES_PER_BLOCK; j++) {
			if (strcmp(path, (fs.inodes[i] + j)->path) == 0) {
				printf("[debug] inode_find_by_path - found "
				       "inode at index: %lu, inode_block_id: "
				       "%lu, "
				       "offset: %lu\n",
				       (i * MAX_INODES_PER_BLOCK) + j,
				       i,
				       j);
				return fs.inodes[i] + j;
			}
		}
	}
	printf("[debug] inode_find_by_path - not found inode at path %s\n", path);
	return NULL;
}

char *
inode_get_file_name(struct inode *inode)
{
	if (inode == NULL) {
		return NULL;
	}

	if (inode->path[0] == '\0') {
		return NULL;
	}

	char *file_name = calloc(MAX_PATH_LENGTH, sizeof(char));
	strcpy(file_name, strrchr(inode->path, '/') + 1);

	if (file_name[0] == '\0') {
		file_name[0] = '/';
	}

	return file_name;
}

char *
inode_get_dir_path(struct inode *inode)
{
	if (inode == NULL) {
		return NULL;
	}

	if (inode->path[0] == '\0') {
		return NULL;
	}

	char *dir_path = calloc(MAX_PATH_LENGTH, sizeof(char));
	strcpy(dir_path, inode->path);

	char *file_name = inode_get_file_name(inode);

	if (file_name == NULL) {
		return dir_path;
	}

	size_t sz = strlen(dir_path) - strlen(file_name);
	free(file_name);

	if (sz > strlen(dir_path)) {
		return NULL;
	}

	dir_path[sz] = '\0';

	if (sz > 1) {
		dir_path[sz - 1] = '\0';
	}

	return dir_path;
}

int
inode_dir_not_empty(struct inode *inode)
{
	for (size_t i = 0; i < INODE_BLOCKS; i++) {
		for (size_t j = 0; j < MAX_INODES_PER_BLOCK; j++) {
			if ((fs.inodes[i] + j) == inode ||
			    strlen((fs.inodes[i] + j)->path) <= 1) {
				continue;
			}

			char *buff = inode_get_dir_path(fs.inodes[i] + j);
			int same_dir_path = strcmp(inode->path, buff);

			free(buff);

			if (same_dir_path == 0) {
				printf("[debug] inode_dir_not_empty - found "
				       "inode pointing to a block inside "
				       "dir\n");

				return 1;
			}
		}
	}

	return 0;
}


void
inode_load_all(FILE *file)
{
	for (size_t i = 0; i < INODE_BLOCKS; i++) {
		if (fread(fs.inodes[i],
		          sizeof(struct inode),
		          MAX_INODES_PER_BLOCK,
		          file) <= 0) {
			errno = EIO;
			fprintf(stderr,
			        "[debug] Error loading inodes: %s\n",
			        strerror(errno));
			fclose(file);
			exit(-errno);
		}
	}
}

void
inode_save_all(FILE *file)
{
	for (size_t i = 0; i < INODE_BLOCKS; i++) {
		if (fwrite(fs.inodes[i],
		           sizeof(struct inode),
		           MAX_INODES_PER_BLOCK,
		           file) <= 0) {
			errno = EIO;
			fprintf(stderr,
			        "[debug] Error saving inodes: %s\n",
			        strerror(errno));
			fclose(file);
			exit(-errno);
		}
	}
}