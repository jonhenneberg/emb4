obj-m += ofcd.o

PWD   := $(shell pwd)
KDIR  := ${PWD}/../..

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean

