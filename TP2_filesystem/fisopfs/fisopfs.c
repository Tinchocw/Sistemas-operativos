#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "structures.h"
#include "inode.h"
#include "bmap.h"

#define FISOPFS_FILE "file.fisopfs"

struct file_system fs;

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	struct inode *inode = inode_find_by_path(path);

	if (inode == NULL) {
		return -ENOENT;
	}

	st->st_atime = inode->time;
	st->st_ctime = inode->ctime;
	st->st_gid = inode->gid;
	st->st_mode = inode->mode;
	st->st_mtime = inode->mtime;
	st->st_nlink = inode->link_count;
	st->st_size = inode->content_size;
	st->st_uid = inode->uid;
	st->st_dev = 0;

	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	struct inode *inode = inode_find_by_path(path);

	if (inode == NULL) {
		errno = ENOENT;
		fprintf(stderr,
		        "[debug] Error not a dir named %s: %s\n",
		        path,
		        strerror(errno));
		return -errno;
	}

	if ((inode->mode & __S_IFDIR) != __S_IFDIR) {
		errno = ENOTDIR;
		fprintf(stderr, "[debug] Error not a dir: %s\n", strerror(errno));
		return -errno;
	}

	inode->time = time(NULL);

	for (size_t i = 0; i < INODE_BLOCKS; i++) {
		for (size_t j = 0; j < MAX_INODES_PER_BLOCK; j++) {
			char *buff = inode_get_dir_path(fs.inodes[i] + j);

			if (buff == NULL) {
				continue;
			}

			if (strcmp(path, buff) == 0) {
				free(buff);
				buff = inode_get_file_name(fs.inodes[i] + j);

				if (buff == NULL) {
					continue;
				}

				filler(buffer, buff, NULL, 0);
			}

			free(buff);
		}
	}

	return 0;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);

	if (size > MAX_BLOCK_SIZE) {
		errno = EINVAL;
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		return -EINVAL;
	}

	struct inode *inode = inode_find_by_path(path);

	if (inode == NULL) {
		errno = ENOENT;
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		return -ENOENT;
	}

	memset(fs.data_blocks[inode->block_id] + size, 0, MAX_BLOCK_SIZE - size);

	inode->content_size = size;
	inode->mtime = time(NULL);

	return 0;
}


static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	struct inode *inode = inode_find_by_path(path);

	if (inode == NULL) {
		errno = ENOENT;
		fprintf(stderr,
		        "[debug] Cannot access %s: %s\n",
		        path,
		        strerror(errno));
		return -errno;
	}

	if ((inode->mode & __S_IFREG) != __S_IFREG) {
		errno = EISDIR;
		fprintf(stderr, "[debug] %s: %s\n", path, strerror(errno));
		return -errno;
	}

	if (offset + size > inode->content_size)
		size = inode->content_size - offset;

	size = size > 0 ? size : 0;

	memcpy(buffer, fs.data_blocks[inode->block_id] + offset, size);

	inode->time = time(NULL);

	return size;
}

static int
fisopfs_write(const char *path,
              const char *data,
              size_t size_data,
              off_t offset,
              struct fuse_file_info *fuse_info)
{
	printf("[debug] fisopfs_write - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size_data);

	struct inode *inode = inode_find_by_path(path);


	if (inode == NULL) {
		errno = ENOENT;
		fprintf(stderr,
		        "[debug] Cannot access %s: %s\n",
		        path,
		        strerror(errno));
		return -errno;
	}

	if ((inode->mode & __S_IFREG) != __S_IFREG) {
		errno = EISDIR;
		fprintf(stderr, "[debug] %s: %s\n", path, strerror(errno));
		return -errno;
	}

	if (inode->block_id == -1) {
		errno = ENOMEM;
		fprintf(stderr,
		        "[debug] Error writing inode: %s\n",
		        strerror(errno));
		return -errno;
	}

	if (offset > fs.sb.block_size) {
		errno = ENOMEM;
		fprintf(stderr, "[debug] %s: %s\n", path, strerror(errno));
		return -errno;
	}

	if (offset > inode->content_size) {
		offset = (off_t) inode->content_size;
	}

	if (offset + size_data > fs.sb.block_size) {
		size_data = fs.sb.block_size - offset;
	}

	memcpy(fs.data_blocks[inode->block_id] + offset, data, size_data);

	inode->content_size += size_data;
	inode->time = time(NULL);
	inode->mtime = time(NULL);

	return size_data;
}


void
init_root(void)
{
	struct inode *inode = fs.inodes[0];
	inode->block_id = 0;
	time_t t = time(NULL);
	inode->time = t;
	inode->mtime = t;
	inode->ctime = t;
	inode->gid = getgid();
	strcpy(inode->path, "/");
	inode->mode = __S_IFDIR | 0755;
	inode->uid = 1717;
	inode->link_count = 2;

	bitmap_set_block(&fs.imap, 0);
	bitmap_set_block(&fs.dmap, 0);
}

void
fs_init()
{
	memset(&fs, 0, sizeof fs);

	fs.sb.block_size = MAX_BLOCK_SIZE;
	fs.sb.data_count = DATA_BLOCKS;
	fs.sb.inode_size = sizeof(struct inode);
	fs.sb.inodes_count = INODE_BLOCKS * MAX_INODES_PER_BLOCK;

	bitmap_init(&fs.imap, fs.sb.inodes_count);

	bitmap_init(&fs.dmap, fs.sb.data_count);

	for (size_t i = 0; i < INODE_BLOCKS; i++) {
		fs.inodes[i] = calloc(MAX_INODES_PER_BLOCK, sizeof(struct inode));
	}

	fs.data_blocks = malloc(DATA_BLOCKS * sizeof(char *));
	memset(fs.data_blocks, 0, DATA_BLOCKS * sizeof(char *));

	for (size_t i = 0; i < DATA_BLOCKS; i++) {
		fs.data_blocks[i] = malloc(MAX_BLOCK_SIZE * sizeof(char));
		memset(fs.data_blocks[i], 0, MAX_BLOCK_SIZE * sizeof(char));
	}

	init_root();
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] Initializing filesystem.\n");

