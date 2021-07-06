#! /bin/bash

cd module
sudo rmmod RTOS_module_GF
rm /dev/RTOS_module_GF_c
echo "module removed. "
cd ..
