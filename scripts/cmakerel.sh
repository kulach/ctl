#!/bin/bash

COMMAND="rm CMakeCache.txt; CC=clang CXX=clang++ cmake -DCMAKE_BUILD_TYPE=Release -DADDRESS_SANITIZER=FALSE -DAVX2=TRUE -DPERF=FALSE ."
echo $COMMAND
eval $COMMAND
