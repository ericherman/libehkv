# SPDX-License-Identifier: LGPL-3.0-or-later
# Makefile: build instructions for a key-value table
# Copyright (C) 2021 Eric Herman <eric@freesa.org>
# https://github.com/ericherman/libehkv

# $@ : target label
# $< : the first prerequisite after the colon
# $^ : all of the prerequisite files
# $* : wildcard matched part
# Target-specific Variable syntax:
# https://www.gnu.org/software/make/manual/html_node/Target_002dspecific.html
#
# patsubst : $(patsubst pattern,replacement,text)
#       https://www.gnu.org/software/make/manual/html_node/Text-Functions.html


SHELL=/bin/bash
BROWSER=firefox

CSTD_CFLAGS=-std=c11
NOISY_CFLAGS=-Wall -Wextra -pedantic -Werror

COMMON_CFLAGS=$(CSTD_CFLAGS) $(NOISY_CFLAGS) -g -I./src -fPIC -pipe $(CFLAGS)

BUILD_CFLAGS=-O2 -DNDEBUG $(COMMON_CFLAGS)

DEBUG_CFLAGS=-O0 -DDEBUG $(COMMON_CFLAGS) \
	-fno-inline-small-functions \
	-fkeep-inline-functions \
	-fkeep-static-functions \
	--coverage

VALGRIND=$(shell which valgrind)

# extracted from https://github.com/torvalds/linux/blob/master/scripts/Lindent
LINDENT=indent -npro -kr -i8 -ts8 -sob -l80 -ss -ncs -cp1 -il0

default: all

demos: build/all-on-stack build/token-frequency

all: demos check

build/ehkv.o: src/ehkv.c src/ehkv.h
	mkdir -pv build
	echo "BUILD_CFLAGS=$(BUILD_CFLAGS)"
	$(CC) -c $(BUILD_CFLAGS) $< -o $@

debug/ehkv.o: src/ehkv.c src/ehkv.h
	mkdir -pv debug
	$(CC) -c $(DEBUG_CFLAGS) $< -o $@

build/all-on-stack: demos/all-on-stack.c src/ehkv.c src/ehkv.h
	mkdir -pv build
	$(CC) $(BUILD_CFLAGS) $^ -o $@

debug/all-on-stack: demos/all-on-stack.c src/ehkv.c src/ehkv.h
	mkdir -pv debug
	$(CC) $(DEBUG_CFLAGS) $^ -o $@

build/token-frequency: demos/token-frequency.c src/ehkv.c src/ehkv.h
	mkdir -pv build
	$(CC) $(BUILD_CFLAGS) $^ -o $@

debug/token-frequency: demos/token-frequency.c src/ehkv.c src/ehkv.h
	mkdir -pv debug
	$(CC) $(DEBUG_CFLAGS) $^ -o $@

build/test_ehkv_put_get_del: tests/test_ehkv_put_get_del.c build/ehkv.o
	mkdir -pv build
	$(CC) $(BUILD_CFLAGS) $^ -o $@ $(LDFLAGS)

debug/test_ehkv_put_get_del: tests/test_ehkv_put_get_del.c debug/ehkv.o
	mkdir -pv debug
	$(CC) $(DEBUG_CFLAGS) $^ -o $@ $(LDFLAGS)

check-bin: build/test_ehkv_put_get_del
	build/test_ehkv_put_get_del
	@echo "SUCCESS! ($@)"

check-debug: debug/test_ehkv_put_get_del
	debug/test_ehkv_put_get_del
	@echo "SUCCESS! ($@)"

check-code-coverage: html-report
	# expect two: one for lines, one for functions
	if [ $$(grep -c 'headerCovTableEntryHi">100.0' \
		./coverage_html/src/ehkv.c.gcov.html) -eq 2 ]; \
		then true; else false; fi
	@echo "SUCCESS! ($@)"

check: check-bin check-code-coverage
	@echo "SUCCESS! ($@)"

line-cov: check-debug
	lcov    --checksum \
		--capture \
		--base-directory . \
		--directory . \
		--output-file coverage.info

html-report: line-cov
	mkdir -pv ./coverage_html
	genhtml coverage.info --output-directory coverage_html

coverage: html-report
	$(BROWSER) ./coverage_html/src/index.html


tidy:
	$(LINDENT) \
		-T FILE \
		-T size_t \
		-T ehkv \
		-T ehkv_entry \
		`find src tests demos -name '*.h' -o -name '*.c'`

clean:
	rm -rvf build debug `cat .gitignore | sed -e 's/#.*//'`
	pushd src; rm -rvf `cat ../.gitignore | sed -e 's/#.*//'`; popd
	pushd tests; rm -rvf `cat ../.gitignore | sed -e 's/#.*//'`; popd

mrproper:
	git clean -dffx
	git submodule foreach --recursive git clean -dffx