	FILE *file = fopen(FISOPFS_FILE, "r");

	fs_init();

	if (file) {
		printf("[debug] loading bitmaps\n");
		bitmap_load(&fs.imap, file);
		bitmap_load(&fs.dmap, file);

		printf("[debug] loading inodes\n");
		inode_load_all(file);

		printf("[debug] loading data blocks\n");

		for (int i = 0; i < DATA_BLOCKS; ++i) {
			int data_block_read =
			        fread(fs.data_blocks[i], MAX_BLOCK_SIZE, 1, file);

			if (data_block_read != 1) {
				errno = EIO;
				fprintf(stderr,
				        "[debug] Error loading data blocks: "
				        "%s\n",
				        strerror(errno));
				fclose(file);
				exit(-errno);
			}
		}
		fclose(file);

	} else {
		printf("[debug] file.fisopfs: No such file or directory, a "
		       "clean fs is set.\n");
	}

	return &fs;
}

int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush(%s)\n", path);

	FILE *file = fopen(FISOPFS_FILE, "w");
	if (!file) {
		errno = ENOENT;
		fprintf(stderr,
		        "[debug] Error saving filesystem in %s: %s\n",
		        FISOPFS_FILE,
		        strerror(errno));
		return -ENOENT;
	}

	printf("[debug] flushing bitmaps\n");
	bitmap_save(&fs.imap, file);
	bitmap_save(&fs.dmap, file);

	printf("[debug] flushing inodes\n");
	inode_save_all(file);

	printf("[debug] flushing data blocks\n");
	for (int i = 0; i < DATA_BLOCKS; ++i) {
		int data_block_write =
		        fwrite(fs.data_blocks[i], MAX_BLOCK_SIZE, 1, file);

		if (data_block_write != 1) {
			errno = EIO;
			fprintf(stderr,
			        "[debug] Error saving data blocks: "
			        "%s\n",
			        strerror(errno));
			fclose(file);
			exit(-errno);
		}
	}

	fflush(file);
	fclose(file);
	return 0;
}

void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisop_destroy\n");

	fisopfs_flush(FISOPFS_FILE, NULL);

	bitmap_free(&fs.imap);
	bitmap_free(&fs.dmap);

	for (size_t i = 0; i < INODE_BLOCKS; i++) {
		free(fs.inodes[i]);
	}

	for (int i = 0; i < DATA_BLOCKS; ++i) {
		free(fs.data_blocks[i]);
	}
	free(fs.data_blocks);
}

