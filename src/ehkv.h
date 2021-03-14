/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* ehkv.h: a simple key-value table */
/* Copyright (C) 2021 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehkv */

#ifndef EHKV_H
#define EHKV_H

#ifdef __cplusplus
#define Ehkv_begin_C_functions extern "C" {
#define Ehkv_end_C_functions }
#else
#define Ehkv_begin_C_functions
#define Ehkv_end_C_functions
#endif

Ehkv_begin_C_functions
#undef Ehkv_begin_C_functions
#include <stddef.h>		/* size_t */
    struct ehkv;
/* The struct ehkv is a key-value table of a fixed size.
 * The ehkv functions assume the following are always true:
 * 1) the 'keys' is an array of length 'entries_len'
 * 2) the 'values' is an array of length 'entries_len'
 * 3) the 'entries_used' indicates the number of occupied keys.
 * 4) each occupied value in 'keys' array has a corresponding length in
 *    the 'key_lengths' array
 *
 * The ehkv functions do no memory allocation or de-allocation.
 * The caller is responsible for allocating/freeing each key buffer.
 * The 'ehkv_put' and 'ehkv_del' have optional 'previous' pointers.
 */
struct ehkv {
	size_t entries_len;
	size_t entries_used;
	const void **keys;
	size_t *key_lengths;
	void **values;
};

struct ehkv_entry {
	const void *key;	/* caller may need to free(key) */
	size_t key_len;
	void *value;		/* caller may need to free(value) */
	size_t index;		/* SIZE_MAX == not present */
};

/* returns 1 if there was NOT room to insert the value, and does not update
 * the table, nor populate the 'previous' struct.
 * returns 0 if there was room to insert the value, and populates the
 * 'previous' struct with the buffers previously associated with that key. */
int ehkv_put(struct ehkv *table, const void *key, size_t key_len, void *val,
	     struct ehkv_entry *previous);

void *ehkv_get(struct ehkv *table, const void *key, size_t key_len,
	       size_t *index);

void *ehkv_del(struct ehkv *table, const void *key, size_t key_len,
	       struct ehkv_entry *previous);

/* returns SIZE_MAX if not found or the index of the key if present.
 * NOTE: this index might change if the 'ehkv_del' function is called. */
size_t ehkv_pos(struct ehkv *table, const void *key, size_t key_len);

/* return (table->entries_used < table->entries_len) ? 0 : 1; */
int ehkv_full(struct ehkv *table);

Ehkv_end_C_functions
#undef Ehkv_end_C_functions
#endif /* EHKV_H */
