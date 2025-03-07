#include "structures.h"
#include "bmap.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <errno.h>

size_t
bitmap_size(const struct bmap bmap)
{
	return sizeof(bmap.bit) + sizeof(bmap.bound) +
	       sizeof(uint8_t) * bmap.bound;
}

/*
    Returns 0 if block_id is free
*/
size_t
bitmap_block_is_free(const struct bmap bmap, size_t idx)
{
	if (idx >= bmap.bound * 8) {
		return -1;
	}

	size_t block = idx / bmap.bound;
	idx = idx % bmap.bound;

	uint8_t mask = 1 << idx;

	return bmap.bit[block] & mask;
}

size_t
bitmap_set_block(struct bmap *bmap, size_t idx)
{
	if (idx >= bmap->bound) {
		return -1;
	}

	size_t block = idx / bmap->bound;
	idx = idx % bmap->bound;

	uint8_t mask = 1 << idx;

	bmap->bit[block] = bmap->bit[block] | mask;
	return 0;
}

size_t
bitmap_free_block(struct bmap *bmap, size_t idx)
{
	if (idx >= bmap->bound) {
		return -1;
	}

	size_t block = idx / bmap->bound;
	idx = idx % bmap->bound;

	uint8_t mask = 1 << idx;

	bmap->bit[block] = bmap->bit[block] ^ mask;
	return 0;
}

/*
    if a block_id is free returns < bitmap->bound * 8
    else returns bitmap->bound * 8
*/
size_t
bitmap_get_first_free(const struct bmap bmap, const size_t blocks_q)
{
	for (size_t i = 0; i < bmap.bound; i++) {
		uint8_t mask = 1;

		for (size_t j = 0; j < blocks_q; j++) {
			if ((bmap.bit[i] & mask) == 0) {
				return (i * 8) + j;
			}
			mask = mask << 1;
		}
	}

	return bmap.bound * 8;
}

void
bitmap_free(struct bmap *bmap)
{
	free(bmap->bit);
	bmap->bit = 0;
}

void
bitmap_init(struct bmap *bmap, const size_t q)
{
	memset(bmap, 0, sizeof(struct bmap));
	bmap->bound = (size_t) ceilf(q / 8.0f);
	bmap->bit = (uint8_t *) calloc(bmap->bound, sizeof(uint8_t));
}


void
bitmap_load(struct bmap *bmap, FILE *file)
{
	if (fread(bmap->bit, sizeof(uint8_t), bmap->bound, file) <= 0) {
		errno = EIO;
		fprintf(stderr,
		        "[debug] Error loading bitmap: %s\n",
		        strerror(errno));
		fclose(file);
		exit(-errno);
	}
}

void
bitmap_save(const struct bmap *bmap, FILE *file)
{
	if (fwrite(bmap->bit, sizeof(uint8_t), bmap->bound, file) <= 0) {
		errno = EIO;
		fprintf(stderr,
		        "[debug] Error saving bitmap: %s\n",
		        strerror(errno));
		fclose(file);
		exit(-errno);
	};
}
