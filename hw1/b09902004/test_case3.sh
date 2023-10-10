#!/usr/bin/env bash

make clean
make

time ./solve < testcases/31.in
time ./solve < testcases/32.in
time ./solve < testcases/33.in
