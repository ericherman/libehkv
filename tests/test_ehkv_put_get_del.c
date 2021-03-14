/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* test_ehkv_put_get_del: test for a simple key-value table */
/* Copyright (C) 2021 Eric Herman <eric@freesa.org> */
/* https://github.com/ericherman/libehkv */

#include "ehkv.h"

#include <stdint.h>
#include <string.h>

static unsigned check_int(long a, long e, int line);
#define Check_int(a, e) \
	check_int(a, e, __LINE__)

static unsigned check_size(size_t a, size_t e, int line);
#define Check_size(a, e) \
	check_size(a, e, __LINE__)

static unsigned check_null(void *p, int line);
#define Check_null(p) \
	check_null((void *)p, __LINE__)

static unsigned check_str(const char *a, const char *e, int line);
#define Check_str(a, e) \
	check_str((const char *)a, (const char *)e, __LINE__)

unsigned test_ehkv_put_get_del(void)
{
	const size_t max_keys = 9;
	const void *keys[max_keys];
	size_t key_lenghts[max_keys];
	void *vals[max_keys];
	struct ehkv stack_table = {
		.entries_len = max_keys,
		.entries_used = 0,
		.keys = keys,
		.key_lengths = key_lenghts,
		.values = vals
	};
	struct ehkv *table = &stack_table;

	unsigned failures = 0;
	int err = 0;

	const char *key = "key1";
	failures += Check_size(ehkv_pos(table, key, strlen(key)), SIZE_MAX);
	failures += Check_size(ehkv_pos(table, key, strlen(key)), SIZE_MAX);

	struct ehkv_entry prev = { NULL, 0, NULL, 0 };
	size_t pos = 23;
	char *val = ehkv_get(table, key, strlen(key), &pos);
	failures += Check_null(val);
	failures += Check_size(pos, SIZE_MAX);

	val = "foo";
	err = ehkv_put(table, key, strlen(key), val, &prev);
	failures += Check_int(err, 0);
	failures += Check_null(prev.key);
	failures += Check_int(prev.key_len, 0);
	failures += Check_null(prev.value);
	failures += Check_size(prev.index, SIZE_MAX);
	failures += Check_size(ehkv_pos(table, key, strlen(key)), 0);

	const char *rv = ehkv_get(table, key, strlen(key), &pos);
	failures += Check_str(rv, val);
	failures += Check_int(pos, 0);

	err = ehkv_put(table, key, strlen(key), "bar", &prev);
	failures += Check_int(err, 0);
	failures += Check_str(prev.key, key);
	failures += Check_int(prev.key_len, strlen(key));
	failures += Check_str(prev.value, "foo");
	failures += Check_size(prev.index, 0);
	failures += Check_size(ehkv_pos(table, key, strlen(key)), 0);

	val = ehkv_get(table, "baz", strlen("baz"), &pos);
	failures += Check_null(val);
	failures += Check_size(pos, SIZE_MAX);

	val = ehkv_get(table, key, strlen(key), &pos);
	failures += Check_str(val, "bar");

	key = "two";
	err = ehkv_put(table, key, strlen(key), "2", &prev);
	failures += Check_int(err, 0);
	failures += Check_size(table->entries_used, 2);

	key = "three";
	err = ehkv_put(table, key, strlen(key), "3", &prev);
	failures += Check_int(err, 0);
	failures += Check_size(table->entries_used, 3);

	key = "four";
	err = ehkv_put(table, key, strlen(key), "4", &prev);
	failures += Check_int(err, 0);
	failures += Check_size(table->entries_used, 4);

	key = "ping";
	err = ehkv_put(table, key, strlen(key), "pong", &prev);
	failures += Check_int(err, 0);
	failures += Check_size(table->entries_used, 5);

	key = "whiz";
	err = ehkv_put(table, key, strlen(key), "bang", &prev);
	failures += Check_int(err, 0);
	failures += Check_size(table->entries_used, 6);

	key = "seven";
	err = ehkv_put(table, key, strlen(key), "7", &prev);
	failures += Check_int(err, 0);
	failures += Check_size(table->entries_used, 7);

	key = "eight";
	err = ehkv_put(table, key, strlen(key), "8", &prev);
	failures += Check_int(err, 0);
	failures += Check_size(table->entries_used, 8);

	key = "nine";
	err = ehkv_put(table, key, strlen(key), "9", &prev);
	failures += Check_int(err, 0);
	failures += Check_size(table->entries_used, 9);

	err = ehkv_put(table, "FULL", strlen("FULL"), "BANG", &prev);
	failures += Check_int(err, 1);
	failures += Check_size(table->entries_used, 9);

	key = "ping";
	failures += Check_size(ehkv_pos(table, key, strlen(key)), 4);
	val = ehkv_del(table, key, strlen(key), &prev);
	failures += Check_str(val, "pong");
	failures += Check_str(prev.key, "ping");
	failures += Check_size(prev.key_len, strlen("ping"));
	failures += Check_str(prev.value, "pong");
	failures += Check_size(prev.index, 4);
	failures += Check_size(ehkv_pos(table, key, strlen(key)), SIZE_MAX);
	val = ehkv_get(table, "ping", strlen(key), &pos);
	failures += Check_null(val);
	failures += Check_size(table->entries_used, 8);

	failures += Check_size(table->entries_used, 8);
	val = ehkv_del(table, "bogus", strlen("bogus"), &prev);
	failures += Check_size(table->entries_used, 8);
	failures += Check_null(val);

	ehkv_del(table, "key1", strlen("key1"), &prev);
	ehkv_del(table, "two", strlen("two"), &prev);
	ehkv_del(table, "nine", strlen("nine"), &prev);
	ehkv_del(table, "three", strlen("three"), &prev);
	ehkv_del(table, "eight", strlen("eight"), &prev);
	ehkv_del(table, "four", strlen("four"), &prev);
	ehkv_del(table, "seven", strlen("seven"), &prev);

	failures += Check_size(table->entries_used, 1);
	val = ehkv_del(table, "whiz", strlen("whiz"), &prev);
	failures += Check_str(val, "bang");
	failures += Check_size(table->entries_used, 0);

	return failures;
}

#include <stdio.h>

static unsigned check_int(long a, long e, int line)
{
	if (a == e) {
		return 0;
	}
	fflush(stdout);
	fprintf(stderr, "FAIL %s:%d: expected %ld but was %ld\n", __FILE__,
		line, e, a);
	return 1;
}

static unsigned check_size(size_t a, size_t e, int line)
{
	if (a == e) {
		return 0;
	}
	fflush(stdout);
	fprintf(stderr, "FAIL %s:%d: expected %zu but was %zu\n", __FILE__,
		line, e, a);
	return 1;
}

static unsigned check_null(void *p, int line)
{
	if (p == NULL) {
		return 0;
	}
	fflush(stdout);
	fprintf(stderr, "FAIL %s:%d: expected NULL but was %p\n", __FILE__,
		line, p);
	return 1;
}

static unsigned check_str(const char *a, const char *e, int line)
{
	if (a == NULL && e == NULL) {
		return 0;
	}
	if (a != NULL && e != NULL && (strcmp(a, e) == 0)) {
		return 0;
	}
	fflush(stdout);
	fprintf(stderr, "FAIL %s:%d: expected '%s' but was '%s'\n", __FILE__,
		line, e ? e : "(null)", a ? a : "(null)");

	return 1;
}

int main(void)
{
	unsigned failures = 0;
	failures += test_ehkv_put_get_del();
	return (failures) ? 1 : 0;
}
