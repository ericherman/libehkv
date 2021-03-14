/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* ehkv.h: a simple key-value table */
/* Copyright (C) 2021 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehkv */

#include "ehkv.h"
#include <stdint.h>

#ifndef ehkv_memcmp
int memcmp(const void *s1, const void *s2, size_t n);
#define ehkv_memcmp(s1, s2, n) memcmp(s1, s2, n)
#endif /* ehkv_memcmp */

size_t ehkv_pos(struct ehkv *table, const void *key, size_t key_len)
{
	for (size_t i = 0; i < table->entries_used; ++i) {
		if (key_len == table->key_lengths[i]) {
			if (0 == ehkv_memcmp(key, table->keys[i], key_len)) {
				return i;
			}
		}
	}
	return SIZE_MAX;
}

int ehkv_full(struct ehkv *table)
{
	return (table->entries_used < table->entries_len) ? 0 : 1;
}

/* returns 1 if there was NOT room to insert the value, and does not update
 * the table, nor populate the 'previous' struct.
 * returns 0 if there was room to insert the value, and populates the
 * 'previous' struct with the buffers previously associated with that key. */
int ehkv_put(struct ehkv *table, const void *key, size_t key_len, void *val,
	     struct ehkv_entry *previous)
{
	size_t pos = ehkv_pos(table, key, key_len);

	if (pos == SIZE_MAX) {
		if (ehkv_full(table)) {
			int error = 1;
			return error;
		}

		if (previous) {
			previous->key = NULL;
			previous->key_len = 0;
			previous->value = NULL;
			previous->index = SIZE_MAX;
		}

		pos = table->entries_used++;
	} else {
		if (previous) {
			previous->key = table->keys[pos];
			previous->key_len = table->key_lengths[pos];
			previous->value = table->values[pos];
			previous->index = pos;
		}
	}

	table->keys[pos] = key;
	table->key_lengths[pos] = key_len;
	table->values[pos] = val;

	int error = 0;
	return error;
}

void *ehkv_get(struct ehkv *table, const void *key, size_t key_len,
	       size_t *index)
{
	size_t pos = ehkv_pos(table, key, key_len);
	if (index) {
		*index = pos;
	}
	return (pos == SIZE_MAX) ? NULL : table->values[pos];
}

void *ehkv_del(struct ehkv *table, const void *key, size_t key_len,
	       struct ehkv_entry *previous)
{
	size_t pos = ehkv_pos(table, key, key_len);

	if (pos == SIZE_MAX) {
		if (previous) {
			previous->key = NULL;
			previous->key_len = 0;
			previous->value = NULL;
			previous->index = SIZE_MAX;
		}
		return NULL;
	}

	void *was = table->values[pos];

	if (previous) {
		previous->key = table->keys[pos];
		previous->key_len = table->key_lengths[pos];
		previous->value = was;
		previous->index = pos;
	}

	if (table->entries_used < 2) {
		table->keys[pos] = NULL;
		table->key_lengths[pos] = 0;
		table->values[pos] = NULL;
	} else {
		size_t end = table->entries_used - 1;

		table->keys[pos] = table->keys[end];
		table->key_lengths[pos] = table->key_lengths[end];
		table->values[pos] = table->values[end];

		table->keys[end] = NULL;
		table->key_lengths[end] = 0;
		table->values[end] = NULL;
	}

	--table->entries_used;

	return was;
}
