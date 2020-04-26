#!/bin/bash

if [ -x "$(command -v clang++)" ]; then
    CXX=clang++
else
    CXX=g++
fi

PYTHON_INCLUDE="$(python3-config --includes | cut -d ' ' -f 1)"
PYTHON_LIB="${PYTHON_INCLUDE##*/}"
BOOST_LIB="$(echo "boost_$PYTHON_LIB" | tr --delete .m)"

$CXX                            \
    -std=c++17 -Wall            \
    -Ofast -march=native -fPIC  \
    -l$BOOST_LIB -l$PYTHON_LIB  \
    $PYTHON_INCLUDE             \
    -shared                     \
    -o bytepair.so              \
    bytepair.cpp
