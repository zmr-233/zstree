CC:=gcc
CCFLAGS:=-std=c11 -O2 -g

ROOT:=$(shell pwd)
OUTPUT:=$(ROOT)/bin


SRCS:=$(wildcard *.c)

.PHONY: run deb all

all: $(OUTPUT)/zstree

run: $(OUTPUT)/zstree
	$(OUTPUT)/zstree

deb: $(OUTPUT)/zstree
	gdb $(OUTPUT)/zstree


$(OUTPUT)/zstree: $(SRCS)
	mkdir -p $(OUTPUT)
	$(CC) $(CCFLAGS) -o $@ $^
