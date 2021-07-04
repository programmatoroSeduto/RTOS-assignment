#! /bin/bash

ifneq ($(KERNELRELEASE),)
	obj-m := module_main.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif 
