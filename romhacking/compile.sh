#!/bin/sh

if [ -x "$(command -v clang++)" ]; then
    CXX=clang++
else
    CXX=g++
fi

$CXX                             \
    -std=c++17 -Wall             \
    -Ofast -march=native -fPIC   \
    -lboost_python38 -lpython3.8 \
    -I/usr/include/python3.8     \
    -shared                      \
    -o bytepair.so               \
    bytepair.cpp
