#ifndef INODE_H
#define INODE_H

#include <sys/types.h>
#include <stdio.h>

extern struct file_system fs;

struct inode *inode_find_by_index(size_t idx);

char *inode_get_file_name(struct inode *inode);

char *inode_get_dir_name(struct inode *inode);

struct inode *inode_find_by_path(const char *path);

char *inode_get_dir_path(struct inode *inode);

size_t inode_find_index(const struct inode *inode);

int inode_dir_not_empty(struct inode *inode);

void inode_load_all(FILE *file);

void inode_save_all(FILE *file);

#endif