#! /bin/bash
set -e # halt script on error

gcc -c main.c
gcc -c timespectools.c
gcc -c module_interface.c
gcc -c tasks.c

gcc -o main main.o timespectools.o tasks.o module_interface.o -pthread

rm main.o timespectools.o tasks.o module_interface.o

# module test
cd module/test
gcc module_test_write.c -o module_test_write
gcc module_test_read.c -o module_test_read
gcc module_test_read_write.c -o module_test_read_write
cd ../..
