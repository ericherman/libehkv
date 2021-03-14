/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* demos/token-frequency.c: demo of a simple key-value table */
/* Copyright (C) 2021 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehkv */

#include "ehkv.h"

#include <errno.h>		/* errno strerror */
#include <stdint.h>
#include <stdio.h>		/* fprintf fscanf printf */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* strlen */

static void *calloc_or_die(size_t n, size_t size, const char *file, int line);
#define Calloc_or_die(n, size) \
	calloc_or_die(n, size, __FILE__, __LINE__)

static void *realloc_or_die(void *p, size_t size, const char *file, int line);
#define Realloc_or_die(ptr, size) \
	realloc_or_die(ptr, size, __FILE__, __LINE__);

static void grow_table_or_die(struct ehkv *table);

static void ensure_counter_for_key(struct ehkv *table, const char *str);

int main(int argc, char **argv)
{
	char *file_name = (argc > 1) ? argv[1] : "COPYING";
	int threshold = (argc > 2) ? atoi(argv[2]) : 64;

	printf("finding words in %s which appear %d or more times\n", file_name,
	       threshold);

	size_t max_keys = 1024;
	struct ehkv *table = Calloc_or_die(1, sizeof(struct ehkv));
	table->entries_len = max_keys;
	table->entries_used = 0;
	table->keys = Calloc_or_die(max_keys, sizeof(void *));
	table->key_lengths = Calloc_or_die(max_keys, sizeof(size_t));
	table->values = Calloc_or_die(max_keys, sizeof(void *));

	errno = 0;
	FILE *file = fopen(file_name, "r");
	if (!file) {
		int save_errno = errno;
		fflush(stdout);
		fprintf(stderr, "%s: %s\n", strerror(save_errno), file_name);
		return 2;

	}
	while (!feof(file)) {
		char buf[80];
		buf[0] = '\0';
		if (!fscanf(file, "%79s", buf)) {
			continue;
		}

		ensure_counter_for_key(table, buf);

		int *counter = ehkv_get(table, buf, strlen(buf), NULL);

		++*counter;
	}
	fclose(file);

	printf("of the %zu unique words found:\n", table->entries_used);

	for (size_t i = 0; i < table->entries_used; ++i) {
		const char *str = table->keys[i];
		int *counter = table->values[i];
		if (*counter >= threshold) {
			printf("%4u: %s\n", *counter, str);
		}
	}

	while (table->entries_used) {
		size_t i = table->entries_used - 1;
		struct ehkv_entry entry;
		ehkv_del(table, table->keys[i], table->key_lengths[i], &entry);
		free((void *)entry.key);
		free(entry.value);
	}
	free(table->values);
	free(table->key_lengths);
	free(table->keys);
	free(table);

	return 0;
}

static void grow_table_or_die(struct ehkv *table)
{
	const double growth_factor = 1.25;
	size_t old_max = table->entries_len;
	size_t new_max = old_max * growth_factor;

	table->entries_len = new_max;

	size_t size = new_max * sizeof(void *);
	table->keys = Realloc_or_die(table->keys, size);
	table->values = Realloc_or_die(table->values, size);

	size = new_max * sizeof(size_t);
	table->key_lengths = Realloc_or_die(table->key_lengths, size);
}

static void ensure_counter_for_key(struct ehkv *table, const char *str)
{
	size_t key_len = strlen(str);

	size_t idx = ehkv_pos(table, str, key_len);
	if (idx == SIZE_MAX) {

		size_t size = key_len + 1;
		char *key = Calloc_or_die(1, size);
		strncpy(key, str, size);

		int *counter = Calloc_or_die(1, sizeof(int));

		if (ehkv_full(table)) {
			grow_table_or_die(table);
		}

		struct ehkv_entry *previous = NULL;
		ehkv_put(table, key, key_len, counter, previous);
	}
}

static void *realloc_or_die(void *p, size_t size, const char *file, int line)
{

	void *ptr = realloc(p, size);
	if (ptr) {
		return ptr;
	}

	fflush(stdout);
	fprintf(stderr, "%s:%d: ", file, line);
	fprintf(stderr, "could not realloc(%p, %zu)?", p, size);
	exit(EXIT_FAILURE);
}

static void *calloc_or_die(size_t n, size_t size, const char *file, int line)
{
	void *ptr = calloc(n, size);
	if (ptr) {
		return ptr;
	}

	fflush(stdout);
	fprintf(stderr, "%s:%d: ", file, line);
	fprintf(stderr, "could not calloc(%zu, %zu) (%zu bytes)?",
		n, size, (n * size));
	exit(EXIT_FAILURE);
}
