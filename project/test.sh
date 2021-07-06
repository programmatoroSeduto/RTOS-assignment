#! /bin/bash

clear

# build program and module
./build
cd module
make
cd ..

# mount the module
cd module
./mount.sh
./mount.sh
cd ..

# start the test program
./module_test_read_write [2 1] [1 [2 [3 2] 3] 1]

# dismount the module
cd module
./unmount.sh
cd ..
