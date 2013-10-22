# Makefile for compiling Kernel
# modules on the fly.

obj-m += gpio-irq-test.o

PWD   := $(shell pwd)
KDIR  := ${PWD}/..

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean

