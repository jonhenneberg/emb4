obj-m := ofcd.o

PWD   := $(shell pwd)
KDIR  := ${PWD}/..

default:
	make -C $(KDIR) SUBDIRS=$(PWD) modules
