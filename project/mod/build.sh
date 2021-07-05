#! /bin/bash

set -e

rm .module_main.ko.cmd .module_main.mod.cmd .module_main.mod.o.cmd .module_main.o.cmd module_main.ko module_main.mod module_main.mod.c module_main.mod.o module_main.o modules.order Module.symvers
clear
make
