#!/usr/bin/env bash

make clean
make

time ./solve < testcases/21.in
time ./solve < testcases/22.in
time ./solve < testcases/23.in
