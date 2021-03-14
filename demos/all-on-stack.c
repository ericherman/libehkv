/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* demo-ehht-as-array.c: demo of a simple OO hashtable */
/* Copyright (C) 2016, 2017, 2018, 2019, 2020 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehht */

#include <stdio.h>		/* fprintf fscanf printf */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* strlen */
#include <strings.h>		/* strdup */

#include "ehkv.h"

int main(void)
{

	const size_t max_keys = 5;
	const void *keys[max_keys];
	size_t key_lenghts[max_keys];
	void *vals[max_keys];
	struct ehkv table;

	table.entries_len = max_keys;
	table.entries_used = 0;
	table.keys = keys;
	table.key_lengths = key_lenghts;
	table.values = vals;

	struct ehkv_entry *prev = NULL;
	ehkv_put(&table, "foo", strlen("foo"), "bar", prev);
	ehkv_put(&table, "fizz", strlen("fizz"), "buzz", prev);
	ehkv_put(&table, "to", strlen("to"), "BAD", prev);
	ehkv_put(&table, "a", strlen("a"), "WRONG", prev);
	ehkv_put(&table, "whiz", strlen("whiz"), "BANG", prev);

	int expect_error = ehkv_put(&table, "bang", strlen("bang"), "b", prev);
	if (!expect_error) {
		fprintf(stderr, "should have run out of space\n");
	}

	/* replace existing entries */
	int err = 0;
	err += ehkv_put(&table, "to", strlen("to"), "legit", prev);
	err += ehkv_put(&table, "whiz", strlen("whiz"), "kid", prev);
	err += ehkv_put(&table, "a", strlen("a"), "okay", prev);
	if (err) {
		fprintf(stderr, "errors?\n");
	}

	for (size_t i = 0; i < table.entries_used; ++i) {
		const char *k = (const char *)table.keys[i];
		const char *v = (const char *)table.values[i];
		printf("%s => %s\n", k, v);
	}

	return err ? EXIT_FAILURE : EXIT_SUCCESS;
}
