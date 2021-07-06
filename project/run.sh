#! /bin/bash

./build.sh
cd module
make
# cd ..

# cd module
./mount.sh
cd ..

# run the application
./main | tee log.log

# dismount the module
cd module
./unmount.sh
cd ..

# dmesg
