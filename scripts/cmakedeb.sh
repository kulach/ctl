#!/bin/bash

COMMAND="rm CMakeCache.txt; CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Debug -DADDRESS_SANITIZER=TRUE -DAVX2=TRUE -DPERF=FALSE ."
echo $COMMAND
eval $COMMAND
