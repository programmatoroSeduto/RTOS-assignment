#! /bin/bash

# clear
echo "mounting the module..."

insmod ./RTOS_module_GF.ko

echo "mounting the module... done. " 
echo $(lsmod | grep RTOS_module_GF)

major_number=$(echo $(awk '/RTOS_module_GF/ {print $1}' /proc/devices) | sed -r 's/^([^.]+).*$/\1/; s/^[^0-9]*([0-9]+).*$/\1/')
echo "-> Major Number is $major_number"
mknod /dev/RTOS_module_GF_c c $major_number 0

echo "   ->   ready!   <-"
