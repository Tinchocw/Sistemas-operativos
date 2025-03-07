#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdint.h>
#include <sys/types.h>

/// FS macros
#define MAX_PATH_LENGTH 200
#define MAX_BLOCK_SIZE 4096
#define MAX_DIRECTORY_SIZE 1024
#define MAX_FS_BLOCKS 128


struct inode {
	uid_t uid;
	gid_t gid;
	size_t content_size;
	char path[MAX_PATH_LENGTH];
	mode_t mode;
	uint32_t link_count;
	time_t time;   // Last access time
	time_t ctime;  // Creation time
	time_t mtime;  // Modification time
	int64_t block_id;

};  // Inode size --> 256 bytes

// log_2(sizeof(inode)) must be an integer
#define MAX_INODES_PER_BLOCK (MAX_BLOCK_SIZE / sizeof(struct inode))
#define INODE_BLOCKS (128 / MAX_INODES_PER_BLOCK)
#define DATA_BLOCKS (MAX_FS_BLOCKS - INODE_BLOCKS - 3)  // 1 superb + 2 bmap


struct bmap {
	uint8_t *bit;
	size_t bound;  // bit array length
};                     // size --> bound + sizeof(unsigned int)


struct superblock {
	size_t block_size;
	size_t inode_size;
	uint8_t inodes_count;
	uint8_t data_count;
};

struct file_system {
	struct superblock sb;
	struct bmap imap;
	struct bmap dmap;
	struct inode *inodes[INODE_BLOCKS];
	char **data_blocks;
};


#endif  // STRUCTURES_H
