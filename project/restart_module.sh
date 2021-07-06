#! /bin/bash

cd module
./unmount.sh
make
./mount.sh
cd ..
