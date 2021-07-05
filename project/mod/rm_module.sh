#! /bin/bash

clear

set -e
module = "module_main"

# rm module
rmmod $module

# delete the only special file created for the module
rm /dev/RTOS_$module
