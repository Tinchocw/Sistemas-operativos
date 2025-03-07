#ifndef BITMAP_H
#define BITMAP_H

#include <sys/types.h>
#include <stdio.h>

size_t bitmap_size(const struct bmap bmap);

size_t bitmap_block_is_free(const struct bmap bmap, size_t idx);

size_t bitmap_set_block(struct bmap *bmap, size_t idx);

size_t bitmap_free_block(struct bmap *bmap, size_t idx);

size_t bitmap_get_first_free(const struct bmap bmap, const size_t blocks_q);

void bitmap_free(struct bmap *bmap);

void bitmap_init(struct bmap *bmap, const size_t q);

void bitmap_load(struct bmap *bmap, FILE *file);

void bitmap_save(const struct bmap *bmap, FILE *file);

#endif
