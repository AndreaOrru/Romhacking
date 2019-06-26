#!/bin/sh

clang++                           \
    -std=c++17 -Wall              \
    -Ofast -march=native -fPIC    \
    -lboost_python37 -lpython3.7m \
    -I/usr/include/python3.7m     \
    -shared                       \
    -o bytepair.so                \
    bytepair.cpp
