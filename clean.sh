#!/bin/bash

make clean
make
rm output
rm volD
rm volC
cp volCCopy volC
cp volDCopy volD
./test_file
