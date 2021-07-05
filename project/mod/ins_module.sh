#! /bin/bash

clear

set -e
module = "module_main"

# load the module
insmod module_main.ko

# retrieve the major number
major = $(awk "\\$2 == \"$module\" {print \\$1}" /proc/devices)

# create the channel
mknod /dev/RTOS_$module c $major 1
