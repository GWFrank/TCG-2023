#!/usr/bin/env bash

make clean
make

time ./solve < testcases/11.in
time ./solve < testcases/12.in
time ./solve < testcases/13.in
time ./solve < testcases/14.in