int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *finfo)
{
	printf("[debug] fisopfs_create - path: %s\n", path);


	if (strlen(path) > MAX_PATH_LENGTH) {
		errno = ENAMETOOLONG;
		fprintf(stderr, "[debug] Error new_inode: %s\n", strerror(errno));
		return -errno;
	}

	size_t inode_idx = bitmap_get_first_free(fs.imap, fs.sb.inodes_count);
	size_t data_idx = bitmap_get_first_free(fs.dmap, fs.sb.data_count);

	if (inode_idx > fs.sb.inodes_count) {
		errno = ENOSPC;
		fprintf(stderr, "[debug] Error new_inode: %s\n", strerror(errno));
		return -errno;
	}

	if (data_idx > fs.sb.data_count) {
		errno = ENOSPC;
		fprintf(stderr,
		        "[debug] Error new_data_block: %s\n",
		        strerror(errno));
		return -errno;
	}

	struct inode *inode = inode_find_by_index(inode_idx);

	inode->block_id = (int64_t) data_idx;
	inode->uid = getuid();
	inode->gid = getgid();
	strcpy(inode->path, path);
	time_t t = time(NULL);
	inode->time = t;
	inode->ctime = t;
	inode->mtime = t;
	inode->mode = mode | __S_IFREG | 0644;
	inode->link_count = 1;

	bitmap_set_block(&fs.imap, inode_idx);
	bitmap_set_block(&fs.dmap, data_idx);

	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	struct inode *inode = inode_find_by_path(path);

	if (inode == NULL) {
		errno = ENOENT;
		fprintf(stderr, "[debug] Error utimens: %s\n", strerror(errno));
		return -ENOENT;
	}

	inode->time = tv[0].tv_sec;
	inode->mtime = tv[1].tv_sec;

	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	if (strlen(path) > MAX_PATH_LENGTH) {
		errno = ENAMETOOLONG;
		fprintf(stderr, "[debug] Error new_inode: %s\n", strerror(errno));
		return -errno;
	}

	size_t inode_idx = bitmap_get_first_free(fs.imap, fs.sb.inodes_count);
	size_t data_idx = bitmap_get_first_free(fs.dmap, fs.sb.data_count);

	if (inode_idx > fs.sb.inodes_count) {
		errno = ENOSPC;
		fprintf(stderr, "[debug] Error new_inode: %s\n", strerror(errno));
		return -errno;
	}

	if (data_idx > fs.sb.data_count) {
		errno = ENOSPC;
		fprintf(stderr,
		        "[debug] Error new_data_block: %s\n",
		        strerror(errno));
		return -errno;
	}

	struct inode *inode = inode_find_by_index(inode_idx);

	inode->block_id = (int64_t) data_idx;
	inode->uid = getuid();
	inode->gid = getgid();
	strcpy(inode->path, path);
	time_t t = time(NULL);
	inode->time = t;
	inode->ctime = t;
	inode->mtime = t;
	inode->mode = mode | __S_IFDIR | 0755;
	inode->link_count = 2;

	bitmap_set_block(&fs.imap, inode_idx);
	bitmap_set_block(&fs.dmap, data_idx);

	return 0;
}


static int
fisopfs_rmdir(const char *path)
{
	if (strlen(path) > MAX_PATH_LENGTH) {
		errno = ENAMETOOLONG;
		fprintf(stderr,
		        "[debug] Error removing inode: %s\n",
		        strerror(errno));
		return -errno;
	}

	struct inode *inode = inode_find_by_path(path);

	if (inode == NULL) {
		errno = ENOENT;
		fprintf(stderr,
		        "[debug] Error removing inode: %s\n",
		        strerror(errno));
		return -errno;
	}

	if ((inode->mode & __S_IFDIR) != __S_IFDIR) {
		errno = ENOTDIR;
		fprintf(stderr,
		        "[debug] Error removing inode: %s\n",
		        strerror(errno));
		return -errno;
	}

	if (inode_dir_not_empty(inode)) {
		errno = ENOTEMPTY;
		fprintf(stderr,
		        "[debug] rmdir: failed to remove %s: %s\n",
		        path,
		        strerror(errno));
		return -errno;
	}

	size_t inode_idx = inode_find_index(inode);

	bitmap_free_block(&fs.imap, inode_idx);
	bitmap_free_block(&fs.dmap, inode->block_id);

	memset(inode, 0, sizeof(struct inode));


	return 0;
}

static int
fisopfs_unlink(const char *path)
{
	struct inode *inode = inode_find_by_path(path);
	size_t inode_idx = inode_find_index(inode);

	if (inode == NULL) {
		errno = ENOENT;
		fprintf(stderr,
		        "[debug] rm: cannot remove: %s : %s\n",
		        path,
		        strerror(errno));
		return -errno;
	}

	if ((inode->mode & __S_IFREG) != __S_IFREG) {
		errno = EISDIR;
		fprintf(stderr,
		        "[debug] rm: cannot remove: %s : %s\n",
		        path,
		        strerror(errno));
		return -errno;
	}

	bitmap_free_block(&fs.imap, inode_idx);
	bitmap_free_block(&fs.dmap, inode->block_id);

	memset(fs.data_blocks[inode->block_id], 0, MAX_BLOCK_SIZE);
	memset(inode, 0, sizeof(struct inode));


	return 0;
}


static struct fuse_operations operations = { .getattr = fisopfs_getattr,
	                                     .readdir = fisopfs_readdir,
	                                     .read = fisopfs_read,
	                                     .mkdir = fisopfs_mkdir,
	                                     .init = fisopfs_init,
	                                     .write = fisopfs_write,
	                                     .create = fisopfs_create,
	                                     .rmdir = fisopfs_rmdir,
	                                     .flush = fisopfs_flush,
	                                     .destroy = fisopfs_destroy,
	                                     .truncate = fisopfs_truncate,
	                                     .utimens = fisopfs_utimens,
	                                     .unlink = fisopfs_unlink };

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
