CC:=gcc
CCFLAGS:=-std=c11 -O2 -g

ROOT:=$(shell pwd)
OUTPUT:=$(ROOT)/bin


SRCS:=$(wildcard *.c)

.PHONY: run deb all tmp tmp2 clean

all: $(OUTPUT)/zstree

run: clean $(OUTPUT)/zstree
	$(OUTPUT)/zstree

deb: $(OUTPUT)/zstree
	gdb $(OUTPUT)/zstree

tmp:
	$(CC) $(CCFLAGS) -o $(OUTPUT)/tmp $(ROOT)/tmp/tmp.c
	$(OUTPUT)/tmp

tmp2:
	$(CC) $(CCFLAGS) -o $(OUTPUT)/tmp2 $(ROOT)/tmp/tmp2.c
	$(OUTPUT)/tmp2 -h qwe azxc -hhh

$(OUTPUT)/zstree: $(SRCS)
	mkdir -p $(OUTPUT)
	$(CC) $(CCFLAGS) -o $@ $^

clean:
	rm -rf $(OUTPUT)
